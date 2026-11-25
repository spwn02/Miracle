import std;
import Miracle;
import Switch;

using namespace Miracle;

namespace Tests::diagnostic {

enum class[[ = diagnostics::prefix("E"), = diagnostics::align(3) ]] Code : u8 {
  Zero = 0,
  Fallback = 7,
  Annotated[[= diagnostics::message("annotated failure")]] = 42,
  Canonical[[= diagnostics::message("canonical alias")]] = 123,
  Alias = 123,
};

enum class[[= diagnostics]] Prefixless : u8 {
  Value = 4,
};

enum class[[ = diagnostics::prefix("L"), = diagnostics::align(3) ]] LargeCode : u16 {
  Value = 1234,
};

enum class[[= diagnostics::prefix("S")]] SignedCode : i8 {
  Zero = 0,
  Positive = 8,
};

struct ForeignAnnotation final {};
inline constexpr ForeignAnnotation foreignAnnotation{};

enum class[[ = diagnostics::prefix("C"), = foreignAnnotation ]] ComposedCode : u8 {
  Value[[= foreignAnnotation]] = 5,
};

enum class[[= diagnostics::prefix("N")]] NegativeCode : i8 {
  Invalid = -1,
};

enum class[[ = diagnostics, = diagnostics::prefix("X") ]] MarkerAndPrefix : u8 {
  Value = 1,
};

enum class[[= diagnostics::align(3)]] AlignmentWithoutPrefix : u8 {
  Value = 1,
};

enum class[[ = diagnostics, = diagnostics::message("invalid") ]] MessageOnDomain : u8 {
  Value = 1,
};

enum class[[ = diagnostics::prefix("A"), = diagnostics::prefix("B") ]] DuplicatePrefix : u8 {
  Value = 1,
};

enum class[[= diagnostics]] InvalidEnumeratorAnnotation : u8 {
  Value[[= diagnostics::prefix("X")]] = 1,
};

enum class[[= diagnostics::prefix("")]] EmptyPrefix : u8 {
  Value = 1,
};

enum class[[
  = diagnostics::prefix("D"),
  = diagnostics::align(2),
  = diagnostics::align(3)
]] DuplicateAlignment : u8 {
  Value = 1,
};

enum class[[= diagnostics]] DuplicateMessage : u8 {
  Value[[ = diagnostics::message("first"), = diagnostics::message("second") ]] = 1,
};

enum class[[= diagnostics]] AlignmentOnEnumerator : u8 {
  Value[[= diagnostics::align(3)]] = 1,
};

static_assert(DiagnosticCode<Code>);
static_assert(DiagnosticCode<Prefixless>);
static_assert(DiagnosticCode<LargeCode>);
static_assert(DiagnosticCode<SignedCode>);
static_assert(DiagnosticCode<ComposedCode>);
static_assert(not DiagnosticCode<NegativeCode>);
static_assert(not DiagnosticCode<MarkerAndPrefix>);
static_assert(not DiagnosticCode<AlignmentWithoutPrefix>);
static_assert(not DiagnosticCode<MessageOnDomain>);
static_assert(not DiagnosticCode<DuplicatePrefix>);
static_assert(not DiagnosticCode<InvalidEnumeratorAnnotation>);
static_assert(not DiagnosticCode<EmptyPrefix>);
static_assert(not DiagnosticCode<DuplicateAlignment>);
static_assert(not DiagnosticCode<DuplicateMessage>);
static_assert(not DiagnosticCode<AlignmentOnEnumerator>);
static_assert(not std::copy_constructible<Diagnostic>);
static_assert(not std::is_copy_assignable_v<Diagnostic>);
static_assert(not std::copy_constructible<DiagnosticSpan>);
static_assert(not std::is_copy_assignable_v<DiagnosticSpan>);

constexpr RenderOptions plainCompact{
    .color = ColorMode::Never,
    .source = SourceMode::None,
    .detail = Miracle::DetailMode::Compact,
    .presentation = Presentation::Plain,
};

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto codeDescriptors() -> void {
  const auto fallback = Diagnostic::error(Code::Fallback);
  Switch::check(fallback.code().domain == "Code");
  Switch::check(fallback.code().prefix == "E");
  Switch::check(fallback.code().enumerator == "Fallback");
  Switch::check(fallback.code().canonicalMessage == "Fallback");
  Switch::check(fallback.code().numeric == static_cast<u64>(std::to_underlying(Code::Fallback)));
  Switch::check(fallback.code().alignment == 3);
  Switch::check(fallback.messageText() == "Fallback");

  const auto annotated = Diagnostic::error(Code::Annotated);
  Switch::check(annotated.messageText() == "annotated failure");

  const auto alias = Diagnostic::error(Code::Alias);
  Switch::check(alias.code().enumerator == "Canonical");
  Switch::check(alias.messageText() == "canonical alias");

  constexpr auto unknownValue = static_cast<Code>(250);
  const auto unknown = Diagnostic::error(unknownValue);
  Switch::check(unknown.code().enumerator == "<unknown>");
  Switch::check(unknown.messageText() == "unknown diagnostic code");
  Switch::check(unknown.code().numeric == static_cast<u64>(std::to_underlying(unknownValue)));
  Switch::check(render(unknown, plainCompact).contains("error[E250]: unknown diagnostic code"));

  const auto signedCode = Diagnostic::error(SignedCode::Positive);
  Switch::check(signedCode.code().numeric == static_cast<u64>(std::to_underlying(SignedCode::Positive)));
  Switch::check(signedCode.code().enumerator == "Positive");
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto renderingCodesAndSeverity()
    -> void {
  const String seven = render(Diagnostic::error(Code::Fallback), plainCompact);
  Switch::check(seven == "error[E007]: Fallback\n");

  const String fortyTwo = render(Diagnostic::warning(Code::Annotated), plainCompact);
  Switch::check(fortyTwo == "warning[E042]: annotated failure\n");

  const String large = render(Diagnostic::note(LargeCode::Value), plainCompact);
  Switch::check(large == "note[L1234]: Value\n");

  const String prefixless = render(Diagnostic::debug(Prefixless::Value), plainCompact);
  Switch::check(prefixless == "debug: Value\n");
  Switch::check(not prefixless.contains("[4]"));
  Switch::check(not prefixless.contains("\033["));

  const String forcedColourPlain = render(Diagnostic::error(Code::Fallback),
      {.color = ColorMode::Always,
          .source = SourceMode::None,
          .detail = Miracle::DetailMode::Compact,
          .presentation = Presentation::Plain});
  Switch::check(not forcedColourPlain.contains("\033["));
}

// TEST-DIAG-PARITY-001 / INV-DIAG-001: the compile-time Message and the runtime Diagnostic share one heading
// grammar, code encoder and zero padding for equivalent semantic input.
[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto compileTimeRuntimeHeadingParity()
    -> void {
  constexpr auto compileTime =
      format::Message{Severity::Warning, format::code("E", 42), "annotated failure"};
  Switch::check(render(Diagnostic::warning(Code::Annotated), plainCompact) == compileTime.view());

  constexpr auto wide = format::Message{Severity::Note, format::code<3>("L", 1234), "Value"};
  Switch::check(render(Diagnostic::note(LargeCode::Value), plainCompact) == wide.view());

  constexpr auto prefixless = format::Message{Severity::Debug, format::code("", 4), "Value"};
  Switch::check(render(Diagnostic::debug(Prefixless::Value), plainCompact) == prefixless.view());
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto terminalStyling() -> void {
  constexpr usize selectionBegin = 5;
  constexpr usize selectionEnd = 10;
  const std::source_location location = std::source_location::current();
  Diagnostic diagnostic = Diagnostic::error(Code::Annotated, location)
                              .note("context note")
                              .help("actionable help")
                              .span(DiagnosticSpan{"name = value", location}
                                      .select({.begin = selectionBegin, .end = selectionEnd})
                                      .label("expected a value"));

  const String styled = render(diagnostic,
      {.color = ColorMode::Always,
          .source = SourceMode::Snippet,
          .detail = Miracle::DetailMode::Full,
          .presentation = Presentation::Terminal});

  Switch::check(styled.contains("\033[1m\033[38;5;9merror[E042]\033[0m"));
  Switch::check(styled.contains("\033[1m\033[38;5;12m--> \033[0m"));
  Switch::check(styled.contains("\033[1m\033[38;5;9m^^^^^ expected a value\033[0m"));
  Switch::check(styled.contains("\033[1m\033[38;5;14mnote:\033[0m context note"));
  Switch::check(styled.contains("\033[1m\033[38;5;10mhelp:\033[0m actionable help"));
  Switch::check(styled.contains("\033[1m\033[38;5;14mdiagnostic:\033[0m Code::Annotated (42)"));

  const usize firstLocation = styled.find("--> ");
  Switch::require(firstLocation != String::npos);
  Switch::check(styled.find("--> ", firstLocation + 4) == String::npos);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto sourceAndAuxiliaryMessages()
    -> void {
  constexpr usize primaryBegin = 6;
  constexpr usize oversizedEnd = 999;
  constexpr usize secondaryBegin = 8;
  constexpr usize secondaryEnd = 2;
  const auto location = std::source_location::current();
  auto diagnostic = Diagnostic::error(Code::Annotated, location)
                        .message("specific occurrence")
                        .note("context note")
                        .help("actionable help")
                        .span(DiagnosticSpan{"alpha beta gamma", location}
                                .select({.begin = primaryBegin, .end = oversizedEnd})
                                .label("selected text"))
                        .span(DiagnosticSpan{"secondary", location}
                                .select({.begin = secondaryBegin, .end = secondaryEnd})
                                .label("secondary span")
                                .role(SpanRole::Secondary));

  const String text = render(diagnostic,
      {.color = ColorMode::Never,
          .source = SourceMode::Snippet,
          .detail = Miracle::DetailMode::Full,
          .presentation = Presentation::Plain});
  Switch::check(text.contains("specific occurrence"));
  Switch::check(text.contains("--> "));
  Switch::check(text.contains("alpha beta gamma"));
  Switch::check(text.contains("selected text"));
  Switch::check(text.contains("secondary span"));
  Switch::check(text.contains("note: context note"));
  Switch::check(text.contains("help: actionable help"));
  Switch::check(text.contains("diagnostic: Code::Annotated (42)"));
  Switch::check(text.contains(std::to_string(location.line())));
  Switch::check(not text.contains("\033["));

  const String locationOnly = render(diagnostic,
      {.color = ColorMode::Never,
          .source = SourceMode::Location,
          .detail = Miracle::DetailMode::Compact,
          .presentation = Presentation::Plain});
  Switch::check(locationOnly.contains("--> "));
  Switch::check(not locationOnly.contains("alpha beta gamma"));
}

namespace {

constexpr RenderOptions snippetOptions{
    .color = ColorMode::Never,
    .source = SourceMode::Snippet,
    .detail = Miracle::DetailMode::Compact,
    .presentation = Presentation::Plain,
};

[[nodiscard]] auto snippetOf(DiagnosticSpan span) -> String {
  return render(Diagnostic::error(Code::Fallback).span(std::move(span)), snippetOptions);
}

[[nodiscard]] auto afterHeading(const String &text) -> String {
  const usize newline = text.find('\n');
  return newline == String::npos ? String{} : text.substr(newline + 1);
}

[[nodiscard]] auto gutterLines(const String &text) -> Vec<String> {
  Vec<String> lines;
  std::istringstream input{text};
  String line;
  while (std::getline(input, line)) {
    if (line.starts_with(" | "))
      lines.push_back(line);
  }
  return lines;
}

} // namespace

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto snippetEdgeCases() -> void {
  const auto location = std::source_location::current();

  // Empty snippet: no gutter lines at all, even with a label.
  const String empty = snippetOf(DiagnosticSpan{"", location}.select({.begin = 0, .end = 3}).label("x"));
  Switch::check(gutterLines(empty).empty());

  // No selection: source line only; label (if any) on its own gutter line, no carets.
  const String unselected = snippetOf(DiagnosticSpan{"abc def", location});
  Switch::check(gutterLines(unselected) == Vec<String>{" | abc def"});
  const String labelled = snippetOf(DiagnosticSpan{"abc def", location}.label("why"));
  Switch::check(gutterLines(labelled) == Vec<String>{" | abc def", " | why"});

  // Zero-length selection still draws one marker at the offset.
  const String zero = snippetOf(DiagnosticSpan{"abc def", location}.select({.begin = 4, .end = 4}));
  Switch::check(gutterLines(zero) == Vec<String>{" | abc def", " |     ^"});

  // Out-of-range offsets clamp to the snippet; huge values never overflow or throw.
  const String large =
      snippetOf(DiagnosticSpan{"abc", location}.select({.begin = 2, .end = std::numeric_limits<usize>::max()}));
  Switch::check(gutterLines(large) == Vec<String>{" | abc", " |   ^"});
  const String beyond = snippetOf(DiagnosticSpan{"abc", location}.select({.begin = 99, .end = 100}));
  Switch::check(gutterLines(beyond).size() == 2);

  // Reversed endpoints normalise to the same marker as the ordered pair.
  const String ordered = snippetOf(DiagnosticSpan{"abcdef", location}.select({.begin = 1, .end = 4}));
  const String reversed = snippetOf(DiagnosticSpan{"abcdef", location}.select({.begin = 4, .end = 1}));
  Switch::check(gutterLines(ordered) == Vec<String>{" | abcdef", " |  ^^^"});
  Switch::check(gutterLines(reversed) == gutterLines(ordered));

  // Multi-line source: only the selection's line is shown; markers are relative to that line.
  const String multi = snippetOf(DiagnosticSpan{"one\ntwo three\nfour", location}.select({.begin = 8, .end = 13}));
  Switch::check(gutterLines(multi) == Vec<String>{" | two three", " |     ^^^^^"});

  // A selection crossing a line boundary is truncated at the end of its first line.
  const String crossing = snippetOf(DiagnosticSpan{"ab\ncd", location}.select({.begin = 1, .end = 5}));
  Switch::check(gutterLines(crossing) == Vec<String>{" | ab", " |  ^"});

  // Secondary spans use '-' markers.
  const String secondary =
      snippetOf(DiagnosticSpan{"abc", location}.select({.begin = 0, .end = 2}).role(SpanRole::Secondary));
  Switch::check(gutterLines(secondary) == Vec<String>{" | abc", " | --"});
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto zeroWidthCodes() -> void {
  // align(0) / no align: numeric printed without padding; padding never truncates wide values.
  const String unpadded = render(Diagnostic::error(Code::Fallback), plainCompact);
  Switch::check(unpadded.starts_with("error[E007]"));
  const String wide = render(Diagnostic::error(LargeCode::Value), plainCompact);
  Switch::check(wide.starts_with("error[L1234]"));
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto styleResetsAfterTree() -> void {
  const auto diagnostic = Diagnostic::error(Code::Annotated).note("n").cause(Diagnostic::warning(Code::Zero));
  const String text = render(diagnostic,
      {.color = ColorMode::Always,
          .source = SourceMode::None,
          .detail = Miracle::DetailMode::Compact,
          .presentation = Presentation::Terminal});
  // Every SGR start is matched by a reset before the next text/newline; the tree never ends styled.
  Switch::check(text.ends_with("\n"));
  const usize lastReset = text.rfind("\033[0m");
  const usize lastStart = text.rfind("\033[1m");
  Switch::require(lastReset != String::npos);
  Switch::check(lastStart == String::npos or lastStart < lastReset);
  Switch::check(text.find("\033[", text.rfind('\n') + 1) == String::npos);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto errorAdapter() -> void {
  Error error{Error::Message{"inner"}};
  error.with(Error::Message{"middle"}).with(Error::Message{"outer"});

  const Diagnostic diagnostic = diagnose(error);
  Switch::check(diagnostic.messageText() == "outer");
  Switch::require(diagnostic.causes().size() == 1);
  Switch::check(diagnostic.causes().front().messageText() == "middle");
  Switch::require(diagnostic.causes().front().causes().size() == 1);
  Switch::check(diagnostic.causes().front().causes().front().messageText() == "inner");
}

// TEST-DIAG-TREE-002: siblings render in insertion order, depth-first, with per-depth indentation.
[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto causeOrderAndIndentation() -> void {
  auto first = Diagnostic::error(Code::Zero).message("first").note("n1");
  first = std::move(first).cause(Diagnostic::error(Code::Zero).message("nested").help("h"));
  const auto root = Diagnostic::error(Code::Zero)
                        .message("root")
                        .cause(std::move(first))
                        .cause(Diagnostic::warning(Code::Zero).message("second"));
  const String text = render(root, plainCompact);
  Switch::check(text ==
                "error[E000]: root\n"
                "caused by:\n"
                "  error[E000]: first\n"
                "  note: n1\n"
                "  caused by:\n"
                "    error[E000]: nested\n"
                "    help: h\n"
                "caused by:\n"
                "  warning[E000]: second\n");
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto deepCausesAreIterative()
    -> void {
  constexpr usize depth = 4096;
  auto diagnostic = Diagnostic::error(Code::Zero).message("leaf");
  for (usize index{}; index < depth; ++index) {
    diagnostic = Diagnostic::error(Code::Zero).message("node").cause(std::move(diagnostic));
  }

  const String text = render(diagnostic, plainCompact);
  Switch::check(not text.empty());
  Switch::check(text.contains("caused by:"));
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("diagnostic") ]] auto wideCausesAreLinear()
    -> void {
  constexpr usize width = 4096;
  auto diagnostic = Diagnostic::error(Code::Zero).message("root");
  for (usize index{}; index < width; ++index) {
    diagnostic = std::move(diagnostic).cause(Diagnostic::note(Code::Zero).message("child"));
  }

  const String text = render(diagnostic, plainCompact);
  Switch::check(not text.empty());
  Switch::check(text.contains("note[E000]: child"));
}

} // namespace Tests::diagnostic

consteval {
  Switch::discover<^^Tests::diagnostic>();
}
