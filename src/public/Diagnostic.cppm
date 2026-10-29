export module Miracle:Diagnostic;

import std;
import :Types;
import :StaticString;

export namespace Miracle {

/// Annotation vocabulary for reflected diagnostic-code enums.
///
/// Apply the marker itself for a prefix-less domain, or use `prefix(...)` for a prefixed domain. Enumerator
/// messages are optional and fall back to the reflected identifier. `align(n)` sets a minimum zero-padded
/// numeric width.
// Lowercase is intentional: annotations read as a small language
// (`[[= diagnostics ]]`, `diagnostics::prefix(...)`, ...).
// NOLINTNEXTLINE(readability-identifier-naming)
struct diagnostics final {
  /// Structural annotation payload storing a diagnostic-code prefix.
  template <usize Capacity>
  struct Prefix final {
    /// Owned structural prefix text.
    StaticString<Capacity> value;
  };

  /// Structural annotation payload storing a canonical enumerator message.
  template <usize Capacity>
  struct Message final {
    /// Owned structural message text.
    StaticString<Capacity> value;
  };

  /// Structural annotation payload storing a minimum numeric code width.
  struct Alignment final {
    /// Minimum numeric width; wider values are never truncated.
    usize width{};
  };

  /// Creates a structural prefix annotation from a string literal.
  template <usize N>
  // Array-reference spelling preserves literal extent for structural annotation storage.
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
  [[nodiscard]] static consteval auto prefix(const char (&value)[N]) -> Prefix<N - 1> {
    return Prefix<N - 1>{StaticString<N - 1>{value}};
  }

  /// Creates a canonical-message annotation from a string literal.
  template <usize N>
  // Array-reference spelling preserves literal extent for structural annotation storage.
  // NOLINTNEXTLINE(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
  [[nodiscard]] static consteval auto message(const char (&value)[N]) -> Message<N - 1> {
    return Message<N - 1>{StaticString<N - 1>{value}};
  }

  /// Creates a minimum-width numeric-code annotation.
  [[nodiscard]] static consteval auto align(usize width) -> Alignment {
    return Alignment{width};
  }
};

/// Inline annotation façade used by `[[= diagnostics ]]` and its factories.
inline constexpr struct diagnostics diagnostics{};

/// Diagnostic importance used by rendering and filtering layers.
enum class Severity : u8 {
  /// Developer-oriented diagnostic information.
  Debug,
  /// Informational diagnostic context.
  Note,
  /// Recoverable or cautionary condition.
  Warning,
  /// Failure requiring caller attention.
  Error,
};

/// Normalized runtime identity for an enum-backed diagnostic code.
///
/// The descriptor owns all reflected text so a `Diagnostic` does not remain templated on, or otherwise depend
/// on, the originating enum type at runtime.
struct DiagnosticCodeDescriptor final {
  /// Reflected enum type identifier.
  String domain;
  /// Optional human-facing code prefix.
  String prefix;
  /// Canonical first-declared enumerator name, or `<unknown>`.
  String enumerator;
  /// Canonical annotation message or enumerator-name fallback.
  String canonicalMessage;
  /// Non-negative numeric code representation.
  u64 numeric{};
  /// Resolved minimum zero-padding width.
  usize alignment{};
};

} // namespace Miracle

