module Miracle;

import std;
import :Types;
import :Diagnostic;

namespace Miracle {

namespace {

[[nodiscard]] constexpr auto severityName(Severity severity) noexcept -> StringView {
  switch (severity) {
    case Severity::Debug: return "debug";
    case Severity::Note: return "note";
    case Severity::Warning: return "warning";
    case Severity::Error: return "error";
  }
  return "error";
}

[[nodiscard]] constexpr auto severityAnsi(Severity severity) noexcept -> StringView {
  switch (severity) {
    case Severity::Debug: return "\033[36m";
    case Severity::Note: return "\033[34m";
    case Severity::Warning: return "\033[33m";
    case Severity::Error: return "\033[31m";
  }
  return "\033[31m";
}

[[nodiscard]] constexpr auto spanMarker(SpanRole role) noexcept -> char {
  return role == SpanRole::Primary ? '^' : '-';
}

[[nodiscard]] auto semanticName(std::contracts::evaluation_semantic semantic) -> StringView {
  using Semantic = std::contracts::evaluation_semantic;
  switch (semantic) {
    case Semantic::enforce: return "enforce";
    case Semantic::observe: return "observe";
    case Semantic::__unknown: return "unknown";
  }
  return "unknown";
}

[[nodiscard]] auto detectionName(std::contracts::detection_mode mode) -> StringView {
  using Mode = std::contracts::detection_mode;
  switch (mode) {
    case Mode::unspecified: return "unspecified";
    case Mode::predicate_false: return "predicate_false";
    case Mode::evaluation_exception: return "evaluation_exception";
  }
  return "unspecified";
}

class DiagnosticRenderer final {
public:
  DiagnosticRenderer(std::ostream &output, RenderOptions options)
      : output_(output)
      , options_(options) {
  }

  auto render(const Diagnostic &diagnostic) -> void {
    renderOne(diagnostic, 0, false);
    renderCauses(diagnostic);
  }

private:
  struct TraversalFrame final {
    const Diagnostic *parent{};
    usize nextChild{};
    usize depth{};
  };

  [[nodiscard]] auto coloursEnabled() const noexcept -> bool {
    if (options_.presentation == Presentation::Plain or options_.color == ColorMode::Never) {
      return false;
    }
    return options_.color == ColorMode::Always or options_.color == ColorMode::Automatic;
  }

  auto indent(usize depth) -> void {
    for (usize index{}; index < depth * 2; ++index) {
      output().put(' ');
    }
  }

  auto renderCode(const DiagnosticCodeDescriptor &code) -> void {
    if (code.prefix.empty()) {
      return;
    }
    output().put('[');
    output() << code.prefix;
    if (code.alignment == 0) {
      output() << code.numeric;
    } else {
      output() << std::format("{:0{}}", code.numeric, code.alignment);
    }
    output().put(']');
  }

  auto renderHeader(const Diagnostic &diagnostic, usize depth) -> void {
    indent(depth);
    if (coloursEnabled()) {
      output() << severityAnsi(diagnostic.severity());
    }
    output() << severityName(diagnostic.severity());
    renderCode(diagnostic.code());
    if (coloursEnabled()) {
      output() << "\033[0m";
    }
    output() << ": " << diagnostic.messageText() << '\n';
  }

  auto renderLocation(std::source_location location, usize depth) -> void {
    if (options_.source == SourceMode::None) {
      return;
    }
    indent(depth);
    output() << "--> " << location.file_name() << ':' << location.line() << ':' << location.column() << '\n';
  }

  auto renderSnippet(const DiagnosticSpan &span, usize depth) -> void {
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
    const StringView line = source.substr(lineStart, lineEndFound - lineStart);

    indent(depth);
    output() << " | " << line << '\n';

    if (span.selection()) {
      const usize markerBegin = std::min(begin, lineEnd) - lineStart;
      const usize markerEnd = std::min(std::max(end, begin + 1UZ), lineEnd) - lineStart;
      indent(depth);
      output() << " | ";
      for (usize index{}; index < markerBegin; ++index) {
        output().put(' ');
      }
      const usize markerLength = std::max(1UZ, markerEnd > markerBegin ? markerEnd - markerBegin : 1);
      for (usize index{}; index < markerLength; ++index) {
        output().put(spanMarker(span.spanRole()));
      }
      if (not span.labelText().empty()) {
        output() << ' ' << span.labelText();
      }
      output().put('\n');
    } else if (not span.labelText().empty()) {
      indent(depth);
      output() << " | " << span.labelText() << '\n';
    }
  }

  auto renderSpans(const Diagnostic &diagnostic, usize depth) -> void {
    for (const auto &span : diagnostic.spans()) {
      renderLocation(span.location(), depth);
      if (options_.source == SourceMode::Snippet) {
        renderSnippet(span, depth);
      }
    }
  }

  auto renderNotes(const Diagnostic &diagnostic, usize depth) -> void {
    for (const String &note : diagnostic.notes()) {
      indent(depth);
      output() << "note: " << note << '\n';
    }
    for (const String &help : diagnostic.helpMessages()) {
      indent(depth);
      output() << "help: " << help << '\n';
    }
  }

  auto renderTrace(const Diagnostic &diagnostic, usize depth) -> void {
    if (options_.detail == DetailMode::Compact or not diagnostic.stacktrace()) {
      return;
    }
    indent(depth);
    output() << "stacktrace:\n";
    std::istringstream input{std::to_string(*diagnostic.stacktrace())};
    String line;
    while (std::getline(input, line)) {
      indent(depth + 1);
      output() << line << '\n';
    }
  }

  auto renderMetadata(const Diagnostic &diagnostic, usize depth) -> void {
    if (options_.detail != DetailMode::Full) {
      return;
    }
    indent(depth);
    output() << "diagnostic: " << diagnostic.code().domain << "::" << diagnostic.code().enumerator << " ("
             << diagnostic.code().numeric << ")\n";
  }

  auto renderOne(const Diagnostic &diagnostic, usize depth, bool cause) -> void {
    if (cause) {
      indent(depth);
      output() << "caused by:\n";
      ++depth;
    }
    renderHeader(diagnostic, depth);
    renderLocation(diagnostic.location(), depth);
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

  [[nodiscard]] auto output() noexcept -> std::ostream & {
    return output_.get();
  }

  std::reference_wrapper<std::ostream> output_;
  RenderOptions options_;
};

[[nodiscard]] auto contractCode(std::contracts::assertion_kind kind) -> ContractCode {
  using Kind = std::contracts::assertion_kind;
  switch (kind) {
    case Kind::pre: return ContractCode::Precondition;
    case Kind::post: return ContractCode::Postcondition;
    case Kind::assert:
    case Kind::manual:
    case Kind::cassert:
    case Kind::__unknown: return ContractCode::Assertion;
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
