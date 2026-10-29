import std;
import Miracle.Meta;

using namespace Miracle::meta;

namespace {

consteval auto nestedCapturedQueryStaysAliveDuringFlatten() -> bool {
  std::array limits{2, 3};
  std::array values{1, 2, 3, 4};
  std::size_t sum{};

  auto flattened = query(limits).flatMap([values](int limit) consteval -> auto {
    return query(values).filter([limit](const int &value) consteval -> bool { return value <= limit; });
  });
  flattened.forEach([&](const int &value) consteval -> void { sum += static_cast<std::size_t>(value); });
  constexpr std::size_t expectedCount{2 + 3};
  constexpr std::size_t expectedSum{1 + 2 + 1 + 2 + 3};
  return flattened.count() == expectedCount && sum == expectedSum;
}

static_assert(nestedCapturedQueryStaysAliveDuringFlatten());

consteval auto temporaryRangeFlattenOwnsElements() -> bool {
  std::array source{1, 2};
  constexpr int offset{10};
  auto flattened =
      query(source).flatMap([](int value) consteval -> auto { return std::array{value, value + offset}; });
  static_assert(std::same_as<decltype(flattened)::Value, int>);
  const auto values = flattened.materialize();
  return values.size() == 4 && values.at(0) == 1 && values.at(1) == 1 + offset && values.at(2) == 2 &&
         values.at(3) == 2 + offset;
}

static_assert(temporaryRangeFlattenOwnsElements());

} // namespace
