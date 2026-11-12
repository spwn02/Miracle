export module Miracle.Format:Render;

import std;
import Miracle.Numeric;

import :Message;

export namespace Miracle::format {

/// Controls ANSI colour emission.
enum class ColorMode : u8 {
  /// Use presentation policy to decide whether colour in appropriate.
  Automatic,
  /// Request colour for terminal presentation.
  Always,
  /// Suppress ANSI colour unconditionally.
  Never,
};

/// Selects plain text versus terminal-oriented presentation.
enum class Presentation : u8 {
  /// Stable text presentation with no terminal escape sequences.
  Plain,
  /// Terminal-oriented presentation eligible for ANSI styling.
  Terminal,
};

/// Controls how a message is rendered to an output iterator.
struct RenderPolicy final {
  /// Selects plain text versus terminal-oriented presentation.
  Presentation presentation{Presentation::Plain};
  /// Controls ANSI colour emission.
  ColorMode colorMode{ColorMode::Never};
};

/// Renders a message to an output iterator according to the specified render policy.
template <class OutputIterator>
  requires std::output_iterator<OutputIterator, char>
auto writeFragment(OutputIterator out,
    const std::string_view text,
    const Style style,
    const RenderPolicy policy) -> OutputIterator {
  const bool ansi =
      (policy.colorMode == ColorMode::Always) or
      (policy.colorMode == ColorMode::Automatic and policy.presentation == Presentation::Terminal);

  if (!ansi or (policy.presentation == Presentation::Plain && policy.colorMode == ColorMode::Never)) {
    out = std::ranges::copy(text, out).out;
    return out;
  }

  // Emit SGR style attributes
  if (style.bold)
    out = std::ranges::copy(std::string_view{"\033[1m"}, out).out;
  if (style.dim)
    out = std::ranges::copy(std::string_view{"\033[2m"}, out).out;
  if (style.italic)
    out = std::ranges::copy(std::string_view{"\033[3m"}, out).out;
  if (style.underline)
    out = std::ranges::copy(std::string_view{"\033[4m"}, out).out;

  // Emit foreground color
  if (style.foreground.kind == ColorKind::Indexed) {
    out = std::ranges::copy(std::string_view{"\033[38;5;"}, out).out;
    const auto indexStr = std::to_string(style.foreground.index);
    out = std::ranges::copy(indexStr, out).out;
    *out++ = 'm';
  } else if (style.foreground.kind == ColorKind::Rgb) {
    out = std::ranges::copy(std::string_view{"\033[38;2;"}, out).out;
    out = std::ranges::copy(std::to_string(style.foreground.rgb.red), out).out;
    *out++ = ';';
    out = std::ranges::copy(std::to_string(style.foreground.rgb.green), out).out;
    *out++ = ';';
    out = std::ranges::copy(std::to_string(style.foreground.rgb.blue), out).out;
    *out++ = 'm';
  }

  // Emit background color
  if (style.background.kind == ColorKind::Indexed) {
    out = std::ranges::copy(std::string_view{"\033[48;5;"}, out).out;
    const auto indexStr = std::to_string(style.background.index);
    out = std::ranges::copy(indexStr, out).out;
    *out++ = 'm';
  } else if (style.background.kind == ColorKind::Rgb) {
    out = std::ranges::copy(std::string_view{"\033[48;2;"}, out).out;
    out = std::ranges::copy(std::to_string(style.background.rgb.red), out).out;
    *out++ = ';';
    out = std::ranges::copy(std::to_string(style.background.rgb.green), out).out;
    *out++ = ';';
    out = std::ranges::copy(std::to_string(style.background.rgb.blue), out).out;
    *out++ = 'm';
  }

  // Copy text content
  out = std::ranges::copy(text, out).out;

  // Reset all attributes at the end of the styled fragment
  out = std::ranges::copy(std::string_view{"\033[0m"}, out).out;

  return out;
}

/// Renders a message to an output iterator according to the specified render policy.
template <class MessageType, class OutputIterator>
  requires requires(const MessageType &message) {
    { message.view() } -> std::convertible_to<std::string_view>;
    { message.styles() } -> std::convertible_to<std::span<const StyleSpan>>;
  } and std::output_iterator<OutputIterator, char>
[[nodiscard]] auto renderTo(const MessageType &message, OutputIterator out, const RenderPolicy policy = {})
    -> OutputIterator {
  const std::span<const StyleSpan> styles = message.styles();

  if ((policy.presentation == Presentation::Plain and policy.colorMode == ColorMode::Never) or
      styles.empty()) {
    out = std::ranges::copy(message.view(), out).out;
    return out;
  }

  const auto view = message.view();
  const bool ansi =
      (policy.colorMode == ColorMode::Always) or
      (policy.colorMode == ColorMode::Automatic and policy.presentation == Presentation::Terminal);

  for (usize index{}; const auto &style : styles) {
    if (style.begin > index) {
      out = std::ranges::copy(view.substr(index, style.begin - index), out).out;
    }

    if (ansi) {
      writeFragment(out, view.substr(style.begin, style.end - style.begin), style.style, policy);
    } else {
      out = std::ranges::copy(view.substr(style.begin, style.end - style.begin), out).out;
    }

    index += style.end - index;
  }

  if (const auto &lastStyle = styles.back(); lastStyle.end < view.size()) {
    out = std::ranges::copy(view.substr(lastStyle.end), out).out;
  }

  return out;
}

/// Renders a message to a string according to the specified render policy.
template <class MessageType>
[[nodiscard]] auto render(const MessageType &message, RenderPolicy policy = {}) -> std::string {
  std::string result;
  auto _ = renderTo(message, std::back_inserter(result), policy);
  return result;
}

} // namespace Miracle::format
