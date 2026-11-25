export module Miracle.Format:Emit;

import std;
import Miracle.Numeric;

import :Types;
import :Style;
import :Text;

export namespace Miracle::format {

/// Position inside the canonical diagnostic grammar. Sections only ever advance.
enum class Section : u8 {
  /// Ordinary text; no diagnostic grammar.
  Plain,
  /// The single `severity[code]: summary` line.
  Heading,
  /// The optional, at most one, `--> context` line.
  Context,
  /// `key: value` lines.
  Fields,
  /// `= note:` lines.
  Notes,
  /// `= help:` lines.
  Help,
};

/// Borrowed projection of a domain-owned diagnostic code: prefix, number and minimum digit width.
struct CodeView final {
  /// Code prefix; an empty prefix omits the bracketed code altogether.
  std::string_view prefix{};
  /// Numeric part, zero-padded on the left to `minimumDigits` and never truncated.
  u64 numeric{};
  /// Minimum number of decimal digits.
  usize minimumDigits{3}; // NOLINT(readability-magic-numbers)
};

/// What a single presentation event asks the emitter to write.
enum class EventKind : u8 { Text, Heading, Context, Field, Note, Help };

/// One immediate-consumption presentation command. Every view is read only during the `emit` call.
struct Event final {
  /// Event discriminator; unused members are ignored.
  EventKind kind{EventKind::Text};
  /// Text, summary, context, field value, note or help body.
  std::string_view text{};
  /// Field key (`Field` only).
  std::string_view key{};
  /// Style applied to `text`.
  Style style{};
  /// Severity (`Heading` only).
  Severity severity{Severity::Note};
  /// Code (`Heading` only).
  CodeView code{};
};

/// Outcome of `emit` and `finish`.
enum class WriteResult : u8 {
  /// The whole event was accepted by the sink.
  Written,
  /// The event violates the grammar; nothing was written and the state is unchanged.
  InvalidOrder,
  /// The sink refused bytes; the state is permanently failed and the output must be abandoned.
  SinkFailed,
};

/// Selects plain versus terminal-styled output.
struct RenderPolicy final {
  /// Plain presentation never emits ANSI.
  Presentation presentation{Presentation::Plain};
  /// `Never` never emits ANSI; `Always` and `Automatic` emit it for terminal presentation only.
  ColorMode color{ColorMode::Never};
};

/// Short-lived per-message renderer state owned by the caller. Cheap to copy; no hidden global state.
struct RenderState final {
  /// Rendering policy for this message.
  RenderPolicy policy{};
  /// Style the sink is currently under; default when no escape is outstanding.
  Style activeStyle{};
  /// Last completed section.
  Section section{Section::Plain};
  /// True once a heading was emitted.
  bool started{};
  /// Irreversible: set when the sink failed. Every later call returns `SinkFailed`.
  bool failed{};
};

/// A statically dispatched byte destination. Failure is reported by returning false.
template <class Sink>
concept TextSink = requires(Sink &sink, std::string_view text) {
  { sink.write(text) } -> std::same_as<bool>;
};

} // namespace Miracle::format

