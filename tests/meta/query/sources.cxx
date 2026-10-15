import std;
import Miracle.Meta;

using namespace Miracle::meta;

namespace {

struct NonStructural final {
  constexpr explicit NonStructural(int input)
      : value_(input) {
  }

  [[nodiscard]] constexpr auto value() const -> int {
    return value_;
  }

private:
  int value_{};
};

static_assert(not std::meta::is_structural_type(^^NonStructural));

consteval auto chainChildTerminationIsLocal() -> bool {
  std::array left{1, 2, 3};
  std::array right{2, 3};
  std::size_t rightVisits{};

  auto result =
      query(left).take(1).chain(query(right).inspect([&](int &) consteval -> void { ++rightVisits; }));
  return result.count() == 3 && rightVisits == 2;
}

static_assert(chainChildTerminationIsLocal());

consteval auto chainDownstreamCancellationStopsRight() -> bool {
  std::array left{1, 2};
  std::array right{3, 4};
  std::size_t rightVisits{};

  auto result = query(left).chain(query(right).inspect([&](int &) consteval -> void { ++rightVisits; }));
  const auto firstValue = result.first();
  return firstValue.has_value() && *firstValue == 1 && rightVisits == 0;
}

static_assert(chainDownstreamCancellationStopsRight());

consteval auto chainDoesNotIntroducePersistence() -> bool {
  std::array left{1, 2};
  std::array right{3, 4};
  auto lhs = query(left).map([](int &value) consteval -> NonStructural { return NonStructural{value}; });
  auto rhs = query(right).map([](int &value) consteval -> NonStructural { return NonStructural{value}; });
  return lhs.chain(rhs).count() == 4;
}

static_assert(chainDoesNotIntroducePersistence());

consteval auto zipIsEagerAndUsesShorterCardinality() -> bool {
  std::array left{1, 2, 3};
  std::array right{2, 3};
  std::size_t leftVisits{};
  std::size_t rightVisits{};

  auto zipped = query(left)
                    .inspect([&](int &) consteval -> void { ++leftVisits; })
                    .zip(query(right).inspect([&](int &) consteval -> void { ++rightVisits; }));
  const auto count = zipped.take(1).count();
  return count == 1 && leftVisits == 3 && rightVisits == 2;
}

static_assert(zipIsEagerAndUsesShorterCardinality());

consteval auto zipPreservesReferenceIdentity() -> bool {
  std::array left{1, 2};
  std::array right{3, 4};
  std::size_t index{};
  bool exact{true};

  query(left).zip(query(right)).forEach([&](int &lhs, int &rhs) consteval -> void {
    exact = exact && std::addressof(lhs) == std::addressof(left.at(index));
    exact = exact && std::addressof(rhs) == std::addressof(right.at(index));
    ++index;
  });
  return exact && index == 2;
}

static_assert(zipPreservesReferenceIdentity());

consteval auto zipDoesNotRequireStaticBacking() -> bool {
  std::array left{1, 2};
  std::array right{3, 4};
  auto lhs = query(left).map([](int &value) consteval -> NonStructural { return NonStructural{value}; });
  auto rhs = query(right).map([](int &value) consteval -> NonStructural { return NonStructural{value}; });
  return lhs.zip(rhs).count() == 2;
}

static_assert(zipDoesNotRequireStaticBacking());

consteval auto nestedQueryTerminationIsLocal() -> bool {
  std::array outer{1, 2, 3};
  std::array nested{1, 2};
  auto flattened = query(outer).flatMap([nested](int &) consteval -> auto { return query(nested).take(1); });
  return flattened.count() == 3;
}

static_assert(nestedQueryTerminationIsLocal());

} // namespace
