export module Miracle:Iter;

import std;
import :Types;
import :Range;

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

/// Zero/one-element borrowed view over an lvalue Optional-like object. Present values are exposed by
/// reference and therefore follow the source object's lifetime.
template <class Maybe>
class MaybeRefView final {
public:
  constexpr MaybeRefView() = default;
  constexpr explicit MaybeRefView(Maybe &maybe)
      : maybe_(std::addressof(maybe)) {
  }

  [[nodiscard]] constexpr auto begin() const {
    using Pointer = decltype(std::addressof(**maybe_));
    return maybe_ != nullptr and maybe_->has_value() ? std::addressof(**maybe_) : Pointer{};
  }

  [[nodiscard]] constexpr auto end() const {
    using Pointer = decltype(std::addressof(**maybe_));
    return maybe_ != nullptr and maybe_->has_value() ? std::addressof(**maybe_) + 1 : Pointer{};
  }

  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return maybe_ != nullptr and maybe_->has_value() ? 1U : 0U;
  }

private:
  Maybe *maybe_{};
};

/// Owning zero/one-element view for rvalue Optional-like objects. `iter(rvalue)` later applies `as_rvalue` so
/// materialization can move the contained value.
template <class Maybe>
class MaybeValueView final {
public:
  constexpr MaybeValueView()
    requires std::default_initializable<Maybe>
  = default;
  constexpr explicit MaybeValueView(Maybe maybe)
      : maybe_(std::move(maybe)) {
  }

  [[nodiscard]] constexpr auto begin() {
    using Pointer = decltype(std::addressof(*maybe_));
    return maybe_.has_value() ? std::addressof(*maybe_) : Pointer{};
  }

  [[nodiscard]] constexpr auto end() {
    using Pointer = decltype(std::addressof(*maybe_));
    return maybe_.has_value() ? std::addressof(*maybe_) + 1 : Pointer{};
  }

  [[nodiscard]] constexpr auto begin() const {
    using Pointer = decltype(std::addressof(*maybe_));
    return maybe_.has_value() ? std::addressof(*maybe_) : Pointer{};
  }

  [[nodiscard]] constexpr auto end() const {
    using Pointer = decltype(std::addressof(*maybe_));
    return maybe_.has_value() ? std::addressof(*maybe_) + 1 : Pointer{};
  }

  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return maybe_.has_value() ? 1U : 0U;
  }

private:
  Maybe maybe_{};
};

/// Lazy one-element source used by `onceWith()`. The iterator owns the generated value so repeated
/// dereference never repeats user computation and move-only results remain consumable.
template <class Function>
class OnceWith final {
  using Value = std::remove_cvref_t<std::invoke_result_t<Function &>>;

  class Sentinel final {};

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Value;
    using difference_type = isize;

    Iterator() = default;
    constexpr explicit Iterator(OnceWith *parent) {
      value_.emplace(std::invoke(parent->function_.get()));
    }

    [[nodiscard]] constexpr auto operator*() const -> Value & {
      return *value_;
    }

    constexpr auto operator++() -> Iterator & {
      value_.reset();
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] Sentinel sentinel) -> bool {
      return not iterator.value_.has_value();
    }

  private:
    /// A const input iterator must remain indirectly readable; mutable cache storage lets `as_rvalue` consume
    /// the generated value while repeated ordinary dereference still observes the same object.
    mutable Option<Value> value_{};
  };

public:
  constexpr OnceWith()
    requires std::default_initializable<Function>
  = default;
  constexpr explicit OnceWith(Function function)
      : function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> Sentinel {
    return {};
  }

private:
  [[no_unique_address]] MovableBox<Function> function_;
};

/// Endless callable source used by `repeatWith()`. Each increment generates and caches exactly one logical
/// iter, so repeated dereference has no user-visible work beyond reading the cache.
template <class Function>
class RepeatWith final {
  using Value = std::remove_cvref_t<std::invoke_result_t<Function &>>;

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Value;
    using difference_type = isize;

    Iterator() = default;
    constexpr explicit Iterator(RepeatWith *parent)
        : parent_(parent) {
      load();
    }

    [[nodiscard]] constexpr auto operator*() const -> Value & {
      return *value_;
    }

    constexpr auto operator++() -> Iterator & {
      load();
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==([[maybe_unused]] const Iterator &iterator,
        [[maybe_unused]] std::unreachable_sentinel_t sentinel) noexcept -> bool {
      return false;
    }

  private:
    constexpr auto load() -> void {
      value_.reset();
      value_.emplace(std::invoke(parent_->function_.get()));
    }

    RepeatWith *parent_{};
    mutable Option<Value> value_{};
  };

public:
  constexpr RepeatWith()
    requires std::default_initializable<Function>
  = default;
  constexpr explicit RepeatWith(Function function)
      : function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> std::unreachable_sentinel_t {
    return {};
  }

private:
  [[no_unique_address]] MovableBox<Function> function_;
};

/// Optional-producing callable source used by `fromFn()`. Generation is lazy and cached per logical item; the
/// first empty result permanently ends that traversal.
template <class Function>
class FromFn final {
  using Maybe = std::remove_cvref_t<std::invoke_result_t<Function &>>;
  static_assert(OptionalLike<Maybe>);
  using Value = Maybe::value_type;

  class Sentinel final {};

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Value;
    using difference_type = isize;

    Iterator() = default;
    constexpr explicit Iterator(FromFn *parent)
        : parent_(parent) {
      load();
    }

    [[nodiscard]] constexpr auto operator*() const -> Value & {
      return *value_;
    }

    constexpr auto operator++() -> Iterator & {
      load();
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] Sentinel sentinel) -> bool {
      return not iterator.value_.has_value();
    }

  private:
    constexpr auto load() -> void {
      Maybe generated = std::invoke(parent_->function_.get());
      value_.reset();
      if (generated.has_value()) {
        value_.emplace(std::move(*generated));
      }
    }

    FromFn *parent_{};
    mutable Option<Value> value_{};
  };

public:
  constexpr FromFn()
    requires std::default_initializable<Function>
  = default;
  constexpr explicit FromFn(Function function)
      : function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> Sentinel {
    return {};
  }

private:
  [[no_unique_address]] MovableBox<Function> function_;
};

/// Stateful successor source. The next value is computed from the current value before the current value is
/// exposed, so downstream consumers may move from an item without corrupting successor generation.
template <class Value, class Function>
class Successors final {
  using Maybe = std::remove_cvref_t<decltype(adaptiveInvoke(std::declval<Function &>(),
      std::declval<const Value &>()))>;
  static_assert(OptionalLike<Maybe>);
  static_assert(std::same_as<std::remove_cvref_t<typename Maybe::value_type>, Value>);

  class Sentinel final {};

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Value;
    using difference_type = isize;

    Iterator() = default;
    constexpr Iterator(Successors *parent, Value seed)
        : parent_(parent)
        , current_(std::move(seed)) {
      prepareNext();
    }

    [[nodiscard]] constexpr auto operator*() const -> Value & {
      return *current_;
    }

    constexpr auto operator++() -> Iterator & {
      current_.reset();
      if (next_.has_value()) {
        current_.emplace(std::move(*next_));
        prepareNext();
      }
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] Sentinel sentinel) -> bool {
      return not iterator.current_.has_value();
    }

  private:
    constexpr auto prepareNext() -> void {
      Maybe generated = adaptiveInvoke(parent_->function_.get(), std::as_const(*current_));
      next_.reset();
      if (generated.has_value()) {
        next_.emplace(std::move(*generated));
      }
    }

    Successors *parent_{};
    mutable Option<Value> current_{};
    Option<Value> next_{};
  };

public:
  constexpr Successors(Value seed, Function function)
      : seed_(std::move(seed))
      , function_(std::move(function)) {
  }

  constexpr Successors(Option<Value> seed, Function function)
      : seed_(std::move(seed))
      , function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    if (not seed_.has_value()) {
      return {};
    }
    Value seed = std::move(*seed_);
    seed_.reset();
    return Iterator{this, std::move(seed)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> Sentinel {
    return {};
  }

private:
  Option<Value> seed_{};
  [[no_unique_address]] MovableBox<Function> function_;
};

/// Restartable endless view used by `cycle()`. It stores no element buffer; reaching the base sentinel simply
/// reacquires the base begin iterator. Empty sources remain empty and stronger base categories intentionally
/// narrow to forward traversal because an unbounded cycle has no meaningful random-access end.
template <std::ranges::view View>
  requires std::ranges::forward_range<View>
class Cycle final {
  class Sentinel final {};

  class Iterator final {
  public:
    using iterator_concept = std::forward_iterator_tag;
    using value_type = std::ranges::range_value_t<View>;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator() = default;
    constexpr Iterator(Cycle *parent, std::ranges::iterator_t<View> current, bool empty)
        : parent_(parent)
        , current_(std::move(current))
        , empty_(empty) {
    }

    [[nodiscard]] constexpr auto operator*() const -> decltype(auto) {
      return *current_;
    }

    constexpr auto operator++() -> Iterator & {
      ++current_;
      if (current_ == std::ranges::end(parent_->view_)) {
        current_ = std::ranges::begin(parent_->view_);
      }
      return *this;
    }

    constexpr auto operator++(int) -> Iterator {
      auto previous = *this;
      ++*this;
      return previous;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &left, const Iterator &right) -> bool {
      if (left.empty_ or right.empty_) {
        return left.empty_ == right.empty_ and left.parent_ == right.parent_;
      }
      return left.parent_ == right.parent_ and left.current_ == right.current_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] Sentinel sentinel) noexcept -> bool {
      return iterator.empty_;
    }

  private:
    Cycle *parent_{};
    std::ranges::iterator_t<View> current_{};
    bool empty_{true};
  };

public:
  Cycle()
    requires std::default_initializable<View>
  = default;
  constexpr explicit Cycle(View view)
      : view_(std::move(view)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    auto first = std::ranges::begin(view_);
    return Iterator{this, first, first == std::ranges::end(view_)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> Sentinel {
    return {};
  }

private:
  View view_{};
};

/// Fused map->filter representation. Ordinary iteration preserves standard transform/filter semantics, while
/// `visitWhile()` lets Miracle terminals evaluate the projection exactly once per source item and reuse the
/// projected value for both filtering and consumption.
template <std::ranges::view View, class Projection, class Predicate>
class ProjectedFilterView final {
  class Iterator final {
  public:
    using iterator_concept =
        std::conditional_t<std::ranges::bidirectional_range<View> and std::ranges::common_range<View>,
            std::bidirectional_iterator_tag,
            std::conditional_t<std::ranges::forward_range<View>,
                std::forward_iterator_tag,
                std::input_iterator_tag>>;
    using value_type = std::remove_cvref_t<decltype(std::invoke(std::declval<Projection &>(),
        *std::declval<std::ranges::iterator_t<View> &>()))>;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator() = default;
    constexpr Iterator(ProjectedFilterView *parent, std::ranges::iterator_t<View> current)
        : parent_(parent)
        , current_(std::move(current)) {
      satisfyForward();
    }

    [[nodiscard]] constexpr auto operator*() const -> decltype(auto) {
      return std::invoke(parent_->projection_.get(), *current_);
    }

    constexpr auto operator++() -> Iterator & {
      ++current_;
      satisfyForward();
      return *this;
    }

    constexpr auto operator++(int) {
      if constexpr (std::ranges::forward_range<View>) {
        auto previous = *this;
        ++*this;
        return previous;
      } else {
        ++*this;
      }
    }

    constexpr auto operator--() -> Iterator &
      requires std::ranges::bidirectional_range<View> and std::ranges::common_range<View>
    {
      --current_;
      while (not parent_->matches(current_)) {
        --current_;
      }
      return *this;
    }

    constexpr auto operator--(int)
      requires std::ranges::bidirectional_range<View> and std::ranges::common_range<View>
    {
      auto previous = *this;
      --*this;
      return previous;
    }

    [[nodiscard]] constexpr auto base() const -> const std::ranges::iterator_t<View> & {
      return current_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &left, const Iterator &right) -> bool
      requires std::equality_comparable<std::ranges::iterator_t<View>>
    {
      return left.current_ == right.current_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        const std::ranges::sentinel_t<View> &sentinel) -> bool {
      return iterator.current_ == sentinel;
    }

  private:
    constexpr auto satisfyForward() -> void {
      const auto end = std::ranges::end(parent_->view_);
      while (current_ != end and not parent_->matches(current_)) {
        ++current_;
      }
    }

    ProjectedFilterView *parent_{};
    std::ranges::iterator_t<View> current_{};
  };

public:
  ProjectedFilterView()
    requires std::default_initializable<View> and std::default_initializable<Projection> and
                 std::default_initializable<Predicate>
  = default;

  constexpr ProjectedFilterView(View view, Projection projection, Predicate predicate)
      : view_(std::move(view))
      , projection_(std::move(projection))
      , predicate_(std::move(predicate)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this, std::ranges::begin(view_)};
  }

  [[nodiscard]] constexpr auto end() {
    if constexpr (std::ranges::common_range<View>) {
      return Iterator{this, std::ranges::end(view_)};
    } else {
      return std::ranges::end(view_);
    }
  }

  template <class Visitor>
  constexpr auto visitWhile(Visitor visitor) -> bool {
    auto iterator = std::ranges::begin(view_);
    const auto end = std::ranges::end(view_);
    for (; iterator != end; ++iterator) {
      using Result = decltype(std::invoke(projection_.get(), *iterator));
      if constexpr (std::is_reference_v<Result>) {
        auto &&mapped = std::invoke(projection_.get(), *iterator);
        if (std::invoke(predicate_.get(), mapped) and
            not std::invoke(visitor, std::forward<Result>(mapped))) {
          return false;
        }
      } else {
        auto mapped = std::invoke(projection_.get(), *iterator);
        if (std::invoke(predicate_.get(), mapped) and not std::invoke(visitor, std::move(mapped))) {
          return false;
        }
      }
    }
    return true;
  }

private:
  [[nodiscard]] constexpr auto matches(const std::ranges::iterator_t<View> &iterator) -> bool {
    decltype(auto) mapped = std::invoke(projection_.get(), *iterator);
    return static_cast<bool>(std::invoke(predicate_.get(), mapped));
  }

  View view_{};
  [[no_unique_address]] MovableBox<Projection> projection_;
  [[no_unique_address]] MovableBox<Predicate> predicate_;
};

