export module Miracle.Iter:Detail;

import std;
import Miracle.Numeric;
import Miracle.Types;
import Miracle.Range;

namespace Miracle {

/// Miracle-owned range values opt into the standard view concept structurally rather than through
/// view_interface inheritance. Public semantic adaptor names are exported here while implementation helpers
/// remain module-private.
template <class Maybe>
class MaybeRefView;
template <class Maybe>
class MaybeValueView;
export template <class Function>
class OnceWith;
export template <class Function>
class RepeatWith;
export template <class Function>
class FromFn;
export template <class Value, class Function>
class Successors;
export template <std::ranges::view View>
  requires std::ranges::forward_range<View>
class Cycle;
template <std::ranges::view View, class Projection, class Predicate>
class ProjectedFilterView;
export template <std::ranges::view View, class State, class Function>
class Scan;
export template <std::ranges::view View>
class Intersperse;
export template <std::ranges::view View, class Function>
class IntersperseWith;
export template <std::ranges::view View, class Function, usize N>
class MapWindows;
export template <std::ranges::view View, usize N>
class ArrayChunks;
export template <std::ranges::view Iteration>
class Peekable;
export template <std::ranges::view View, class Projection = std::identity>
class Iter;

} // namespace Miracle

// Standard customization-point spelling is fixed by the C++ ranges API.
// NOLINTBEGIN(readability-identifier-naming)
template <class Maybe>
inline constexpr bool std::ranges::enable_view<Miracle::MaybeRefView<Maybe>> = true;
template <class Maybe>
inline constexpr bool std::ranges::enable_view<Miracle::MaybeValueView<Maybe>> = true;
template <class Function>
inline constexpr bool std::ranges::enable_view<Miracle::OnceWith<Function>> = true;
template <class Function>
inline constexpr bool std::ranges::enable_view<Miracle::RepeatWith<Function>> = true;
template <class Function>
inline constexpr bool std::ranges::enable_view<Miracle::FromFn<Function>> = true;
template <class Value, class Function>
inline constexpr bool std::ranges::enable_view<Miracle::Successors<Value, Function>> = true;
template <std::ranges::view View>
  requires std::ranges::forward_range<View>
inline constexpr bool std::ranges::enable_view<Miracle::Cycle<View>> = true;
template <std::ranges::view View, class Projection, class Predicate>
inline constexpr bool std::ranges::enable_view<Miracle::ProjectedFilterView<View, Projection, Predicate>> =
    true;
template <std::ranges::view View, class State, class Function>
inline constexpr bool std::ranges::enable_view<Miracle::Scan<View, State, Function>> = true;
template <std::ranges::view View>
inline constexpr bool std::ranges::enable_view<Miracle::Intersperse<View>> = true;
template <std::ranges::view View, class Function>
inline constexpr bool std::ranges::enable_view<Miracle::IntersperseWith<View, Function>> = true;
template <std::ranges::view View, class Function, Miracle::usize N>
inline constexpr bool std::ranges::enable_view<Miracle::MapWindows<View, Function, N>> = true;
template <std::ranges::view View, Miracle::usize N>
inline constexpr bool std::ranges::enable_view<Miracle::ArrayChunks<View, N>> = true;
template <std::ranges::view Iteration>
inline constexpr bool std::ranges::enable_view<Miracle::Peekable<Iteration>> = true;
template <std::ranges::view View, class Projection>
inline constexpr bool std::ranges::enable_view<Miracle::Iter<View, Projection>> = true;
// NOLINTEND(readability-identifier-naming)

