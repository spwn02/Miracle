export module Miracle.Iter:Views;

import std;
import Miracle.Numeric;
import Miracle.Types;
import Miracle.Range;
import :Detail;

namespace Miracle {

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
