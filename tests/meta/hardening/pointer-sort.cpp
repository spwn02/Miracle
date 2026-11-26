import std;
import Miracle.Meta;
constexpr int first = 1, second = 2;
constexpr std::array pointers{&second, &first};
constexpr std::array nullable{&second, static_cast<const int *>(nullptr), &first};
consteval bool test() {
  auto result = Miracle::meta::query(pointers)
                    .map([](const int *p) consteval -> const int * { return p; })
                    .sortBy([](const int *a, const int *b) consteval { return *a < *b; });
  return result.first().value() == &first;
}
static_assert(test());
consteval bool nullPointers() {
  auto result = Miracle::meta::query(nullable)
                    .map([](const int *p) consteval -> const int * { return p; })
                    .sortBy([](const int *a, const int *b) consteval {
                      return a == nullptr ? b != nullptr : b != nullptr && *a < *b;
                    })
                    .materialize();
  return result[0] == nullptr && result[1] == &first && result[2] == &second;
}
static_assert(nullPointers());

auto main() -> int {
}
