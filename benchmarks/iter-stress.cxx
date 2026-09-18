import std;
import Miracle;

using namespace Miracle;

namespace {

constexpr usize parallelRounds = 100;
constexpr usize sequencedRounds = 20;
constexpr usize elementCount = 20'000;

} // namespace

auto main() -> int { // NOLINT
  Vec<i64> values(elementCount);
  std::ranges::iota(values, i64{});
  const i64 expectedSum = static_cast<i64>(elementCount - 1) * static_cast<i64>(elementCount) / 2;

  for (usize round{}; round < parallelRounds; ++round) {
    const Vec<i64> doubled =
        iter(values).parallel(std::execution::par).map([](i64 value) -> i64 { return value * 2; }).toVec();
    if (doubled.size() != elementCount or doubled.front() != 0 or
        doubled.back() != 2 * static_cast<i64>(elementCount - 1)) {
      return 1;
    }

    if (not iter(values).parallel().all([](i64 value) -> bool { return value >= 0; })) {
      return 2;
    }

    std::atomic<i64> total{};
    iter(values).parallel().forEach(
        [&total](i64 value) -> void { total.fetch_add(value, std::memory_order_relaxed); });
    if (total.load(std::memory_order_relaxed) != expectedSum) {
      return 3;
    }
  }

  for (usize round{}; round < sequencedRounds; ++round) {
    const Vec<i64> mapped =
        iter(values).parallel(std::execution::seq).map([](i64 value) -> i64 { return value + 1; }).toVec();
    if (mapped.size() != elementCount or mapped.front() != 1 or
        mapped.back() != static_cast<i64>(elementCount)) {
      return 4;
    }
  }

  return 0;
}
