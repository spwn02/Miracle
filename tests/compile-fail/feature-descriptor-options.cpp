import Miracle;

using namespace Miracle;

// FEA009: a Feature may carry at most one description option.
inline constexpr auto twice = feature::define<"twice">(feature::description<"first">, feature::description<"second">);

auto main() -> int {
  return static_cast<int>(sizeof(twice));
}
