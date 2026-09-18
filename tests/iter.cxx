import std;
import Miracle;
import Switch;

using namespace Miracle;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace Tests::iteration {

constexpr auto isEven(i32 value) noexcept -> bool {
  return value % 2 == 0;
}

constexpr auto square(i32 value) noexcept -> i32 {
  return value * value;
}

template <class T>
concept AddableToOne = requires(T value) { value + 1; };

static_assert(not std::convertible_to<Infinity, usize>);
static_assert(not std::integral<Infinity>);
static_assert(not AddableToOne<Infinity>);

struct Point final {
  i32 x, y;
};

struct Tracked final {
  static inline i32 copies{};
  static inline i32 moves{};

  i32 value{};

  Tracked() = default;
  ~Tracked() = default;
  explicit Tracked(i32 input)
      : value(input) {
  }
  Tracked(const Tracked &other)
      : value(other.value) {
    ++copies;
  }
  Tracked(Tracked &&other) noexcept
      : value(other.value) {
    ++moves;
    other.value = -1;
  }
  auto operator=(const Tracked &other) -> Tracked & {
    value = other.value;
    ++copies;
    return *this;
  }
  auto operator=(Tracked &&other) noexcept -> Tracked & {
    value = other.value;
    ++moves;
    other.value = -1;
    return *this;
  }
};

static_assert([] -> bool {
  i32 values[]{1, 2, 3, 4, 5, 6}; // NOLINT
  auto pipeline = iter(values);
  static_assert(std::ranges::view<decltype(pipeline)>);
  static_assert(std::ranges::borrowed_range<decltype(pipeline)>);
  static_assert(std::ranges::random_access_range<decltype(pipeline)>);
  static_assert(std::ranges::sized_range<decltype(pipeline)>);

  auto mapped = pipeline.map(square);
  static_assert(std::ranges::random_access_range<decltype(mapped)>);
  static_assert(std::ranges::sized_range<decltype(mapped)>);
  static_assert(not std::ranges::contiguous_range<decltype(mapped)>);

  auto filtered = iter(values).map(square).filter(isEven);
  static_assert(std::ranges::forward_range<decltype(filtered)>);
  static_assert(std::ranges::bidirectional_range<decltype(filtered)>);
  static_assert(not std::ranges::random_access_range<decltype(filtered)>);

  auto cached = iter(values).map(square).cacheLatest();
  static_assert(std::ranges::input_range<decltype(cached)>);
  static_assert(not std::ranges::forward_range<decltype(cached)>);

  return iter(values).filter(isEven).map(square).toVec() == Vec<i32>{4, 16, 36}; // NOLINT
}());

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto constructionAndNamedFunctions()
    -> void {
  Vec<i32> values{1, 2, 3, 4, 5, 6};

  Switch::check(iter(values).filter(isEven).map(square).rev().skip(1).toVec() == Vec<i32>{16, 4});
  Switch::check(iter(5).toVec() == Vec<i32>{0, 1, 2, 3, 4});
  Switch::check(iter(2, 8).toVec() == Vec<i32>{2, 3, 4, 5, 6, 7});
  Switch::check(iter(2, 11, 3).toVec() == Vec<i32>{2, 5, 8});
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto projectionFusion() -> void {
  Vec<i32> values{1, 2, 3, 4, 5, 6};
  i32 firstCalls{};
  i32 secondCalls{};

  const Vec<i32> result = iter(values)
                              .map([&](i32 value) -> i32 {
                                ++firstCalls;
                                return value * 2;
                              })
                              .filter([](i32 value) -> bool { return value % 4 == 0; })
                              .map([&](i32 value) -> i32 {
                                ++secondCalls;
                                return value + 1;
                              })
                              .toVec();

  Switch::check(result == Vec<i32>{5, 9, 13});
  Switch::check(firstCalls == static_cast<i32>(values.size()));
  Switch::check(secondCalls == 3);

  i32 terminalCalls{};
  Switch::check(iter(values)
          .map([&](i32 value) -> i32 {
            ++terminalCalls;
            return value * 2;
          })
          .all([](i32 value) -> bool { return value > 0; }));
  Switch::check(terminalCalls == static_cast<i32>(values.size()));
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto adaptiveInvocation() -> void {
  Vec<i32> values{1, 2, 3};
  const Vec<i32> tupleValues =
      iter(values)
          .enumerate()
          .map([](isize index, i32 value) -> i32 { return static_cast<i32>(index) + value; })
          .toVec();
  Switch::check(tupleValues == Vec<i32>{1, 3, 5});

  Vec<Point> points{{.x = 1, .y = 2}, {.x = 3, .y = 4}};
  i32 aggregate{};
  iter(points).forEach([&aggregate](i32 x, i32 y) -> void { aggregate += x + y; }); // NOLINT
  Switch::check(aggregate == 10);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto optionAndExpected() -> void {
  Option<i32> some{9};
  Option<i32> none{};
  std::expected<i32, i32> ok{7}; // NOLINT
  std::expected<i32, i32> error{std::unexpected{3}};

  Switch::check(iter(some).toVec() == Vec<i32>{9});
  Switch::check(iter(none).toVec().empty());
  Switch::check(iter(ok).toVec() == Vec<i32>{7});
  Switch::check(iter(error).toVec().empty());
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto lazyAdaptors() -> void {
  Vec<i32> values{1, 2, 3, 4, 5, 6, 7, 8};
  Vec<Vec<i32>> nested{{1, 2}, {}, {3, 4}};

  Switch::check(
      iter(values)
          .filterMap([](i32 value) -> Option<i32> { return value % 2 == 0 ? Option<i32>{value * 10} : None; })
          .toVec() == Vec<i32>{20, 40, 60, 80});
  Switch::check(iter(nested).flatten().toVec() == Vec<i32>{1, 2, 3, 4});
  Switch::check(
      iter(nested).flatMap([](auto &value) -> auto & { return value; }).toVec() == Vec<i32>{1, 2, 3, 4});
  Switch::check(iter(values).take(2).chain(Range{20, 23}).toVec() == Vec<i32>{1, 2, 20, 21, 22});
  Switch::check(iter(values)
                    .take(3)
                    .zip(Range{10, 13})
                    .map([](i32 left, i32 right) -> i32 { return left + right; })
                    .toVec() == Vec<i32>{11, 13, 15});
  Switch::check(iter(values).skip(2).take(3).toVec() == Vec<i32>{3, 4, 5});
  Switch::check(
      iter(values).takeWhile([](i32 value) -> bool { return value < 5; }).toVec() == Vec<i32>{1, 2, 3, 4});
  Switch::check(
      iter(values).skipWhile([](i32 value) -> bool { return value < 5; }).toVec() == Vec<i32>{5, 6, 7, 8});
  Switch::check(iter(values).stepBy(3).toVec() == Vec<i32>{1, 4, 7});
  Switch::check(iter(values).rev().take(3).toVec() == Vec<i32>{8, 7, 6});
  Switch::check(iter(values).slice(2, 5).toVec() == Vec<i32>{3, 4, 5});
  Switch::check(iter(values).slice(5, infinity).toVec() == Vec<i32>{6, 7, 8});
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto compoundViews() -> void {
  Vec<i32> values{1, 2, 3, 4, 5, 6, 7, 8};

  const Vec<i32> chunks =
      iter(values)
          .chunks(3)
          .map([](auto range) -> i32 { return std::ranges::fold_left(range, 0, std::plus{}); })
          .toVec();
  const Vec<i32> windows =
      iter(values)
          .windows(3)
          .map([](auto range) -> i32 { return std::ranges::fold_left(range, 0, std::plus{}); })
          .toVec();

  Switch::check(chunks == Vec<i32>{6, 15, 15});
  Switch::check(windows == Vec<i32>{6, 9, 12, 15, 18, 21});
  Switch::check(iter(values).intersperse(0).toVec() == Vec<i32>{1, 0, 2, 0, 3, 0, 4, 0, 5, 0, 6, 0, 7, 0, 8});

  static_assert(std::ranges::random_access_range<decltype(iter(values).chunks(2))>);
  static_assert(std::ranges::sized_range<decltype(iter(values).chunks(2))>);
  static_assert(std::ranges::random_access_range<decltype(iter(values).windows(2))>);
  static_assert(std::ranges::sized_range<decltype(iter(values).windows(2))>);
  static_assert(std::ranges::random_access_range<decltype(iter(values).intersperse(0))>);
  static_assert(std::ranges::sized_range<decltype(iter(values).intersperse(0))>);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto statefulAdaptors() -> void {
  Vec<i32> values{1, 2, 3, 4, 5, 6};
  i32 inspected{};
  const Vec<i32> inspectedValues =
      iter(values).inspect([&inspected](i32 value) -> void { inspected += value; }).take(3).toVec();
  Switch::check(inspectedValues == Vec<i32>{1, 2, 3});
  Switch::check(inspected == 6);

  Switch::check(iter(values)
                    .scan(0,
                        [](i32 &state, i32 value) -> Option<i32> {
                          state += value;
                          return state < 10 ? Option<i32>{state} : None;
                        })
                    .toVec() == Vec<i32>{1, 3, 6});
  Switch::check(
      iter(values)
          .mapWhile([](i32 value) -> Option<i32> { return value < 5 ? Option<i32>{value * 2} : None; })
          .toVec() == Vec<i32>{2, 4, 6, 8});

  auto peekable = iter(values).peekable();
  Switch::check(peekable.peek()->get() == 1);
  Switch::check(peekable.toVec() == values);
  Switch::check(iter(values).fuse().toVec() == values);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto terminals() -> void {
  Vec<i32> values{1, 2, 3, 4, 5, 6, 7, 8};

  Switch::check(iter(values).find([](i32 value) -> bool { return value == 4; })->get() == 4);
  Switch::check(iter(values).findMap([](i32 value) -> Option<i32> {
    return value == 5 ? Option<i32>{50} : None;
  }) == Option<i32>{50});
  Switch::check(iter(values).position([](i32 value) -> bool { return value == 5; }) == Option<usize>{4});
  Switch::check(iter(values).rposition([](i32 value) -> bool { return value % 3 == 0; }) == Option<usize>{5});
  Switch::check(iter(values).any([](i32 value) -> bool { return value == 8; }));
  Switch::check(iter(values).all([](i32 value) -> bool { return value > 0; }));
  Switch::check(iter(values).count() == 8);
  Switch::check(iter(values).nth(3)->get() == 4);
  Switch::check(iter(values).last()->get() == 8);
  Switch::check(iter(values).min()->get() == 1);
  Switch::check(iter(values).max()->get() == 8);
  Switch::check(iter(values).minBy(std::ranges::greater{})->get() == 8);
  Switch::check(iter(values).maxBy(std::ranges::greater{})->get() == 1);
  Switch::check(iter(values).minByKey([](i32 value) -> i32 { return -value; })->get() == 8);
  Switch::check(iter(values).maxByKey([](i32 value) -> i32 { return -value; })->get() == 1);
  Switch::check(iter(values).sum() == 36);
  Switch::check(iter(values).product() == 40'320);
  Switch::check(iter(values).fold(0, std::plus{}) == 36);
  Switch::check(iter(values).reduce(std::plus{}) == Option<i32>{36});

  const auto partitioned = iter(values).partition(isEven);
  Switch::check(partitioned.first == Vec<i32>{2, 4, 6, 8});
  Switch::check(partitioned.second == Vec<i32>{1, 3, 5, 7});

  const auto unzipped = iter(values).take(3).enumerate().unzip();
  Switch::check(unzipped.first == Vec<isize>{0, 1, 2});
  Switch::check(unzipped.second == Vec<i32>{1, 2, 3});
  Switch::check(iter(values).collect<std::deque<i32>>() == std::deque<i32>{1, 2, 3, 4, 5, 6, 7, 8});
  Switch::check(iter(values).eq(values));
  Switch::check(not iter(values).ne(values));
  Switch::check(iter(values).lt(Vec<i32>{1, 2, 3, 4, 5, 6, 7, 9}));
  Switch::check(iter(values).le(values));
  Switch::check(iter(values).gt(Vec<i32>{1, 2, 3, 4, 5, 6, 7, 7}));
  Switch::check(iter(values).ge(values));
  Switch::check(iter(values).isSorted());
  Switch::check(iter(values).isSortedBy(std::ranges::less{}));
  Switch::check(iter(values).isSortedByKey([](i32 value) -> i32 { return value; }));
  Switch::check(not iter(values).rev().isSorted());
  Switch::check(iter(Vec<i32>{2, 4, 6, 1, 3}).isPartitioned(isEven));
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto referenceAndSizeSemantics()
    -> void {
  Vec<i32> values{1, 2, 3};
  auto item = iter(values).nth(1);
  item->get() = 99;
  Switch::check(values[1] == 99);

  const SizeHint exact = iter(values).sizeHint();
  Switch::check(exact.exact());
  Switch::check(exact.lower == 3);
  Switch::check(exact.upper == Option<usize>{3});

  std::istringstream stream{"1 2 3"};
  const SizeHint unknown = iter(std::views::istream<i32>(stream)).sizeHint();
  Switch::check(not unknown.exact());
  Switch::check(not unknown.upper.has_value());
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto ownershipAndSinglePass() -> void {
  std::istringstream stream{"1 2 3 4"};
  auto input = std::views::istream<i32>(stream);
  static_assert(std::ranges::input_range<decltype(iter(input))>);
  static_assert(not std::ranges::forward_range<decltype(iter(input))>);
  Switch::check(iter(input).filter(isEven).toVec() == Vec<i32>{2, 4});

  Vec<Tracked> tracked;
  tracked.emplace_back(1);
  tracked.emplace_back(2);
  tracked.emplace_back(3);
  Tracked::copies = 0;
  Tracked::moves = 0;
  const Vec<Tracked> moved = iter(std::move(tracked)).toVec();
  Switch::check(moved.size() == 3);
  Switch::check(Tracked::copies == 0);

  Vec<UPtr<i32>> owners;
  owners.emplace_back(std::make_unique<i32>(4));
  owners.emplace_back(std::make_unique<i32>(5));
  Switch::check(
      iter(std::move(owners)).map([](UPtr<i32> &&value) -> i32 { return *value; }).toVec() == Vec<i32>{4, 5});

  Vec<UPtr<i32>> foldOwners;
  foldOwners.emplace_back(std::make_unique<i32>(2));
  foldOwners.emplace_back(std::make_unique<i32>(3));
  Switch::check(iter(std::move(foldOwners)).fold(0, [](i32 sum, UPtr<i32> &&value) -> i32 {
    return sum + *value;
  }) == 5);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto cacheLatest() -> void {
  Vec<i32> values{1, 2, 3, 4};
  i32 calls{};
  auto cached = iter(values)
                    .map([&](i32 value) -> i32 {
                      ++calls;
                      return value * 3;
                    })
                    .cacheLatest();

  auto iterator = cached.begin();
  Switch::check(*iterator == 3);
  Switch::check(*iterator == 3);
  Switch::check(calls == 1);
}

[[ = Switch::test, = Switch::group("Core"), = Switch::tag("iter") ]] auto standardParallelPolicies() -> void {
  Vec<i32> values(20'000);
  std::ranges::iota(values, 0);

  const Vec<i32> mapped = iter(values).parallel().map(square).toVec();
  Switch::check(mapped.size() == values.size());
  Switch::check(mapped.front() == 0);
  Switch::check(mapped.back() == square(19'999));
  Switch::check(iter(values).parallel(std::execution::par).all([](i32 value) -> bool { return value >= 0; }));
  Switch::check(
      iter(values).parallel(std::execution::seq).any([](i32 value) -> bool { return value == 12'345; }));
}

} // namespace Tests::iteration
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  Switch::discover<^^Tests::iteration>();
}
