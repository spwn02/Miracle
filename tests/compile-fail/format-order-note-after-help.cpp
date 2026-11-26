import Miracle;

using namespace Miracle;

namespace {
constexpr auto base = format::Message{Severity::Error, format::code("E", 1), "boom"};

constexpr auto bad = base.help("h").note("n");
}

auto main() -> int {
  return 0;
}
