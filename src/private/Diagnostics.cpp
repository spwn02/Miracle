module Miracle.Diagnostics;

namespace Miracle {

namespace {

using format::Style;
using format::TerminalColor;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
constexpr Style sourceStyle{.foreground = TerminalColor::indexed(12), .bold = true};
constexpr Style auxiliaryStyle{.foreground = TerminalColor::indexed(14), .bold = true};
constexpr Style helpStyle{.foreground = TerminalColor::indexed(10), .bold = true};
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

/// Byte destination over a standard stream, adapting it to Format's `TextSink` protocol.
class StreamSink final {
public:
  explicit StreamSink(std::ostream &output) noexcept
      : output_(output) {
  }

  auto write(const StringView text) -> bool {
    output_.get().write(text.data(), static_cast<std::streamsize>(text.size()));
    return static_cast<bool>(output_.get());
  }

private:
  std::reference_wrapper<std::ostream> output_;
};

[[nodiscard]] constexpr auto spanMarker(SpanRole role) noexcept -> char {
  return role == SpanRole::Primary ? '^' : '-';
}

[[nodiscard]] constexpr auto spanStyle(SpanRole role, Severity severity) noexcept -> Style {
  return role == SpanRole::Primary ? format::severityStyle(severity) : auxiliaryStyle;
}

[[nodiscard]] auto displayPath(std::source_location location) -> String {
  const char *fileName = location.file_name();
  if (fileName == nullptr or *fileName == '\0') {
    return {};
  }

  const Path file{fileName};
  if (not file.is_absolute()) {
    return file.generic_string();
  }

  std::error_code error;
  const Path cwd = std::filesystem::current_path(error);
  if (error) {
    return file.generic_string();
  }

  const Path relative = file.lexically_relative(cwd);
  if (relative.empty()) {
    return file.generic_string();
  }

  const Path::iterator first = relative.begin();
  if (first != relative.end() and *first == Path{".."}) {
    return file.generic_string();
  }

  return relative.generic_string();
}

[[nodiscard]] auto semanticName(std::contracts::evaluation_semantic semantic) -> StringView {
  using Semantic = std::contracts::evaluation_semantic;
  switch (semantic) {
    case Semantic::enforce: return "enforce";
    case Semantic::observe: return "observe";
    case Semantic::ignore: return "ignore";
    case Semantic::quick_enforce: return "quick_enforce";
  }
  return "unknown";
}

[[nodiscard]] auto detectionName(std::contracts::detection_mode mode) -> StringView {
  using Mode = std::contracts::detection_mode;
  switch (mode) {
    case Mode::predicate_false: return "predicate_false";
    case Mode::evaluation_exception: return "evaluation_exception";
  }
  return "unspecified";
}

class DiagnosticRenderer final {
public:
  DiagnosticRenderer(std::ostream &output, RenderOptions options)
      : sink_(output)
      , state_{.policy = {.presentation = options.presentation, .color = options.color}}
      , writer_(sink_, state_)
      , options_(options) {
  }

  auto render(const Diagnostic &diagnostic) -> void {
    renderOne(diagnostic, 0, false);
    renderCauses(diagnostic);
    // Console style must be back to default after every diagnostic tree.
    auto _ = writer_.finish();
  }

private:
  struct TraversalFrame final {
    const Diagnostic *parent{};
    usize nextChild{};
    usize depth{};
  };

  /// Writes `text` under `style`; the shared writer owns SGR encoding and plain/terminal suppression.
  auto put(const StringView text, const Style &style = {}) -> void {
    auto _ = writer_.put(text, style);
  }

  /// Untrusted runtime text (messages, notes, labels, paths, source): control bytes are escaped, never emitted raw.
  auto putText(const StringView text, const Style &style = {}) -> void {
    auto _ = format::writeEscaped(writer_, text, style);
  }

  auto putNumber(const u64 value, const Style &style = {}) -> void {
    put(std::to_string(value), style);
  }

