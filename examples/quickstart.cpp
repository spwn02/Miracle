import std;
import Miracle;

using namespace Miracle;

auto reciprocal(i32 value) -> Result<f64> {
  if (value == 0)
    return bail({"Cannot divide by zero."});

  return 1.0 / static_cast<f64>(value);
}

auto main() -> int { // NOLINT(bugprone-exception-escape)
  Result<f64> value = reciprocal(4);
  if (not value) {
    std::println(std::cerr, "{}", value.error().display());
    return 1;
  }

  std::println("{}", *value);
  return 0;
}
