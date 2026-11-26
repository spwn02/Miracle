// TEST-REST-004: Bitflags is usable through its direct import.
import Miracle.Bitflags;

enum class[[= Miracle::bitflags]] Probe : unsigned char {
  A = 1,
  B = 2,
};

static_assert(Miracle::all<Probe>() == static_cast<Probe>(3));

auto main() -> int {
  return 0;
}
