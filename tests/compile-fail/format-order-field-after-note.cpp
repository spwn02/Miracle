import Miracle;

using namespace Miracle;

namespace {
constexpr auto base = format::Message{Severity::Error, format::code("E", 1), "boom"};

constexpr auto bad = base.note("n").field("k", "v");
}

auto main() -> int {
  return 0;
}