namespace Miracle {

/// Detects tuple-like values through the standard tuple protocol. This remains module-private:
/// callers only observe the adaptive invocation behavior, not this implementation vocabulary.
template <class T>
concept HasTupleProtocol = requires { typename std::tuple_size<std::remove_cvref_t<T>>::type; };

/// Materializes the accessible non-static data-member reflections once per aggregate type so adaptive
/// invocation can expand aggregate fields without rebuilding the query at each call site.
template <class T>
inline constexpr auto reflectedMembers =
    std::define_static_array(std::meta::nonstatic_data_members_of(^^T, std::meta::access_context::current()));

/// Checks the reflected-aggregate fallback without instantiating the actual invocation path.
template <class Function, class Item, usize... Index>
consteval auto reflectedInvocable([[maybe_unused]] std::index_sequence<Index...> indices) -> bool {
  using Object = std::remove_cvref_t<Item>;
  return requires(Function &&function, Item &&item) {
    std::invoke(
        std::forward<Function>(function), std::forward<Item>(item).[:reflectedMembers<Object>[Index]:]...);
  };
}

/// Invokes a callable with an aggregate's reflected members in declaration order.
template <class Function, class Item, usize... Index>
constexpr auto invokeReflected(Function &&function,
    Item &&item,
    [[maybe_unused]] std::index_sequence<Index...> indices) -> decltype(auto) {
  using Object = std::remove_cvref_t<Item>;
  return std::invoke(
      std::forward<Function>(function), std::forward<Item>(item).[:reflectedMembers<Object>[Index]:]...);
}

/// Applies Miracle's callable precedence: direct item first, tuple decomposition second, reflected aggregate
/// decomposition last. Keeping this logic centralized prevents adaptors and terminals from independently
/// re-implementing decomposition rules.
template <class Function, class Item>
constexpr auto adaptiveInvoke(Function &&function, Item &&item) -> decltype(auto) {
  if constexpr (std::invocable<Function, Item>) {
    return std::invoke(std::forward<Function>(function), std::forward<Item>(item));
  } else if constexpr (HasTupleProtocol<Item>) {
    constexpr auto tupleSize = std::tuple_size_v<std::remove_cvref_t<Item>>;
    return [&]<usize... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      constexpr bool canInvoke = requires {
        std::invoke(std::forward<Function>(function), std::get<Index>(std::forward<Item>(item))...);
      };
      if constexpr (canInvoke) {
        return std::invoke(std::forward<Function>(function), std::get<Index>(std::forward<Item>(item))...);
      } else {
        static_assert(canInvoke,
            "Miracle adaptive invocation cannot invoke the callable with this tuple-like item's elements");
      }
    }(std::make_index_sequence<tupleSize>{});
  } else if constexpr (std::is_aggregate_v<std::remove_cvref_t<Item>>) {
    using Object = std::remove_cvref_t<Item>;
    constexpr auto memberCount = reflectedMembers<Object>.size();
    constexpr bool canInvoke = reflectedInvocable<Function, Item>(std::make_index_sequence<memberCount>{});
    if constexpr (canInvoke) {
      return invokeReflected(std::forward<Function>(function),
          std::forward<Item>(item),
          std::make_index_sequence<memberCount>{});
    } else {
      static_assert(canInvoke,
          "Miracle adaptive invocation requires a callable accepting the item directly, its tuple elements, "
          "or its reflected aggregate members");
    }
  } else {
    static_assert(std::invocable<Function, Item>,
        "Miracle adaptive invocation requires a callable accepting the item directly, its tuple elements, or "
        "its reflected aggregate members");
  }
}

/// Prefix-aware counterpart used by folds/reductions where an accumulator precedes the item.
template <class Function, class Prefix, class Item, usize... Index>
consteval auto reflectedInvocableWithPrefix([[maybe_unused]] std::index_sequence<Index...> indices) -> bool {
  using Object = std::remove_cvref_t<Item>;
  return requires(Function &&function, Prefix &&prefix, Item &&item) {
    std::invoke(std::forward<Function>(function),
        std::forward<Prefix>(prefix),
        std::forward<Item>(item).[:reflectedMembers<Object>[Index]:]...);
  };
}

/// Invokes `function(prefix, members...)` for reflected aggregate items.
template <class Function, class Prefix, class Item, usize... Index>
constexpr auto invokeReflectedWithPrefix(Function &&function,
    Prefix &&prefix,
    Item &&item,
    [[maybe_unused]] std::index_sequence<Index...> indices) -> decltype(auto) {
  using Object = std::remove_cvref_t<Item>;
  return std::invoke(std::forward<Function>(function),
      std::forward<Prefix>(prefix),
      std::forward<Item>(item).[:reflectedMembers<Object>[Index]:]...);
}

