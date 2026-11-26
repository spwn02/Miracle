export module Miracle.Format:Fragment;

import std;
import Miracle.Numeric;

import :Style;
import :Text;

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
export namespace Miracle::format {

/// Owns up to `Characters` text bytes plus one `Style`. Embeds no ANSI; `data()` and `view()` are plain text.
template <usize Characters>
class Fragment final {
public:
  /// Copies a string literal up to its first NUL; a literal with an interior NUL is shortened there.
  constexpr Fragment(const char (&text)[Characters + 1], const Style style = {}) noexcept // NOLINT
      : style_(style) {
    const std::string_view content = literalView(text);
    std::ranges::copy(content, text_.begin());
    length_ = content.size();
  }

  /// Copies exactly `Characters` bytes, including embedded NULs.
  constexpr Fragment(const std::array<char, Characters> &text, const Style style = {}) noexcept // NOLINT
      : text_(text)
      , length_(Characters)
      , style_(style) {}

  /// Returns the number of owned bytes.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return length_;
  }

  /// Returns the owned bytes; not NUL-terminated.
  [[nodiscard]] constexpr auto data() const noexcept -> const char * {
    return text_.data();
  }

  /// Returns the owned bytes as a view that borrows this fragment.
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
    return {text_.data(), length_};
  }

  /// Returns the style associated with the whole fragment.
  [[nodiscard]] constexpr auto style() const noexcept -> const Style & {
    return style_;
  }

private:
  std::array<char, Characters> text_{};
  usize length_{};
  Style style_{};
};

template <usize N>
Fragment(const char (&)[N], Style = {}) -> Fragment<N - 1>;

template <usize N>
Fragment(const std::array<char, N> &, Style = {}) -> Fragment<N>;

/// Constructs a fragment from a string literal; the terminating NUL is not stored.
template <usize N>
[[nodiscard]] constexpr auto fragment(const char (&text)[N], const Style style = {}) noexcept -> Fragment<N - 1> {
  return Fragment<N - 1>{text, style};
}

/// Constructs a fragment from exactly `N` bytes, including embedded NULs.
template <usize N>
[[nodiscard]] constexpr auto fragment(const std::array<char, N> &text, const Style style = {}) noexcept
    -> Fragment<N> {
  return Fragment<N>{text, style};
}

/// Clarity alias for `fragment(literal, style)`.
template <usize N>
[[nodiscard]] constexpr auto styled(const char (&text)[N], const Style style) noexcept -> Fragment<N - 1> {
  return Fragment<N - 1>{text, style};
}

} // namespace Miracle::format
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
