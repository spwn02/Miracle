import std;
import Miracle;
import Switch;

using namespace Miracle;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace {

constexpr format::Style accent{.foreground = format::TerminalColor::indexed(14), .bold = true, .italic = true};
constexpr format::Style rgbStyle{.foreground = format::TerminalColor::trueColor(Color::rgb(255, 159, 67)),
    .background = format::TerminalColor::trueColor(Color::rgb(1, 2, 3))};

// --- Severity / ColorMode / Presentation live in Miracle and keep their ordinals. ---
static_assert(std::to_underlying(Severity::Debug) == 0 and std::to_underlying(Severity::Error) == 3);
static_assert(std::to_underlying(ColorMode::Automatic) == 0 and std::to_underlying(ColorMode::Never) == 2);
static_assert(std::to_underlying(Presentation::Plain) == 0 and std::to_underlying(Presentation::Terminal) == 1);
static_assert(severityName(Severity::Debug) == "debug" and severityName(Severity::Note) == "note" and
              severityName(Severity::Warning) == "warning" and severityName(Severity::Error) == "error");

// --- TerminalColor keeps its tag: an index is never turned into RGB. ---
static_assert(format::TerminalColor::indexed(1).kind == format::ColorKind::Indexed);
static_assert(format::TerminalColor::indexed(1).index == 1 and format::TerminalColor::indexed(1).rgb == Color{});
static_assert(format::TerminalColor::indexed(1) != format::TerminalColor::trueColor(Color::rgb(255, 0, 0)));
static_assert(format::TerminalColor::trueColor(Color::rgb(255, 159, 67)).rgb == Color::rgb(255, 159, 67));
static_assert(format::TerminalColor::defaultColor() == format::TerminalColor{});

// --- Fragment: owns exactly its bytes, no NUL for literals, embedded NULs survive. ---
static_assert(format::fragment("abc").size() == 3 and format::fragment("abc").view() == "abc");
static_assert(format::fragment("").size() == 0 and format::fragment("").view().empty());
static_assert(format::styled("x", accent).view() == "x" and format::styled("x", accent).style() == accent);
static_assert(format::fragment("abc").style() == format::Style{});
static_assert(format::fragment(std::array<char, 3>{'a', '\0', 'b'}).size() == 3);
static_assert(format::fragment(std::array<char, 3>{'a', '\0', 'b'}).view() == std::string_view{"a\0b", 3});
static_assert(format::fragment(std::array<char, 0>{}).size() == 0);
static_assert(not std::same_as<decltype(format::fragment("ab")), decltype(format::fragment("abc"))>);

// --- Code: prefix + zero-padded number, width is a minimum, never truncates. ---
static_assert(format::code<3>("ITR", 4).view() == "ITR004");
static_assert(format::code<5>("CTR", 7).view() == "CTR00007");
static_assert(format::code<2>("E", 12345).view() == "E12345");
static_assert(format::code<0>("E", 0).view() == "E0");
static_assert(format::code("ITR", 4).view() == "ITR004");
static_assert(format::code<3>("X", std::numeric_limits<u64>::max()).view() == "X18446744073709551615");
static_assert(format::code<3>("ITR", 4).codeView().prefix == "ITR" and format::code<3>("ITR", 4).codeView().numeric == 4);

// --- Plain messages: NUL-terminated at the active size, immutable growth, no phantom runs. ---
constexpr auto abc = format::Message{"abc"};
static_assert(abc.size() == 3 and abc.view() == "abc" and abc.data()[3] == '\0');
constexpr auto abcx = abc.append("x");
static_assert(abc.view() == "abc" and abcx.view() == "abcx" and abcx.data()[4] == '\0');
static_assert(format::Message{"Hello "}.append(format::styled("world", accent)).view() == "Hello world");
static_assert(format::Message{"Hello "}.append(format::styled("world", accent)).styles().size() == 1);
static_assert(format::Message{"Hello "}.append(format::styled("world", accent)).styles()[0] ==
              format::StyleSpan{.begin = 6, .end = 11, .style = accent});
static_assert(format::Message{"a"}.append(format::fragment("")).styles().empty());
static_assert(format::Message{"a"}.append(format::styled("", accent)).styles().empty());
static_assert(format::Message{"a"}.append(format::fragment("b")).styles().empty());
static_assert(format::Message{"a"}.append(format::styled("b", accent)).append(format::styled("c", accent)).styles().size() == 1);
static_assert(format::Message{"a"}.append(format::styled("b", accent)).append(format::styled("c", accent)).styles()[0].end == 3);
static_assert(format::Message{"a"}.append(format::styled("b", accent)).append("-").append(format::styled("c", accent)).styles().size() == 2);
static_assert(format::Message{"a"}.append(format::fragment(std::array<char, 3>{'x', '\0', 'y'})).view() ==
              std::string_view{"ax\0y", 4});
static_assert(format::Message{"a"}.append(format::fragment(std::array<char, 3>{'x', '\0', 'y'})).data()[4] == '\0');

// --- Decimal append ---
static_assert(format::Message{""}.append(0).view() == "0");
static_assert(format::Message{""}.append(std::numeric_limits<u64>::max()).view() == "18446744073709551615");
static_assert(format::Message{""}.append(std::numeric_limits<i64>::min()).view() == "-9223372036854775808");
static_assert(format::Message{""}.append(std::numeric_limits<i64>::max()).view() == "9223372036854775807");
static_assert(format::Message{""}.append(std::numeric_limits<i8>::min()).view() == "-128");
static_assert(format::Message{""}.append(-7).append(9u).view() == "-79");

template <class Message, class Value>
concept CanAppend = requires(Message message, Value value) { message.append(value); };
static_assert(CanAppend<decltype(abc), int> and CanAppend<decltype(abc), std::uint8_t>);
static_assert(not CanAppend<decltype(abc), bool>);
static_assert(not CanAppend<decltype(abc), char>);
static_assert(not CanAppend<decltype(abc), double>);
static_assert(not CanAppend<decltype(abc), wchar_t>);

// --- Diagnostic grammar ---
constexpr auto heading = format::Message{Severity::Error, format::code<3>("ITR", 4), "callable cannot accept"};
static_assert(heading.view() == "error[ITR004]: callable cannot accept\n");
static_assert(format::Message{Severity::Warning, format::code("W", 12345), "x"}.view() == "warning[W12345]: x\n");
static_assert(format::Message{Severity::Note, format::code<0>("", 5), "x"}.view() == "note: x\n");
static_assert(format::Message{Severity::Debug, format::code("D", 0), "x"}.view() == "debug[D000]: x\n");

constexpr auto full = format::Message{Severity::Error, format::code<3>("STR", 1), "invalid character type"}
                          .context("BasicStaticString")
                          .field("actual", format::fragment("UnstableChar"))
                          .field("size", "3")
                          .note("the character type is not standard-layout")
                          .note("second note")
                          .help("use a trivially-copyable, standard-layout character type")
                          .help("second help");
static_assert(full.view() ==
              "error[STR001]: invalid character type\n"
              "  --> BasicStaticString\n"
              "\n"
              "  actual: UnstableChar\n"
              "  size: 3\n"
              "\n"
              "   = note: the character type is not standard-layout\n"
              "   = note: second note\n"
              "\n"
              "   = help: use a trivially-copyable, standard-layout character type\n"
              "   = help: second help\n");
static_assert(full.view().back() == '\n' and not full.view().ends_with("\n\n"));
static_assert(full.data()[full.size()] == '\0');

// Omitted sections leave no gap.
static_assert(format::Message{Severity::Note, format::code("N", 1), "h"}.note("n").view() ==
              "note[N001]: h\n\n   = note: n\n");
static_assert(format::Message{Severity::Note, format::code("N", 1), "h"}.context("c").help("p").view() ==
              "note[N001]: h\n  --> c\n\n   = help: p\n");

// Styled fragments and labels: runs cover exact byte ranges, ascending and non-overlapping.
constexpr auto styledMessage = format::Message{Severity::Error, format::code("ITR", 4), "bad"}
                                   .context(format::fragment("Iter::map", accent))
                                   .field("callable", format::fragment("F", accent))
                                   .note(format::fragment("n", accent));
consteval auto runsAreOrdered(std::span<const format::StyleSpan> runs, const usize size) -> bool {
  usize previous{};
  for (const auto &run : runs) {
    if (run.begin >= run.end or run.begin < previous or run.end > size)
      return false;
    previous = run.end;
  }
  return true;
}
static_assert(runsAreOrdered(styledMessage.styles(), styledMessage.size()));
static_assert(styledMessage.view().substr(styledMessage.styles()[0].begin,
                  styledMessage.styles()[0].end - styledMessage.styles()[0].begin) == "error[ITR004]");
static_assert(styledMessage.styles()[0].style == format::severityStyle(Severity::Error));

// Control bytes are escaped inside single-line fields, byte-preserved in plain text.
static_assert(format::Message{Severity::Error, format::code("E", 1), "a\nb\rc\td"}.view() ==
              "error[E001]: a\\nb\\rc\\td\n");
static_assert(format::Message{Severity::Error, format::code("E", 1), "x"}
                  .field("k", format::fragment(std::array<char, 5>{'\0', '\x1b', '\x01', '\x7f', '\xc3'}))
                  .view() == "error[E001]: x\n\n  k: \\0\\x1B\\x01\\x7F\xc3\n");
static_assert(format::Message{"a\nb\x1b"}.view() == "a\nb\x1b");

// Extension never changes the original value and the types are distinct.
static_assert(not std::same_as<decltype(abc), decltype(abcx)>);

// Ordering is enforced by constraints.
template <class Message>
concept CanContext = requires(Message message) { message.context("c"); };
template <class Message>
concept CanField = requires(Message message) { message.field("k", "v"); };
template <class Message>
concept CanFieldEmptyKey = requires(Message message) { message.field("", "v"); };
template <class Message>
concept CanNote = requires(Message message) { message.note("n"); };
template <class Message>
concept CanHelp = requires(Message message) { message.help("h"); };
template <class Message>
concept CanAppendText = requires(Message message) { message.append("x"); };

using HeadingMessage = std::remove_cv_t<decltype(heading)>;
using ContextMessage = decltype(heading.context("c"));
using FieldMessage = decltype(heading.field("k", "v"));
using NoteMessage = decltype(heading.note("n"));
using HelpMessage = decltype(heading.help("h"));

static_assert(CanContext<HeadingMessage> and CanField<HeadingMessage> and CanNote<HeadingMessage>);
static_assert(not CanContext<ContextMessage> and CanField<ContextMessage>);
static_assert(not CanContext<FieldMessage> and CanField<FieldMessage> and CanNote<FieldMessage>);
static_assert(not CanContext<NoteMessage> and not CanField<NoteMessage> and CanNote<NoteMessage> and CanHelp<NoteMessage>);
static_assert(not CanContext<HelpMessage> and not CanField<HelpMessage> and not CanNote<HelpMessage> and CanHelp<HelpMessage>);
static_assert(not CanFieldEmptyKey<HeadingMessage>);
static_assert(not CanAppendText<HeadingMessage> and not CanAppendText<HelpMessage>);

// A generated static_assert message works (positive case).
static_assert(true, full);

// --- Runtime event emitter ---
struct StringSink {
  std::string bytes;
  std::size_t failAfter{std::string::npos};
  auto write(std::string_view text) -> bool {
    if (bytes.size() + text.size() > failAfter) {
      bytes.append(text.substr(0, failAfter - bytes.size()));
      return false;
    }
    bytes.append(text);
    return true;
  }
};
static_assert(format::TextSink<StringSink>);
struct NotASink {
  auto write(std::string_view) -> void {}
};
static_assert(not format::TextSink<NotASink>);

constexpr auto ansiAlways = format::RenderPolicy{Presentation::Terminal, ColorMode::Always};

} // namespace