namespace Miracle {

[[nodiscard]] consteval auto diagnosticAnnotationType(std::meta::info annotation) -> std::meta::info {
  return std::meta::remove_const(std::meta::type_of(annotation));
}

[[nodiscard]] consteval auto diagnosticIsPrefix(std::meta::info annotation) -> bool {
  const auto type = diagnosticAnnotationType(annotation);
  return std::meta::has_template_arguments(type) and std::meta::template_of(type) == ^^diagnostics::Prefix;
}

[[nodiscard]] consteval auto diagnosticIsMessage(std::meta::info annotation) -> bool {
  const auto type = diagnosticAnnotationType(annotation);
  return std::meta::has_template_arguments(type) and std::meta::template_of(type) == ^^diagnostics::Message;
}

[[nodiscard]] consteval auto diagnosticIsAlignment(std::meta::info annotation) -> bool {
  return diagnosticAnnotationType(annotation) == ^^diagnostics::Alignment;
}

[[nodiscard]] consteval auto diagnosticIsMarker(std::meta::info annotation) -> bool {
  return diagnosticAnnotationType(annotation) == std::meta::remove_const(^^decltype(diagnostics));
}

template <std::meta::info Annotation>
[[nodiscard]] consteval auto diagnosticAnnotationText() -> StringView {
  static_assert(diagnosticIsPrefix(Annotation) or diagnosticIsMessage(Annotation));
  constexpr auto type = diagnosticAnnotationType(Annotation);
  using AnnotationType = [:type:];
  static constexpr auto annotationValue_ = std::meta::extract<AnnotationType>(Annotation);
  constexpr StringView text = annotationValue_.value.view();
  return {std::define_static_string(text), text.size()};
}

[[nodiscard]] consteval auto diagnosticAlignmentValue(std::meta::info annotation) -> usize {
  return std::meta::extract<diagnostics::Alignment>(annotation).width;
}

template <class Code>
[[nodiscard]] consteval auto diagnosticDomainValid() -> bool {
  constexpr auto domainAnnotations = std::define_static_array(std::meta::annotations_of(^^Code));
  constexpr auto typedAlignments =
      std::define_static_array(std::meta::annotations_of_with_type(^^Code, ^^diagnostics::Alignment));
  usize markers{};
  usize prefixes{};

  template for (constexpr auto annotation : domainAnnotations) {
    if (diagnosticIsMarker(annotation)) {
      ++markers;
    } else if constexpr (diagnosticIsPrefix(annotation)) {
      ++prefixes;
      if constexpr (diagnosticAnnotationText<annotation>().empty()) {
        return false;
      }
    } else if (diagnosticIsMessage(annotation)) {
      return false;
    }
  }

  if (markers + prefixes != 1 or markers > 1 or prefixes > 1 or typedAlignments.size() > 1) {
    return false;
  }
  return typedAlignments.empty() or prefixes != 0;
}

template <class Code>
[[nodiscard]] consteval auto diagnosticEnumeratorsValid() -> bool {
  constexpr auto enumerators = std::define_static_array(std::meta::enumerators_of(^^Code));
  using Underlying = std::underlying_type_t<Code>;

  template for (constexpr auto enumerator : enumerators) {
    constexpr auto annotations = std::define_static_array(std::meta::annotations_of(enumerator));
    usize messages{};
    template for (constexpr auto annotation : annotations) {
      if (diagnosticIsMessage(annotation)) {
        ++messages;
      } else if (diagnosticIsMarker(annotation) or diagnosticIsPrefix(annotation) or
                 diagnosticIsAlignment(annotation)) {
        return false;
      }
    }
    if (messages > 1) {
      return false;
    }

    if constexpr (std::signed_integral<Underlying>) {
      const auto value =
          static_cast<Underlying>(std::meta::extract<Code>(std::meta::constant_of(enumerator)));
      if (value < 0) {
        return false;
      }
    }
  }
  return true;
}

template <class Code>
[[nodiscard]] consteval auto diagnosticCodeValid() -> bool {
  if constexpr (not std::is_enum_v<Code>) {
    return false;
  } else {
    return diagnosticDomainValid<Code>() and diagnosticEnumeratorsValid<Code>();
  }
}

} // namespace Miracle

export namespace Miracle {

/// Describes an enum that forms a valid reflected diagnostic-code domain.
///
/// A domain has exactly one Miracle diagnostic marker (`diagnostics` or `diagnostics::prefix(...)`), optional
/// alignment only when prefixed, and non-negative declared enumerator values. Foreign annotations are ignored
/// so diagnostic domains compose with later annotation systems.
template <class Code>
concept DiagnosticCode = diagnosticCodeValid<std::remove_cvref_t<Code>>();

} // namespace Miracle

