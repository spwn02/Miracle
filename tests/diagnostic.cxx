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
    .color = Miracle::ColorMode::Never,
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
      {.color = Miracle::ColorMode::Always,
          .source = SourceMode::None,
          .detail = Miracle::DetailMode::Compact,
          .presentation = Presentation::Plain});
  Switch::check(not forcedColourPlain.contains("\033["));
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
      {.color = Miracle::ColorMode::Never,
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
      {.color = Miracle::ColorMode::Never,
          .source = SourceMode::Location,
          .detail = Miracle::DetailMode::Compact,
          .presentation = Presentation::Plain});
  Switch::check(locationOnly.contains("--> "));
  Switch::check(not locationOnly.contains("alpha beta gamma"));
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
