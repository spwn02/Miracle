import std;
import Miracle;

using namespace Miracle;

// TEST-FMT-OVERFLOW-001: a type-level capacity of SIZE_MAX would wrap `Capacity + 1` to an empty buffer.
constexpr auto size = sizeof(format::Message<std::numeric_limits<std::size_t>::max(), 0>);

auto main() -> int {
  return 0;
}
