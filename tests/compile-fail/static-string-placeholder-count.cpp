import Miracle;

using namespace Miracle;

// TEST-STR-NEG-002: placeholder/argument count mismatch reports the authored message.
constexpr auto text = staticFormat<"{} {}">(1);

auto main() -> int {
  return 0;
}
