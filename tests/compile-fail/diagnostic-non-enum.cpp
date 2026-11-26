import Miracle;

using namespace Miracle;

struct NotAnEnum {
  int value;
};

static_assert(DiagnosticCode<NotAnEnum>);

auto main() -> int {
  return 0;
}