template <class T>
struct IsProjectedFilterView : std::false_type {};

template <class View, class Projection, class Predicate>
struct IsProjectedFilterView<ProjectedFilterView<View, Projection, Predicate>> : std::true_type {};

template <class T>
inline constexpr bool isProjectedFilterView = IsProjectedFilterView<std::remove_cvref_t<T>>::value;

/// Stateful single-pass view used by `scan()`. Each increment advances the source, updates state, and caches
/// the next yielded value so dereference never repeats user computation.
template <std::ranges::view View, class State, class Function>
class Scan final {
  using SourceReference = std::ranges::range_reference_t<View>;
  using Maybe = std::remove_cvref_t<decltype(adaptiveInvokeWithPrefix(std::declval<Function &>(),
      std::declval<State &>(),
      std::declval<SourceReference>()))>;
  static_assert(OptionalLike<Maybe>);

  class Sentinel final {};

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Maybe::value_type;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator() = default;
    constexpr Iterator(Scan *parent, std::ranges::iterator_t<View> current)
        : parent_(parent)
        , current_(std::move(current)) {
      load();
    }

    [[nodiscard]] constexpr auto operator*() const -> value_type & {
      return *cache_;
    }

    constexpr auto operator++() -> Iterator & {
      if (current_ != std::ranges::end(parent_->view_)) {
        ++current_;
      }
      cache_.reset();
      load();
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] Sentinel sentinel) -> bool {
      return iterator.done_;
    }

  private:
    constexpr auto load() -> void {
      if (current_ == std::ranges::end(parent_->view_)) {
        done_ = true;
        return;
      }
      Maybe result = adaptiveInvokeWithPrefix(parent_->function_.get(), parent_->state_, *current_);
      if (result.has_value()) {
        cache_.emplace(std::move(*result));
      } else {
        done_ = true;
      }
    }

    Scan *parent_{};
    std::ranges::iterator_t<View> current_{};
    mutable Option<value_type> cache_{};
    bool done_{};
  };

public:
  Scan()
    requires std::default_initializable<View> and std::default_initializable<State> and
                 std::default_initializable<Function>
  = default;

  constexpr Scan(View view, State state, Function function)
      : view_(std::move(view))
      , state_(std::move(state))
      , function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this, std::ranges::begin(view_)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> Sentinel {
    return {};
  }

private:
  View view_{};
  State state_{};
  [[no_unique_address]] MovableBox<Function> function_{};
};

/// Inserts one stored separator between adjacent source values. One semantic type conditionally preserves
/// indexed traversal instead of exposing separate implementation types for random-access and streaming bases.
template <std::ranges::view View>
class Intersperse final {
  using Value = std::ranges::range_value_t<View>;
  using SourceReference = std::ranges::range_reference_t<View>;
  static constexpr bool randomAccess_ =
      std::ranges::random_access_range<View> and std::ranges::sized_range<View>;
  static constexpr bool stableReference_ = std::is_lvalue_reference_v<SourceReference>;
  using CommonReference = std::common_reference_t<SourceReference, const Value &>;

  class Iterator final {
  public:
    using iterator_concept = std::conditional_t<randomAccess_,
        std::random_access_iterator_tag,
        std::conditional_t<std::ranges::forward_range<View>,
            std::forward_iterator_tag,
            std::input_iterator_tag>>;
    using value_type = Value;
    using difference_type = std::ranges::range_difference_t<View>;
    using reference = std::conditional_t<randomAccess_ and not stableReference_, Value, CommonReference>;

    Iterator() = default;

    constexpr Iterator(Intersperse *parent, std::ranges::iterator_t<View> current)
      requires(not randomAccess_)
        : parent_(parent)
        , current_(std::move(current))
        , done_(current_ == std::ranges::end(parent_->view_)) {
    }

    constexpr Iterator(Intersperse *parent, difference_type position)
      requires randomAccess_
        : parent_(parent)
        , position_(position) {
    }

    [[nodiscard]] constexpr auto operator*() const -> reference {
      if constexpr (randomAccess_) {
        if (position_ % 2 != 0) {
          return parent_->separator_;
        }
        auto &&item = std::ranges::begin(parent_->view_)[position_ / 2];
        if constexpr (stableReference_) {
          return item;
        } else {
          return Value{std::forward<decltype(item)>(item)};
        }
      } else {
        return separatorNext_ ? CommonReference{parent_->separator_} : CommonReference{*current_};
      }
    }

    [[nodiscard]] constexpr auto operator[](difference_type offset) const -> reference
      requires randomAccess_
    {
      return *(*this + offset);
    }

    constexpr auto operator++() -> Iterator & {
      if constexpr (randomAccess_) {
        ++position_;
      } else if (separatorNext_) {
        separatorNext_ = false;
      } else {
        ++current_;
        if (current_ == std::ranges::end(parent_->view_)) {
          done_ = true;
        } else {
          separatorNext_ = true;
        }
      }
      return *this;
    }

    constexpr auto operator++(int) {
      if constexpr (randomAccess_ or std::ranges::forward_range<View>) {
        auto previous = *this;
        ++*this;
        return previous;
      } else {
        ++*this;
      }
    }

    constexpr auto operator--() -> Iterator &
      requires randomAccess_
    {
      --position_;
      return *this;
    }
    constexpr auto operator--(int) -> Iterator
      requires randomAccess_
    {
      auto previous = *this;
      --*this;
      return previous;
    }
    constexpr auto operator+=(difference_type offset) -> Iterator &
      requires randomAccess_
    {
      position_ += offset;
      return *this;
    }
    constexpr auto operator-=(difference_type offset) -> Iterator &
      requires randomAccess_
    {
      position_ -= offset;
      return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(Iterator iterator, difference_type offset) -> Iterator
      requires randomAccess_
    {
      iterator += offset;
      return iterator;
    }
    [[nodiscard]] friend constexpr auto operator+(difference_type offset, Iterator iterator) -> Iterator
      requires randomAccess_
    {
      return iterator + offset;
    }
    [[nodiscard]] friend constexpr auto operator-(Iterator iterator, difference_type offset) -> Iterator
      requires randomAccess_
    {
      iterator -= offset;
      return iterator;
    }
    [[nodiscard]] friend constexpr auto operator-(const Iterator &left, const Iterator &right)
        -> difference_type
      requires randomAccess_
    {
      return left.position_ - right.position_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &left, const Iterator &right) -> bool
      requires(randomAccess_ or std::ranges::forward_range<View>)
    {
      if constexpr (randomAccess_) {
        return left.position_ == right.position_;
      } else {
        return left.current_ == right.current_ and left.separatorNext_ == right.separatorNext_;
      }
    }

    [[nodiscard]] friend constexpr auto operator<=>(const Iterator &left, const Iterator &right)
      requires randomAccess_
    {
      return left.position_ <=> right.position_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] std::default_sentinel_t sentinel) -> bool
      requires(not randomAccess_)
    {
      return iterator.done_;
    }

  private:
    Intersperse *parent_{};
    std::ranges::iterator_t<View> current_{};
    difference_type position_{};
    bool separatorNext_{};
    bool done_{true};
  };

public:
  Intersperse()
    requires std::default_initializable<View> and std::default_initializable<Value>
  = default;

  constexpr Intersperse(View view, Value separator)
      : view_(std::move(view))
      , separator_(std::move(separator)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    if constexpr (randomAccess_) {
      return Iterator{this, 0};
    } else {
      return Iterator{this, std::ranges::begin(view_)};
    }
  }

  [[nodiscard]] constexpr auto end() {
    if constexpr (randomAccess_) {
      return Iterator{this, static_cast<std::ranges::range_difference_t<View>>(size())};
    } else {
      return std::default_sentinel;
    }
  }

  [[nodiscard]] constexpr auto size()
    requires std::ranges::sized_range<View>
  {
    const auto count = static_cast<usize>(std::ranges::size(view_));
    return count == 0 ? 0 : (count * 2) - 1;
  }

  [[nodiscard]] constexpr auto size() const
    requires std::ranges::sized_range<const View>
  {
    const auto count = static_cast<usize>(std::ranges::size(view_));
    return count == 0 ? 0 : (count * 2) - 1;
  }

private:
  View view_{};
  Value separator_{};
};

/// Lazily generates separators between adjacent source values. A generated separator is cached so repeated
/// dereference of the same logical position never invokes the callable twice.
template <std::ranges::view View, class Function>
class IntersperseWith final {
  using Value = std::ranges::range_value_t<View>;
  using SourceReference = std::ranges::range_reference_t<View>;
  using Generated = std::remove_cvref_t<std::invoke_result_t<Function &>>;
  static_assert(std::constructible_from<Value, Generated>);
  using CommonReference = std::common_reference_t<SourceReference, const Value &>;

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Value;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator()
      requires std::default_initializable<std::ranges::iterator_t<View>>
    = default;
    constexpr Iterator(IntersperseWith *parent, std::ranges::iterator_t<View> current)
        : parent_(parent)
        , current_(std::move(current))
        , done_(current_ == std::ranges::end(parent_->view_)) {
    }

    [[nodiscard]] constexpr auto operator*() const -> CommonReference {
      if (separatorNext_) {
        if (not separator_.has_value()) {
          separator_.emplace(std::invoke(parent_->function_.get()));
        }
        return CommonReference{*separator_};
      }
      return CommonReference{*current_};
    }

    constexpr auto operator++() -> Iterator & {
      if (separatorNext_) {
        separator_.reset();
        separatorNext_ = false;
      } else {
        ++current_;
        if (current_ == std::ranges::end(parent_->view_)) {
          done_ = true;
        } else {
          separatorNext_ = true;
        }
      }
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] std::default_sentinel_t sentinel) -> bool {
      return iterator.done_;
    }

  private:
    IntersperseWith *parent_{};
    std::ranges::iterator_t<View> current_;
    mutable Option<Value> separator_{};
    bool separatorNext_{};
    bool done_{true};
  };

public:
  IntersperseWith()
    requires std::default_initializable<View> and std::default_initializable<Function>
  = default;

  constexpr IntersperseWith(View view, Function function)
      : view_(std::move(view))
      , function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this, std::ranges::begin(view_)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> std::default_sentinel_t {
    return {};
  }

  [[nodiscard]] constexpr auto size() const
    requires std::ranges::sized_range<const View>
  {
    const auto count = static_cast<usize>(std::ranges::size(view_));
    return count == 0 ? 0 : (count * 2) - 1;
  }

private:
  View view_{};
  [[no_unique_address]] MovableBox<Function> function_{};
};

