// TEST-FMT-ORDER-004: context after note is rejected.
import Miracle;

using namespace Miracle;

namespace {
constexpr auto base = format::Message{Severity::Error, format::code("E", 1), "boom"};

constexpr auto bad = base.note("n").context("c");
}

auto main() -> int {
  return 0;
}