namespace Miracle::format::detail {

/// Receives styled spans; shared by the ANSI byte encoder and the compile-time Message storage.
template <class Writer>
concept SpanWriter = requires(Writer &writer, std::string_view text, const Style &style) {
  { writer.put(text, style) } -> std::same_as<bool>;
};

/// Writes text with C0/DEL bytes replaced by escape spellings; every piece carries `style`.
template <SpanWriter Writer>
constexpr auto putEscaped(Writer &writer, const std::string_view text, const Style &style) -> bool {
  usize start{};
  for (usize index{}; index < text.size(); ++index) {
    const auto byte = static_cast<unsigned char>(text[index]);
    if (not needsEscape(byte))
      continue;
    if (index > start and not writer.put(text.substr(start, index - start), style))
      return false;

    std::array<char, maxEscapeBytes> spelling{};
    const usize length = encodeEscape(byte, spelling);
    if (not writer.put(std::string_view{spelling.data(), length}, style))
      return false;
    start = index + 1;
  }
  return start == text.size() or writer.put(text.substr(start), style);
}

/// Writes `value` left-padded with zeros to `minimumDigits`; never truncates.
template <SpanWriter Writer>
constexpr auto putPaddedDecimal(Writer &writer, const u64 value, const usize minimumDigits, const Style &style)
    -> bool {
  std::array<char, maxU64Digits> digits{};
  const usize count = encodeDecimal(value, digits);
  for (usize pad = count; pad < minimumDigits; ++pad)
    if (not writer.put("0", style))
      return false;
  return writer.put(std::string_view{digits.data(), count}, style);
}

[[nodiscard]] constexpr auto sectionOf(const EventKind kind) noexcept -> Section {
  switch (kind) {
    case EventKind::Text: return Section::Plain;
    case EventKind::Heading: return Section::Heading;
    case EventKind::Context: return Section::Context;
    case EventKind::Field: return Section::Fields;
    case EventKind::Note: return Section::Notes;
    case EventKind::Help: return Section::Help;
  }
  std::unreachable();
}

/// Checks grammar ordering without touching any output or state.
[[nodiscard]] constexpr auto validate(const RenderState &state, const Event &event) noexcept -> WriteResult {
  if (state.failed)
    return WriteResult::SinkFailed;

  switch (event.kind) {
    case EventKind::Text:
      return (state.started or state.section != Section::Plain) ? WriteResult::InvalidOrder
                                                                 : WriteResult::Written;
    case EventKind::Heading:
      return state.started ? WriteResult::InvalidOrder : WriteResult::Written;
    default: break;
  }

  if (not state.started)
    return WriteResult::InvalidOrder;

  const Section target = sectionOf(event.kind);
  if (target < state.section or (event.kind == EventKind::Context and state.section >= Section::Context))
    return WriteResult::InvalidOrder;
  if (event.kind == EventKind::Field and event.key.empty())
    return WriteResult::InvalidOrder;
  return WriteResult::Written;
}

/// The one canonical layout algorithm. Header: `severity[code]: summary\n`; context hugs the header; every
/// later section is preceded by one blank line; each item ends in exactly one LF; text is escaped.
/// Writes `[prefix` + zero-padded number + `]`; nothing for an empty prefix. The one code encoder.
template <SpanWriter Writer>
constexpr auto putCode(Writer &writer, const CodeView &code, const Style &style) -> bool {
  if (code.prefix.empty())
    return true;
  return writer.put("[", style) and putEscaped(writer, code.prefix, style) and
         putPaddedDecimal(writer, code.numeric, code.minimumDigits, style) and writer.put("]", style);
}

template <SpanWriter Writer>
constexpr auto layout(Writer &writer, RenderState &state, const Event &event) -> WriteResult {
  if (const WriteResult order = validate(state, event); order != WriteResult::Written)
    return order;

  const auto fail = [&state]() noexcept {
    state.failed = true;
    return WriteResult::SinkFailed;
  };
  const Style plain{};
  const Style label = severityStyle(Severity::Note);

  if (event.kind == EventKind::Text)
    return writer.put(event.text, event.style) ? WriteResult::Written : fail();

  if (event.kind == EventKind::Heading) {
    const Style severity = severityStyle(event.severity);
    bool ok = writer.put(severityName(event.severity), severity);
    ok = ok and putCode(writer, event.code, severity);
    ok = ok and writer.put(": ", plain) and putEscaped(writer, event.text, event.style) and
         writer.put("\n", plain);
    if (not ok)
      return fail();
    state.started = true;
    state.section = Section::Heading;
    return WriteResult::Written;
  }

  const Section target = sectionOf(event.kind);
  bool ok = true;
  if (target > state.section and not(target == Section::Context and state.section == Section::Heading))
    ok = writer.put("\n", plain);

  switch (event.kind) {
    case EventKind::Context:
      ok = ok and writer.put("  ", plain) and writer.put("-->", label) and writer.put(" ", plain) and
           putEscaped(writer, event.text, event.style);
      break;
    case EventKind::Field:
      ok = ok and writer.put("  ", plain) and putEscaped(writer, event.key, plain) and
           writer.put(": ", plain) and putEscaped(writer, event.text, event.style);
      break;
    case EventKind::Note:
      ok = ok and writer.put("   ", plain) and writer.put("= note:", label) and writer.put(" ", plain) and
           putEscaped(writer, event.text, event.style);
      break;
    case EventKind::Help:
      ok = ok and writer.put("   ", plain) and writer.put("= help:", label) and writer.put(" ", plain) and
           putEscaped(writer, event.text, event.style);
      break;
    default: std::unreachable();
  }
  ok = ok and writer.put("\n", plain);
  if (not ok)
    return fail();

  state.section = target;
  return WriteResult::Written;
}

/// Encodes style transitions as SGR sequences around the sink's plain bytes. All state lives in `RenderState`.
template <TextSink Sink>
class AnsiWriter final {
public:
  constexpr AnsiWriter(Sink &sink, RenderState &state) noexcept
      : sink_(sink)
      , state_(state)
      , ansi_(state.policy.presentation == Presentation::Terminal and state.policy.color != ColorMode::Never) {}

  constexpr auto put(const std::string_view text, const Style &style) -> bool {
    if (text.empty())
      return true;
    if (ansi_ and style != state_.activeStyle) {
      if (state_.activeStyle != Style{} and not sink_.write(resetSequence))
        return false;
      if (style != Style{}) {
        std::array<char, maxSgrBytes> buffer{};
        const usize length = encodeSgr(style, buffer);
        if (not sink_.write(std::string_view{buffer.data(), length}))
          return false;
      }
      state_.activeStyle = style;
    }
    return sink_.write(text);
  }

  /// Emits the final reset when a non-default style is still active.
  constexpr auto finish() -> bool {
    if (state_.activeStyle == Style{})
      return true;
    if (not sink_.write(resetSequence))
      return false;
    state_.activeStyle = Style{};
    return true;
  }

