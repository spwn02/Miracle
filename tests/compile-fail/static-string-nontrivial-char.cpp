import Miracle;

using namespace Miracle;

// TEST-STR-NEG-003: a non-trivial character type is rejected at the BasicStaticString instantiation boundary.
struct NonTrivial final {
  NonTrivial() {}
};
constexpr auto size = sizeof(BasicStaticString<NonTrivial, 1>);

auto main() -> int {
  return 0;
}
