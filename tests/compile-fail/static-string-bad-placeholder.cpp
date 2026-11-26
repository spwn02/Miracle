import Miracle;

using namespace Miracle;

// TEST-STR-NEG-001: only '{}' placeholders are supported; '{x}' is rejected before any buffer is sized.
constexpr auto text = staticFormat<"value {x}">(1);

auto main() -> int {
  return 0;
}