/// Compile-time overlapping window mapper. The fixed window is represented as borrowed references so input
/// sources and move-only values are supported without copying. Miracle adaptive invocation accepts either the
/// whole fixed reference array or N decomposed element references.
template <std::ranges::view View, class Function, usize N>
class MapWindows final {
  static_assert(N > 0);
  using SourceReference = std::ranges::range_reference_t<View>;
  using Item = std::remove_cvref_t<SourceReference>;
  static constexpr bool borrowed_ =
      std::ranges::forward_range<View> and std::is_lvalue_reference_v<SourceReference>;
  using Slot = std::conditional_t<borrowed_, Ref<std::remove_reference_t<SourceReference>>, Item>;
  using Window = Array<Ref<const Item>, N>;
  using Mapped =
      std::remove_cvref_t<decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Window &>()))>;
  static_assert(not std::is_void_v<Mapped>);

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Mapped;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator()
      requires std::default_initializable<std::ranges::iterator_t<View>>
    = default;
    constexpr Iterator(MapWindows *parent, std::ranges::iterator_t<View> current)
        : parent_(parent)
        , current_(std::move(current)) {
      initialize();
    }

    [[nodiscard]] constexpr auto operator*() const -> Mapped & {
      return *mapped_;
    }

    constexpr auto operator++() -> Iterator & {
      if (done_) {
        return *this;
      }
      if (current_ == std::ranges::end(parent_->view_)) {
        mapped_.reset();
        done_ = true;
        return *this;
      }
      for (usize index{1}; index < N; ++index) {
        slots_[index - 1] = std::move(slots_[index]);
      }
      slots_[N - 1].reset();
      store(N - 1, *current_);
      ++current_;
      generate();
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] std::default_sentinel_t sentinel) -> bool {
      return iterator.done_;
    }

  private:
    template <class Reference>
    constexpr auto store(usize index, Reference &&reference) -> void {
      if constexpr (borrowed_) {
        slots_[index].emplace(reference);
      } else {
        slots_[index].emplace(std::forward<Reference>(reference));
      }
    }

    [[nodiscard]] constexpr auto item(usize index) const -> const Item & {
      if constexpr (borrowed_) {
        return slots_[index]->get();
      } else {
        return *slots_[index];
      }
    }

    template <usize... Index>
    [[nodiscard]] constexpr auto window([[maybe_unused]] std::index_sequence<Index...> indices) const
        -> Window {
      return {std::cref(item(Index))...};
    }

    constexpr auto generate() -> void {
      auto refs = window(std::make_index_sequence<N>{});
      mapped_.reset();
      mapped_.emplace(adaptiveInvoke(parent_->function_.get(), refs));
    }

    constexpr auto initialize() -> void {
      for (usize index{}; index < N; ++index) {
        if (current_ == std::ranges::end(parent_->view_)) {
          done_ = true;
          return;
        }
        store(index, *current_);
        ++current_;
      }
      generate();
    }

    MapWindows *parent_{};
    std::ranges::iterator_t<View> current_;
    Array<Option<Slot>, N> slots_{};
    mutable Option<Mapped> mapped_{};
    bool done_{};
  };

public:
  MapWindows()
    requires std::default_initializable<View> and std::default_initializable<Function>
  = default;

  constexpr MapWindows(View view, Function function)
      : view_(std::move(view))
      , function_(std::move(function)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this, std::ranges::begin(view_)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> std::default_sentinel_t {
    return {};
  }

  [[nodiscard]] constexpr auto size() const
    requires std::ranges::sized_range<const View>
  {
    const auto count = static_cast<usize>(std::ranges::size(view_));
    return count < N ? 0U : count - N + 1U;
  }

private:
  View view_{};
  [[no_unique_address]] MovableBox<Function> function_{};
};

/// Fixed-size non-overlapping chunk source. Borrowed random-access sources preserve indexed/double-ended
/// traversal; streaming or owning sources use a single-pass cache so move-only packet elements remain safe.
template <std::ranges::view View, usize N>
class ArrayChunks final {
  static_assert(N > 0);
  using SourceReference = std::ranges::range_reference_t<View>;
  using Item = std::remove_cvref_t<SourceReference>;
  static constexpr bool borrowed_ =
      std::ranges::forward_range<View> and std::is_lvalue_reference_v<SourceReference>;
  static constexpr bool indexed_ =
      borrowed_ and std::ranges::random_access_range<View> and std::ranges::sized_range<View>;
  using Slot = std::conditional_t<borrowed_, Ref<std::remove_reference_t<SourceReference>>, Item>;

public:
  using Chunk = Array<Slot, N>;

private:
  class Iterator final {
  public:
    using iterator_concept =
        std::conditional_t<indexed_, std::random_access_iterator_tag, std::input_iterator_tag>;
    using value_type = Chunk;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator()
      requires(indexed_ or std::default_initializable<std::ranges::iterator_t<View>>)
    = default;

    constexpr Iterator(ArrayChunks *parent, difference_type position)
      requires indexed_
        : parent_(parent)
        , position_(position) {
    }

    constexpr Iterator(ArrayChunks *parent, std::ranges::iterator_t<View> current)
      requires(not indexed_)
        : parent_(parent)
        , current_(std::move(current)) {
      load();
    }

    [[nodiscard]] constexpr auto operator*() const -> decltype(auto) {
      if constexpr (indexed_) {
        return indexedChunk(std::make_index_sequence<N>{});
      } else {
        return static_cast<Chunk &>(*chunk_);
      }
    }

    [[nodiscard]] constexpr auto operator[](difference_type offset) const -> Chunk
      requires indexed_
    {
      return *(*this + offset);
    }

    constexpr auto operator++() -> Iterator & {
      if constexpr (indexed_) {
        ++position_;
      } else {
        load();
      }
      return *this;
    }

    constexpr auto operator++(int) {
      if constexpr (indexed_) {
        auto previous = *this;
        ++*this;
        return previous;
      } else {
        ++*this;
      }
    }

    constexpr auto operator--() -> Iterator &
      requires indexed_
    {
      --position_;
      return *this;
    }

    constexpr auto operator--(int) -> Iterator
      requires indexed_
    {
      auto previous = *this;
      --*this;
      return previous;
    }

    constexpr auto operator+=(difference_type offset) -> Iterator &
      requires indexed_
    {
      position_ += offset;
      return *this;
    }

    constexpr auto operator-=(difference_type offset) -> Iterator &
      requires indexed_
    {
      position_ -= offset;
      return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(Iterator iterator, difference_type offset) -> Iterator
      requires indexed_
    {
      iterator += offset;
      return iterator;
    }

    [[nodiscard]] friend constexpr auto operator+(difference_type offset, Iterator iterator) -> Iterator
      requires indexed_
    {
      return iterator + offset;
    }

    [[nodiscard]] friend constexpr auto operator-(Iterator iterator, difference_type offset) -> Iterator
      requires indexed_
    {
      iterator -= offset;
      return iterator;
    }

    [[nodiscard]] friend constexpr auto operator-(const Iterator &left, const Iterator &right)
        -> difference_type
      requires indexed_
    {
      return left.position_ - right.position_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &left, const Iterator &right) -> bool
      requires indexed_
    {
      return left.position_ == right.position_ and left.parent_ == right.parent_;
    }

    [[nodiscard]] friend constexpr auto operator<=>(const Iterator &left, const Iterator &right)
      requires indexed_
    {
      return left.position_ <=> right.position_;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] std::default_sentinel_t sentinel) -> bool
      requires(not indexed_)
    {
      return not iterator.chunk_.has_value();
    }

  private:
    template <usize... Index>
    [[nodiscard]] constexpr auto indexedChunk([[maybe_unused]] std::index_sequence<Index...> indices) const
        -> Chunk
      requires indexed_
    {
      auto first = std::ranges::begin(parent_->view_) + (position_ * static_cast<difference_type>(N));
      return {std::ref(first[static_cast<difference_type>(Index)])...};
    }

    template <class Reference>
    constexpr auto store(usize index, Reference &&reference) -> void
      requires(not indexed_)
    {
      if constexpr (borrowed_) {
        slots_[index].emplace(reference);
      } else {
        slots_[index].emplace(std::forward<Reference>(reference));
      }
    }

    template <usize... Index>
    [[nodiscard]] constexpr auto makeChunk([[maybe_unused]] std::index_sequence<Index...> indices) -> Chunk
      requires(not indexed_)
    {
      return {std::move(*slots_[Index])...};
    }

    constexpr auto load() -> void
      requires(not indexed_)
    {
      chunk_.reset();
      for (auto &slot : slots_) {
        slot.reset();
      }
      for (usize index{}; index < N; ++index) {
        if (*current_ == std::ranges::end(parent_->view_)) {
          return;
        }
        store(index, **current_);
        ++*current_;
      }
      chunk_.emplace(makeChunk(std::make_index_sequence<N>{}));
    }

    ArrayChunks *parent_{};
    difference_type position_{};
    std::conditional_t<indexed_, std::monostate, Option<std::ranges::iterator_t<View>>> current_{};
    Array<Option<Slot>, N> slots_{};
    mutable Option<Chunk> chunk_{};
  };

public:
  ArrayChunks()
    requires std::default_initializable<View>
  = default;

  constexpr explicit ArrayChunks(View view)
      : view_(std::move(view)) {
  }

  [[nodiscard]] constexpr auto begin() {
    if constexpr (indexed_) {
      return Iterator{this, 0};
    } else {
      return Iterator{this, std::ranges::begin(view_)};
    }
  }

  [[nodiscard]] constexpr auto end() {
    if constexpr (indexed_) {
      return Iterator{this, static_cast<std::ranges::range_difference_t<View>>(size())};
    } else {
      return std::default_sentinel;
    }
  }

  [[nodiscard]] constexpr auto size() const
    requires std::ranges::sized_range<const View>
  {
    return static_cast<usize>(std::ranges::size(view_)) / N;
  }

private:
  View view_{};
};

} // namespace Miracle

export namespace Miracle {

/// Represents an unbounded positional limit. `Infinity` is deliberately not numeric.
/// This token is used only where an API needs an open upper positional bound, such as `slice(start,
/// infinity)`; it does not make ordinary Iter pipelines cardinality-aware.
struct Infinity final {};
/// Shared open-bound token for positional APIs.
inline constexpr Infinity infinity{};

/// Conservative dynamic bounds for the number of remaining values in a pipeline.
struct SizeHint final {
  /// Guaranteed number of values remaining.
  usize lower{};
  /// Known upper bound when available; an empty Option means the bound is unknown.
  Option<usize> upper;

  /// Returns true when the lower and upper bounds describe one exact remaining size.
  [[nodiscard]] constexpr auto exact() const noexcept -> bool {
    return upper.has_value() and *upper == lower;
  }
};

/// Standard-execution-policy terminal façade produced by `Iter::parallel(...)`.
template <class Iteration, class Policy>
class ParallelIter;

} // namespace Miracle

namespace Miracle {

/// Maps a terminal's reference category to the correct Option payload: lvalues remain direct C++26 optional
/// references, while xvalues/prvalues become owned values.
template <class Result>
struct TerminalOption;

template <class T>
struct TerminalOption<T &> {
  using Type = Option<T &>;
};

template <class T>
struct TerminalOption<const T &> {
  using Type = Option<const T &>;
};

template <class T>
struct TerminalOption<T &&> {
  using Type = Option<std::remove_cvref_t<T>>;
};

template <class T>
struct TerminalOption {
  using Type = Option<std::remove_cvref_t<T>>;
};

template <class Result>
using TerminalOptionT = TerminalOption<Result>::Type;

/// Captures a terminal result without losing reference identity or accidentally copying and rvalue.
template <class Result>
[[nodiscard]] constexpr auto makeTerminalValue(Result &&result) -> TerminalOptionT<Result &&> {
  if constexpr (std::is_lvalue_reference_v<Result &&>) {
    return result;
  } else {
    return std::remove_cvref_t<Result>{std::forward<Result>(result)};
  }
}

/// Constructs the empty form matching `makeTerminalValue`'s reference/value policy.
template <class Result>
[[nodiscard]] constexpr auto emptyTerminalValue() -> TerminalOptionT<Result> {
  return {};
}

template <class Container, class Value>
concept PushBackContainer =
    requires(Container container, Value &&value) { container.push_back(std::forward<Value>(value)); };

template <class Container>
concept ReservableContainer = requires(Container container, usize count) { container.reserve(count); };

} // namespace Miracle

