export module Miracle:Range;

import std;
import :Types;

namespace Miracle {

template <class T>
concept RangeElement =
    std::integral<T> and std::same_as<T, std::remove_cv_t<T>> and not std::same_as<T, bool>;

template <class Left, class Right>
concept CompatibleRangeEndpoints =
    RangeElement<Left> and RangeElement<Right> and (std::is_signed_v<Left> == std::is_signed_v<Right>);

} // namespace Miracle

export namespace Miracle {

/// Finite half-open unit-stride integral interval `[start, stop)`.
///
/// Reversed endpoints represent an empty interval; they never imply traverse iteration.
template <class T>
  requires RangeElement<T>
struct Range final {
  using Value = T;
  using Size = std::make_unsigned_t<T>;

  T start{};
  T stop{};

  constexpr Range() noexcept = default;

  /// Creates `[0, stop)`.
  constexpr explicit Range(T stop) noexcept
      : stop(stop) {
  }

  /// Creates `[start, stop)`.
  constexpr explicit Range(T start, T stop) noexcept
      : start(start)
      , stop(stop) {
  }

  /// Returns an iterator over the first generated value.
  [[nodiscard]] constexpr auto begin() const noexcept {
    return view().begin();
  }

  /// Returns the common end iterator.
  [[nodiscard]] constexpr auto end() const noexcept {
    return view().end();
  }

  /// Returns whether this interval contains no values.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return start >= stop;
  }

  /// Returns the number of generated values without signed-overflow arithmetic.
  [[nodiscard]] constexpr auto size() const noexcept -> Size {
    if (empty()) {
      return Size{};
    }
    return static_cast<Size>(stop) - static_cast<Size>(start);
  }

  /// Returns whether `value` belongs to this half-open interval.
  [[nodiscard]] constexpr auto contains(T value) const noexcept -> bool {
    return start <= value and value < stop;
  }

  constexpr auto operator==(const Range &) const noexcept -> bool = default;

private:
  /// Normalizes only the generated view so reversed ranges remain truthful values while iterating as empty.
  [[nodiscard]] constexpr auto view() const noexcept -> std::ranges::iota_view<T, T> {
    const T first = start < stop ? start : stop;
    return std::views::iota(first, stop);
  }
};

template <RangeElement T>
Range(T) -> Range<T>;

template <class Left, class Right>
  requires CompatibleRangeEndpoints<Left, Right>
Range(Left, Right) -> Range<std::common_type_t<Left, Right>>;

} // namespace Miracle

export template <Miracle::RangeElement T>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool std::ranges::enable_view<Miracle::Range<T>> = true;

export template <Miracle::RangeElement T>
// NOLINTNEXTLINE(readability-identifier-naming)
inline constexpr bool std::ranges::enable_borrowed_range<Miracle::Range<T>> = true;
