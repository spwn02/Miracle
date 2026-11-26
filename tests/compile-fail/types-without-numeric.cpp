// Pins the REQ-MOD-004 contract: Miracle.Types imports Miracle.Numeric but does not re-export it, so a consumer
// of Types alone must import Miracle.Numeric (or the umbrella) to name `Miracle::usize`.
import Miracle.Types;

auto size() -> Miracle::usize {
  return 0;
}

auto main() -> int {
  return static_cast<int>(size());
}