export namespace Miracle {

/// Stateful one-element lookahead adaptor. Peekable owns one logical cursor over its source and keeps at most
/// one front item cached; unlike ordinary Iter it intentionally exposes a narrow cursor API.
template <std::ranges::view Iteration>
class Peekable final {
  using SourceReference = std::ranges::range_reference_t<Iteration>;
  using Item = std::remove_cvref_t<SourceReference>;
  static constexpr bool borrowed_ =
      std::ranges::forward_range<Iteration> and std::is_lvalue_reference_v<SourceReference>;
  static constexpr bool mutableBorrow_ =
      not borrowed_ or not std::is_const_v<std::remove_reference_t<SourceReference>>;
  using Borrowed = std::remove_reference_t<SourceReference>;
  using Cache = std::conditional_t<borrowed_, Option<Borrowed &>, Option<Item>>;
  using NextResult = std::conditional_t<borrowed_, Option<Borrowed &>, Option<Item>>;
  static constexpr bool doubleEnded_ =
      std::ranges::bidirectional_range<Iteration> and std::ranges::common_range<Iteration>;

  class Cursor final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Item;
    using difference_type = std::ranges::range_difference_t<Iteration>;

    Cursor() = default;
    constexpr explicit Cursor(Peekable *parent)
        : parent_(parent) {
    }

    [[nodiscard]] constexpr auto operator*() const -> decltype(auto) {
      return parent_->frontReference();
    }

    constexpr auto operator++() -> Cursor & {
      parent_->discardFront();
      return *this;
    }

    constexpr auto operator++(int) -> void {
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Cursor &iterator,
        [[maybe_unused]] std::default_sentinel_t sentinel) -> bool {
      return iterator.parent_ == nullptr or not iterator.parent_->hasRemaining();
    }

  private:
    Peekable *parent_{};
  };

public:
  using value_type = Item;

  Peekable()
    requires std::default_initializable<Iteration>
  = default;

  constexpr explicit Peekable(Iteration iteration)
      : iteration_(std::move(iteration)) {
    if constexpr (std::ranges::sized_range<Iteration>) {
      exactRemaining_ = static_cast<usize>(std::ranges::size(iteration_));
    }
  }

  [[nodiscard]] constexpr auto next() -> NextResult {
    if (not fillFront()) {
      return {};
    }
    auto result = takeCached();
    consumeOne();
    return result;
  }

  [[nodiscard]] constexpr auto nextBack() -> NextResult
    requires doubleEnded_
  {
    ensureCursor();
    if (cache_.has_value()) {
      if (cacheConsumesSource_) {
        auto last = *back_;
        if (last == *front_) {
          return {};
        }
        --last;
        if (last == *front_) {
          return next();
        }
        *back_ = last;
        auto result = makeTerminalValue(**back_);
        consumeOne();
        return result;
      }
      if (*front_ == *back_) {
        auto result = takeCached();
        consumeOne();
        return result;
      }
    }
    if (*front_ == *back_) {
      return {};
    }
    --*back_;
    auto result = makeTerminalValue(**back_);
    consumeOne();
    return result;
  }

  [[nodiscard]] constexpr auto peek() -> Option<const Item &> {
    if (not fillFront()) {
      return {};
    }
    return std::as_const(*this).cachedItem();
  }

  [[nodiscard]] constexpr auto peekMut() -> Option<Item &>
    requires mutableBorrow_
  {
    if (not fillFront()) {
      return {};
    }
    return cachedItem();
  }

  template <class Predicate>
  [[nodiscard]] constexpr auto nextIf(Predicate predicate) -> NextResult {
    auto value = peek();
    if (value.has_value() and static_cast<bool>(adaptiveInvoke(predicate, *value))) {
      return next();
    }
    return {};
  }

  template <class T>
  [[nodiscard]] constexpr auto nextIfEq(const T &value) -> NextResult
    requires requires(const Item &item) {
      { item == value } -> std::convertible_to<bool>;
    }
  {
    return nextIf([&](const Item &item) -> bool { return item == value; });
  }

  template <class Function>
  [[nodiscard]] constexpr auto nextIfMap(Function function) {
    auto candidate = next();
    using CandidateReference = std::conditional_t<borrowed_, decltype(*candidate), Item &&>;
    using Outcome = std::remove_cvref_t<decltype(adaptiveInvoke(
        std::declval<Function &>(), std::declval<CandidateReference>()))>;
    static_assert(ExpectedLike<Outcome>, "Peekable::nextIfMap requires an expected-like return value");
    using Mapped = std::remove_cvref_t<typename Outcome::value_type>;

    if (not candidate.has_value()) {
      return Option<Mapped>{};
    }

    Outcome outcome = [&] -> Outcome {
      if constexpr (borrowed_) {
        return adaptiveInvoke(function, *candidate);
      } else {
        return adaptiveInvoke(function, std::move(*candidate));
      }
    }();
    if (outcome.has_value()) {
      return Option<Mapped>{Mapped{std::move(*outcome)}};
    }

    using Error = Outcome::error_type;
    if constexpr (borrowed_) {
      if constexpr (std::same_as<std::remove_cvref_t<Error>, Ref<Borrowed>>) {
        cache_.emplace(outcome.error().get());
      } else {
        static_assert(std::same_as<std::remove_cvref_t<Error>, Ref<Borrowed>>,
            "nextIfMap on a borrowed Peekable must reject with Ref<Item> so the put-back item cannot dangle");
      }
    } else {
      static_assert(std::constructible_from<Item, Error &&>,
          "nextIfMap rejection must carry an item that can be put back into the Peekable cache");
      cache_.emplace(std::move(outcome.error()));
    }
    cacheConsumesSource_ = false;
    restoreOne();
    return Option<Mapped>{};
  }

  template <class Function>
  [[nodiscard]] constexpr auto nextIfMapMut(Function function)
    requires mutableBorrow_
  {
    using Result =
        std::remove_cvref_t<decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Item &>()))>;
    static_assert(OptionalLike<Result>, "Peekable::nextIfMapMut requires an Optional-like return value");
    if (not fillFront()) {
      return Result{};
    }
    Result result = adaptiveInvoke(function, cachedItem());
    if (result.has_value()) {
      discardFront();
    }
    return result;
  }

  [[nodiscard]] constexpr auto size() -> usize
    requires std::ranges::sized_range<Iteration>
  {
    return exactRemaining_.value_or(0U);
  }

  [[nodiscard]] constexpr auto sizeHint() -> SizeHint {
    if (exactRemaining_.has_value()) {
      return {.lower = *exactRemaining_, .upper = *exactRemaining_};
    }
    return {.lower = cache_.has_value() ? 1U : 0U, .upper = None};
  }

  [[nodiscard]] constexpr auto begin() -> Cursor {
    return Cursor{this};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> std::default_sentinel_t {
    return {};
  }

private:
  constexpr auto ensureCursor() -> void {
    if (initialized_) {
      return;
    }
    front_.emplace(std::ranges::begin(iteration_));
    end_.emplace(std::ranges::end(iteration_));
    if constexpr (doubleEnded_) {
      back_.emplace(std::ranges::end(iteration_));
    }
    initialized_ = true;
  }

  [[nodiscard]] constexpr auto sourceEmpty() -> bool {
    ensureCursor();
    if constexpr (doubleEnded_) {
      return *front_ == *back_;
    } else {
      return *front_ == *end_;
    }
  }

  [[nodiscard]] constexpr auto fillFront() -> bool {
    if (cache_.has_value()) {
      return true;
    }
    if (sourceEmpty()) {
      return false;
    }
    auto &&value = **front_;
    if constexpr (borrowed_) {
      cache_.emplace(value);
    } else {
      cache_.emplace(std::forward<decltype(value)>(value));
    }
    cacheConsumesSource_ = true;
    return true;
  }

  [[nodiscard]] constexpr auto cachedItem() -> Item &
    requires mutableBorrow_
  {
    if constexpr (borrowed_) {
      return const_cast<Item &>(*cache_);
    } else {
      return *cache_;
    }
  }

  [[nodiscard]] constexpr auto cachedItem() const -> const Item & {
    return *cache_;
  }

  [[nodiscard]] constexpr auto frontReference() -> decltype(auto) {
    if (not fillFront()) {
      std::terminate();
    }
    if constexpr (borrowed_) {
      return static_cast<Borrowed &>(*cache_);
    } else {
      return static_cast<Item &>(*cache_);
    }
  }

  [[nodiscard]] constexpr auto takeCached() -> NextResult {
    const bool advanceSource = cacheConsumesSource_;
    if constexpr (borrowed_) {
      NextResult result{*cache_};
      cache_.reset();
      cacheConsumesSource_ = false;
      if (advanceSource) {
        ++*front_;
      }
      return result;
    } else {
      NextResult result{std::move(*cache_)};
      cache_.reset();
      cacheConsumesSource_ = false;
      if (advanceSource) {
        ++*front_;
      }
      return result;
    }
  }

  constexpr auto discardFront() -> void {
    if (fillFront()) {
      const bool advanceSource = cacheConsumesSource_;
      cache_.reset();
      cacheConsumesSource_ = false;
      if (advanceSource) {
        ++*front_;
      }
      consumeOne();
    }
  }

  [[nodiscard]] constexpr auto hasRemaining() -> bool {
    return cache_.has_value() or not sourceEmpty();
  }

  constexpr auto consumeOne() -> void {
    if (exactRemaining_.has_value()) {
      --*exactRemaining_;
    }
  }

  constexpr auto restoreOne() -> void {
    if (exactRemaining_.has_value()) {
      ++*exactRemaining_;
    }
  }

  Iteration iteration_{};
  Option<std::ranges::iterator_t<Iteration>> front_{};
  Option<std::ranges::sentinel_t<Iteration>> end_{};
  std::conditional_t<doubleEnded_, Option<std::ranges::iterator_t<Iteration>>, std::monostate> back_{};
  Cache cache_{};
  Option<usize> exactRemaining_;
  bool cacheConsumesSource_{};
  bool initialized_{};
};

/// Lazy range façade that keeps at most one pending map projection.
/// Standard views own traversal semantics; Miracle adds adaptive invocation and fuses projection-aware
/// terminals where doing so avoids redundant work.
template <std::ranges::view View, class Projection>
class Iter final {
public:
  /// Reference yielded by the stored standard view before Miracle's pending projection.
  using BaseReference = std::ranges::range_reference_t<View>;
  /// Reference/value category observed by Iter consumers after applying the pending projection.
  using Reference = std::conditional_t<isIdentityProjection<Projection>,
      BaseReference,
      std::invoke_result_t<Projection &, BaseReference>>;
  /// Materialized value type of the pipeline.
  using Value = std::remove_cvref_t<Reference>;

  /// Default construction is available only when both stored components support it.
  constexpr Iter()
    requires std::default_initializable<View> and std::default_initializable<Projection>
  = default;

  /// Constructs an Iter directly from an already-normalized view with no pending map projection.
  constexpr explicit Iter(View view)
    requires std::same_as<Projection, std::identity>
      : view_(std::move(view))
      , projection_(std::identity{}) {
  }

  /// Internal/publicly visible structural constructor used to carry one composed projection.
  constexpr Iter(View view, Projection projection)
      : view_(std::move(view))
      , projection_(std::move(projection)) {
  }

  /// Returns the underlying iterator directly when no projection is pending; otherwise returns a
  /// category-preserving projected iterator.
  [[nodiscard]] constexpr auto begin() {
    if constexpr (isIdentityProjection<Projection>) {
      return std::ranges::begin(view_);
    } else {
      return ProjectedIterator<std::ranges::iterator_t<View>, Projection>{
          std::ranges::begin(view_), std::addressof(projection_.get())};
    }
  }

  /// Returns a projected iterator or sentinel matching the common-range property of the base.
  [[nodiscard]] constexpr auto end() {
    if constexpr (isIdentityProjection<Projection>) {
      return std::ranges::end(view_);
    } else if constexpr (std::ranges::common_range<View>) {
      return ProjectedIterator<std::ranges::iterator_t<View>, Projection>{
          std::ranges::end(view_), std::addressof(projection_.get())};
    } else {
      return ProjectedSentinel<std::ranges::sentinel_t<View>>{std::ranges::end(view_)};
    }
  }

  /// Const iteration is available only when the base and projection are const-invocable.
  [[nodiscard]] constexpr auto begin() const
    requires std::ranges::range<const View> and
             (isIdentityProjection<Projection> or
                 std::regular_invocable<const Projection &, std::ranges::range_reference_t<const View>>)
  {
    if constexpr (isIdentityProjection<Projection>) {
      return std::ranges::begin(view_);
    } else {
      return ProjectedIterator<std::ranges::iterator_t<const View>, const Projection>{
          std::ranges::begin(view_), std::addressof(projection_.get())};
    }
  }