  static constexpr std::string_view resetSequence = "\x1b[0m";

private:
  static constexpr usize maxSgrBytes = 4 * 4 + 2 * 19; // four attributes, two RGB colours

  static constexpr auto appendNumber(std::array<char, maxSgrBytes> &out, usize &at, const u8 value) noexcept
      -> void {
    std::array<char, maxU64Digits> digits{};
    const usize count = encodeDecimal(value, digits);
    for (usize index{}; index < count; ++index)
      out[at++] = digits[index];
  }

  static constexpr auto appendSequence(std::array<char, maxSgrBytes> &out,
      usize &at,
      const std::string_view fixed) noexcept -> void {
    for (const char character : fixed)
      out[at++] = character;
  }

  static constexpr auto appendColor(std::array<char, maxSgrBytes> &out,
      usize &at,
      const TerminalColor &color,
      const u8 base) noexcept -> void {
    if (color.kind == ColorKind::Default)
      return;
    appendSequence(out, at, "\x1b[");
    appendNumber(out, at, base);
    if (color.kind == ColorKind::Indexed) {
      appendSequence(out, at, ";5;");
      appendNumber(out, at, color.index);
    } else {
      appendSequence(out, at, ";2;");
      appendNumber(out, at, color.rgb.red);
      appendSequence(out, at, ";");
      appendNumber(out, at, color.rgb.green);
      appendSequence(out, at, ";");
      appendNumber(out, at, color.rgb.blue);
    }
    appendSequence(out, at, "m");
  }

  /// Complete style as consecutive SGR sequences: bold 1, dim 2, italic 3, underline 4, foreground, background.
  static constexpr auto encodeSgr(const Style &style, std::array<char, maxSgrBytes> &out) noexcept -> usize {
    usize at{};
    if (style.bold)
      appendSequence(out, at, "\x1b[1m");
    if (style.dim)
      appendSequence(out, at, "\x1b[2m");
    if (style.italic)
      appendSequence(out, at, "\x1b[3m");
    if (style.underline)
      appendSequence(out, at, "\x1b[4m");
    appendColor(out, at, style.foreground, 38); // NOLINT(readability-magic-numbers)
    appendColor(out, at, style.background, 48); // NOLINT(readability-magic-numbers)
    return at;
  }

  Sink &sink_;
  RenderState &state_;
  bool ansi_;
};

} // namespace Miracle::format::detail

export namespace Miracle::format {

/// Writes a diagnostic code through any writer with `put(text, style)`; shared by compile-time and runtime paths.
template <class Writer>
constexpr auto writeCode(Writer &writer, const CodeView &code, const Style &style) -> bool {
  return detail::putCode(writer, code, style);
}

/// Writes untrusted `text` through any writer with `put(text, style)`, replacing C0/DEL bytes by their escape
/// spellings (`\n`, `\r`, `\t`, `\0`, `\xNN`). Used for runtime diagnostic text so ESC or line breaks cannot forge
/// terminal output.
template <class Writer>
constexpr auto writeEscaped(Writer &writer, const std::string_view text, const Style &style) -> bool {
  return detail::putEscaped(writer, text, style);
}

/// Number of bytes `writeEscaped` produces for `text`; equals `text.size()` when nothing needs escaping.
[[nodiscard]] constexpr auto escapedSize(const std::string_view text) noexcept -> usize {
  usize size{};
  for (const char character : text) {
    const auto byte = static_cast<unsigned char>(character);
    if (detail::needsEscape(byte)) {
      std::array<char, detail::maxEscapeBytes> spelling{};
      size += detail::encodeEscape(byte, spelling);
    } else {
      ++size;
    }
  }
  return size;
}

/// Style-aware byte writer shared by every renderer: encodes style transitions as SGR sequences, honours
/// `RenderState::policy` and keeps all state in the caller's `RenderState`.
template <TextSink Sink>
using AnsiWriter = detail::AnsiWriter<Sink>;

/// Writes one event into `sink` using the shared grammar. Misordered events write nothing and leave `state`
/// untouched. A sink failure is irreversible. Not `noexcept`: sinks may allocate or throw.
template <TextSink Sink>
[[nodiscard]] constexpr auto emit(Sink &sink, RenderState &state, const Event &event) -> WriteResult {
  detail::AnsiWriter<Sink> writer{sink, state};
  return detail::layout(writer, state, event);
}

/// Resets outstanding ANSI styling. Adds no LF: every grammar line already ends in one.
template <TextSink Sink>
[[nodiscard]] constexpr auto finish(Sink &sink, RenderState &state) -> WriteResult {
  if (state.failed)
    return WriteResult::SinkFailed;
  detail::AnsiWriter<Sink> writer{sink, state};
  if (not writer.finish()) {
    state.failed = true;
    return WriteResult::SinkFailed;
  }
  return WriteResult::Written;
}

} // namespace Miracle::format
