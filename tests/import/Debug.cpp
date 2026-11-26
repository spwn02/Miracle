// TEST-REST-004: Debug is usable through its direct import.
import Miracle.Debug;

static_assert(sizeof(Miracle::debug::DebugRename<1>) > 0);

auto main() -> int {
  return 0;
}
