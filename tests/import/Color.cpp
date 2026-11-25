import Miracle.Color;

static_assert(Miracle::Color::rgb(1, 2, 3).toHex() == 0x010203U);

auto main() -> int {
  return 0;
}