namespace Miracle {

template <DiagnosticCode Code>
[[nodiscard]] auto makeDiagnosticCodeDescriptor(Code code) -> DiagnosticCodeDescriptor {
  constexpr auto domainAnnotations = std::define_static_array(std::meta::annotations_of(^^Code));
  constexpr auto typedAlignments =
      std::define_static_array(std::meta::annotations_of_with_type(^^Code, ^^diagnostics::Alignment));
  constexpr auto enumerators = std::define_static_array(std::meta::enumerators_of(^^Code));

  String prefix;
  constexpr usize resolvedAlignment = [] consteval -> usize {
    if constexpr (typedAlignments.empty()) {
      return 0;
    } else {
      constexpr auto annotation = typedAlignments.front();
      return diagnosticAlignmentValue(annotation);
    }
  }();
  usize alignment{resolvedAlignment};
  template for (constexpr auto annotation : domainAnnotations) {
    if constexpr (diagnosticIsPrefix(annotation)) {
      constexpr StringView text = diagnosticAnnotationText<annotation>();
      prefix.assign(text);
    }
  }

  const auto numeric = static_cast<u64>(std::to_underlying(code));

  DiagnosticCodeDescriptor descriptor{
      .domain = String{std::meta::identifier_of(^^Code)},
      .prefix = std::move(prefix),
      .enumerator = "<unknown>",
      .canonicalMessage = "unknown diagnostic code",
      .numeric = numeric,
      .alignment = alignment,
  };

  template for (constexpr auto enumerator : enumerators) {
    constexpr auto declared = std::meta::extract<Code>(std::meta::constant_of(enumerator));
    if (declared == code) {
      descriptor.enumerator = String{std::meta::identifier_of(enumerator)};
      descriptor.canonicalMessage = descriptor.enumerator;
      constexpr auto annotations = std::define_static_array(std::meta::annotations_of(enumerator));
      template for (constexpr auto annotation : annotations) {
        if constexpr (diagnosticIsMessage(annotation)) {
          constexpr StringView text = diagnosticAnnotationText<annotation>();
          descriptor.canonicalMessage.assign(text);
        }
      }
      break;
    }
  }

  return descriptor;
}

} // namespace Miracle

export namespace Miracle {

/// Half-open byte/code-unit selection into explicitly supplied snippet text.
struct SourceRange final {
  /// Inclusive selection start offset.
  usize begin{};
  /// Exclusive selection end offset.
  usize end{};
};

/// Visual role of a diagnostic source span.
enum class SpanRole : u8 {
  /// Principal source selection associated with the diagnostic.
  Primary,
  /// Supporting source selection.
  Secondary,
};

/// Move-only source snippet and selection attached to a diagnostic.
///
/// The renderer never reads `source_location::file_name()`; snippet text must be supplied explicitly through
/// this value. Builder methods consume an rvalue to preserve construction-time mutation semantics.
class DiagnosticSpan final {
public:
  /// Creates a span over explicitly supplied snippet text.
  explicit DiagnosticSpan(String source, std::source_location location = std::source_location::current())
      : source_(std::move(source))
      , location_(location) {
  }

  ~DiagnosticSpan() = default;
  DiagnosticSpan(const DiagnosticSpan &) = delete ("DiagnosticSpan is move-only");
  auto operator=(const DiagnosticSpan &) -> DiagnosticSpan & = delete ("DiagnosticSpan is move-only");
  DiagnosticSpan(DiagnosticSpan &&) noexcept = default;
  auto operator=(DiagnosticSpan &&) noexcept -> DiagnosticSpan & = default;

  /// Replaces the source location associated with this span while building it.
  [[nodiscard]] auto at(this DiagnosticSpan &&self, std::source_location location) -> DiagnosticSpan {
    self.location_ = location;
    return std::move(self);
  }

  /// Selects a half-open source range; rendering clamps both endpoints safely.
  [[nodiscard]] auto select(this DiagnosticSpan &&self, SourceRange selection) -> DiagnosticSpan {
    self.selection_ = selection;
    return std::move(self);
  }

  /// Attaches a human-facing label to this span.
  [[nodiscard]] auto label(this DiagnosticSpan &&self, String label) -> DiagnosticSpan {
    self.label_ = std::move(label);
    return std::move(self);
  }

  /// Sets whether this is a primary or secondary span.
  [[nodiscard]] auto role(this DiagnosticSpan &&self, SpanRole role) -> DiagnosticSpan {
    self.role_ = role;
    return std::move(self);
  }

