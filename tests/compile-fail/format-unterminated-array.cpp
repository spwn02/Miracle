import Miracle;

using namespace Miracle;

// INV-REC-CAP-003: a `char[N]` that is not NUL-terminated is a byte buffer, not a string literal. Taking N - 1
// bytes would silently drop its last byte, so the literal-style overloads reject it.
constexpr char bytes[3] = {'a', 'b', 'c'};
constexpr auto message = format::Message{bytes};

auto main() -> int {
  return 0;
}
