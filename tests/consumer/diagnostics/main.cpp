import std;
import Miracle.Diagnostics;

using namespace Miracle;

enum class[[ = diagnostics::prefix("E"), = diagnostics::align(3) ]] ConsumerCode : std::uint8_t {
  Broken[[= diagnostics::message("broken input")]] = 7,
};

// Severity/ColorMode/Presentation have a single definition shared with Format.
static_assert(std::is_same_v<decltype(Diagnostic::error(ConsumerCode::Broken).severity()), Severity>);
static_assert(std::is_same_v<decltype(RenderOptions{}.color), ColorMode>);
static_assert(std::is_same_v<decltype(RenderOptions{}.presentation), Presentation>);
static_assert(format::severityStyle(Severity::Error) == format::severityStyle(Severity::Error));

auto main() -> int {
  const auto diagnostic =
      Diagnostic::error(ConsumerCode::Broken)
          .span(DiagnosticSpan{"name = ?"}.select({.begin = 7, .end = 8}).label("expected a value"))
          .note("context")
          .help("provide a value")
          .cause(Diagnostic::note(ConsumerCode::Broken).message("inner"));

  const std::string plain = render(diagnostic,
      {.color = ColorMode::Always,
          .source = SourceMode::Snippet,
          .detail = DetailMode::Normal,
          .presentation = Presentation::Plain});
  const std::string colored = render(diagnostic,
      {.color = ColorMode::Always,
          .source = SourceMode::Snippet,
          .detail = DetailMode::Normal,
          .presentation = Presentation::Terminal});

  const bool ok = plain.starts_with("error[E007]: broken input\n") and not plain.contains("\033[") and
                  plain.contains("^ expected a value") and plain.contains("note: context") and
                  plain.contains("help: provide a value") and plain.contains("caused by:") and
                  colored.contains("\033[") and colored.contains("\033[0m");
  return ok ? 0 : 1;
}
