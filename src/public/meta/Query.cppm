export module Miracle.Meta:Query;

import std;
import :Types;
import :Cache;
import :Reflect;
import :QueryIR;
import :QueryPlan;
import :QueryState;
import :QueryEval;

namespace Miracle::meta {

/// Declared before the public Query definition because ordering/set operations call it from exported template
/// bodies.
template <class Value, class Stored = StoredT<Value>>
consteval auto makeStoredQuery(const std::vector<Stored> &values);

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Finite, compile-time-only metadata query algebra.
///
/// `State` contains one source value, one flat non-structural capture environment, and immutable structural
/// Program/SegmentPlan values. The query is not a runtime range and deliberately does not inherit from a
/// range/view hierarchy. Logical reference types are preserved through the compiled lazy evaluator.
template <class State>
struct Query final {
  /// Exact logical element type of the current query; reference qualifiers are intentionally preserved.
  using Value = State::value_type;

  /// Constructs a query from its internal state. The state type is implementation vocabulary; callers
  /// normally use `query(source)` and never spell it directly.
  explicit consteval Query(State state)
      : state_(std::move(state)) {
  }

  /// Maps every logical value using the Meta-specific adaptive invocation rules.
  template <class Self, class Function>
  [[nodiscard]] consteval auto map(this Self &&self, Function function) {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(
        not std::is_void_v<Result>, "Query::map() cannot return void; use forEach() for terminal work");
    auto next =
        appendQueryCallable<Result>(std::forward<Self>(self).state_, QueryOp::Map, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Keeps values accepted by a consteval predicate while preserving the exact logical value type.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto filter(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(std::convertible_to<Result, bool>, "Query::filter() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::Filter, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Maps values to Optional-like results and emits only engaged results.
  /// C++26 `std::optional<T&>` results preserve `T&` as the next logical value; ordinary owning optionals
  /// produce their `value_type`.
  template <class Self, class Function>
  [[nodiscard]] consteval auto filterMap(this Self &&self, Function function) {
    using Maybe = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(OptionalLike<Maybe>, "Query::filterMap() requires an Optional-like result");
    using Result = OptionalResultT<Maybe>;
    auto next =
        appendQueryCallable<Result>(std::forward<Self>(self).state_, QueryOp::FilterMap, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Flattens a map result that is itself a finite range or another Query.
  template <class Self, class Function>
  [[nodiscard]] consteval auto flatMap(this Self &&self, Function function) {
    return std::forward<Self>(self).map(std::move(function)).flatten();
  }

  /// Flattens one level of Query or input-range values without materializing an intermediate sequence.
  template <class Self>
  [[nodiscard]] consteval auto flatten(this Self &&self) {
    static_assert(requires(Value value) { value; }, "Query::flatten() requires a logical value type");
    using Result = FlattenValue<Value>::type;
    auto next =
        appendQueryInstruction<Result>(std::forward<Self>(self).state_, Instruction{.op = QueryOp::Flatten});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Observes each logical value without changing the sequence.
  template <class Self, class Function>
  [[nodiscard]] consteval auto inspect(this Self &&self, Function function) {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(std::same_as<Result, void>, "Query::inspect() callback must return void");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::Inspect, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Stops after `count` logical values.
  template <class Self>
  [[nodiscard]] consteval auto take(this Self &&self, std::size_t count) {
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Take, .a = count});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Skips the first `count` logical values.
  template <class Self>
  [[nodiscard]] consteval auto skip(this Self &&self, std::size_t count) {
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Skip, .a = count});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Takes values while the predicate remains true; the first failure terminates traversal.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto takeWhile(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::takeWhile() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::TakeWhile, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Skips values while the predicate remains true, then passes every later value.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto skipWhile(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::skipWhile() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::SkipWhile, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Selects a half-open positional slice `[start, stop)` using the same semantics as Miracle Range.
  template <class Self>
  [[nodiscard]] consteval auto slice(this Self &&self, std::size_t start, std::size_t stop) {
    const auto take = stop > start ? stop - start : std::size_t{};
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Slice, .a = start, .b = take});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Accepts any finite integral endpoint range, including Miracle's `Range`, as `[start, stop)`.
  template <class Self, std::ranges::input_range Range>
    requires std::integral<std::ranges::range_value_t<Range>>
  [[nodiscard]] consteval auto slice(this Self &&self, Range range) {
    // Miracle::Range is itself an input range, but iterating it yields the values *inside* [start, stop), not
    // the two endpoints. Prefer an exposed integral start/stop pair when the range supplies one; generic
    // endpoint carriers such as std::array{start, stop} retain the established first-two-values behavior.
    if constexpr (requires {
                    range.start;
                    range.stop;
                  }) {
      using Start = std::remove_cvref_t<decltype(range.start)>;
      using Stop = std::remove_cvref_t<decltype(range.stop)>;
      if constexpr (std::integral<Start> and std::integral<Stop>) {
        return std::forward<Self>(self).slice(
            static_cast<std::size_t>(range.start), static_cast<std::size_t>(range.stop));
      }
    }

    auto iterator = std::ranges::begin(range);
    const auto last = std::ranges::end(range);
    if (iterator == last) {
      return std::forward<Self>(self).slice(0, 0);
    }
    const auto start = static_cast<std::size_t>(*iterator);
    ++iterator;
    if (iterator == last) {
      return std::forward<Self>(self).slice(start, start);
    }
    return std::forward<Self>(self).slice(start, static_cast<std::size_t>(*iterator));
  }

  /// Enumerates logical values as `(index, value)` pairs without materializing the source.
  template <class Self>
  [[nodiscard]] consteval auto enumerate(this Self &&self) {
    using Result = std::pair<std::size_t, Value>;
    auto next = appendQueryInstruction<Result>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Enumerate});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Concatenates two queries with identical logical value types.
  ///
  /// Child-query termination is local to that child: `left.take(n)` ending naturally still advances into the
  /// right source. Only downstream terminal cancellation stops the whole chain. No persistence barrier is
  /// used.
  template <class Self, class OtherState>
  [[nodiscard]] consteval auto chain(this Self &&self, Query<OtherState> other)
    requires std::same_as<Value, typename Query<OtherState>::Value>
  {
    using Source = ChainSource<State, OtherState>;
    auto next = makeInitialQueryPlan<Value>(Source{std::forward<Self>(self).state_, std::move(other.state_)});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Zips two queries until either child query is exhausted.
  ///
  /// Miracle preserves the established evaluation order in which both child queries are evaluated before
  /// pairing. Pairing storage is evaluation-local only; no child result is promoted into hidden static
  /// backing.
  template <class Self, class OtherState>
  [[nodiscard]] consteval auto zip(this Self &&self, Query<OtherState> other) {
    using Result = std::pair<Value, typename Query<OtherState>::Value>;
    using Source = ZipSource<State, OtherState>;
    auto next =
        makeInitialQueryPlan<Result>(Source{std::forward<Self>(self).state_, std::move(other.state_)});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Returns the first logical value, preserving references through C++26 `optional<T&>`.
  template <class Self>
  [[nodiscard]] consteval auto first(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return false;
    });
    return result;
  }

  /// Returns the last logical value, preserving references where the logical value is a reference.
  template <class Self>
  [[nodiscard]] consteval auto last(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return true;
    });
    return result;
  }

  /// Returns the zero-based `index`th logical value, preserving the exact logical value type.
  template <class Self>
  [[nodiscard]] consteval auto nth(this Self &&self, std::size_t index) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::size_t current{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (current++ == index) {
        result.emplace(std::forward<decltype(item)>(item));
        return false;
      }
      return true;
    });
    return result;
  }

  /// Finds the first value accepted by `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto find(this Self &&self, Predicate predicate) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).filter(std::move(predicate)).first();
  }

  /// Returns the first value produced by an Optional-like mapper.
  template <class Self, class Function>
  [[nodiscard]] consteval auto findMap(this Self &&self, Function function) {
    return std::forward<Self>(self).filterMap(std::move(function)).first();
  }

  /// Returns whether any logical value compares equal to `value`.
  template <class Self, class T>
  [[nodiscard]] consteval auto contains(this Self &&self, const T &value) -> bool {
    bool found{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (std::forward<decltype(item)>(item) == value) {
        found = true;
        return false;
      }
      return true;
    });
    return found;
  }

