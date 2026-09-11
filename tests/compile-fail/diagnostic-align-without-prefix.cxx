import Miracle;

using namespace Miracle;

enum class[[= diagnostics::align(3)]] InvalidAlignment : u8 {
  Value = 1,
};

static_assert(DiagnosticCode<InvalidAlignment>);

auto main() -> int {
  return 0;
}
