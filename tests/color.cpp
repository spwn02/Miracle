import std;

import Switch;

import Miracle;

using namespace Miracle;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace colorTests {

[[= Switch::test]] auto parseHex() -> void {
  Switch::check(Color::parseHex("#F00FFF") == Color::rgb(240, 15, 255));
  Switch::check(Color::parseHex("#FF0") == Color::rgb(255, 15, 240));
  Switch::check(Color::parseHex("#FF0")->toHex() == 0xFF0FF0U);

  Hsv hsv{.hueDegrees = 0, .saturation = 1, .value = 1};
  Switch::check(Color::fromHsv(hsv)->toHsv() == hsv);
  Switch::check(Color::fromHsv(hsv)->toHex() == 0xFF0000U);
}

} // namespace colorTests
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  Switch::discover<^^colorTests>();
}
