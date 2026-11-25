import Miracle;

using namespace Miracle;

enum class[[= diagnostics::prefix("")]] EmptyPrefix : u8 {
  Value = 1,
};

static_assert(DiagnosticCode<EmptyPrefix>);

auto main() -> int {
  return 0;
}
