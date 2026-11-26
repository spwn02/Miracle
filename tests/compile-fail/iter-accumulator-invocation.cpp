import Miracle;

using namespace Miracle;

// ITR002: an accumulator callable must accept the accumulator followed by the item or its decomposed fields.
auto main() -> int {
  const int values[]{1, 2, 3};
  static_cast<void>(iter(values).fold(0, [](int, int, int, int) -> int { return 0; }));
}