  /// Const sentinel counterpart to `begin() const`.
  [[nodiscard]] constexpr auto end() const
    requires std::ranges::range<const View> and
             (isIdentityProjection<Projection> or
                 std::regular_invocable<const Projection &, std::ranges::range_reference_t<const View>>)
  {
    if constexpr (isIdentityProjection<Projection>) {
      return std::ranges::end(view_);
    } else if constexpr (std::ranges::common_range<const View>) {
      return ProjectedIterator<std::ranges::iterator_t<const View>, const Projection>{
          std::ranges::end(view_), std::addressof(projection_.get())};
    } else {
      return ProjectedSentinel<std::ranges::sentinel_t<const View>>{std::ranges::end(view_)};
    }
  }

  /// Forwards exact size when the underlying view is sized; a pending projection never changes cardinality.
  [[nodiscard]] constexpr auto size()
    requires std::ranges::sized_range<View>
  {
    return std::ranges::size(view_);
  }

  /// Const exact-size query for sized const views.
  [[nodiscard]] constexpr auto size() const
    requires std::ranges::sized_range<const View>
  {
    return std::ranges::size(view_);
  }

  /// Returns an exact hint for sized views and an unknown bound otherwise. Iter deliberately does not
  /// maintain a separate cardinality type system.
  [[nodiscard]] constexpr auto sizeHint() const -> SizeHint {
    if constexpr (std::ranges::sized_range<const View>) {
      const auto count = static_cast<usize>(std::ranges::size(view_));
      return {.lower = count, .upper = count};
    }
    return {};
  }

