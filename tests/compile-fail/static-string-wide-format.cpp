import Miracle;

using namespace Miracle;

// REQ-STR-002: staticFormat requires a char format string.
constexpr auto text = staticFormat<u"{}">(1);

auto main() -> int {
  return 0;
}
