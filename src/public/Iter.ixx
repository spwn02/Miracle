export module Miracle:Iter;

import std;
import :Types;
import :Range;

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
          "or its reflected aggregate memebers");
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
    requires std::copy_constructible<T>
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
      std::is_nothrow_move_constructible_v<T> and std::is_nothrow_move_assignable_v<T>) -> MovableBox & {
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
class MaybeRefView final : public std::ranges::view_interface<MaybeRefView<Maybe>> {
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
class MaybeValueView final : public std::ranges::view_interface<MaybeValueView<Maybe>> {
public:
  constexpr MaybeValueView()
    requires std::default_initializable<Maybe>
  = default;
  constexpr explicit MaybeValueView(Maybe maybe)
      : maybe_(std::move(maybe)) {
  }

  [[nodiscard]] constexpr auto begin() const {
    using Value = Maybe::value_type;
    return maybe_.has_value() ? std::addressof(*maybe_) : static_cast<Value *>(nullptr);
  }

  [[nodiscard]] constexpr auto end() const {
    using Value = Maybe::value_type;
    return maybe_.has_value() ? std::addressof(*maybe_) + 1 : static_cast<Value *>(nullptr);
  }

  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return maybe_.has_value() ? 1U : 0U;
  }

private:
  Maybe maybe_{};
};

/// Fused map->filter representation. Ordinary iteration preserves standard transform/filter semantics, while
/// `visitWhile()` lets Miracle terminals evaluate the projection exactly once per source item and reuse the
/// projected value for both filtering and consumption.
template <std::ranges::view View, class Projection, class Predicate>
class ProjectedFilterView final
    : public std::ranges::view_interface<ProjectedFilterView<View, Projection, Predicate>> {
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

  [[nodiscard]] constexpr auto end() -> Iterator {
    if constexpr (std::ranges::common_range<View>) {
      return Iterator{this, std::ranges::end(view_)};
    } else {
      return std::ranges::end(view_);
    }
  }

  template <class Visitor>
  constexpr auto visitWhile(Visitor visitor) -> bool {
    for (auto iterator = std::ranges::begin(view_), end = std::ranges::end(view_); iterator != end;
        ++iterator) {
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
class ScanView final : public std::ranges::view_interface<ScanView<View, State, Function>> {
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
    constexpr Iterator(ScanView *parent, std::ranges::iterator_t<View> current)
        : parent_(parent)
        , current_(std::move(current)) {
      load();
    }

    [[nodiscard]] constexpr auto operator*() const -> const value_type & {
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

    ScanView *parent_{};
    std::ranges::iterator_t<View> current_{};
    Option<value_type> cache_{};
    bool done_{};
  };

public:
  ScanView()
    requires std::default_initializable<View> and std::default_initializable<State> and
                 std::default_initializable<Function>
  = default;

  constexpr ScanView(View view, State state, Function function)
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

/// Forward/input intersperse implementation for sources that cannot provide indexed access. It alternates
/// source values and the stored separator without materializing the sequence.
template <std::ranges::view View>
class IntersperseView final : public std::ranges::view_interface<IntersperseView<View>> {
  using Value = std::ranges::range_value_t<View>;
  using SourceReference = std::ranges::range_reference_t<View>;
  using CommonReference = std::common_reference_t<SourceReference, const Value &>;

  class Iterator final {
  public:
    using iterator_concept = std::input_iterator_tag;
    using value_type = Value;
    using difference_type = std::ranges::range_difference_t<View>;

    Iterator() = default;
    constexpr Iterator(IntersperseView *parent, std::ranges::iterator_t<View> current)
        : parent_(parent)
        , current_(std::move(current))
        , done_(current_ == std::ranges::end(parent_->view_)) {
    }

    [[nodiscard]] constexpr auto operator*() const -> CommonReference {
      return separatorNext_ ? CommonReference{parent_->separator_} : CommonReference{*current_};
    }

    constexpr auto operator++() -> Iterator & {
      if (separatorNext_) {
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
      ++*this;
    }

    [[nodiscard]] friend constexpr auto operator==(const Iterator &iterator,
        [[maybe_unused]] std::default_sentinel_t sentinel) -> bool {
      return iterator.done_;
    }

  private:
    IntersperseView *parent_{};
    std::ranges::iterator_t<View> current_{};
    bool separatorNext_{};
    bool done_{true};
  };

public:
  IntersperseView()
    requires std::default_initializable<View> and std::default_initializable<Value>
  = default;
  constexpr IntersperseView(View view, Value separator)
      : view_(std::move(view))
      , separator_(std::move(separator)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this, std::ranges::begin(view_)};
  }

  [[nodiscard]] constexpr auto end() const noexcept -> std::default_sentinel_t {
    return {};
  }

private:
  View view_{};
  Value separator_{};
};

/// Random-access intersperse specialization. Logical positions map directly to either a source element or the
/// separator, preserving random-access and sized-range capabilities.
template <std::ranges::view View>
  requires std::ranges::random_access_range<View> and std::ranges::sized_range<View>
class RandomAccessIntersperseView final
    : public std::ranges::view_interface<RandomAccessIntersperseView<View>> {
  using Value = std::ranges::range_value_t<View>;
  using SourceReference = std::ranges::range_reference_t<View>;
  static constexpr bool stableReference_ = std::is_lvalue_reference_v<SourceReference>;
  using StableReference = std::common_reference_t<SourceReference, const Value &>;

  class Iterator final {
  public:
    using iterator_concept = std::random_access_iterator_tag;
    using value_type = Value;
    using difference_type = std::ranges::range_difference_t<View>;
    using reference = std::conditional_t<stableReference_, StableReference, Value>;

    Iterator() = default;
    constexpr Iterator(RandomAccessIntersperseView *parent, difference_type position)
        : parent_(parent)
        , position_(position) {
    }

    [[nodiscard]] constexpr auto operator*() const -> reference {
      if (position_ % 2 != 0) {
        return parent_->separator_;
      }
      auto &&item = std::ranges::begin(parent_->view_)[position_ / 2];
      if constexpr (stableReference_) {
        return item;
      } else {
        return Value{std::forward<decltype(item)>(item)};
      }
    }

    [[nodiscard]] constexpr auto operator[](difference_type offset) const -> reference {
      return *(*this + offset);
    }

    constexpr auto operator++() -> Iterator & {
      ++position_;
      return *this;
    }
    constexpr auto operator++(int) -> Iterator {
      auto previous = *this;
      ++*this;
      return previous;
    }
    constexpr auto operator--() -> Iterator & {
      --position_;
      return *this;
    }
    constexpr auto operator--(int) -> Iterator {
      auto previous = *this;
      --*this;
      return previous;
    }
    constexpr auto operator+=(difference_type offset) -> Iterator & {
      position_ += offset;
      return *this;
    }
    constexpr auto operator-=(difference_type offset) -> Iterator & {
      position_ -= offset;
      return *this;
    }

    [[nodiscard]] friend constexpr auto operator+(Iterator iterator, difference_type offset) -> Iterator {
      iterator += offset;
      return iterator;
    }
    [[nodiscard]] friend constexpr auto operator+(difference_type offset, Iterator iterator) -> Iterator {
      return iterator + offset;
    }
    [[nodiscard]] friend constexpr auto operator-(Iterator iterator, difference_type offset) -> Iterator {
      iterator -= offset;
      return iterator;
    }
    [[nodiscard]] friend constexpr auto operator-(const Iterator &left, const Iterator &right)
        -> difference_type {
      return left.position_ - right.position_;
    }
    [[nodiscard]] friend constexpr auto operator==(const Iterator &, const Iterator &) -> bool = default;
    [[nodiscard]] friend constexpr auto operator<=>(const Iterator &left, const Iterator &right) {
      return left.position_ <=> right.position_;
    }

  private:
    RandomAccessIntersperseView *parent_{};
    difference_type position_{};
  };

public:
  RandomAccessIntersperseView()
    requires std::default_initializable<View> and std::default_initializable<Value>
  = default;
  constexpr RandomAccessIntersperseView(View view, Value separator)
      : view_(std::move(view))
      , separator_(std::move(separator)) {
  }

  [[nodiscard]] constexpr auto begin() -> Iterator {
    return Iterator{this, 0};
  }
  [[nodiscard]] constexpr auto end() -> Iterator {
    return Iterator{this, static_cast<std::ranges::range_difference_t<View>>(size())};
  }

  [[nodiscard]] constexpr auto size() const -> usize {
    const auto count = static_cast<usize>(std::ranges::size(view_));
    return count == 0 ? 0 : (count * 2) - 1;
  }

private:
  View view_{};
  Value separator_{};
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

/// Primary lazy iteration façade. `Projection` is normally private implementation state carried by `map()`;
/// users construct Iter pipelines through `iter(...)`.
template <std::ranges::view View, class Projection = std::identity>
class Iter;

/// Standard-execution-policy terminal façade produced by `Iter::parallel(...)`.
template <class Iteration, class Policy>
class ParallelIter;

} // namespace Miracle

namespace Miracle {

// Maps a terminal's reference category to the correct Option payload: lvalues remain references through
// `Ref`, while xvalues/prvalues become owned values.
template <class Result>
struct TerminalOption;

template <class T>
struct TerminalOption<T &> {
  using Type = Option<Ref<T>>;
};

template <class T>
struct TerminalOption<const T &> {
  using Type = Option<Ref<const T>>;
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
    return std::ref(result);
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

/// Lazy range façade that keeps at most one pending map projection.
/// Standard views own traversal semantics; Miracle adds adaptive invocation and fuses projection-aware
/// terminals where doing so avoids redundant work.
template <std::ranges::view View, class Projection>
class Iter final : public std::ranges::view_interface<Iter<View, Projection>> {
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
  /// item intead of the naïve transform/filter twice.
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
    auto values = std::views::transform(std::move(present),
        [](auto &&value) -> decltype(auto) { return *std::forward<decltype(value)>(value); });
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

  /// Compatibility adaptor: standard forward Iter pipelines are already safely peekable, so this returns the
  /// pipeline unchanged instaed of allocating or adding cache state.
  template <class Self>
  [[nodiscard]] constexpr auto peekable(this Self &&self) {
    return std::forward<Self>(self);
  }

  /// Observes the next item without advancing a forward pipeline. References remain references.
  [[nodiscard]] constexpr auto peek() &
    requires std::ranges::forward_range<View>
  {
    auto first = begin();
    if (first == end()) {
      using Result = decltype(*first);
      return emptyTerminalValue<Result>();
    }
    return makeTerminalValue(*first);
  }

  /// Compatibility adaptor. Standard range exhaustion is already stable, so no wrapper is needed.
  template <class Self>
  [[nodiscard]] constexpr auto fuse(this Self &&self) {
    return std::forward<Self>(self);
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
    using Scan = ScanView<decltype(base), std::decay_t<State>, std::decay_t<Function>>;
    return Iter<Scan>{Scan{std::move(base), std::move(state), std::move(function)}};
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
    auto values = std::views::transform(std::move(prefix),
        [](auto &&value) -> decltype(auto) { return *std::forward<decltype(value)>(value); });
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

  /// Inserts `separator` between adjacent values while preserving random access when the source supports it.
  template <class Self>
  [[nodiscard]] constexpr auto intersperse(this Self &&self, Value separator) {
    auto base = lower(std::forward<Self>(self));
    if constexpr (std::ranges::random_access_range<decltype(base)> and
                  std::ranges::sized_range<decltype(base)>) {
      using Result = RandomAccessIntersperseView<decltype(base)>;
      return Iter<Result>{Result{std::move(base), std::move(separator)}};
    } else {
      using Result = IntersperseView<decltype(base)>;
      return Iter<Result>{Result{std::move(base), std::move(separator)}};
    }
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

  /// Returns the zero-based index of the last matching item. Requires bidirectional traversal.
  template <class Predicate>
  [[nodiscard]] constexpr auto rposition(Predicate predicate) -> Option<usize> {
    usize index{};
    Option<usize> found{};
    auto wrapped = adapt(std::move(predicate));
    visitProjected([&](auto &&item) -> bool {
      if (static_cast<bool>(std::invoke(wrapped, item))) {
        found = index;
      }
      ++index;
      return true;
    });
    return found;
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

  /// Counts remaining items. Sized ranges use their standard distance/size machinery.
  [[nodiscard]] constexpr auto count() -> usize {
    return static_cast<usize>(std::ranges::distance(view_));
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
      result.first.emplace_back(std::get<0>(item));
      result.second.emplace_back(std::get<1>(item));
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

  /// Returns the logical negation of sequence equality.
  template <std::ranges::input_range Other>
  [[nodiscard]] constexpr auto ne(Other &&other) -> bool {
    return not eq(std::forward<Other>(other));
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
    requires std::is_execution_policy_v<std::remove_cvref_t<Policy>> and
             std::ranges::random_access_range<View> and std::ranges::sized_range<View> and
             std::ranges::common_range<View>;

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
    auto valueOf = [](auto &option) -> decltype(auto) {
      if constexpr (requires { option->get(); }) {
        return option->get();
      } else {
        return *option;
      }
    };
    visitProjected([&](auto &&item) -> bool {
      if (not best.has_value()) {
        best = makeTerminalValue(std::forward<decltype(item)>(item));
        return true;
      }
      const bool replace =
          maximum ? std::invoke(compare, valueOf(best), item) : std::invoke(compare, item, valueOf(best));
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
    Result best{};
    auto key = adapt(std::move(projection));
    auto valueOf = [](auto &option) -> decltype(auto) {
      if constexpr (requires { option->get(); }) {
        return option->get();
      } else {
        return *option;
      }
    };
    visitProjected([&](auto &&item) -> bool {
      if (not best.has_value()) {
        best = makeTerminalValue(std::forward<decltype(item)>(item));
        return true;
      }
      auto &&bestValue = valueOf(best);
      const auto bestKey = std::invoke(key, bestValue);
      const auto itemKey = std::invoke(key, item);
      const bool replace =
          maximum ? std::ranges::less{}(bestKey, itemKey) : std::ranges::less{}(itemKey, bestKey);
      if (replace) {
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
    requires std::default_initializable<std::ranges::range_value_t<Iteration>>
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
  requires std::is_execution_policy_v<std::remove_cvref_t<Policy>> and
           std::ranges::random_access_range<View> and std::ranges::sized_range<View> and
           std::ranges::common_range<View>
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
  auto base = std::views::iota(
                  std::common_type_t<Left, Right, Step>{start}, std::common_type_t<Left, Right, Step>{stop}) |
              std::views::stride(std::common_type_t<Left, Right, Step>{step});
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

} // namespace Miracle

// Standard customization-point spelling is fixed by the C++ ranges API.
// NOLINTBEGIN(readability-identifier-naming)
export template <class Maybe>
inline constexpr bool std::ranges::enable_borrowed_range<Miracle::MaybeRefView<Maybe>> = true;

export template <std::ranges::view View, class Projection>
inline constexpr bool std::ranges::enable_borrowed_range<Miracle::Iter<View, Projection>> =
    std::same_as<Projection, std::identity> and std::ranges::borrowed_range<View>;
// NOLINTEND(readability-identifier-naming)
