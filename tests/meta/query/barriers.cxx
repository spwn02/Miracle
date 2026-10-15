import std;
import Miracle.Meta;

using namespace Miracle::meta;

namespace {

constexpr std::array kValues{3, 1, 3, 2, 1};
constexpr std::array kBoolValues{true, false, true};

struct MoveAware final {
  int value{};

  constexpr MoveAware() = default;

  constexpr explicit MoveAware(int input)
      : value(input) {
  }

  constexpr MoveAware(const MoveAware &) = default;
  constexpr auto operator=(const MoveAware &) -> MoveAware & = default;

  constexpr MoveAware(MoveAware &&other) noexcept
      : value(other.value) {
    other.value = -1;
  }

  constexpr auto operator=(MoveAware &&other) noexcept -> MoveAware & {
    value = other.value;
    other.value = -1;
    return *this;
  }

  constexpr ~MoveAware() = default;
};

static_assert(std::meta::is_structural_type(^^MoveAware));

consteval auto reversePreservesReferenceIdentity() -> bool {
  constexpr std::array<std::size_t, 5> expected{4, 3, 2, 1, 0};
  std::size_t index{};
  bool exact{true};

  query(kValues).reverse().forEach([&](const int &value) consteval -> void {
    exact = exact && std::addressof(value) == std::addressof(kValues.at(expected.at(index)));
    ++index;
  });
  return exact && index == expected.size();
}

static_assert(reversePreservesReferenceIdentity());

consteval auto sortIsAscendingAndPreservesReferenceIdentity() -> bool {
  constexpr std::array<const int *, 5> expected{std::addressof(kValues.at(1)),
      std::addressof(kValues.at(4)),
      std::addressof(kValues.at(3)),
      std::addressof(kValues.at(0)),
      std::addressof(kValues.at(2))};
  std::size_t index{};
  bool exact{true};

  query(kValues).sort().forEach([&](const int &value) consteval -> void {
    exact = exact && std::addressof(value) == expected.at(index);
    ++index;
  });
  return exact && index == expected.size();
}

static_assert(sortIsAscendingAndPreservesReferenceIdentity());

consteval auto boolSortPreservesReferenceIdentity() -> bool {
  constexpr std::array<const bool *, 3> expected{std::addressof(kBoolValues.at(1)),
      std::addressof(kBoolValues.at(0)),
      std::addressof(kBoolValues.at(2))};
  std::size_t index{};
  bool exact{true};

  query(kBoolValues).sort().forEach([&](const bool &value) consteval -> void {
    exact = exact && std::addressof(value) == expected.at(index);
    ++index;
  });
  return exact && index == expected.size();
}

static_assert(boolSortPreservesReferenceIdentity());

consteval auto uniqueKeepsFirstReference() -> bool {
  constexpr std::array<const int *, 3> expected{
      std::addressof(kValues.at(0)), std::addressof(kValues.at(1)), std::addressof(kValues.at(3))};
  std::size_t index{};
  bool exact{true};

  query(kValues).unique().forEach([&](const int &value) consteval -> void {
    exact = exact && std::addressof(value) == expected.at(index);
    ++index;
  });
  return exact && index == expected.size();
}

static_assert(uniqueKeepsFirstReference());

consteval auto partitionPreservesReferenceIdentity() -> bool {
  auto [odd, even] =
      query(kValues).partition([](const int &value) consteval -> bool { return value % 2 != 0; });
  constexpr std::array<const int *, 4> oddExpected{std::addressof(kValues.at(0)),
      std::addressof(kValues.at(1)),
      std::addressof(kValues.at(2)),
      std::addressof(kValues.at(4))};
  constexpr std::array<const int *, 1> evenExpected{std::addressof(kValues.at(3))};
  std::size_t oddIndex{};
  std::size_t evenIndex{};
  bool exact{true};

  odd.forEach([&](const int &value) consteval -> void {
    exact = exact && std::addressof(value) == oddExpected.at(oddIndex++);
  });
  even.forEach([&](const int &value) consteval -> void {
    exact = exact && std::addressof(value) == evenExpected.at(evenIndex++);
  });
  return exact && oddIndex == oddExpected.size() && evenIndex == evenExpected.size();
}

static_assert(partitionPreservesReferenceIdentity());

consteval auto partitionObservesBeforeStorageMove() -> bool {
  std::array source{1, 2, 3};
  auto owning = query(source).map([](int &value) consteval -> MoveAware { return MoveAware{value}; });
  auto [positive, rejected] =
      owning.partition([](const MoveAware &value) consteval -> bool { return value.value > 0; });
  return positive.count() == 3 && rejected.count() == 0;
}

static_assert(partitionObservesBeforeStorageMove());

consteval auto boolSortDoesNotLeakStorageRepresentation() -> bool {
  std::array<bool, 3> source{true, false, true};
  auto owning = query(source).map([](bool &value) consteval -> bool { return value; });
  auto sorted = owning.sortBy([]<class Left, class Right>(Left left, Right right) consteval -> bool {
    static_assert(std::same_as<std::remove_cvref_t<Left>, bool>);
    static_assert(std::same_as<std::remove_cvref_t<Right>, bool>);
    return static_cast<bool>(left) < static_cast<bool>(right);
  });
  const auto values = sorted.materialize();
  return values.size() == 3 && not values.at(0) && values.at(1) && values.at(2);
}

static_assert(boolSortDoesNotLeakStorageRepresentation());

consteval auto storedQueriesRemainReusable() -> bool {
  auto stored = query(kValues).sort().unique();
  const auto first = stored.first();
  return stored.count() == 3 && stored.count() == 3 && first.has_value() && *first == 1;
}

static_assert(storedQueriesRemainReusable());

consteval auto materializePreservesOwningLogicalType() -> bool {
  std::array source{3, 1, 2};
  auto values = query(source).map([](int &value) consteval -> int { return value; }).sort().materialize();
  static_assert(std::same_as<decltype(values), std::span<const int>>);
  return values.size() == 3 && values.at(0) == 1 && values.at(1) == 2 && values.at(2) == 3;
}

static_assert(materializePreservesOwningLogicalType());

} // namespace