  /// Returns the explicitly supplied source snippet.
  [[nodiscard]] auto source() const noexcept -> StringView {
    return source_;
  }
  /// Returns the location metadata associated with this span.
  [[nodiscard]] auto location() const noexcept -> std::source_location {
    return location_;
  }
  /// Returns the optional source selection.
  [[nodiscard]] auto selection() const noexcept -> Option<SourceRange> {
    return selection_;
  }
  /// Returns this span's optional label text.
  [[nodiscard]] auto labelText() const noexcept -> StringView {
    return label_;
  }
  /// Returns this span's rendering role.
  [[nodiscard]] auto spanRole() const noexcept -> SpanRole {
    return role_;
  }

private:
  String source_;
  std::source_location location_;
  Option<SourceRange> selection_;
  String label_;
  SpanRole role_{SpanRole::Primary};
};

/// Move-only structured diagnostic with owned runtime presentation data.
///
/// Construction starts from an enum-backed severity factory. Rvalue-only builders attach occurence-specific
/// context without exposing mutable public state after construction.
class Diagnostic final {
public:
  ~Diagnostic() = default;
  Diagnostic(const Diagnostic &) = delete ("Diagnostic is move-only");
  auto operator=(const Diagnostic &) -> Diagnostic & = delete ("Diagnostic is move-only");
  Diagnostic(Diagnostic &&) noexcept = default;
  auto operator=(Diagnostic &&) noexcept -> Diagnostic & = default;

  /// Creates a debug diagnostic using the code's canonical message.
  template <DiagnosticCode Code>
  [[nodiscard]] static auto debug(Code code, std::source_location location = std::source_location::current())
      -> Diagnostic {
    return Diagnostic{Severity::Debug, makeDiagnosticCodeDescriptor(code), location};
  }

  /// Creates a note diagnostic using the code's canonical message.
  template <DiagnosticCode Code>
  [[nodiscard]] static auto note(Code code, std::source_location location = std::source_location::current())
      -> Diagnostic {
    return Diagnostic{Severity::Note, makeDiagnosticCodeDescriptor(code), location};
  }

  /// Creates a warning diagnostic using the code's canonical message.
  template <DiagnosticCode Code>
  [[nodiscard]] static auto warning(Code code,
      std::source_location location = std::source_location::current()) -> Diagnostic {
    return Diagnostic{Severity::Warning, makeDiagnosticCodeDescriptor(code), location};
  }

  /// Creates a error diagnostic using the code's canonical message.
  template <DiagnosticCode Code>
  [[nodiscard]] static auto error(Code code, std::source_location location = std::source_location::current())
      -> Diagnostic {
    return Diagnostic{Severity::Error, makeDiagnosticCodeDescriptor(code), location};
  }

  /// Overrides the canonical code message for this specific occurence.
  [[nodiscard]] auto message(this Diagnostic &&self, String message) -> Diagnostic {
    self.message_ = std::move(message);
    return std::move(self);
  }

  /// Replaces the diagnostic's primary source location.
  [[nodiscard]] auto at(this Diagnostic &&self, std::source_location location) -> Diagnostic {
    self.location_ = location;
    return std::move(self);
  }

  /// Appends a structured source span.
  [[nodiscard]] auto span(this Diagnostic &&self, DiagnosticSpan span) -> Diagnostic {
    self.spans_.push_back(std::move(span));
    return std::move(self);
  }

  /// Appends contextual note text.
  [[nodiscard]] auto note(this Diagnostic &&self, String note) -> Diagnostic {
    self.notes_.push_back(std::move(note));
    return std::move(self);
  }

  /// Appends actionable help text.
  [[nodiscard]] auto help(this Diagnostic &&self, String help) -> Diagnostic {
    self.help_.push_back(std::move(help));
    return std::move(self);
  }

  /// Appends a causal child diagnostic.
  [[nodiscard]] auto cause(this Diagnostic &&self, Diagnostic cause) -> Diagnostic {
    self.causes_.push_back(std::move(cause));
    return std::move(self);
  }

  /// Attaches a captured stacktrace.
  [[nodiscard]] auto trace(this Diagnostic &&self, std::stacktrace trace) -> Diagnostic {
    self.trace_ = std::move(trace);
    return std::move(self);
  }

