export module Miracle.Iter:SourceViews;

import std;
import Miracle.Numeric;
import Miracle.Types;
import Miracle.Range;
import :Detail;

namespace Miracle {

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

} // namespace Miracle

// Standard customization-point spelling is fixed by the C++ ranges API.
// NOLINTBEGIN(readability-identifier-naming)
template <class Maybe>
inline constexpr bool std::ranges::enable_borrowed_range<Miracle::MaybeRefView<Maybe>> = true;
// NOLINTEND(readability-identifier-naming)
