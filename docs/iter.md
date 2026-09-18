# Iter

`Miracle:Iter` is a thin Rust-inspired façade over the standard C++ ranges ecosystem. It keeps fluent method syntax, adaptive callable invocation, and a small set of measured fusion optimizations while delegating traversal and range-category semantics to `std::ranges` whenever the standard facility already matches Miracle's behavior.

```cpp
import Miracle;

using namespace Miracle;

constexpr auto even(i32 value) noexcept -> bool {
  return value % 2 == 0;
}

constexpr auto square(i32 value) noexcept -> i32 {
  return value * value;
}

const Vec<i32> values{1, 2, 3, 4, 5, 6};
const Vec<i32> result = iter(values).filter(even).map(square).toVec();
```

The umbrella `import Miracle;` exports `Iter`, `ParallelIter`, `iter(...)`, `Infinity`/`infinity`, and `SizeHint`. Adaptive invocation, projection composition, fused traversal helpers, and custom helper views remain module-private.

## Construction and ownership

`iter(range)` follows standard viewable-range ownership rules. Lvalue ranges are borrowed through `std::views::all`; owned rvalue ranges retain their storage and expose consuming references through `std::views::as_rvalue`.

```cpp
iter(values);
iter(Vec<i32>{1, 2, 3});
iter(Range{10});
iter(10);          // [0, 10)
iter(2, 10);       // [2, 10)
iter(2, 10, 2);    // [2, 10) with stride 2
```

`Option` and `std::expected` participate as zero-or-one success ranges: present/success values yield one item and absent/error values yield none.

## Standard-first lowering

Most adaptors are direct façades over standard views. `take`, `skip`, `takeWhile`, `skipWhile`, `stepBy`, `rev`, `enumerate`, `zip`, `chain`, `chunks`, and `windows` lower to their corresponding standard range facilities. `fold`/`reduce`, search predicates, ordering queries, and comparisons similarly delegate to standard algorithms where their semantics match.

The implementation deliberately does not maintain a second cardinality/category type system. `Iter` reports exactly the capabilities of its actual underlying range. A single-pass source remains single-pass; a random-access source remains random-access until an adaptor such as filtering truthfully weakens it.

Infinite standard ranges are accepted under the same termination responsibility as ordinary `std::ranges`: Miracle no longer propagates `Finite`/`Infinite`/`Unknown` metadata or rejects exhaustive terminals by type. `infinity` remains only a nonnumeric positional-bound token for operations such as `slice(5, infinity)`.

## Pending projection and map fusion

`map()` is optimized without building an expression-tree optimizer. `Iter` carries at most one pending projection. Consecutive maps compose into that projection rather than nesting `std::ranges::transform_view` layers:

```cpp
iter(values)
    .map(parse)
    .map(normalize)
    .map(project);
```

This preserves the base range for projection-aware standard algorithms and reduces iterator/template layers. Operations that cannot exploit a pending projection lower it to a standard transform view.

### Map followed by filter

A naïve C++ pipeline such as

```cpp
values | std::views::transform(expensive) | std::views::filter(predicate)
```

may evaluate the transformation once while searching and again when an accepted iterator is dereferenced. Inserting `std::views::cache_latest` avoids that recomputation but deliberately degrades the outward range to input-range semantics.

Miracle instead uses a narrow projected-filter representation. Its outward iteration preserves the normal transform/filter category and reference behavior, while Miracle materializers and compatible terminals fuse traversal so the projection is evaluated once per source item. This optimization is local; there is no general Iter IR or optimizer pass framework.

## Explicit `cacheLatest()`

`cacheLatest()` is a direct façade over `std::views::cache_latest` for cases where the caller explicitly wants cached dereference outside Miracle's recognized fusion paths:

```cpp
const auto cached = iter(values)
    .map(expensive)
    .cacheLatest();
```

The capability trade is intentional: the resulting standard `cache_latest_view` models only an input range and is not a borrowed/common range. Miracle does not inject it blindly into pipelines because doing so would silently weaken observable capabilities.

## Adaptive invocation

