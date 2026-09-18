import Miracle;

using namespace Miracle;

struct Point final {
  int x, y;
};

auto main() -> int {
  Point values[]{{1, 2}};
  iter(values).forEach([](int, int, int) -> void {});
}
