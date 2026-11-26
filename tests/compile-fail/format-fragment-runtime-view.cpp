import std;
import Miracle;

using namespace Miracle;

// §5.4 negative space: `fragment` exposes only literal and std::array overloads, never a runtime view.
auto runtimeFragment(const std::string_view text) {
  return format::fragment(text);
}

auto main() -> int {
  return 0;
}
