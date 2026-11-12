export module Miracle.Format:Style;

import std;
import Miracle.Numeric;
import Miracle.Color;

import :Types;

export namespace Miracle::format {

/// Controls the kind of colour representation used in formatted output.
enum class ColorKind : u8 {
  /// Use the default colour representation for the current platform.
  Default,
  /// Use ANSI escape sequences for colour representation.
  Indexed,
  /// Use RGB escape sequences for colour representation.
  Rgb,
};

/// Represents a colour in a terminal or formatted output context.
struct TerminalColor final {
  /// The kind of colour representation used.
  ColorKind kind{ColorKind::Default};
  /// The index of the colour in the terminal's colour palette (0-255).
  u8 index{};
  /// The RGB value of the colour, used when kind is ColorKind::Rgb.
  Color rgb{};

  /// Returns the default terminal colour representation.
  [[nodiscard]] static constexpr auto defaultColor() noexcept -> TerminalColor {
    return TerminalColor{.kind = ColorKind::Default};
  }

  /// Returns an indexed terminal colour representation using the specified index (0-255).
  [[nodiscard]] static constexpr auto indexed(const u8 index) noexcept -> TerminalColor {
    return TerminalColor{.kind = ColorKind::Indexed, .index = index};
  }

  /// Returns a true colour terminal representation using the specified RGB value.
  [[nodiscard]] static constexpr auto trueColor(const Color color) noexcept -> TerminalColor {
    return TerminalColor{.kind = ColorKind::Rgb, .rgb = color};
  }

  constexpr auto operator<=>(const TerminalColor &) const noexcept -> std::strong_ordering = default;
};

/// Represents a text style in a terminal or formatted output context.
struct Style final {
  /// The foreground colour of the text.
  TerminalColor foreground{};
  /// The background colour of the text.
  TerminalColor background{};
  /// Whether the text is bold.
  bool bold{};
  /// Whether the text is dimmed.
  bool dim{};
  /// Whether the text is italicized.
  bool italic{};
  /// Whether the text is underlined.
  bool underline{};

  constexpr auto operator<=>(const Style &) const noexcept -> std::strong_ordering = default;
};

/// Returns the style associated with the specified diagnostic severity.
[[nodiscard]] constexpr auto severityStyle(const Severity severity) noexcept -> Style {
  // NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  switch (severity) {
    case Severity::Debug: return Style{.foreground = TerminalColor::indexed(8), .dim = true};
    case Severity::Note: return Style{.foreground = TerminalColor::indexed(14), .bold = true};
    case Severity::Warning: return Style{.foreground = TerminalColor::indexed(11), .bold = true};
    case Severity::Error: return Style{.foreground = TerminalColor::indexed(9), .bold = true};
  }
  // NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  std::unreachable();
}

} // namespace Miracle::format