  /// Returns the first logical index accepted by `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto position(this Self &&self, Predicate predicate) -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = index;
        return false;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns the greatest logical index accepted by `predicate`, or no value when none matches.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto rposition(this Self &&self, Predicate predicate)
      -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = index;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns whether every logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto all(this Self &&self, Predicate predicate) -> bool {
    bool result{true};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = false;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether at least one logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto any(this Self &&self, Predicate predicate) -> bool {
    bool result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = true;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether no logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto none(this Self &&self, Predicate predicate) -> bool {
    return not std::forward<Self>(self).any(std::move(predicate));
  }

  /// Counts logical values by evaluating the fused pipeline.
  template <class Self>
  [[nodiscard]] consteval auto count(this Self &&self) -> std::size_t {
    std::size_t result{};
    std::forward<Self>(self).visit([&](auto &&) consteval -> bool {
      ++result;
      return true;
    });
    return result;
  }

  /// Counts logical values accepted by `predicate`.
  ///
  /// The predicate uses the same adaptive invocation rules as `filter`, `all`, `any`, and `find`, so raw
  /// `std::meta` predicates, Miracle wrappers, lambdas, and user callables compose identically.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto count(this Self &&self, Predicate predicate) -> std::size_t {
    std::size_t result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        ++result;
      }
      return true;
    });
    return result;
  }

  /// Returns whether the logical sequence contains no values.
  template <class Self>
  [[nodiscard]] consteval auto isEmpty(this Self &&self) -> bool {
    bool empty{true};
    std::forward<Self>(self).visit([&](auto &&) consteval -> bool {
      empty = false;
      return false;
    });
    return empty;
  }

  /// Returns the direct source cardinality before any lazy IR has been appended.
  ///
  /// Query operation topology is now value IR rather than part of the Query C++ type, so member availability
  /// can no longer encode “empty program”. Calling `size()` after an adaptor is therefore a semantic error;
  /// use `count()`.
  [[nodiscard]] consteval auto size(this const Query &self)
    requires requires(const State::source_type &source) { source.values.size(); }
  {
    if (self.state_.program != Info{}) {
      throw std::meta::exception{
          "Query::size() is available only for an unmodified direct source; use count()",
          self.state_.program};
    }
    return self.state_.source.values.size();
  }

  /// Reverses the logical sequence while preserving references. This is an eager barrier only at the
  /// operation itself; later stages are still fused into the resulting Query state.
  template <class Self>
  [[nodiscard]] consteval auto reverse(this Self &&self) {
    if constexpr (not persistentQueryBacking<Value>) {
      rejectPersistentQueryBacking<Value>(self.state_.program);
    } else {
      std::vector<StoredT<Value>> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        return true;
      });
      std::ranges::reverse(values);
      return makeStoredQuery<Value>(values);
    }
  }

  /// Sorts logical values using ordinary ordering. Reference values are sorted by referent without changing
  /// their identity.
  template <class Self>
  [[nodiscard]] consteval auto sort(this Self &&self) {
    return std::forward<Self>(self).sortBy(std::less<>{});
  }

  /// Sorts logical values with a binary comparator. The comparator receives the exact logical value types.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto sortBy(this Self &&self, Compare compare) {
    constexpr bool byteBackedBool =
        std::same_as<std::remove_cvref_t<Value>, bool> and not std::is_reference_v<Value>;
    using Stored = std::conditional_t<byteBackedBool, std::uint8_t, StoredT<Value>>;
    if constexpr (not persistentQueryBacking<Value, Stored>) {
      rejectPersistentQueryBacking<Value, Stored>(self.state_.program);
    } else {
      std::vector<Stored> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        if constexpr (byteBackedBool) {
          values.push_back(static_cast<std::uint8_t>(static_cast<bool>(item)));
        } else {
          values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        }
        return true;
      });
      for (std::size_t index{1}; index < values.size(); ++index) {
        auto current = std::move(values[index]);
        std::size_t position = index;
        while (position != 0) {
          auto &previous = values[position - 1];
          bool precedes{};
          if constexpr (byteBackedBool) {
            // `uint8_t` only avoids owning `vector<bool>` storage. The comparator must still observe
            // Query's logical `bool` values rather than that storage representation.
            bool currentValue = current != 0;
            bool previousValue = previous != 0;
            precedes = static_cast<bool>(adaptiveInvokeBinary(compare, currentValue, previousValue));
          } else {
            precedes = static_cast<bool>(adaptiveInvokeBinary(
                compare, orderingOperand<Value>(current), orderingOperand<Value>(previous)));
          }
          if (not precedes) {
            break;
          }
          values[position] = std::move(previous);
          --position;
        }
        values[position] = std::move(current);
      }
      return makeStoredQuery<Value>(values);
    }
  }

  /// Sorts logical values by an adaptive key projection while retaining the original logical elements.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto sortByKey(this Self &&self, Projection projection) {
    auto comparator = [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
      return adaptiveInvoke(projection, std::forward<decltype(left)>(left)) <
             adaptiveInvoke(projection, std::forward<decltype(right)>(right));
    };
    return std::forward<Self>(self).sortBy(std::move(comparator));
  }

  /// Returns the minimum logical value under ordinary ordering.
  template <class Self>
  [[nodiscard]] consteval auto min(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).minBy(std::less<>{});
  }

  /// Returns the maximum logical value under ordinary ordering.
  template <class Self>
  [[nodiscard]] consteval auto max(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).maxBy(std::less<>{});
  }

  /// Returns the logical minimum according to a binary ordering predicate.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto minBy(this Self &&self, Compare compare) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, item, *result))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical maximum according to a binary ordering predicate.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto maxBy(this Self &&self, Compare compare) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, *result, item))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical minimum according to an adaptive key projection.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto minByKey(this Self &&self, Projection projection) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).minBy(
        [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
          return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
        });
  }

  /// Returns the logical maximum according to an adaptive key projection.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto maxByKey(this Self &&self, Projection projection) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).maxBy(
        [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
          return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
        });
  }

  /// Removes consecutive duplicate logical values. The operation is fused by keeping only the previous value.
  template <class Self>
  [[nodiscard]] consteval auto unique(this Self &&self) {
    return std::forward<Self>(self).uniqueBy(std::identity{});
  }

  /// Removes every later logical element whose projected key has already occurred, preserving encounter
  /// order.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto uniqueBy(this Self &&self, Projection projection) {
    if constexpr (not persistentQueryBacking<Value>) {
      rejectPersistentQueryBacking<Value>(self.state_.program);
    } else {
      // `unique` is implemented as a stage-like eager normalization so arbitrary reference values remain
      // references.
      using Key = std::remove_cvref_t<decltype(adaptiveInvoke(projection, std::declval<Value>()))>;
      static_assert(std::copy_constructible<Key>, "Query::uniqueBy() requires a copy-constructible key");
      std::vector<Key> keys;
      std::vector<StoredT<Value>> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        auto key = adaptiveInvoke(projection, item);
        bool duplicate{};
        for (const Key &existing : keys) {
          if (existing == key) {
            duplicate = true;
            break;
          }
        }
        if (duplicate) {
          return true;
        }
        keys.push_back(key);
        values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        return true;
      });
      return makeStoredQuery<Value>(values);
    }
  }

  /// Splits logical values into two independently persistent queries while preserving encounter order.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto partition(this Self &&self, Predicate predicate) {
    using Stored = StoredT<Value>;
    if constexpr (not persistentQueryBacking<Value, Stored>) {
      rejectPersistentQueryBacking<Value, Stored>(self.state_.program);
    } else {
      std::vector<Stored> yes;
      std::vector<Stored> rejected;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        const bool accepted = static_cast<bool>(adaptiveInvoke(predicate, item));
        auto stored = storeValueAs<Value>(std::forward<decltype(item)>(item));
        if (accepted) {
          yes.push_back(std::move(stored));
        } else {
          rejected.push_back(std::move(stored));
        }
        return true;
      });
      return std::pair{makeStoredQuery<Value>(yes), makeStoredQuery<Value>(rejected)};
    }
  }

  /// Executes a compile-time callback for every logical value and returns no runtime result.
  template <class Self, class Function>
  consteval auto forEach(this Self &&self, Function function) -> void {
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      adaptiveInvoke(function, std::forward<decltype(item)>(item));
      return true;
    });
  }

  /// Promotes a value query into static standard storage. Reference-valued queries cannot be silently
  /// converted into a different element representation, so they fail instead of manufacturing a wrapper type.
  template <class Self>
  [[nodiscard]] consteval auto materialize(this Self &&self) {
    requireMaterializable<Value>();
    using Element = std::remove_cvref_t<Value>;
    std::vector<Element> values;
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      values.emplace_back(std::forward<decltype(item)>(item));
      return true;
    });
    return promoteVector(values);
  }