namespace Tests::formatting {


[[= Switch::test]] auto renderPlainEqualsView() -> void {
  Switch::check(format::render(styledMessage) == styledMessage.view());
  Switch::check(format::render(full) == full.view());
  Switch::check(format::render(styledMessage, {Presentation::Terminal, ColorMode::Never}) ==
                styledMessage.view());
  Switch::check(format::render(styledMessage, {Presentation::Plain, ColorMode::Always}) ==
                styledMessage.view());
  Switch::check(format::render(styledMessage, {Presentation::Plain, ColorMode::Automatic}) ==
                styledMessage.view());
}

[[= Switch::test]] auto renderAnsiExact() -> void {
  constexpr auto message = format::Message{"a"}
                               .append(format::styled("b", accent))
                               .append("c")
                               .append(format::styled("d", rgbStyle));
  Switch::check(format::render(message, ansiAlways) ==
                "a\x1b[1m\x1b[3m\x1b[38;5;14mb\x1b[0mc\x1b[38;2;255;159;67m\x1b[48;2;1;2;3md\x1b[0m");
  Switch::check(format::render(message, {Presentation::Terminal, ColorMode::Automatic}) ==
                format::render(message, ansiAlways));

  // Default style never emits escapes, even in Always mode.
  constexpr auto unstyled = format::Message{"a"}.append(format::fragment("b"));
  Switch::check(format::render(unstyled, ansiAlways) == "ab");

  // dim and underline
  constexpr auto attributes =
      format::Message{""}.append(format::styled("x", {.dim = true, .underline = true}));
  Switch::check(format::render(attributes, ansiAlways) == "\x1b[2m\x1b[4mx\x1b[0m");
}

[[= Switch::test]] auto renderAnsiTransitionsDoNotLeak() -> void {
  constexpr format::Style bold{.bold = true};
  constexpr auto message = format::Message{""}
                               .append(format::styled("A", bold))
                               .append("m")
                               .append(format::styled("B", bold));
  Switch::check(format::render(message, ansiAlways) == "\x1b[1mA\x1b[0mm\x1b[1mB\x1b[0m");

  // Adjacent different styles switch with one reset, no redundant sequences.
  constexpr auto adjacent = format::Message{""}
                                .append(format::styled("A", bold))
                                .append(format::styled("B", {.italic = true}));
  Switch::check(format::render(adjacent, ansiAlways) == "\x1b[1mA\x1b[0m\x1b[3mB\x1b[0m");
}

[[= Switch::test]] auto renderToIteratorMatchesString() -> void {
  std::string viaIterator;
  auto _ = format::renderTo(styledMessage, std::back_inserter(viaIterator), ansiAlways);
  Switch::check(viaIterator == format::render(styledMessage, ansiAlways));
  Switch::check(viaIterator.size() > styledMessage.size());
}

[[= Switch::test]] auto emitMatchesCompileTimeMessage() -> void {
  StringSink sink;
  format::RenderState state;
  const auto head = format::Event{.kind = format::EventKind::Heading,
      .text = "invalid character type",
      .severity = Severity::Error,
      .code = {.prefix = "STR", .numeric = 1, .minimumDigits = 3}};
  Switch::check(format::emit(sink, state, head) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Context, .text = "BasicStaticString"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Field, .text = "UnstableChar", .key = "actual"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Field, .text = "3", .key = "size"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Note, .text = "the character type is not standard-layout"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Note, .text = "second note"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Help, .text = "use a trivially-copyable, standard-layout character type"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Help, .text = "second help"}) == format::WriteResult::Written);
  Switch::check(format::finish(sink, state) == format::WriteResult::Written);
  Switch::check(sink.bytes == full.view());
}

