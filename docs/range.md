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

`size()` is a constant-time and computes signed endpoint distance through the corresponding unsigned representation so even the full representable signed interval and does not perform overflowing signed arithmetic.

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

Miracle reserves an `Infinity`/`infinity` vocabulary for the unbounded range family, but it is intentionally **not a numeric value** and is not part of finite `Range<T>` storage. Miracle currently locks that vocabulary and its safety rules without exposing an orphan unbounded factory before `Iter`/slicing has a consumer for it.

A strictly increasing endless sequence of a fixed-width C++ integral type cannot be represented forever without eventually overflowing, wrapping, repeating, or changing value type. Miracle therefore does not pretend that `Range<i64>` can become mathematically infinite merely by replacing `stop` with a sentinel.

The future unbounded form uses infinity as a **bound token** for interval/slicing/matching semantics. `Iter` will track source cardinality at the tpye level (`finite`, `unknown`, `provably infinite`) so Miracle-owned terminal operations can reject obvious non-terminating materialization such as collecting a provably infinite source. Bounding adaptors such as `take(n)` turn such a source finite again. Short-circuiting operations may remain valid while still documenting that termination depends on the predicate/input.

This policy prevents hidden allocation growth in Miracle-owned collection machinery without claiming that arbitrary standard-library consumers or user callbacks can be made termination-proof.

## Future integration

Range is the common interval vocabulary for `Iter` slicing, contiguous container slicing, and scalar `Match` patterns. Direct container slicing arrives when Miracle's container façades are revisited; `Iter` consumes Range in the next phase.
