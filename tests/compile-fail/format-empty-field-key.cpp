// TEST-FMT-ORDER-007: an empty field key is rejected, so no malformed `  : value` line can be produced.
import Miracle;

using namespace Miracle;

namespace {
constexpr auto base = format::Message{Severity::Error, format::code("E", 1), "boom"};

constexpr auto bad = base.field("", "v");
}

auto main() -> int {
  return 0;
}