[[= Switch::test]] auto emitRuntimeCodeParity() -> void {
  for (const u64 number : {u64{0}, u64{4}, u64{12345}, std::numeric_limits<u64>::max()}) {
    StringSink sink;
    format::RenderState state;
    Switch::check(format::emit(sink, state, {.kind = format::EventKind::Heading, .text = "s", .severity = Severity::Warning,
                                        .code = {.prefix = "ITR", .numeric = number, .minimumDigits = 5}}) ==
                  format::WriteResult::Written);
    const auto expected = std::string{"warning[ITR"} +
                          std::string(5 > std::to_string(number).size() ? 5 - std::to_string(number).size() : 0, '0') +
                          std::to_string(number) + "]: s\n";
    Switch::check(sink.bytes == expected);
  }
}

[[= Switch::test]] auto emitRejectsInvalidOrderWithoutSideEffects() -> void {
  StringSink sink;
  format::RenderState state;
  // Anything but Heading/Text before a heading.
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Note, .text = "n"}) == format::WriteResult::InvalidOrder);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Context, .text = "c"}) == format::WriteResult::InvalidOrder);
  Switch::check(sink.bytes.empty() and not state.started);

  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Heading, .text = "h", .severity = Severity::Error}) == format::WriteResult::Written);
  const std::string afterHeading = sink.bytes;
  Switch::check(afterHeading == "error: h\n");

  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Heading, .text = "h2"}) == format::WriteResult::InvalidOrder);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Text, .text = "t"}) == format::WriteResult::InvalidOrder);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Field, .text = "v", .key = ""}) == format::WriteResult::InvalidOrder);

  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Note, .text = "n"}) == format::WriteResult::Written);
  const std::string afterNote = sink.bytes;
  const auto sectionBefore = state.section;
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Field, .text = "v", .key = "k"}) == format::WriteResult::InvalidOrder);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Context, .text = "c"}) == format::WriteResult::InvalidOrder);
  Switch::check(sink.bytes == afterNote and state.section == sectionBefore);

  // Second context is rejected too.
  StringSink other;
  format::RenderState fresh;
  Switch::check(format::emit(other, fresh, {.kind = format::EventKind::Heading, .text = "h"}) == format::WriteResult::Written);
  Switch::check(format::emit(other, fresh, {.kind = format::EventKind::Context, .text = "c"}) == format::WriteResult::Written);
  Switch::check(format::emit(other, fresh, {.kind = format::EventKind::Context, .text = "c"}) == format::WriteResult::InvalidOrder);
}