  /// Returns the diagnostic severity.
  [[nodiscard]] auto severity() const noexcept -> Severity {
    return severity_;
  }
  /// Returns the normalized runtime code descriptor.
  [[nodiscard]] auto code() const noexcept -> const DiagnosticCodeDescriptor & {
    return code_;
  }
  /// Returns the occurence message after any override.
  [[nodiscard]] auto messageText() const noexcept -> StringView {
    return message_;
  }
  /// Returns the diagnostic's primary location metadata.
  [[nodiscard]] auto location() const noexcept -> std::source_location {
    return location_;
  }
  /// Returns attached source spans in insertion order.
  [[nodiscard]] auto spans() const noexcept -> const Vec<DiagnosticSpan> & {
    return spans_;
  }
  /// Returns contextual notes in insertion order.
  [[nodiscard]] auto notes() const noexcept -> const Vec<String> & {
    return notes_;
  }
  /// Returns help messages in insertion order.
  [[nodiscard]] auto helpMessages() const noexcept -> const Vec<String> & {
    return help_;
  }
  /// Returns causal child diagnostics in insertion order.
  [[nodiscard]] auto causes() const noexcept -> const Vec<Diagnostic> & {
    return causes_;
  }
  /// Returns the optional captured stacktrace.
  [[nodiscard]] auto stacktrace() const noexcept -> const Option<std::stacktrace> & {
    return trace_;
  }

private:
  Diagnostic(Severity severity, DiagnosticCodeDescriptor code, std::source_location location)
      : severity_(severity)
      , code_(std::move(code))
      , message_(code_.canonicalMessage)
      , location_(location) {
  }

  Severity severity_;
  DiagnosticCodeDescriptor code_;
  String message_;
  std::source_location location_;
  Vec<DiagnosticSpan> spans_;
  Vec<String> notes_;
  Vec<String> help_;
  Vec<Diagnostic> causes_;
  Option<std::stacktrace> trace_;
};

/// Controls ANSI colour emission.
enum class ColorMode : u8 {
  /// Use presentation policy to decide whether colour in appropriate.
  Automatic,
  /// Request colour for terminal presentation.
  Always,
  /// Suppress ANSI colour unconditionally.
  Never,
};

/// Controls how source-location and explicit snippet data are rendered.
enum class SourceMode : u8 {
  /// Suppress source information.
  None,
  /// Render source-location metadata only.
  Location,
  /// Render locations and explicitly supplied snippet text.
  Snippet,
};

/// Controls optional diagnostic metadata and stacktrace detail.
enum class DetailMode : u8 {
  /// Emit the concise diagnostic tree without traces or metadata.
  Compact,
  /// Include stacktraces when present.
  Normal,
  /// Include stacktraces and normalized code metadata.
  Full,
};

/// Selects plain text versus terminal-oriented presentation.
enum class Presentation : u8 {
  /// Stable text presentation with no terminal escape sequences.
  Plain,
  /// Terminal-oriented presentation eligible for ANSI styling.
  Terminal,
};

/// Rendering policy shared by string and stream output.
struct RenderOptions final {
  /// ANSI colour policy.
  ColorMode color{ColorMode::Automatic};
  /// Source-location/snippet policy.
  SourceMode source{SourceMode::Snippet};
  /// Optional-detail policy.
  DetailMode detail{DetailMode::Normal};
  /// Plain versus terminal presentation policy.
  Presentation presentation{Presentation::Terminal};
};

/// Renders a diagnostic into an owned string.
[[nodiscard]] auto render(const Diagnostic &diagnostic, RenderOptions options = {}) -> String;
/// Renders a diagnostic directly into an output stream.
auto render(const Diagnostic &diagnostic, std::ostream &output, RenderOptions options = {}) -> void;

/// Stable diagnostic codes used by the standardized contracts adapter.
enum class[[ = diagnostics::prefix("CTR"), = diagnostics::align(3) ]] ContractCode : u8 {
  /// Precondition contract violation.
  Precondition = 1,
  /// Postcondition contract violation.
  Postcondition = 2,
  /// Assertion/manual/C-assert or unknown contract violation.
  Assertion = 3,
};

/// Adapts a standardized contract violation into Miracle's diagnostic model.
[[nodiscard]] auto diagnose(const std::contracts::contract_violation &violation) -> Diagnostic;

} // namespace Miracle