/// Adaptive invocation for binary accumulator-style operations. The prefix is never decomposed; only the
/// range item participates in tuple/aggregate expansion.
template <class Function, class Prefix, class Item>
constexpr auto adaptiveInvokeWithPrefix(Function &&function, Prefix &&prefix, Item &&item) -> decltype(auto) {
  if constexpr (std::invocable<Function, Prefix, Item>) {
    return std::invoke(
        std::forward<Function>(function), std::forward<Prefix>(prefix), std::forward<Item>(item));
  } else if constexpr (HasTupleProtocol<Item>) {
    constexpr auto count = std::tuple_size_v<std::remove_cvref_t<Item>>;
    return [&]<usize... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      constexpr bool canInvoke = requires {
        std::invoke(std::forward<Function>(function),
            std::forward<Prefix>(prefix),
            std::get<Index>(std::forward<Item>(item))...);
      };
      if constexpr (canInvoke) {
        return std::invoke(std::forward<Function>(function),
            std::forward<Prefix>(prefix),
            std::get<Index>(std::forward<Item>(item))...);
      } else {
        static_assert(canInvoke,
            "Miracle adaptive invocation requires a callable accepting the accumulator followed by the item "
            "or its decomposed fields");
      }
    }(std::make_index_sequence<count>{});
  } else if constexpr (std::is_aggregate_v<std::remove_cvref_t<Item>>) {
    using Object = std::remove_cvref_t<Item>;
    constexpr auto count = reflectedMembers<Object>.size();
    constexpr bool canInvoke =
        reflectedInvocableWithPrefix<Function, Prefix, Item>(std::make_index_sequence<count>{});
    if constexpr (canInvoke) {
      return invokeReflectedWithPrefix(std::forward<Function>(function),
          std::forward<Prefix>(prefix),
          std::forward<Item>(item),
          std::make_index_sequence<count>{});
    } else {
      static_assert(canInvoke,
          "Miracle adaptive invocation requires a callable accepting the accumulator followed by the item or "
          "its decomposed fields");
    }
  } else {
    static_assert(std::invocable<Function, Prefix, Item>,
        "Miracle adaptive invocation requires a callable accepting the accumulator followed by the item or "
        "its decomposed fields");
  }
}

/// Makes a move-constructible callable satisfy `movable` without allocation. Non-assignable lambdas are
/// reconstructed in-place on assignment, matching the semantic role of the standard library's exposition-only
/// movable box.
template <class T>
class MovableBox final {
public:
  constexpr MovableBox()
    requires std::default_initializable<T>
  = default;
  constexpr explicit MovableBox(T value)
      : value_(std::move(value)) {
  }

  constexpr MovableBox(const MovableBox &)
    requires std::copy_constructible<T>
  = default;
  constexpr MovableBox(MovableBox &&) noexcept(std::is_nothrow_move_constructible_v<T>) = default;
  constexpr ~MovableBox() = default;

  constexpr auto operator=(const MovableBox &other) -> MovableBox &
    requires std::copy_constructible<T> and
             (std::is_copy_assignable_v<T> or std::is_nothrow_copy_constructible_v<T>)
  {
    if (this == std::addressof(other)) {
      return *this;
    }
    if constexpr (std::is_copy_assignable_v<T>) {
      value_ = other.value_;
    } else {
      std::destroy_at(std::addressof(value_));
      std::construct_at(std::addressof(value_), other.value_);
    }
    return *this;
  }

  constexpr auto operator=(MovableBox &&other) noexcept(
      std::is_nothrow_move_assignable_v<T> or not std::is_move_assignable_v<T>) -> MovableBox &
    requires std::move_constructible<T> and
             (std::is_move_assignable_v<T> or std::is_nothrow_move_constructible_v<T>)
  {
    if (this == std::addressof(other)) {
      return *this;
    }
    if constexpr (std::is_move_assignable_v<T>) {
      value_ = std::move(other.value_);
    } else {
      std::destroy_at(std::addressof(value_));
      std::construct_at(std::addressof(value_), std::move(other.value_));
    }
    return *this;
  }

  [[nodiscard]] constexpr auto get() noexcept -> T & {
    return value_;
  }
  [[nodiscard]] constexpr auto get() const noexcept -> const T & {
    return value_;
  }

private:
  [[no_unique_address]] T value_{};
};

/// Turns Miracle adaptive invocation into a regular unary callable accepted by standard views and algorithms.
/// This wrapper is intentionally tiny and allocation-free.
template <class Function>
class AdaptiveCallable final {
public:
  constexpr AdaptiveCallable()
    requires std::default_initializable<Function>
  = default;
  constexpr explicit AdaptiveCallable(Function function)
      : function_(std::move(function)) {
  }

  template <class Item>
  constexpr auto operator()(Item &&item) const -> decltype(auto) {
    return adaptiveInvoke(function_, std::forward<Item>(item));
  }

private:
  mutable Function function_{};
};

template <class Function>
AdaptiveCallable(Function) -> AdaptiveCallable<Function>;

/// Wraps a user callable with Miracle's direct/tuple/reflected invocation semantics.
template <class Function>
[[nodiscard]] constexpr auto adapt(Function function) {
  return AdaptiveCallable<std::decay_t<Function>>{std::move(function)};
}