[[= Switch::test]] auto emitTextAndEscaping() -> void {
  StringSink sink;
  format::RenderState state;
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Text, .text = "raw\n\x1b"}) == format::WriteResult::Written);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Text, .text = "!"}) == format::WriteResult::Written);
  Switch::check(sink.bytes == "raw\n\x1b!");

  StringSink escaped;
  format::RenderState diagnostic;
  Switch::check(format::emit(escaped, diagnostic, {.kind = format::EventKind::Heading, .text = "a\nb", .severity = Severity::Note}) == format::WriteResult::Written);
  Switch::check(format::emit(escaped, diagnostic, {.kind = format::EventKind::Field, .text = "\x1b[31m\x7f\xe2\x82\xac", .key = "k\tk"}) == format::WriteResult::Written);
  Switch::check(escaped.bytes == "note: a\\nb\n\n  k\\tk: \\x1B[31m\\x7F\xe2\x82\xac\n");
}

[[= Switch::test]] auto emitAnsiStateIsLocalAndFinishResets() -> void {
  StringSink sink;
  format::RenderState state{.policy = ansiAlways};
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Text, .text = "x", .style = accent}) == format::WriteResult::Written);
  Switch::check(state.activeStyle == accent);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Text, .text = "y", .style = accent}) == format::WriteResult::Written);
  Switch::check(sink.bytes == "\x1b[1m\x1b[3m\x1b[38;5;14mxy");
  Switch::check(format::finish(sink, state) == format::WriteResult::Written);
  Switch::check(sink.bytes == "\x1b[1m\x1b[3m\x1b[38;5;14mxy\x1b[0m");
  Switch::check(state.activeStyle == format::Style{});
  Switch::check(format::finish(sink, state) == format::WriteResult::Written);
  Switch::check(sink.bytes.size() == std::string_view{"\x1b[1m\x1b[3m\x1b[38;5;14mxy\x1b[0m"}.size());

  // Plain policy never emits ANSI and finish adds nothing.
  StringSink plain;
  format::RenderState plainState;
  Switch::check(format::emit(plain, plainState, {.kind = format::EventKind::Text, .text = "x", .style = accent}) == format::WriteResult::Written);
  Switch::check(format::finish(plain, plainState) == format::WriteResult::Written);
  Switch::check(plain.bytes == "x");
}

[[= Switch::test]] auto sinkFailureIsIrreversible() -> void {
  StringSink sink{.failAfter = 4};
  format::RenderState state;
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Heading, .text = "heading", .severity = Severity::Error}) == format::WriteResult::SinkFailed);
  Switch::check(state.failed);
  const std::string partial = sink.bytes;
  sink.failAfter = std::string::npos;
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Note, .text = "n"}) == format::WriteResult::SinkFailed);
  Switch::check(format::emit(sink, state, {.kind = format::EventKind::Heading, .text = "again"}) == format::WriteResult::SinkFailed);
  Switch::check(format::finish(sink, state) == format::WriteResult::SinkFailed);
  Switch::check(sink.bytes == partial);
}

[[= Switch::test]] auto eventViewsAreConsumedImmediately() -> void {
  StringSink sink;
  format::RenderState state;
  {
    const std::string temporary = "short-lived";
    Switch::check(format::emit(sink, state, {.kind = format::EventKind::Text, .text = temporary}) == format::WriteResult::Written);
  }
  Switch::check(sink.bytes == "short-lived");
}

} // namespace Tests::formatting
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  Switch::discover<^^Tests::formatting>();
}
