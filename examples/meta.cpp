import std;
import Miracle;

using namespace Miracle;

namespace {

struct Transform final {
  float x;
  float y;
  float z;
};

enum class Axis : u8 {
  X,
  Y,
  Z,
};

constexpr std::span<const meta::Info> transformFields = reflect<Transform>().fields();
constexpr std::span<const meta::Info> axisValues = reflect<Axis>().enumerators();

/// Query can filter a reflected source while keeping the whole calculation at compile time.
static_assert(meta::query(transformFields).filter(meta::isField).count() == 3);
static_assert(axisValues.size() == 3);

} // namespace

auto main() -> int { // NOLINT
  std::println("Transform fields:");
  // Each expansion has a compile-time reflection value; its generated statement runs at runtime.
  template for (constexpr meta::Info field : transformFields) {
    std::println("  {}: {}", meta::requireName(field), meta::displayName(meta::type(field)));
  }

  std::println("Axes:");
  template for (constexpr meta::Info value : axisValues) {
    std::println("  {}", meta::requireName(value));
  }
}
