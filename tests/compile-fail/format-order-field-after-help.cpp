// TEST-FMT-ORDER-003
import Miracle;

using namespace Miracle;

// INV-SEC-001: sections only advance; a field after help is rejected by constraints, not rearranged.
constexpr auto message =
    format::Message{Severity::Error, format::code<3>("ITR", 4), "x"}.help("try again").field("key", "value");

auto main() -> int {
  return 0;
}
