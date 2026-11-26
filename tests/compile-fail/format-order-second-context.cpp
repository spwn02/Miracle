// TEST-FMT-ORDER-005: a second context is rejected.
import Miracle;

using namespace Miracle;

namespace {
constexpr auto base = format::Message{Severity::Error, format::code("E", 1), "boom"};

constexpr auto bad = base.context("a").context("b");
}

auto main() -> int {
  return 0;
}