  auto indent(usize depth) -> void {
    put(String(depth * 2, ' '));
  }

  auto renderHeader(const Diagnostic &diagnostic, usize depth) -> void {
    indent(depth);
    const Style style = format::severityStyle(diagnostic.severity());
    put(severityName(diagnostic.severity()), style);
    auto _ = format::writeCode(writer_, diagnostic.code().codeView(), style);
    put(": ");
    putText(diagnostic.messageText());
    put("\n");
  }

  auto renderLocation(std::source_location location, usize depth) -> void {
    if (options_.source == SourceMode::None) {
      return;
    }
    indent(depth);
    put("--> ", sourceStyle);
    putText(displayPath(location));
    put(":");
    putNumber(location.line());
    put(":");
    putNumber(location.column());
    put("\n");
  }

  auto renderGutter(usize depth) -> void {
    indent(depth);
    put(" | ", sourceStyle);
  }

  auto renderSnippet(const DiagnosticSpan &span, Severity severity, usize depth) -> void {
    const StringView source = span.source();
    if (source.empty()) {
      return;
    }

    usize begin{};
    usize end{};
    if (const auto selection = span.selection()) {
      begin = std::min(selection->begin, source.size());
      end = std::min(selection->end, source.size());
      if (end < begin) {
        std::swap(begin, end);
      }
    }

    const usize lineStart = [&] -> usize {
      if (begin == 0) {
        return usize{};
      }
      const usize previous = source.rfind('\n', begin - 1);
      return previous == StringView::npos ? usize{} : previous + 1;
    }();
    const usize lineEndFound = source.find('\n', begin);
    const usize lineEnd = lineEndFound == StringView::npos ? source.size() : lineEndFound;
    const StringView line = source.substr(lineStart, lineEnd - lineStart);

    renderGutter(depth);
    putText(line);
    put("\n");

    const Style marker = spanStyle(span.spanRole(), severity);
    if (span.selection()) {
      const usize markerBegin = std::min(begin, lineEnd) - lineStart;
      const usize markerEnd = std::min(std::max(end, begin + 1UZ), lineEnd) - lineStart;
      renderGutter(depth);
      put(String(format::escapedSize(line.substr(0, markerBegin)), ' '));
      const usize markerLength =
          std::max(1UZ, markerEnd > markerBegin ? format::escapedSize(line.substr(markerBegin, markerEnd - markerBegin)) : 1);
      put(String(markerLength, spanMarker(span.spanRole())), marker);
      if (not span.labelText().empty()) {
        put(" ", marker);
        putText(span.labelText(), marker);
      }
      put("\n");
    } else if (not span.labelText().empty()) {
      renderGutter(depth);
      putText(span.labelText(), marker);
      put("\n");
    }
  }

  auto renderSpans(const Diagnostic &diagnostic, usize depth) -> void {
    for (const DiagnosticSpan &span : diagnostic.spans()) {
      renderLocation(span.location(), depth);
      if (options_.source == SourceMode::Snippet) {
        renderSnippet(span, diagnostic.severity(), depth);
      }
    }
  }

  auto renderLabelled(const StringView label, const Style &style, const String &text, usize depth) -> void {
    indent(depth);
    put(label, style);
    put(" ");
    putText(text);
    put("\n");
  }

  auto renderNotes(const Diagnostic &diagnostic, usize depth) -> void {
    for (const String &note : diagnostic.notes()) {
      renderLabelled("note:", auxiliaryStyle, note, depth);
    }
    for (const String &help : diagnostic.helpMessages()) {
      renderLabelled("help:", helpStyle, help, depth);
    }
  }

  auto renderTrace(const Diagnostic &diagnostic, usize depth) -> void {
    if (options_.detail == DetailMode::Compact or not diagnostic.stacktrace()) {
      return;
    }
    indent(depth);
    put("stacktrace:", auxiliaryStyle);
    put("\n");
    std::istringstream input{std::to_string(*diagnostic.stacktrace())};
    String line;
    while (std::getline(input, line)) {
      indent(depth + 1);
      putText(line);
      put("\n");
    }
  }

