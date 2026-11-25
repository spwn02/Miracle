import Miracle.StaticString;

static_assert(sizeof(Miracle::BasicStaticString<char, 4>) > 0);

auto main() -> int {
  return 0;
}
