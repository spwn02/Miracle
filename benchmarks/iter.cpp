import std;
import Miracle;

using namespace Miracle;

namespace {

constexpr usize elementCount = 300'000;
constexpr usize rounds = 41;

[[nodiscard]] constexpr auto transformValue(i64 value) noexcept -> i64 {
  return (value * 3) + 1;
}

[[nodiscard]] constexpr auto keepValue(i64 value) noexcept -> bool {
  return value % 5 == 0; // NOLINT
}

[[nodiscard]] auto standardNaive(const Vec<i64> &values) -> i64 {
  auto pipeline = values | std::views::transform(transformValue) | std::views::filter(keepValue);
  return std::ranges::fold_left(pipeline, i64{}, std::plus{});
}

[[nodiscard]] auto standardCached(const Vec<i64> &values) -> i64 {
  auto pipeline = values | std::views::transform(transformValue) | std::views::cache_latest |
                  std::views::filter(keepValue);
  return std::ranges::fold_left(pipeline, i64{}, std::plus{});
}

[[nodiscard]] auto manualFused(const Vec<i64> &values) -> i64 {
  i64 result{};
  for (i64 value : values) {
    const i64 mapped = transformValue(value);
    if (keepValue(mapped)) {
      result += mapped;
    }
  }
  return result;
}

[[nodiscard]] auto miracleFused(const Vec<i64> &values) -> i64 {
  return iter(values).map(transformValue).filter(keepValue).sum();
}

template <class Function>
[[nodiscard]] auto measure(Function function) -> Pair<std::chrono::nanoseconds, i64> {
  const auto start = std::chrono::steady_clock::now();
  i64 checksum{};
  for (usize round{}; round < rounds; ++round) {
    checksum += std::invoke(function);
  }
  return {std::chrono::steady_clock::now() - start, checksum};
}

} // namespace

auto main() -> int { // NOLINT
  Vec<i64> values(elementCount);
  std::ranges::iota(values, i64{});

  const i64 expected = manualFused(values);
  if (standardNaive(values) != expected or standardCached(values) != expected or
      miracleFused(values) != expected) {
    std::println(std::cerr, "Iter benchmark semantic mismatch");
    return 1;
  }

  const auto naive = measure([&values] -> i64 { return standardNaive(values); });
  const auto cached = measure([&values] -> i64 { return standardCached(values); });
  const auto manual = measure([&values] -> i64 { return manualFused(values); });
  const auto miracle = measure([&values] -> i64 { return miracleFused(values); });

  std::println("standard naive:  {} ns", naive.first.count());
  std::println("standard cached: {} ns", cached.first.count());
  std::println("manual fused:    {} ns", manual.first.count());
  std::println("Miracle fused:   {} ns", miracle.first.count());
  std::println("checksums: {} {} {} {}", naive.second, cached.second, manual.second, miracle.second);
}