  auto renderMetadata(const Diagnostic &diagnostic, usize depth) -> void {
    if (options_.detail != DetailMode::Full) {
      return;
    }
    indent(depth);
    put("diagnostic:", auxiliaryStyle);
    put(" ");
    putText(diagnostic.code().domain);
    put("::");
    putText(diagnostic.code().enumerator);
    put(" (");
    putNumber(diagnostic.code().numeric);
    put(")\n");
  }

  auto renderOne(const Diagnostic &diagnostic, usize depth, bool cause) -> void {
    if (cause) {
      indent(depth);
      put("caused by:", auxiliaryStyle);
      put("\n");
      ++depth;
    }
    renderHeader(diagnostic, depth);
    // Explicit spans are the authoritative source anchors. Rendering the diagnostic construction location as
    // well would duplicate the path and often point at the builder call rather than the highlighted source.
    if (diagnostic.spans().empty()) {
      renderLocation(diagnostic.location(), depth);
    }
    renderSpans(diagnostic, depth);
    renderNotes(diagnostic, depth);
    renderTrace(diagnostic, depth);
    renderMetadata(diagnostic, depth);
  }

  auto renderCauses(const Diagnostic &root) -> void {
    if (root.causes().empty()) {
      return;
    }

    // Parent/index frames implement preorder DFS. Only one frame per active ancestor is retained, so
    // traversal is O(n) time, O(depth) auxiliary storage, and consumes no native call-stack recursion.
    Vec<TraversalFrame> stack;
    stack.push_back(TraversalFrame{.parent = &root, .nextChild = 0, .depth = 0});

    while (not stack.empty()) {
      auto &frame = stack.back();
      if (frame.nextChild >= frame.parent->causes().size()) {
        stack.pop_back();
        continue;
      }

      const Diagnostic &child = frame.parent->causes().at(frame.nextChild++);
      const usize childDepth = frame.depth + 1;
      renderOne(child, childDepth - 1, true);
      if (not child.causes().empty()) {
        stack.push_back(TraversalFrame{.parent = &child, .nextChild = 0, .depth = childDepth});
      }
    }
  }

  StreamSink sink_;
  format::RenderState state_;
  format::AnsiWriter<StreamSink> writer_;
  RenderOptions options_;
};

[[nodiscard]] auto contractCode(std::contracts::assertion_kind kind) -> ContractCode {
  using Kind = std::contracts::assertion_kind;
  switch (kind) {
    case Kind::pre: return ContractCode::Precondition;
    case Kind::post: return ContractCode::Postcondition;
    case Kind::assert: return ContractCode::Assertion;
  }
  return ContractCode::Assertion;
}

} // namespace

[[nodiscard]] auto render(const Diagnostic &diagnostic, RenderOptions options) -> String {
  std::ostringstream output;
  DiagnosticRenderer renderer{output, options};
  renderer.render(diagnostic);
  return output.str();
}

auto render(const Diagnostic &diagnostic, std::ostream &output, RenderOptions options) -> void {
  DiagnosticRenderer renderer{output, options};
  renderer.render(diagnostic);
}

[[nodiscard]] auto diagnose(const std::contracts::contract_violation &violation) -> Diagnostic {
  auto diagnostic = Diagnostic::error(contractCode(violation.kind()), violation.location());
  if (const char *comment = violation.comment(); comment != nullptr) {
    const StringView commentText{comment};
    if (not commentText.empty()) {
      diagnostic = std::move(diagnostic).message(String{commentText});
    }
  }

  diagnostic = std::move(diagnostic)
                   .note(std::format("semantic: {}", semanticName(violation.semantic())))
                   .note(std::format("detection: {}", detectionName(violation.detection_mode())))
                   .note(std::format("terminating: {}", violation.is_terminating() ? "true" : "false"));
  return diagnostic;
}

} // namespace Miracle
