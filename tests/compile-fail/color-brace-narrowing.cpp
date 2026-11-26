import Miracle;

using namespace Miracle;

// TEST-COL-RGB-004 / RISK-005: `Color::rgb` takes typed channels and does not validate implicit conversions, but
// aggregate brace initialization still rejects a constant that does not fit a channel.
constexpr Color color{300, 0, 0};

auto main() -> int {
  return 0;
}