  /// Composes a lazy projection without creating another transform-view layer.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto map(this Self &&self, Function function) {
    auto next = adapt(std::move(function));
    auto view = takeView(std::forward<Self>(self));
    if constexpr (isIdentityProjection<Projection>) {
      return Iter<View, decltype(next)>{std::move(view), std::move(next)};
    } else {
      auto projection = takeProjection(std::forward<Self>(self));
      auto composed = compose(std::move(projection), std::move(next));
      return Iter<View, decltype(composed)>{std::move(view), std::move(composed)};
    }
  }

  /// Keeps values accepted by an adaptive predicate.
  /// A pending map is retained in a small fused view so Miracle materializers evaluate it once per source
  /// item instead of the naïve transform/filter twice.
  template <class Self, class Predicate>
  [[nodiscard]] constexpr auto filter(this Self &&self, Predicate predicate) {
    auto wrapped = adapt(std::move(predicate));
    auto view = takeView(std::forward<Self>(self));
    if constexpr (isIdentityProjection<Projection>) {
      auto filtered = std::views::filter(std::move(view), std::move(wrapped));
      return Iter<decltype(filtered)>{std::move(filtered)};
    } else {
      auto projection = takeProjection(std::forward<Self>(self));
      using Fused = ProjectedFilterView<View, Projection, decltype(wrapped)>;
      return Iter<Fused>{Fused{std::move(view), std::move(projection), std::move(wrapped)}};
    }
  }

  /// Maps to Option-like values and yields only present results, evaluating the mapper once.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto filterMap(this Self &&self, Function function) {
    auto base = lower(std::forward<Self>(self));
    auto mapped =
        std::views::transform(std::move(base), adapt(std::move(function))) | std::views::cache_latest;
    auto present =
        std::views::filter(std::move(mapped), [](const auto &value) -> bool { return value.has_value(); });
    auto values = std::views::transform(std::move(present), [](auto &&value) -> decltype(auto) {
      return *std::forward<decltype(value)>(value);
    }) | std::views::as_rvalue;
    return Iter<decltype(values)>(std::move(values));
  }

  /// Maps each item to a range and lazily concatenates those ranges.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto flatMap(this Self &&self, Function function) {
    auto mapped = std::forward<Self>(self).map(std::move(function));
    return std::move(mapped).flatten();
  }

  /// Flattens one level of nested ranges using `std::views::join`.
  template <class Self>
  [[nodiscard]] constexpr auto flatten(this Self &&self) {
    auto base = lower(std::forward<Self>(self));
    auto flattened = std::views::join(std::move(base));
    return Iter<decltype(flattened)>{std::move(flattened)};
  }

  /// Yields `(index, item)` pairs using the standard enumerate view.
  template <class Self>
  [[nodiscard]] constexpr auto enumerate(this Self &&self) {
    auto base = lower(std::forward<Self>(self));
    auto enumerated = std::views::enumerate(std::move(base));
    return Iter<decltype(enumerated)>{std::move(enumerated)};
  }

  /// Lazily pairs this pipeline with another range, stopping at the shorter input.
  template <class Self, std::ranges::viewable_range Other>
  [[nodiscard]] constexpr auto zip(this Self &&self, Other &&other) {
    auto left = lower(std::forward<Self>(self));
    auto right = viewForRange(std::forward<Other>(other));
    auto zipped = std::views::zip(std::move(left), std::move(right));
    return Iter<decltype(zipped)>{std::move(zipped)};
  }

  /// Lazily concatenates this pipeline with another compatible range.
  template <class Self, std::ranges::viewable_range Other>
  [[nodiscard]] constexpr auto chain(this Self &&self, Other &&other) {
    auto left = lower(std::forward<Self>(self));
    auto right = viewForRange(std::forward<Other>(other));
    auto chained = std::views::concat(std::move(left), std::move(right));
    return Iter<decltype(chained)>{std::move(chained)};
  }

  /// Positional adaptors commute with map, so they preserve the pending projection.
  template <class Self>
  [[nodiscard]] constexpr auto take(this Self &&self, usize count) {
    auto view = std::views::take(takeView(std::forward<Self>(self)), count);
    return rebindProjection(std::forward<Self>(self), std::move(view));
  }

  /// Drops at most `count` leading items while preserving a pending projection.
  template <class Self>
  [[nodiscard]] constexpr auto skip(this Self &&self, usize count) {
    auto view = std::views::drop(takeView(std::forward<Self>(self)), count);
    return rebindProjection(std::forward<Self>(self), std::move(view));
  }

  /// Yields the longest leading prefix accepted by the adaptive predicate.
  template <class Self, class Predicate>
  [[nodiscard]] constexpr auto takeWhile(this Self &&self, Predicate predicate) {
    auto base = lower(std::forward<Self>(self));
    auto view = std::views::take_while(std::move(base), adapt(std::move(predicate)));
    return Iter<decltype(view)>(std::move(view));
  }

  /// Drops the longest leading prefix accepted by the adaptive predicate.
  template <class Self, class Predicate>
  [[nodiscard]] constexpr auto skipWhile(this Self &&self, Predicate predicate) {
    auto base = lower(std::forward<Self>(self));
    auto view = std::views::drop_while(std::move(base), adapt(std::move(predicate)));
    return Iter<decltype(view)>(std::move(view));
  }

  /// Keeps every `step`th item. `step` must be strictly positive.
  template <class Self>
  [[nodiscard]] constexpr auto stepBy(this Self &&self, usize step) pre(step > 0) {
    if (step == 0) {
      std::terminate();
    }
    auto view = std::views::stride(takeView(std::forward<Self>(self)), step);
    return rebindProjection(std::forward<Self>(self), std::move(view));
  }

  /// Reverses a bidirectional common range without materializing it.
  template <class Self>
    requires std::ranges::bidirectional_range<View> and std::ranges::common_range<View>
  [[nodiscard]] constexpr auto rev(this Self &&self) {
    auto view = std::views::reverse(takeView(std::forward<Self>(self)));
    return rebindProjection(std::forward<Self>(self), std::move(view));
  }

  /// Converts the remaining sequence into a dedicated one-item-lookahead cursor adaptor. Pending projection
  /// state is lowered first so Peekable observes the exact logical items of this pipeline.
  template <class Self>
  [[nodiscard]] constexpr auto peekable(this Self &&self) {
    auto base = lower(std::forward<Self>(self));
    return Peekable<decltype(base)>{std::move(base)};
  }

  /// Compatibility adaptor. Standard range exhaustion is already stable, so no wrapper is needed.
  template <class Self>
  [[nodiscard]] constexpr auto fuse(this Self &&self) {
    return std::forward<Self>(self);
  }

  /// Copies each referenced item into an owned value. This narrow spelling mirrors Rust's `copied()` by
  /// accepting only genuine lvalue-reference pipelines whose values have trivial copy construction.
  template <class Self>
    requires std::is_lvalue_reference_v<Reference> and std::is_trivially_copy_constructible_v<Value> and
             std::constructible_from<Value, Reference>
  [[nodiscard]] constexpr auto copied(this Self &&self) {
    return std::forward<Self>(self).map([](auto &&item) -> Value { return Value{item}; });
  }

  /// Copy-constructs each referenced item into an owned value. Unlike `copied`, non-trivial copies are
  /// accepted, but prvalue/xvalue pipelines remain excluded because they are already owned values.
  template <class Self>
    requires std::is_lvalue_reference_v<Reference> and std::copy_constructible<Value> and
             std::constructible_from<Value, Reference>
  [[nodiscard]] constexpr auto cloned(this Self &&self) {
    return std::forward<Self>(self).map([](auto &&item) -> Value { return Value{item}; });
  }

  /// Repeats a restartable source forever without buffering. Pending projections are lowered before the cycle
  /// boundary so stateful `map(...).cycle()` evaluation is not reordered into `cycle().map(...)`.
  template <class Self>
    requires std::ranges::forward_range<View>
  [[nodiscard]] constexpr auto cycle(this Self &&self) {
    auto base = lower(std::forward<Self>(self));
    using Result = Cycle<decltype(base)>;
    return Iter<Result>{Result{std::move(base)}};
  }

  /// Invokes a side-effect callback for each observed item and forwards the item unchanged.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto inspect(this Self &&self, Function function) {
    auto base = lower(std::forward<Self>(self));
    auto inspected = std::views::transform(
        std::move(base), [function = adapt(std::move(function))](auto &&item) mutable -> decltype(auto) {
          std::invoke(function, item);
          return std::forward<decltype(item)>(item);
        });
    return Iter<decltype(inspected)>{std::move(inspected)};
  }

  /// Threads mutable state through the source and lazily yields the function's successive outputs.
  template <class Self, class State, class Function>
  [[nodiscard]] constexpr auto scan(this Self &&self, State state, Function function) {
    auto base = lower(std::forward<Self>(self));
    using ScanType = Scan<decltype(base), std::decay_t<State>, std::decay_t<Function>>;
    auto view = ScanType{std::move(base), std::move(state), std::move(function)} | std::views::as_rvalue;
    return Iter<decltype(view)>{std::move(view)};
  }

  /// Maps until the function returns an empty Optional-like value, then terminates the sequence.
  /// `cache_latest` ensures the Optional-like result is evaluated once per visited source item.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto mapWhile(this Self &&self, Function function) {
    auto base = lower(std::forward<Self>(self));
    auto mapped =
        std::views::transform(std::move(base), adapt(std::move(function))) | std::views::cache_latest;
    auto prefix = std::views::take_while(
        std::move(mapped), [](const auto &value) -> bool { return value.has_value(); });
    auto values = std::views::transform(std::move(prefix), [](auto &&value) -> decltype(auto) {
      return *std::forward<decltype(value)>(value);
    }) | std::views::as_rvalue;
    return Iter<decltype(values)>{std::move(values)};
  }

  /// Groups consecutive items into non-overlapping chunks of `count`; `count` must be positive.
  template <class Self>
  [[nodiscard]] constexpr auto chunks(this Self &&self, usize count) pre(count > 0) {
    if (count == 0) {
      std::terminate();
    }
    auto base = lower(std::forward<Self>(self));
    auto view = std::views::chunk(std::move(base), count);
    return Iter<decltype(view)>{std::move(view)};
  }

  /// Produces overlapping windows of `count` consecutive items; `count` must be positive.
  template <class Self>
  [[nodiscard]] constexpr auto windows(this Self &&self, usize count) pre(count > 0) {
    if (count == 0) {
      std::terminate();
    }
    auto base = lower(std::forward<Self>(self));
    auto view = std::views::slide(std::move(base), count);
    return Iter<decltype(view)>{std::move(view)};
  }

  /// Inserts `separator` between adjacent values through one semantic adaptor type. Indexed bases retain
  /// indexed traversal while streaming bases remain single-pass/forward as truthful.
  template <class Self>
  [[nodiscard]] constexpr auto intersperse(this Self &&self, Value separator) {
    auto base = lower(std::forward<Self>(self));
    using Result = Intersperse<decltype(base)>;
    return Iter<Result>{Result{std::move(base), std::move(separator)}};
  }

  /// Lazily generates a separator only when traversal actually crosses an adjacent-element boundary.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto intersperseWith(this Self &&self, Function function) {
    auto base = lower(std::forward<Self>(self));
    using Result = IntersperseWith<decltype(base), std::decay_t<Function>>;
    return Iter<Result>{Result{std::move(base), std::move(function)}};
  }

  /// Maps every overlapping compile-time window. The adaptive callable may accept the whole fixed reference
  /// array or N decomposed element references.
  template <usize N, class Self, class Function>
    requires(N > 0)
  [[nodiscard]] constexpr auto mapWindows(this Self &&self, Function function) {
    auto base = lower(std::forward<Self>(self));
    using Result = MapWindows<decltype(base), std::decay_t<Function>, N>;
    auto view = Result{std::move(base), std::move(function)} | std::views::as_rvalue;
    return Iter<decltype(view)>{std::move(view)};
  }

  /// Consumes non-overlapping fixed-size packets and omits a final short remainder. Borrowed sources use
  /// reference-wrapper packet elements; owned source move values into the packet.
  template <usize N, class Self>
    requires(N > 0)
  [[nodiscard]] constexpr auto arrayChunks(this Self &&self) {
    auto base = lower(std::forward<Self>(self));
    using Result = ArrayChunks<decltype(base), N>;
    auto view = Result{std::move(base)} | std::views::as_rvalue;
    return Iter<decltype(view)>{std::move(view)};
  }

  /// Drops the first `start` items and yields the remainder.
  template <class Self>
  [[nodiscard]] constexpr auto slice(this Self &&self, usize start) {
    return std::forward<Self>(self).skip(start);
  }

  /// Yields half-open positional interval `[start, stop)`. Reversed bounds produce an empty slice rather than
  /// reverse traversal.
  template <class Self>
  [[nodiscard]] constexpr auto slice(this Self &&self, usize start, usize stop) {
    const usize count = stop > start ? stop - start : 0U;
    return std::forward<Self>(self).skip(start).take(count);
  }

  /// Range-vocabulary overload for a non-negative half-open positional slice.
  template <class Self, class T>
  [[nodiscard]] constexpr auto slice(this Self &&self, Range<T> range)
      pre(range.start >= T{} and range.stop >= T{}) {
    if (range.start < T{} or range.stop < T{}) {
      std::terminate();
    }
    return std::forward<Self>(self).slice(static_cast<usize>(range.start), static_cast<usize>(range.stop));
  }

  /// Open-ended slice equivalent to `skip(start)`. `infinity` is positional syntax only.
  template <class Self>
  [[nodiscard]] constexpr auto slice(this Self &&self, usize start, [[maybe_unused]] Infinity limit) {
    return std::forward<Self>(self).skip(start);
  }

  /// Explicitly opts into standard cache-latest semantics and their input-range capability reduction.
  template <class Self>
  [[nodiscard]] constexpr auto cacheLatest(this Self &&self) {
    auto base = lower(std::forward<Self>(self));
    auto cached = std::views::cache_latest(std::move(base));
    return Iter<decltype(cached)>{std::move(cached)};
  }

  /// Returns the first item accepted by the predicate, preserving references for borrowed values.
  template <class Predicate>
  [[nodiscard]] constexpr auto find(Predicate predicate) {
    using Result = TerminalOptionT<Reference>;
    Result found{};
    auto wrapped = adapt(std::move(predicate));
    visitProjected([&](auto &&item) -> bool {
      if (static_cast<bool>(std::invoke(wrapped, item))) {
        found = makeTerminalValue(std::forward<decltype(item)>(item));
        return false;
      }
      return true;
    });
    return found;
  }

  /// Searches from the back and returns the first reverse match, preserving borrowed references.
  template <class Predicate>
  [[nodiscard]] constexpr auto rFind(Predicate predicate)
    requires std::ranges::bidirectional_range<Iter> and std::ranges::common_range<Iter>
  {
    using Result = TerminalOptionT<Reference>;
    auto wrapped = adapt(std::move(predicate));
    auto current = end();
    const auto first = begin();
    while (current != first) {
      --current;
      using Item = decltype(*current);
      if constexpr (std::is_reference_v<Item>) {
        auto &&item = *current;
        if (static_cast<bool>(std::invoke(wrapped, item))) {
          return Result{makeTerminalValue(std::forward<Item>(item))};
        }
      } else {
        auto item = *current;
        if (static_cast<bool>(std::invoke(wrapped, item))) {
          return Result{makeTerminalValue(std::move(item))};
        }
      }
    }
    return Result{};
  }

  /// Returns the first present Optional-like value produced by `function`.
  template <class Function>
  [[nodiscard]] constexpr auto findMap(Function function) {
    using Maybe =
        std::remove_cvref_t<decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Reference>()))>;
    static_assert(OptionalLike<Maybe>);
    using Value = Maybe::value_type;
    Option<Value> found{};
    auto wrapped = adapt(std::move(function));
    visitProjected([&](auto &&item) -> bool {
      Maybe result = std::invoke(wrapped, std::forward<decltype(item)>(item));
      if (result.has_value()) {
        found.emplace(std::move(*result));
        return false;
      }
      return true;
    });
    return found;
  }

  /// Returns the zero-based index of the first item accepted by the predicate.
  template <class Predicate>
  [[nodiscard]] constexpr auto position(Predicate predicate) -> Option<usize> {
    usize index{};
    Option<usize> found{};
    auto wrapped = adapt(std::move(predicate));
    visitProjected([&](auto &&item) -> bool {
      if (static_cast<bool>(std::invoke(wrapped, item))) {
        found = index;
        return false;
      }
      ++index;
      return true;
    });
    return found;
  }

  /// Searches from the back and returns the zero-based index of the first reverse match. Exact size is
  /// required so the original forward index can be reported without a preliminary traversal.
  template <class Predicate>
    requires std::ranges::bidirectional_range<Iter> and std::ranges::common_range<Iter> and
             std::ranges::sized_range<Iter>
  [[nodiscard]] constexpr auto rposition(Predicate predicate) -> Option<usize> {
    auto current = end();
    const auto first = begin();
    auto index = static_cast<usize>(std::ranges::size(*this));
    auto wrapped = adapt(std::move(predicate));
    while (current != first) {
      --current;
      --index;
      if (static_cast<bool>(std::invoke(wrapped, *current))) {
        return index;
      }
    }
    return None;
  }

  /// Returns true as soon as one item satisfies the predicate. Pending projections are fused into the
  /// traversal rather than materialized as transform views.
  template <class Predicate>
  [[nodiscard]] constexpr auto any(Predicate predicate) -> bool {
    auto wrapped = adapt(std::move(predicate));
    if constexpr (isProjectedFilterView<View>) {
      bool result{};
      visitProjected([&](auto &&item) -> bool {
        if (static_cast<bool>(std::invoke(wrapped, item))) {
          result = true;
          return false;
        }
        return true;
      });
      return result;
    } else {
      return std::ranges::any_of(view_, std::move(wrapped), projection_.get());
    }
  }

  /// Returns true when every item satisfies the predicate, short-circuiting on the first failure.
  template <class Predicate>
  [[nodiscard]] constexpr auto all(Predicate predicate) -> bool {
    auto wrapped = adapt(std::move(predicate));
    if constexpr (isProjectedFilterView<View>) {
      bool result{true};
      visitProjected([&](auto &&item) -> bool {
        if (not static_cast<bool>(std::invoke(wrapped, item))) {
          result = false;
          return false;
        }
        return true;
      });
      return result;
    } else {
      return std::ranges::all_of(view_, std::move(wrapped), projection_.get());
    }
  }

  /// Consumes the pipeline and counts observed items. Traversal is intentional so pending projections and
  /// side-effecting lazy adaptors such as `inspect()` are evaluated exactly as they are for other terminals.
  [[nodiscard]] constexpr auto count() -> usize {
    usize result{};
    visitProjected([&](auto &&) -> bool {
      ++result;
      return true;
    });
    return result;
  }

  /// Returns the item at `target` if present. The implementation remains single-pass for input ranges and
  /// preserves reference identity for borrowed sources.
  [[nodiscard]] constexpr auto nth(usize target) {
    using Result = TerminalOptionT<Reference>;
    Result found{};
    usize index{};
    visitProjected([&](auto &&item) -> bool {
      if (index == target) {
        found = makeTerminalValue(std::forward<decltype(item)>(item));
        return false;
      }
      ++index;
      return true;
    });
    return found;
  }

  /// Returns the item `target` positions from the back of a bidirectional common range.
  [[nodiscard]] constexpr auto nthBack(usize target)
    requires std::ranges::bidirectional_range<Iter> and std::ranges::common_range<Iter>
  {
    using Result = TerminalOptionT<Reference>;
    auto current = end();
    const auto first = begin();
    usize index{};
    while (current != first) {
      --current;
      if (index == target) {
        using Item = decltype(*current);
        if constexpr (std::is_reference_v<Item>) {
          auto &&item = *current;
          return Result{makeTerminalValue(std::forward<Item>(item))};
        } else {
          auto item = *current;
          return Result{makeTerminalValue(std::move(item))};
        }
      }
      ++index;
    }
    return Result{};
  }

  /// Returns the final item if present, preserving the terminal reference/value category.
  [[nodiscard]] constexpr auto last() {
    using Result = TerminalOptionT<Reference>;
    Result found{};
    visitProjected([&](auto &&item) -> bool {
      found = makeTerminalValue(std::forward<decltype(item)>(item));
      return true;
    });
    return found;
  }

  /// Returns the minimum item according to `compare`.
  template <class Compare = std::ranges::less>
  [[nodiscard]] constexpr auto min(Compare compare = {}) {
    return extremum(std::move(compare), false);
  }

  /// Returns the maximum item according to `compare`.
  template <class Compare = std::ranges::less>
  [[nodiscard]] constexpr auto max(Compare compare = {}) {
    return extremum(std::move(compare), true);
  }

  /// Comparator-named spelling of `min` for Rust-style API symmetry.
  template <class Compare>
  [[nodiscard]] constexpr auto minBy(Compare compare) {
    return extremum(std::move(compare), false);
  }

  /// Comparator-named spelling of `max` for Rust-style API symmetry.
  template <class Compare>
  [[nodiscard]] constexpr auto maxBy(Compare compare) {
    return extremum(std::move(compare), true);
  }

  /// Returns the item whose adaptively projected key is smallest.
  template <class KeyProjection>
  [[nodiscard]] constexpr auto minByKey(KeyProjection projection) {
    return extremumByKey(std::move(projection), false);
  }

  /// Returns the item whose adaptively projected key is largest.
  template <class KeyProjection>
  [[nodiscard]] constexpr auto maxByKey(KeyProjection projection) {
    return extremumByKey(std::move(projection), true);
  }

  /// Adds all projected items, using the value type's `+` semantics.
  [[nodiscard]] constexpr auto sum() {
    if constexpr (isProjectedFilterView<View>) {
      Value result{};
      visitProjected([&](auto &&item) -> bool {
        result += std::forward<decltype(item)>(item);
        return true;
      });
      return result;
    } else {
      return std::ranges::fold_left(*this, Value{}, std::plus{});
    }
  }

  /// Multiplies all projected items, using the value type's `*` semantics.
  [[nodiscard]] constexpr auto product() {
    if constexpr (isProjectedFilterView<View>) {
      Value result{1};
      visitProjected([&](auto &&item) -> bool {
        result *= std::forward<decltype(item)>(item);
        return true;
      });
      return result;
    } else {
      return std::ranges::fold_left(*this, Value{1}, std::multiplies{});
    }
  }

  /// Left-folds the pipeline from an explicit initial accumulator. The item may be decomposed by Miracle
  /// adaptive invocation while the accumulator stays intact.
  template <class Accumulator, class Function>
  [[nodiscard]] constexpr auto fold(Accumulator initial, Function function) -> Accumulator {
    auto wrapped = adaptWithPrefix(std::move(function));
    if constexpr (isProjectedFilterView<View>) {
      visitProjected([&](auto &&item) -> bool {
        initial = std::invoke(wrapped, std::move(initial), std::forward<decltype(item)>(item));
        return true;
      });
      return initial;
    } else {
      return std::ranges::fold_left(*this, std::move(initial), std::move(wrapped));
    }
  }

  /// Left-folds using the first item as the initial accumulator and returns an empty Option for an empty
  /// source. Delegates to `std::ranges::fold_left_first` after lowering the projection.
  template <class Function>
  [[nodiscard]] constexpr auto reduce(Function function) {
    auto wrapped = adaptWithPrefix(std::move(function));
    if constexpr (isProjectedFilterView<View>) {
      Option<Value> result{};
      visitProjected([&](auto &&item) -> bool {
        if (result.has_value()) {
          *result = std::invoke(wrapped, std::move(*result), std::forward<decltype(item)>(item));
        } else {
          result.emplace(std::forward<decltype(item)>(item));
        }
        return true;
      });
      return result;
    } else {
      return std::ranges::fold_left_first(*this, std::move(wrapped));
    }
  }

  /// Right-folds a bidirectional common range from its final item toward its first item.
  template <class Accumulator, class Function>
  [[nodiscard]] constexpr auto rFold(Accumulator initial, Function function) -> Accumulator
    requires std::ranges::bidirectional_range<Iter> and std::ranges::common_range<Iter>
  {
    auto wrapped = adaptWithPrefix(std::move(function));
    auto current = end();
    const auto first = begin();
    while (current != first) {
      --current;
      initial = std::invoke(wrapped, std::move(initial), *current);
    }
    return initial;
  }

  /// Materializes matching and non-matching values into two vectors in encounter order.
  template <class Predicate>
  [[nodiscard]] constexpr auto partition(Predicate predicate) {
    Pair<Vec<Value>, Vec<Value>> result;
    auto wrapped = adapt(std::move(predicate));
    visitProjected([&](auto &&item) -> bool {
      Vec<Value> &destination = static_cast<bool>(std::invoke(wrapped, item)) ? result.first : result.second;
      destination.emplace_back(std::forward<decltype(item)>(item));
      return true;
    });
    return result;
  }

  /// Splits a pair/tuple-of-two pipeline into two vectors, moving from owned rvalue sources.
  [[nodiscard]] constexpr auto unzip()
    requires HasTupleProtocol<Reference> and (std::tuple_size_v<std::remove_cvref_t<Reference>> == 2)
  {
    using Item = std::remove_cvref_t<Reference>;
    using First = std::remove_cvref_t<std::tuple_element_t<0, Item>>;
    using Second = std::remove_cvref_t<std::tuple_element_t<1, Item>>;
    Pair<Vec<First>, Vec<Second>> result;
    visitProjected([&](auto &&item) -> bool {
      result.first.emplace_back(std::get<0>(std::forward<decltype(item)>(item)));
      result.second.emplace_back(std::get<1>(std::forward<decltype(item)>(item)));
      return true;
    });
    return result;
  }

  /// Materializes the pipeline into `Container`. Reservable push-back containers reserve exact capacity when
  /// available; projected-filter pipelines use the fused one-pass traversal.
  template <class Container>
  [[nodiscard]] constexpr auto collect() -> Container {
    if constexpr (PushBackContainer<Container, Reference>) {
      Container result;
      if constexpr (ReservableContainer<Container> and std::ranges::sized_range<View>) {
        result.reserve(static_cast<usize>(std::ranges::size(view_)));
      }
      visitProjected([&](auto &&item) -> bool {
        result.push_back(std::forward<decltype(item)>(item));
        return true;
      });
      return result;
    } else {
      return std::ranges::to<Container>(*this);
    }
  }

  /// Convenience materializer equivalent to `collect<Vec<Value>>()`.
  [[nodiscard]] constexpr auto toVec() {
    Vec<Value> result;
    if constexpr (std::ranges::sized_range<View>) {
      result.reserve(static_cast<usize>(std::ranges::size(view_)));
    }
    visitProjected([&](auto &&item) -> bool {
      result.emplace_back(std::forward<decltype(item)>(item));
      return true;
    });
    return result;
  }

  /// Invokes `function` once for each item. Projection and filter fusion avoid redundant mapping.
  template <class Function>
  constexpr auto forEach(Function function) -> void {
    auto wrapped = adapt(std::move(function));
    if constexpr (isProjectedFilterView<View>) {
      visitProjected([&](auto &&item) -> bool {
        std::invoke(wrapped, std::forward<decltype(item)>(item));
        return true;
      });
    } else {
      std::ranges::for_each(view_, std::move(wrapped), projection_.get());
    }
  }

  /// Lexicographically tests sequence equality against another range.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto eq(Other &&other) -> bool {
    return std::ranges::equal(*this, std::forward<Other>(other));
  }

  /// Sequence equality with a caller-supplied heterogeneous predicate. Traversal short-circuits on the first
  /// mismatch and requires both ranges to end together.
  template <std::ranges::input_range Other, class Predicate>
  [[nodiscard]] constexpr auto eqBy(Other &&other, Predicate predicate) -> bool {
    return std::ranges::equal(*this, std::forward<Other>(other), std::move(predicate));
  }

  /// Returns the logical negation of sequence equality.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto ne(Other &&other) -> bool {
    return not eq(std::forward<Other>(other));
  }

  /// Lexicographically three-way compares this sequence with `other`. Common ranges delegate directly to the
  /// standard algorithm; non-common input ranges use the identical sentinal-aware traversal.
  template <std::ranges::input_range Other, class Compare = std::compare_three_way>
  [[nodiscard]] constexpr auto compare(Other &&other, Compare compare = {}) {
    auto rightRange = viewForRange(std::forward<Other>(other));
    using Compared = std::remove_cvref_t<
        std::invoke_result_t<Compare &, Reference, std::ranges::range_reference_t<decltype(rightRange)>>>;
    using Category = std::common_comparison_category_t<Compared, std::strong_ordering>;
    static_assert(not std::same_as<Category, void>);

    if constexpr (std::ranges::common_range<Iter> and std::ranges::common_range<decltype(rightRange)>) {
      return std::lexicographical_compare_three_way(
          begin(), end(), std::ranges::begin(rightRange), std::ranges::end(rightRange), std::move(compare));
    } else {
      auto left = begin();
      const auto leftEnd = end();
      auto right = std::ranges::begin(rightRange);
      const auto rightEnd = std::ranges::end(rightRange);
      while (left != leftEnd and right != rightEnd) {
        const Category result = std::invoke(compare, *left, *right);
        if (result != 0) {
          return result;
        }
        ++left;
        ++right;
      }
      if (left == leftEnd and right == rightEnd) {
        return Category::equivalent;
      }
      return left == leftEnd ? Category::less : Category::greater;
    }
  }

  /// Lexicographically tests whether this sequence is less than another range.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto lt(Other &&other) -> bool {
    return std::ranges::lexicographical_compare(*this, std::forward<Other>(other));
  }

  /// Lexicographically tests whether this sequence is less than or equal to another range.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto le(Other &&other) -> bool {
    return not std::ranges::lexicographical_compare(std::forward<Other>(other), *this);
  }

  /// Lexicographically tests whether this sequence is greater than another range.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto gt(Other &&other) -> bool {
    return std::ranges::lexicographical_compare(std::forward<Other>(other), *this);
  }

  /// Lexicographically tests whether this sequence is greater than or equal to another range.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto ge(Other &&other) -> bool {
    return not std::ranges::lexicographical_compare(*this, std::forward<Other>(other));
  }

  /// Tests whether all matching items precede all non-matching items.
  template <class Predicate>
  [[nodiscard]] constexpr auto isPartitioned(Predicate predicate) -> bool {
    return std::ranges::is_partitioned(*this, adapt(std::move(predicate)));
  }

  /// Tests whether the pipeline is ordered according to `compare`.
  template <class Compare = std::ranges::less>
  [[nodiscard]] constexpr auto isSorted(Compare compare = {}) -> bool {
    return std::ranges::is_sorted(*this, std::move(compare));
  }

  /// Comparator-named spelling of `isSorted` for API symmetry.
  template <class Compare>
  [[nodiscard]] constexpr auto isSortedBy(Compare compare) -> bool {
    return std::ranges::is_sorted(*this, std::move(compare));
  }

  /// Tests ordering after adaptively projecting a key from each item.
  template <class KeyProjection>
  [[nodiscard]] constexpr auto isSortedByKey(KeyProjection projection) -> bool {
    return std::ranges::is_sorted(*this, std::ranges::less{}, adapt(std::move(projection)));
  }

  /// Switches an rvalue indexable/common pipeline to the default standard parallel policy.
  [[nodiscard]] constexpr auto parallel() &&
    requires std::ranges::random_access_range<View> and std::ranges::sized_range<View> and
             std::ranges::common_range<View>;

  /// Switches an rvalue indexable/common pipeline to an explicit standard execution policy.
  template <class Policy>
  [[nodiscard]] constexpr auto parallel(Policy &&policy) &&
    requires StandardExecutionPolicy<Policy> and std::ranges::random_access_range<View> and
             std::ranges::sized_range<View> and std::ranges::common_range<View>;

