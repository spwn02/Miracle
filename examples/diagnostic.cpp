import std;
import Miracle;

using namespace Miracle;

enum class[[ = diagnostics::prefix("E"), = diagnostics::align(3) ]] ExampleCode : u8 {
  InvalidInput[[= diagnostics::message("invalid input")]] = 7,
};

// The example intentionally uses allocating standard-library formatting and lets process-level exception
// policy handle allocation/stream failures.
// NOLINTNEXTLINE(bugprone-exception-escape)
auto main() -> int {
  const auto diagnostic =
      Diagnostic::error(ExampleCode::InvalidInput)
          .span(DiagnosticSpan{"name = ?"}.select({.begin = 7, .end = 8}).label("expected a value"))
          .help("provide a value after '='");

  std::cout << render(diagnostic,
      {.color = ColorMode::Never,
          .source = SourceMode::Snippet,
          .detail = DetailMode::Normal,
          .presentation = Presentation::Plain});
}