private:
  template <class StateType>
  friend struct Query;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(const Query<StateType> &, Function &&) -> bool
    requires std::copy_constructible<StateType>;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(Query<StateType> &&, Function &&) -> bool;

  template <class Self, class Function>
  consteval auto visit(this Self &&self, Function function) -> bool {
    return evaluateState(std::forward<Self>(self).state_, std::move(function));
  }

  State state_;
};

} // namespace Miracle::meta

namespace Miracle::meta {

template <class State, class Function>
consteval auto evaluateQuery(const Query<State> &query, Function &&function) -> bool
  requires std::copy_constructible<State>
{
  return evaluateState(query.state_, std::forward<Function>(function));
}

template <class State, class Function>
consteval auto evaluateQuery(Query<State> &&query, Function &&function) -> bool {
  return evaluateState(std::move(query).state_, std::forward<Function>(function));
}

/// Promotes a reordered/persisted logical sequence and returns a new Query sourcing that static backing.
///
/// Internal reference storage is allowed through pointers, but the stored representation itself must still be
/// structural so `std::define_static_array` can legally provide program-lifetime backing.
template <class Value, class Stored>
consteval auto makeStoredQuery(const std::vector<Stored> &values) {
  if constexpr (not persistentQueryBacking<Value, Stored>) {
    rejectPersistentQueryBacking<Value, Stored>();
  } else {
    auto span = promoteVector(values);
    using Source = StoredSource<Value, Stored>;
    auto state = makeInitialQueryPlan<Value>(Source{span});
    return Query<decltype(state)>{std::move(state)};
  }
}

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Creates a persistent Meta Query from a standard reflection vector.
///
/// Raw `<meta>` range producers return temporary `std::vector<Info>` values. Query promotes their reflection
/// identities into static backing during this call, so the source vector needs to remain valid only for the
/// call itself. Temporary standard results and existing vectors use the same promotion path;
/// their elements are copied into static backing rather than retaining a view of the input vector.
[[nodiscard]] consteval auto query(const std::vector<Info> &source) {
  return makeStoredQuery<Info>(source);
}

/// Creates a Meta Query over a finite static span without copying its source.
/// The span follows ordinary `std::span` lifetime rules: its referenced storage must remain valid for every
/// Query evaluation that uses it. Query does not take ownership of external span storage.
template <class T, std::size_t Extent>
[[nodiscard]] consteval auto query(std::span<T, Extent> source) {
  using Source = SpanSource<T>;
  auto state = makeInitialQueryPlan<T &>(Source{std::span<T>{source}});
  return Query<decltype(state)>{std::move(state)};
}

/// Creates a Meta Query over a mutable lvalue array without copying its source or discarding mutability.
template <class T, std::size_t N>
[[nodiscard]] consteval auto query(std::array<T, N> &source) {
  return query(std::span<T>{source});
}

/// Creates a Meta Query over a const lvalue array without copying its source.
template <class T, std::size_t N>
[[nodiscard]] consteval auto query(const std::array<T, N> &source) {
  return query(std::span<const T>{source});
}

template <class T, std::size_t N>
consteval auto query(std::array<T, N> &&) = delete (
    "Query cannot borrow a temporary array; keep the source alive or materialize it first");

template <class T, std::size_t N>
consteval auto query(const std::array<T, N> &&) = delete (
    "Query cannot borrow a temporary array; keep the source alive or materialize it first");

} // namespace Miracle::meta
