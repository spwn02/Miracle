import Miracle;

using namespace Miracle;

// FEA008: a build-feature set cannot name a feature twice.
inline constexpr auto duplicated = feature::buildSet<"gpu", "gpu">;

auto main() -> int {
  return static_cast<int>(duplicated.size());
}
