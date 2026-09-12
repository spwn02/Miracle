import std;
import Miracle;
import Switch;

using namespace Miracle;

namespace Tests::range {

constexpr i32 rangeStart = 2;
constexpr i32 rangeStop = 10;
constexpr i32 rangeLast = 9;
constexpr i32 outsideValue = 5;
constexpr usize negativeRangeSize = 6;
constexpr i32 expectedSum = 14;

constexpr Range<i32> structuralRange{rangeStart, rangeStop};

template <Range<i32> Bounds>
struct Bounded final {
  static constexpr Range<i32> value_ = Bounds;
};

template <class Left, class Right>
concept DeducibleRange = requires(Left left, Right right) { Range{left, right}; };

static_assert(Bounded<structuralRange>::value_ == structuralRange);
static_assert(std::is_trivially_copyable_v<Range<i32>>);
static_assert(std::is_standard_layout_v<Range<i32>>);
static_assert(std::ranges::range<Range<i32>>);
static_assert(std::ranges::range<const Range<i32>>);
static_assert(std::ranges::view<Range<i32>>);
static_assert(std::ranges::viewable_range<Range<i32>>);
static_assert(std::ranges::borrowed_range<Range<i32>>);
static_assert(std::ranges::sized_range<Range<i32>>);
static_assert(std::ranges::common_range<Range<i32>>);
static_assert(std::ranges::forward_range<Range<i32>>);
static_assert(std::ranges::bidirectional_range<Range<i32>>);
static_assert(std::ranges::random_access_range<Range<i32>>);
static_assert(not std::ranges::contiguous_range<Range<i32>>);
static_assert(std::same_as<std::ranges::range_value_t<Range<i32>>, i32>);
static_assert(std::same_as<std::ranges::range_reference_t<Range<i32>>, i32>);
static_assert(std::same_as<decltype(Range{short{1}, long{2}}), Range<long>>);
static_assert(DeducibleRange<short, long>);
static_assert(not DeducibleRange<i32, usize>);
static_assert(not DeducibleRange<bool, bool>);

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("range") ]] auto constructionAndMembership()
    -> void {
  constexpr Range<i32> zeroToTen{rangeStop};
  constexpr Range<i32> twoToTen{rangeStart, rangeStop};
  constexpr Range<i32> negative{-3, 3};
  constexpr Range<i32> negativeStop{-5};
  constexpr Range<i32> reversed{rangeStop, rangeStart};
  constexpr usize emptySize{};

  const usize negativeStopSize = negativeStop.size();
  const usize reversedSize = reversed.size();

  Switch::check(zeroToTen == Range<i32>{0, rangeStop});
  Switch::check(twoToTen.start == rangeStart);
  Switch::check(twoToTen.stop == rangeStop);
  Switch::check(twoToTen.contains(rangeStart));
  Switch::check(twoToTen.contains(rangeLast));
  Switch::check(not twoToTen.contains(rangeStop));
  Switch::check(negative.size() == negativeRangeSize);
  Switch::check(negativeStop.empty());
  Switch::check(negativeStopSize == emptySize);
  Switch::check(reversed.empty());
  Switch::check(reversedSize == emptySize);
  Switch::check(not reversed.contains(outsideValue));
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("range") ]] auto constexprIteration() -> void {
  constexpr i32 sum = [] consteval -> i32 {
    i32 result{};
    for (i32 value : Range{2, 6}) {
      result += value;
    }
    return result;
  }();

  Switch::check(sum == expectedSum);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("range") ]] auto standardRangeComposition()
    -> void {
  const Range<i32> range{rangeStart, rangeStop};
  const Vec<i32> expectedSquares{4, 9, 16, 25, 36, 49, 64, 81};
  const Vec<i32> expectedReversed{9, 8, 7, 6, 5, 4, 3, 2};
  const Vec<i32> expectedEvens{2, 4, 6, 8};

  const Vec<i32> squares =
      range | std::views::transform([](i32 value) -> i32 { return value * value; }) | std::ranges::to<Vec>();
  const Vec<i32> reversed = range | std::views::reverse | std::ranges::to<Vec>();
  const Vec<i32> evens =
      range | std::views::filter([](i32 value) -> bool { return value % 2 == 0; }) | std::ranges::to<Vec>();

  Switch::check(squares == expectedSquares);
  Switch::check(reversed == expectedReversed);
  Switch::check(evens == expectedEvens);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("range") ]] auto extremeSignedSizeIsSafe() -> void {
  constexpr Range<i64> fullWithoutUpperEndpoint{
      std::numeric_limits<i64>::min(), std::numeric_limits<i64>::max()};
  Switch::check(fullWithoutUpperEndpoint.size() == std::numeric_limits<u64>::max());
  Switch::check(std::ranges::distance(fullWithoutUpperEndpoint) ==
                static_cast<std::ranges::range_difference_t<Range<i64>>>(std::numeric_limits<u64>::max()));
}

} // namespace Tests::range

consteval {
  Switch::discover<^^Tests::range>();
}
