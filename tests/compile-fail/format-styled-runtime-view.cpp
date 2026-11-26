import std;
import Miracle;

using namespace Miracle;

// §5.4 negative space: a runtime std::string_view has no compile-time extent, so it cannot become an owning
// Fragment; dynamic text goes through the event sink instead.
auto runtimeFragment(const std::string_view text) {
  return format::styled(text, format::Style{});
}

auto main() -> int {
  return 0;
}
