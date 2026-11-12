export module Miracle.Format:Fragment;

import std;
import Miracle.Numeric;

import :Style;

export namespace Miracle::format {

/// Represents a fragment of text with an associated style.
template <usize Capacity>
struct Fragment {
  /// The text content of the fragment.
  std::array<char, Capacity + 1> text{};
  /// The style associated with the fragment.
  Style style{};

  /// Constructs a text fragment with the specified text and style.
  constexpr Fragment(const std::array<char, Capacity> &text, const Style style = {}) noexcept
      : style(style) {
    std::ranges::copy(text, this->text.begin());
  }

  /// Constructs a text fragment with the specified text and style from a string literal.
  constexpr Fragment(const char (&text)[Capacity], const Style style = {}) noexcept // NOLINT
      : style(style) {
    std::ranges::copy(text, this->text.begin());
  }

  // Returns the number of characters in the fragment, excluding the null terminator.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return Capacity;
  }

  /// Returns a pointer to the text content of the fragment, null-terminated for C-style APIs.
  [[nodiscard]] constexpr auto data() const noexcept -> const char * {
    return text.data();
  }

  /// Returns a string view of the text content of the fragment, excluding the null terminator.
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
    return {text.data(), Capacity};
  }
};

/// Constructs a text fragment with the optionally specified style from a string literal.
template <usize N>
[[nodiscard]] constexpr auto fragment(const char (&text)[N], const Style style = {}) noexcept // NOLINT
    -> Fragment<N> {
  return Fragment<N>{text, style};
}

/// Constructs a text fragment with the specified style from a character array.
template <usize N>
[[nodiscard]] constexpr auto fragment(const std::array<char, N> &text, const Style style = {}) noexcept
    -> Fragment<N> {
  return Fragment<N>{text, style};
}

/// Constructs a text fragment with the specified style from a string literal.
template <usize N>
[[nodiscard]] constexpr auto styled(const char (&text)[N], const Style style) noexcept // NOLINT
    -> Fragment<N> {
  return Fragment<N>{text, style};
}

} // namespace Miracle::format
