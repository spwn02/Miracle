# Range

`Miracle:Range` provides Miracle's finite half-open unit-stride integral interval. It is deliberately small, structural, allocation-free, and directly compatible with the standard ranges ecosystem.

```cpp
import Miracle;

Range first{10};          // [0, 10)
Range middle{2, 10};      // [2, 10)
Range<usize> indices{10};
```

## Semantics

`Range<T>{start, stop}` represents `[start, stop)`. The start is inclusive and the stop is exclusive. Iteration always advances by one. Reversed endpoints do not imply reverse iteration: `Range{10, 2}` is an empty interval whose stored endpoints remain `10` and `2`.

Signed endpoints are supported, including negative intervals. `bool` is not a range element type.

`Range` stores only its two endpoints. It is trivially copyable, standard-layout, and structural, so a range value may participate in constant evaluation and NTTP use.

## Standard range model

For ordinary integral `T`, `Range<T>` is a borrowed, sized, common, random-access view. Generated elements are values rather than references into contiguous storage, so it is intentionally not a `contiguous_range`.

```cpp
const Range range{2, 10};

const auto squares =
    range |
    std::views::transform([](i32 value) -> i32 { return value * value; });

const auto backwards = range | std::views::reverse;
```

`size()` is constant-time and computes signed endpoint distance through the corresponding unsigned representation, so even the full representable signed interval avoids overflowing signed arithmetic.

## Endpoint deduction

One endpoint deduces the endpoint type directly. Two integral endpoints of equal signedness may deduce a lossless common type.

```cpp
short first = 2;
long stop = 10;
Range range{first, stop}; // Range<long>
```

Mixed signed/unsigned endpoints deduction is deliberately rejected because a negative signed endpoint could silently become a very large unsigned value. Spell the intended type explicitly instead:

```cpp
Range<usize>{0, values.size()};
Range{usize{0}, values.size()};
```

## Stepping and reverse traversal

`Range` itself is always unit-stride. Reverse traversal is an adaptor:

```cpp
Range{2, 10} | std::views::reverse;
```

Stepping belongs to the `Iter` layer rather than becoming runtime state inside `Range`:

```cpp
iter(Range{0, 10}).stepBy(2);
```

## Infinity and unbounded intervals

Miracle exposes `Infinity`/`infinity` as an unbounded positional bound token. It is intentionally **not a numeric value** and is not part of finite `Range<T>` storage. `Iter` uses it for suffix slicing such as `iter(values).slice(5, infinity)`.

A strictly increasing endless sequence of a fixed-width C++ integral type cannot be represented forever without eventually overflowing, wrapping, repeating, or changing value type. Miracle therefore does not pretend that `Range<i64>` can become mathematically infinite merely by replacing `stop` with a sentinel.

Infinity remains a **bound token** for interval/slicing/matching semantics rather than an integer sentinel. `Iter` deliberately follows the standard ranges termination model instead of maintaining a parallel finite/unknown/infinite type system. Exhaustive consumption of an endless standard range is therefore the caller's responsibility, exactly as it is for direct `std::ranges` use.

## Future integration

Range is the common interval vocabulary for `Iter` slicing, contiguous container slicing, and scalar `Match` patterns. `Iter` consumes finite `Range` values directly currently; direct container slicing arrives when Miracle's container façades are revisited.