/// Binary callable adapter used by folds: the first argument is preserved as an accumulator and the second
/// argument receives Miracle's adaptive decomposition rules.
template <class Function>
class AdaptiveBinaryCallable final {
public:
  constexpr explicit AdaptiveBinaryCallable(Function function)
      : function_(std::move(function)) {
  }

  template <class Prefix, class Item>
  constexpr auto operator()(Prefix &&prefix, Item &&item) const -> decltype(auto) {
    return adaptiveInvokeWithPrefix(function_, std::forward<Prefix>(prefix), std::forward<Item>(item));
  }

private:
  mutable Function function_;
};

/// Wraps an accumulator-style callable for use by standard fold algorithms.
template <class Function>
[[nodiscard]] constexpr auto adaptWithPrefix(Function function) {
  return AdaptiveBinaryCallable<std::decay_t<Function>>{std::move(function)};
}

/// Stores two lazy map projections as one callable so consecutive `map()` calls do not create nested
/// `transform_view` layers.
template <class First, class Second>
class ComposedProjection final {
public:
  constexpr ComposedProjection(First first, Second second)
      : first_(std::move(first))
      , second_(std::move(second)) {
  }

  template <class Item>
  constexpr auto operator()(Item &&item) const -> decltype(auto) {
    return std::invoke(second_.get(), std::invoke(first_.get(), std::forward<Item>(item)));
  }

private:
  mutable MovableBox<First> first_;
  mutable MovableBox<Second> second_;
};

/// Composes projections in pipeline order: `second(first(item))`.
template <class First, class Second>
[[nodiscard]] constexpr auto compose(First first, Second second) {
  return ComposedProjection<std::decay_t<First>, std::decay_t<Second>>{std::move(first), std::move(second)};
}

/// Minimal structural contract shared by Option/Expected-like single-value sources and by filterMap/mapWhile
/// results. It intentionally avoids exporting another public concept.
template <class Optional>
concept OptionalLike = requires(Optional optional) {
  typename std::remove_cvref_t<Optional>::value_type;
  { optional.has_value() } -> std::convertible_to<bool>;
  *optional;
};

/// Minimal expected-like protocol used by Peekable::nextIfMap without coupling Iter to the temporary
/// single-error Miracle::Result alias.
template <class Expected>
concept ExpectedLike = requires(Expected expected) {
  typename std::remove_cvref_t<Expected>::value_type;
  typename std::remove_cvref_t<Expected>::error_type;
  { expected.has_value() } -> std::convertible_to<bool>;
  *expected;
  expected.error();
};

/// Standard execution policies support by ParallelIter. The facade stores only the policy type and rehydrates
/// the matching standard singleton, so user-defined policy types are intentionally excluded.
template <class Policy>
concept StandardExecutionPolicy =
    std::same_as<std::remove_cvref_t<Policy>, std::execution::sequenced_policy> or
    std::same_as<std::remove_cvref_t<Policy>, std::execution::parallel_policy> or
    std::same_as<std::remove_cvref_t<Policy>, std::execution::parallel_unsequenced_policy> or
    std::same_as<std::remove_cvref_t<Policy>, std::execution::unsequenced_policy>;

template <class T>
inline constexpr auto isIdentityProjection = std::same_as<std::remove_cvref_t<T>, std::identity>;

/// Normalizes an arbitrary viewable range into a view. Owned non-borrowed rvalues become `as_rvalue` views so
/// destructive materialization moves elements instead of silently copying.
template <std::ranges::viewable_range RangeType>
[[nodiscard]] constexpr auto viewForRange(RangeType &&range) {
  auto view = std::views::all(std::forward<RangeType>(range));
  if constexpr (not std::is_lvalue_reference_v<RangeType> and not std::ranges::borrowed_range<RangeType>) {
    return std::views::as_rvalue(std::move(view));
  } else {
    return view;
  }
}

/// Iterator facade for a pending projection. It mirrors the underlying iterator category and computes the
/// projection only on dereference, preserving the standard transform-view model.
template <class Iterator, class Projection>
class ProjectedIterator final {
  using ProjectionResult = decltype(std::invoke(std::declval<Projection &>(), *std::declval<Iterator &>()));

public:
  using iterator_concept = std::conditional_t<std::random_access_iterator<Iterator>,
      std::random_access_iterator_tag,
      std::conditional_t<std::bidirectional_iterator<Iterator>,
          std::bidirectional_iterator_tag,
          std::conditional_t<std::forward_iterator<Iterator>,
              std::forward_iterator_tag,
              std::input_iterator_tag>>>;
  using value_type = std::remove_cvref_t<ProjectionResult>;
  using difference_type = std::iter_difference_t<Iterator>;

