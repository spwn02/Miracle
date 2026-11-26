// TEST-FMT-IMPORT-001, TEST-REC-IMPORT-001: a consumer importing only Miracle.Format compiles (Color arrives
// transitively; StaticString and Diagnostics are not imported).
import Miracle.Format;

static_assert(Miracle::format::severityStyle(Miracle::Severity::Error) != Miracle::format::Style{});

auto main() -> int {
  return 0;
}