private:
  /// Moves the stored view from rvalue pipelines and copies it from lvalue pipelines according to
  /// explicit-object forwarding. Adaptors use this instead of duplicating forwarding logic.
  template <class Self>
  [[nodiscard]] static constexpr auto takeView(Self &&self) -> View {
    return std::forward_like<Self>(std::forward<Self>(self).view_);
  }

  /// Expects the pending projection with the same value category as the Iter object.
  template <class Self>
  [[nodiscard]] static constexpr auto takeProjection(Self &&self) -> Projection {
    return std::forward_like<Self>(std::forward<Self>(self).projection_.get());
  }

  /// Materializes the pending projection into a standard transform view only when an operation can no longer
  /// profit from keeping it separately (for example join, scan, or arbitrary views).
  template <class Self>
  [[nodiscard]] static constexpr auto lower(Self &&self) {
    auto view = takeView(std::forward<Self>(self));
    if constexpr (isIdentityProjection<Projection>) {
      return view;
    } else {
      auto projection = takeProjection(std::forward<Self>(self));
      return std::views::transform(std::move(view), std::move(projection));
    }
  }

  /// Rewraps a positional standard view while preserving the pending projection unchanged.
  template <class Self, std::ranges::view NewView>
  [[nodiscard]] static constexpr auto rebindProjection(Self &&self, NewView view) {
    if constexpr (isIdentityProjection<Projection>) {
      return Iter<NewView>{std::move(view)};
    } else {
      auto projection = takeProjection(std::forward<Self>(self));
      return Iter<NewView, Projection>{std::move(view), std::move(projection)};
    }
  }

  /// Applies the pending projection exactly once and forwards the projected value to `visitor`.
  /// Prvalues are kept in a local so the visitor can consume them safely during the call.
  template <class Item, class Visitor>
  constexpr auto projectAndVisit(Item &&item, Visitor &visitor) -> bool {
    if constexpr (isIdentityProjection<Projection>) {
      return static_cast<bool>(std::invoke(visitor, std::forward<Item>(item)));
    } else {
      using Result = decltype(std::invoke(projection_.get(), std::forward<Item>(item)));
      if constexpr (std::is_reference_v<Result>) {
        auto &&projected = std::invoke(projection_.get(), std::forward<Item>(item));
        return static_cast<bool>(std::invoke(visitor, std::forward<Result>(projected)));
      } else {
        auto projected = std::invoke(projection_.get(), std::forward<Item>(item));
        return static_cast<bool>(std::invoke(visitor, std::move(projected)));
      }
    }
  }

  /// Central fused traversal for Miracle-owned terminals. ProjectedFilterView exposes its source traversal
  /// directly so mapping, filtering, and terminal consumption happen in one pass.
  template <class Visitor>
  constexpr auto visitProjected(Visitor &&visitor) -> bool {
    auto &&callable = std::forward<Visitor>(visitor);
    if constexpr (isProjectedFilterView<View>) {
      return view_.visitWhile(
          [&](auto &&item) -> bool { return projectAndVisit(std::forward<decltype(item)>(item), callable); });
    } else {
      for (auto &&item : view_) {
        if (not projectAndVisit(std::forward<decltype(item)>(item), callable)) {
          return false;
        }
      }
      return true;
    }
  }

  /// Shared single-pass min/max implementation that retains a reference for borrowed lvalue items and an
  /// owned value for consumable rvalues.
  template <class Compare>
  [[nodiscard]] constexpr auto extremum(Compare compare, bool maximum) {
    using Result = TerminalOptionT<Reference>;
    Result best{};
    auto valueOf = [](auto &option) -> decltype(auto) { return *option; };
    visitProjected([&](auto &&item) -> bool {
      if (not best.has_value()) {
        best = makeTerminalValue(std::forward<decltype(item)>(item));
        return true;
      }
      // Maximum selection deliberately replaces on equivalence so max/maxBy match Rust's last-equal tie rule.
      // Minimum selection remains strictly less and therefore keeps the first equal minimum.
      const bool replace =
          maximum ? not std::invoke(compare, item, valueOf(best)) : std::invoke(compare, item, valueOf(best));
      if (replace) {
        best = makeTerminalValue(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return best;
  }

  /// Key-based min/max helper. Keys are transient; only the selected original item is retained.
  template <class KeyProjection>
  [[nodiscard]] constexpr auto extremumByKey(KeyProjection projection, bool maximum) {
    using Result = TerminalOptionT<Reference>;
    using Key = std::remove_cvref_t<decltype(adaptiveInvoke(
        std::declval<KeyProjection &>(), std::declval<Reference>()))>;
    Result best{};
    Option<Key> bestKey{};
    auto key = adapt(std::move(projection));
    visitProjected([&](auto &&item) -> bool {
      Key itemKey = std::invoke(key, item);
      if (not best.has_value()) {
        bestKey.emplace(std::move(itemKey));
        best = makeTerminalValue(std::forward<decltype(item)>(item));
        return true;
      }
      // As above, maximum keeps the last equivalent key while minimum keeps the first.
      const bool replace =
          maximum ? not std::ranges::less{}(itemKey, *bestKey) : std::ranges::less{}(itemKey, *bestKey);
      if (replace) {
        bestKey.emplace(std::move(itemKey));
        best = makeTerminalValue(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return best;
  }

  View view_{};
  [[no_unique_address]] MovableBox<Projection> projection_;

  template <class Iteration, class Policy>
  friend class ParallelIter;
};

template <class View>
Iter(View) -> Iter<View>;

/// Explicit standard execution-policy façade for indexable Iter pipelines.
/// Scheduling, worker lifetime, exception semantics, and nesting are owned by the standard execution policy
/// implementation rather than Miracle.
template <class Iteration, class Policy>
class ParallelIter final {
  using Projection = std::remove_cvref_t<decltype(std::declval<Iteration &>().projection_.get())>;

public:
  /// Stores the already-built Iter pipeline; the execution policy remains a compile-time type and therefore
  /// adds no per-object policy storage.
  constexpr explicit ParallelIter(Iteration iteration)
      : iteration_(std::move(iteration)) {
  }

  /// Adds another lazy projection before the eventual policy-backed terminal.
  template <class Self, class Function>
  [[nodiscard]] constexpr auto map(this Self &&self, Function function) {
    auto mapped = std::forward_like<Self>(std::forward<Self>(self).iteration_).map(std::move(function));
    using Result = decltype(mapped);
    return ParallelIter<Result, Policy>{std::move(mapped)};
  }

  /// Policy-backed short-circuit existential test using the pending projection as the algorithm projection
  /// argument.
  template <class Predicate>
  [[nodiscard]] auto any(Predicate predicate) -> bool {
    return std::ranges::any_of(
        policy(), iteration_.view_, adapt(std::move(predicate)), iteration_.projection_.get());
  }

  /// Policy-backed universal predicate test using the pending projection directly.
  template <class Predicate>
  [[nodiscard]] auto all(Predicate predicate) -> bool {
    return std::ranges::all_of(
        policy(), iteration_.view_, adapt(std::move(predicate)), iteration_.projection_.get());
  }

  /// Applies `function` under the selected standard execution policy. Invocation order follows the semantics
  /// of that policy rather than source order.
  template <class Function>
  auto forEach(Function function) -> void {
    std::ranges::for_each(
        policy(), iteration_.view_, adapt(std::move(function)), iteration_.projection_.get());
  }

  /// Materializes an indexable pipeline in source order with `std::transform(policy, ...)`.
  [[nodiscard]] auto toVec()
    requires std::default_initializable<std::ranges::range_value_t<Iteration>> and
             std::copy_constructible<Projection>
  {
    using Value = std::ranges::range_value_t<Iteration>;
    const auto count = static_cast<usize>(std::ranges::size(iteration_.view_));
    Vec<Value> result(count);
    auto projection = iteration_.projection_;
    std::transform(policy(),
        std::ranges::begin(iteration_.view_),
        std::ranges::end(iteration_.view_),
        result.begin(),
        [projection = std::move(projection)](auto &&item) mutable -> Value {
          return Value{std::invoke(projection.get(), std::forward<decltype(item)>(item))};
        });
    return result;
  }

private:
  /// Rehydrates the stateless policy object from its stored policy type.
  [[nodiscard]] static constexpr auto policy() -> const Policy & {
    if constexpr (std::same_as<Policy, std::execution::sequenced_policy>) {
      return std::execution::seq;
    } else if constexpr (std::same_as<Policy, std::execution::parallel_policy>) {
      return std::execution::par;
    } else if constexpr (std::same_as<Policy, std::execution::parallel_unsequenced_policy>) {
      return std::execution::par_unseq;
    } else {
      static_assert(std::same_as<Policy, std::execution::unsequenced_policy>);
      return std::execution::unseq;
    }
  }

  Iteration iteration_;
};

/// Uses `std::execution::par` for a random-access, sized, common rvalue pipeline.
template <std::ranges::view View, class Projection>
[[nodiscard]] constexpr auto Iter<View, Projection>::parallel() &&
  requires std::ranges::random_access_range<View> and std::ranges::sized_range<View> and
           std::ranges::common_range<View>
{
  return ParallelIter<Iter, std::execution::parallel_policy>{std::move(*this)};
}

/// Uses the supplied standard execution policy. Policy objects are stateless vocabulary types; ParallelIter
/// stores only the pipeline and recovers the corresponding policy singleton by type.
template <std::ranges::view View, class Projection>
template <class Policy>
[[nodiscard]] constexpr auto Iter<View, Projection>::parallel([[maybe_unused]] Policy &&policy) &&
  requires StandardExecutionPolicy<Policy> and std::ranges::random_access_range<View> and
           std::ranges::sized_range<View> and std::ranges::common_range<View>
{
  using StoredPolicy = std::remove_cvref_t<Policy>;
  return ParallelIter<Iter, StoredPolicy>{std::move(*this)};
}

/// Creates an Iter from any viewable range. Lvalues remain borrowed where the standard view does; owned
/// non-borrowed rvalues are consumed through `as_rvalue`.
template <std::ranges::viewable_range RangeType>
  requires(not OptionalLike<std::remove_cvref_t<RangeType>>)
[[nodiscard]] constexpr auto iter(RangeType &&range) {
  auto view = viewForRange(std::forward<RangeType>(range));
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates the half-open integer pipeline `[0, stop)`.
template <std::integral T>
  requires(not std::same_as<std::remove_cv_t<T>, bool>)
[[nodiscard]] constexpr auto iter(T stop) {
  return iter(Range<T>{stop});
}

/// Creates the half-open integer pipeline `[start, stop)` using Range's lossless endpoint rules.
template <std::integral Left, std::integral Right>
  requires CompatibleRangeEndpoints<Left, Right>
[[nodiscard]] constexpr auto iter(Left start, Right stop) {
  return iter(Range{start, stop});
}

/// Creates a positively-strided half-open integer pipeline. The step participates in common-type deduction
/// and must have compatible signedness with the endpoints.
template <std::integral Left, std::integral Right, std::integral Step>
  requires CompatibleRangeEndpoints<Left, Right> and (not std::same_as<std::remove_cv_t<Step>, bool>) and
           (std::is_signed_v<Left> == std::is_signed_v<Step>)
[[nodiscard]] constexpr auto iter(Left start, Right stop, Step step) pre(step > 0) {
  if (step <= 0) {
    std::terminate();
  }
  using Common = std::common_type_t<Left, Right, Step>;
  auto base = Range<Common>{Common{start}, Common{stop}} | std::views::stride(Common{step});
  return Iter<decltype(base)>{std::move(base)};
}

/// Views an lvalue Optional/Expected-like object as zero or one borrowed item.
template <OptionalLike Maybe>
[[nodiscard]] constexpr auto iter(Maybe &maybe) {
  using View = MaybeRefView<Maybe>;
  return Iter<View>{View{maybe}};
}

/// Owns a rvalue Optional/Expected-like object as zero or one consumable item.
template <OptionalLike Maybe>
  requires(not std::is_lvalue_reference_v<Maybe>)
[[nodiscard]] constexpr auto iter(Maybe &&maybe) {
  using Stored = std::remove_cvref_t<Maybe>;
  auto view = MaybeValueView<Stored>{std::forward<Maybe>(maybe)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates an empty random-access, sized, borrowed Iter source of `T`.
template <class T>
[[nodiscard]] constexpr auto empty() {
  return Iter{std::views::empty<T>};
}

/// Creates a one-element source. The stored value is exposed as an rvalue so move-only values can be
/// materialized without a copy.
template <class T>
[[nodiscard]] constexpr auto once(T value) {
  auto view = std::views::single(std::move(value)) | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Lazily invokes `function` once when traversal begins and yields its cached result exactly once.
template <class Function>
[[nodiscard]] constexpr auto onceWith(Function function) {
  using Stored = std::decay_t<Function>;
  auto view = OnceWith<Stored>{std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates an endless standard repeat view. Each position references the one stored value, following C++
/// `repeat_view` semantics rather than cloning a new value on every dereference.
template <class T>
[[nodiscard]] constexpr auto repeat(T value) {
  auto view = std::views::repeat(std::move(value));
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates a bounded standard repeat view with exactly `count` positions.
template <class T>
[[nodiscard]] constexpr auto repeatN(T value, usize count) {
  auto view = std::views::repeat(std::move(value), count);
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates an endless callable source that invokes `function` once per increment and caches the result for
/// repeated dereference of that logical item.
template <class Function>
[[nodiscard]] constexpr auto repeatWith(Function function) {
  using Stored = std::decay_t<Function>;
  auto view = RepeatWith<Stored>{std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates a single-pass source from `function() -> Option<T>`. The first empty result terminates traversal.
template <class Function>
[[nodiscard]] constexpr auto fromFn(Function function) {
  using Stored = std::decay_t<Function>;
  auto view = FromFn<Stored>{std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates a single-pass sequence beginning with `seed` and repeatedly invoking `function(const T&) ->
/// Option<T>`. Each successor is computed before its predecessor becomes consumable.
template <class T, class Function>
  requires(not OptionalLike<std::remove_cvref_t<T>>)
[[nodiscard]] constexpr auto successors(T seed, Function function) {
  using Value = std::decay_t<T>;
  using Stored = std::decay_t<Function>;
  auto view = Successors<Value, Stored>{std::move(seed), std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Rust-parity successor source whose optional first value may make the sequence initially exhausted.
template <OptionalLike Maybe, class Function>
[[nodiscard]] constexpr auto successors(Maybe first, Function function) {
  using Value = std::remove_cvref_t<typename std::remove_cvref_t<Maybe>::value_type>;
  using Stored = std::decay_t<Function>;
  Option<Value> seed{};
  if (first.has_value()) {
    seed.emplace(std::move(*first));
  }
  auto view = Successors<Value, Stored>{std::move(seed), std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Free-function spelling equivalent to `iter(first).chain(second)`.
template <std::ranges::viewable_range First, std::ranges::viewable_range Second>
[[nodiscard]] constexpr auto chain(First &&first, Second &&second) {
  return iter(std::forward<First>(first)).chain(std::forward<Second>(second));
}

/// Free-function spelling equivalent to `iter(first).zip(second)`.
template <std::ranges::viewable_range First, std::ranges::viewable_range Second>
[[nodiscard]] constexpr auto zip(First &&first, Second &&second) {
  return iter(std::forward<First>(first)).zip(std::forward<Second>(second));
}

} // namespace Miracle

// Standard customization-point spelling is fixed by the C++ ranges API.
// NOLINTBEGIN(readability-identifier-naming)
export template <class Maybe>
inline constexpr bool std::ranges::enable_borrowed_range<Miracle::MaybeRefView<Maybe>> = true;

export template <std::ranges::view View, class Projection>
inline constexpr bool std::ranges::enable_borrowed_range<Miracle::Iter<View, Projection>> =
    std::same_as<Projection, std::identity> and std::ranges::borrowed_range<View>;
// NOLINTEND(readability-identifier-naming)
