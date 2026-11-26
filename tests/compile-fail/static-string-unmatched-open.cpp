import Miracle;

using namespace Miracle;

// TEST-STR-NEG-001: an unmatched '{' is a grammar failure.
constexpr auto text = staticFormat<"value {">();

auto main() -> int {
  return 0;
}