Miracle wraps user callables in a zero-allocation private adapter. Invocation precedence is:

1. invoke the callable directly with the item;
2. decompose a tuple-protocol item and invoke with its elements;
3. decompose a reflected aggregate and invoke with its members;
4. otherwise emit the adaptive-invocation compile-time diagnostic.

This allows both forms below without changing standard range traversal:

```cpp
iter(values | std::views::enumerate)
    .forEach([](auto &&tuple) {
      // Direct item form wins when valid.
    });

iter(values | std::views::enumerate)
    .forEach([](usize index, auto &&value) {
      // Tuple decomposition fallback.
    });
```

Ordinary named functions work naturally when they denote a concrete function type. A bare overloaded or abbreviated function template such as `auto square(std::integral auto)` is still a C++ overload set and has no single deducible value type; use a concrete overload or callable object/lambda for that language-level case.

## Stateful and custom adaptors

Custom range machinery is intentionally restricted to semantics the standard library does not express directly enough for Miracle:

- `filterMap` evaluates an Option-like mapper once and yields present values;
- `mapWhile` stops after the mapper first returns an empty value;
- `scan` carries explicit mutable state and yields present outputs;
- `intersperse` inserts a separator and preserves random-access/sized traversal when the source supports it.

The values produced by `filterMap`, `mapWhile`, and `scan` are owned by those adaptors and remain consumable, including when the produced value is move-only.

`peekable()` and `fuse()` are zero-work façades where the underlying C++ range already provides the required behavior. `peekable()`/`peek()` require a forward range so observing the next item cannot destructively consume a single-pass source; input-only pipelines do not expose a fake peekable façade.

## Search and reduction

Terminals include `find`, `findMap`, `position`, `rposition`, `any`, `all`, `count`, `nth`, `last`, min/max and key/comparator variants, `sum`, `product`, `fold`, `reduce`, `partition`, `unzip`, `collect`, `toVec`, and `forEach`.

Reference-returning terminals preserve references when the source is borrowed and produce owned values when the range yields rvalues. `reduce` delegates to `std::ranges::fold_left_first` where no fused projected-filter traversal is required.

`count()` deliberately traverses the pipeline instead of using a sized-range shortcut so lazy projections and side effects such as `inspect()` are observed consistently with other consuming terminals.

## Parallel execution

Parallel execution is an explicit terminal policy façade over the standard execution library. Miracle no longer owns a worker pool, thread-count protocol, custom executor abstraction, exception transport, or nested-pool behavior.

```cpp
iter(values).parallel();                    // std::execution::par
iter(values).parallel(std::execution::seq);
iter(values).parallel(std::execution::par);
iter(values).parallel(std::execution::par_unseq);
iter(values).parallel(std::execution::unseq);
```

The current parallel façade is available only when the retained base range is random-access, sized, and common, matching the useful standard policy-backed algorithm surface. `map`, `any`, `all`, `forEach`, and `toVec` delegate scheduling and ordering behavior to the selected standard execution policy. Ordered `toVec()` materialization currently requires a default-initializable result value so a standard policy transform can write directly into pre-sized vector storage without Miracle-owned scratch allocation.

Exception behavior is the selected standard execution policy's behavior; Miracle does not add an exception-transport layer around policy-backed algorithms.

The reference clang-cxx26/libc++ toolchain ships this still-experimental PSTL surface behind libc++'s experimental-library gate. Miracle's build therefore enables `-fexperimental-library` consistently for both standard-module BMI construction and Miracle/consumer compilation.

## Performance contract

Iter's optimization target is the best sensible direct-standard formulation, not merely the most literal view spelling. Validation compares runtime behavior, generated code, allocation behavior, range capabilities, and compiler cost.

The Miracle map/filter benchmark demonstrates the intended lowering: Miracle's fused projected-filter terminal compiles to the same core loop as the equivalent hand-written one-pass implementation while using less generated code than the naïve nested-view and `cache_latest` controls in that benchmark. This is a local measured property, not a claim that every Iter pipeline is universally faster than every standard formulation.