  ProjectedIterator() = default;

  constexpr ProjectedIterator(Iterator current, Projection *projection)
      : current_(std::move(current))
      , projection_(projection) {
  }

  [[nodiscard]] constexpr auto operator*() const -> decltype(auto) {
    return std::invoke(*projection_, *current_);
  }

  constexpr auto operator++() -> ProjectedIterator & {
    ++current_;
    return *this;
  }

  constexpr auto operator++(int) {
    if constexpr (std::forward_iterator<Iterator>) {
      auto previous = *this;
      ++*this;
      return previous;
    } else {
      ++*this;
    }
  }

  constexpr auto operator--() -> ProjectedIterator &
    requires std::bidirectional_iterator<Iterator>
  {
    --current_;
    return *this;
  }

  constexpr auto operator--(int)
    requires std::bidirectional_iterator<Iterator>
  {
    auto previous = *this;
    --*this;
    return previous;
  }

  constexpr auto operator+=(difference_type offset) -> ProjectedIterator &
    requires std::random_access_iterator<Iterator>
  {
    current_ += offset;
    return *this;
  }

  constexpr auto operator-=(difference_type offset) -> ProjectedIterator &
    requires std::random_access_iterator<Iterator>
  {
    current_ -= offset;
    return *this;
  }

  [[nodiscard]] constexpr auto operator[](difference_type offset) const -> decltype(auto)
    requires std::random_access_iterator<Iterator>
  {
    return std::invoke(*projection_, current_[offset]);
  }

  [[nodiscard]] constexpr auto base() const -> const Iterator & {
    return current_;
  }

  [[nodiscard]] friend constexpr auto operator+(ProjectedIterator iterator, difference_type offset)
      -> ProjectedIterator
    requires std::random_access_iterator<Iterator>
  {
    iterator += offset;
    return iterator;
  }

  [[nodiscard]] friend constexpr auto operator+(difference_type offset, ProjectedIterator iterator)
      -> ProjectedIterator
    requires std::random_access_iterator<Iterator>
  {
    return iterator + offset;
  }

  [[nodiscard]] friend constexpr auto operator-(ProjectedIterator iterator, difference_type offset)
      -> ProjectedIterator
    requires std::random_access_iterator<Iterator>
  {
    iterator -= offset;
    return iterator;
  }

  [[nodiscard]] friend constexpr auto operator-(const ProjectedIterator &left, const ProjectedIterator &right)
      -> difference_type
    requires std::sized_sentinel_for<Iterator, Iterator>
  {
    return left.current_ - right.current_;
  }

  [[nodiscard]] friend constexpr auto operator==(const ProjectedIterator &left,
      const ProjectedIterator &right) -> bool
    requires std::equality_comparable<Iterator>
  {
    return left.current_ == right.current_;
  }

  [[nodiscard]] friend constexpr auto operator<=>(const ProjectedIterator &left,
      const ProjectedIterator &right)
    requires std::random_access_iterator<Iterator> and std::three_way_comparable<Iterator>
  {
    return left.current_ <=> right.current_;
  }

private:
  Iterator current_{};
  Projection *projection_{};
};

/// Sentinel companion for projected iterators when the underlying view is not a common range.
template <class Sentinel>
class ProjectedSentinel final {
public:
  ProjectedSentinel() = default;
  constexpr explicit ProjectedSentinel(Sentinel end)
      : end_(std::move(end)) {
  }

  template <class Iterator, class Projection>
  [[nodiscard]] friend constexpr auto operator==(const ProjectedIterator<Iterator, Projection> &iterator,
      const ProjectedSentinel &sentinel) -> bool
    requires std::sentinel_for<Sentinel, Iterator>
  {
    return iterator.base() == sentinel.end_;
  }

  template <class Iterator, class Projection>
  [[nodiscard]] friend constexpr auto operator-(const ProjectedSentinel &sentinel,
      const ProjectedIterator<Iterator, Projection> &iterator) -> std::iter_difference_t<Iterator>
    requires std::sized_sentinel_for<Sentinel, Iterator>
  {
    return sentinel.end_ - iterator.base();
  }

  template <class Iterator, class Projection>
  [[nodiscard]] friend constexpr auto operator-(const ProjectedIterator<Iterator, Projection> &iterator,
      const ProjectedSentinel &sentinel) -> std::iter_difference_t<Iterator>
    requires std::sized_sentinel_for<Sentinel, Iterator>
  {
    return iterator.base() - sentinel.end_;
  }

private:
  Sentinel end_{};
};

} // namespace Miracle
