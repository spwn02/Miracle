import Miracle;

using namespace Miracle;

enum class[[= diagnostics::prefix("E")]] NegativeCode : i8 {
  Invalid = -1,
};

static_assert(DiagnosticCode<NegativeCode>);

auto main() -> int {
  return 0;
}
