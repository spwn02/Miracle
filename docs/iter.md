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

The umbrella `import Miracle;` exports `Iter`, `ParallelIter`, `iter(...)`, the source-constructor functions described below, `Infinity`/`infinity`, `SizeHint`, and the Miracle-owned semantic adaptor types (`Peekable`, `Cycle`, `OnceWith`, `RepeatWith`, `FromFn`, `Successors`, `Scan`, `Intersperse`, `IntersperseWith`, `MapWindows`, and `ArrayChunks`). Projection composition, fused traversal helpers, and other implementation-only range machinery remain module-private. Miracle-owned range values model `std::ranges::view` structurally; they do not inherit `std::ranges::view_interface`.

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

### Source constructors

Source constructors live directly in `Miracle` so they compose with the established `iter(...)` vocabulary without introducing a conflicting `iter` namespace:

```cpp
empty<i32>();

once(42);
onceWith([] { return loadValue(); });

repeat(42);
repeatN(42, 10);
repeatWith([] { return makeValue(); });

fromFn([]() -> Option<Value> { return nextValue(); });
successors(seed, [](const Value &value) -> Option<Value> {
  return nextValueAfter(value);
}
successors(Option<Value>{seed}, nextValueAfter); // empty Option starts exhausted

chain(first, second); // free-function spelling of iter(first).chain(second)
zip(first, second);   // free-function spelling of iter(first).zip(second));
```

`empty`, `once`, `repeat`, and `repeatN` use the corresponding standard views. `repeat` and `repeatN` therefore follow C++ `repeat_view` semantics: every position references one stored value rather than cloning a fresh value on dereference. `once` exposes its stored value consumably so move-only values materialize normally.

`onceWith`, `repeatWith`, `fromFn`, and `successors` are lazy, allocation-free input ranges. Their callable runs exactly once per logical item and the result is cached across repeated dereference. Callables and generated values may be move-only. `fromFn` ends at the first empty Option-like result. `successors` accepts either an always-present seed or an `Option<T>` initial item; the latter mirrors Rust's ability to begin already exhausted. It computes the next value from a const reference to the current value before exposing the current value for consumption, so moving an item downstream cannot corrupt successor generation.

Coroutine-shaped generation needs no Miracle-specific generator type. C++23/26 `std::generator<T>` is an ordinary input range and composes directly:

```cpp
std::generator<i32> fibonacci();
const auto firstTen = iter(fibonacci()).take(10).toVec();
```

Miracle deliberately does not implement its small callable sources in terms of `std::generator`: their allocation-free, constexpr-friendly callable/cache representation is a better fit for their narrower semantics. `std::generator` remains a first-class interoperability target.

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

## Stateful and semantic adaptors

Miracle exposes a concrete semantic type when Miracle itself owns meaningful range state or behavior. Standard-expressible operations continue to use standard views; implementation-only optimization machinery remains private. This keeps the public model Rust-shaped where that improves semantics without wrapping every standard view in a redundant Miracle type.

The public Miracle-owned adaptor family currently includes `Cycle`, `Scan`, `Intersperse`, `IntersperseWith`, `MapWindows`, `ArrayChunks`, and `Peekable`. The callable sources `OnceWith`, `RepeatWith`, `FromFn`, and `Successors` follow the same rule.

- `filterMap` evaluates an Option-like mapper once and yields present values;
- `mapWhile` stops after the mapper first returns an empty value;
- `scan` carries explicit mutable state and yields present outputs;
- `intersperse` inserts a stored separator while preserving random-access/sized traversal when truthful;
- `intersperseWith` lazily creates separators with one callable invocation per separator;
- `cycle` restarts a forward range without allocation or buffering;
- `mapWindows<N>` maintains a minimal overlapping fixed-width window and adaptively invokes either a whole-window callable or an N-argument callable;
- `arrayChunks<N>` emits only complete fixed-size packets; borrowed sources yield arrays of reference wrappers while owning sources yield owned arrays and preserve move-only values.

The values produced by `filterMap`, `mapWhile`, and `scan` are owned by those adaptors and remain consumable, including when the produced value is move-only.

`copied()` and `cloned()` turn genuine lvalue references into owned prvalues without allocation. `copied()` is limited to trivially copy-constructible values; `cloned()` accepts arbitrary copy-constructible values. Neither is exposed for a pipeline that already yields owned prvalues or xvalues.

`cycle()` first lowers any pending projection and then cycles the resulting forward range. This ordering is observable for stateful projections and deliberately prevents `source.map(function).cycle()` from being rewritten as `source.cycle().map(function)`. Empty sources stay empty; nonempty sources are endless. C++ restartability is represented by `forward_range`, rather than Rust's iterator-clone requirement.

### `Peekable<Iteration>`

`peekable()` is a real one-element lookahead state machine rather than an identity façade. It returns the public data-oriented `Peekable<Iteration>` value, which owns one logical cursor and at most one cached front item. It supports single-pass input sources and exposes only the cursor-sensitive operations that need shared state:

```cpp
auto p = fromFn(source).peekable();

p.peek();
p.peekMut();
p.next();
p.nextIf(predicate);
p.nextIfEq(value);
p.nextIfMap(function);
p.nextIfMapMut(function);

// Available when the underlying iteration is bidirectional and common.
p.nextBack();
```

Borrowing uses C++26 reference optionals directly: `peek()` returns `Option<const Item&>` and `peekMut()` returns `Option<Item&>`. Borrowed `next()` results also preserve references when doing so is lifetime-safe; otherwise the adaptor owns/caches the logical item. Repeated `peek()` calls observe the same logical item. Conditional operations consume only on success, and `nextIfMap` can return a rejected item to the cache through an expected-like result.

A `Peekable` is intentionally a narrow cursor-bearing exception to Iter's range façade. It models an input range so the remaining sequence can re-enter the ordinary fluent API explicitly:

```cpp
auto p = iter(values).peekable();
const auto first = p.peek();
const auto rest = iter(std::move(p)).map(transform).toVec();
```

No inheritance hierarchy, CRTP façade, virtual interface, or duplicated forwarding surface is used to make `Peekable` pretend to be `Iter`.

### Selected Rust-nightly parity

Rust nightly status is a reference signal, not a prohibition. Miracle selectively adopts operations that map cleanly to Miracle and C++ ranges:

```cpp
iter(values).intersperseWith(makeSeparator);
iter(values).mapWindows<4>([](auto a, auto b, auto c, auto d) { /* ... */ });
iter(values).arrayChunks<4>();
iter(values).eqBy(other, predicate);
```

`mapWindows<N>` requires `N > 0`, works with input ranges, and uses adaptive invocation: a callable may accept the fixed window object directly or accept its `N` elements as separate arguments. `arrayChunks<N>` also requires `N > 0`; it discards an incomplete tail, reports exact packet count when sized, and preserves indexed/double-ended traversal for borrowed random-access sized inputs. These facilities are useful independently of their current Rust stabilization status, including packet/SIMD-oriented code.

`eqBy` performs a short-circuiting, element-wise heterogeneous comparison and succeeds only when both sequences end together.

`fuse()` remains a zero-work C++ range adaptation. Standard range traversal already stops at its sentinel, so Miracle does not introduce a cursor subsystem solely to reproduce Rust's post-`None` `Fuse` state machine.

## Search and reduction

Terminals include `find`, `rFind`, `findMap`, `position`, `rposition`, `any`, `all`, `count`, `nth`, `nthBack`, `last`, min/max and key/comparator variants, `sum`, `product`, `fold`, `rFold`, `reduce`, `partition`, `unzip`, `collect`, `toVec`, and `forEach`.

Reference-returning terminals preserve references when the source is borrowed and produce owned values when the range yields rvalues. C++26 `std::optional<T&>` makes this representation direct: Miracle uses `Option<T&>`/`Option<const T&>` for optional borrowed terminal results instead of wrapping the reference in `Ref<T>` merely for optional storage. `Ref<T>` remains available where explicit wrapper/value semantics are intentional. `reduce` delegates to `std::ranges::fold_left_first` where no fused projected-filter traversal is required.

`count()` deliberately traverses the pipeline instead of using a sized-range shortcut so lazy projections and side effects such as `inspect()` are observed consistently with other consuming terminals.

`nthBack`, `rFind`, and `rFold` require bidirectional common ranges. `rposition` additionally requires exact size so it can search from the back, short-circuit on the first reverse match, and still report the original forward index. The maximum family selects the last item among equivalent maxima, while the minimum family retains the first equivalent minimum.

`compare(other)` performs lexicographical three-way comparison and returns the element comparison category (`std::strong_ordering`, `std::weak_ordering`, or `std::partial_ordering`). Common ranges delegate to `std::lexicographical_compare_three_way`; a sentinel-aware traversal preserves the same semantics for non-common input ranges, including unordered partial comparisons.

## Sequence API rather than a general cursor API

Ordinary `Iter` remains a range/view façade. It does not grow Rust's mutable-cursor primitives such as `next`, `nextBack`, `byRef`, `advanceBy`, or cursor-consuming chunk primitives. `Peekable` is the deliberately narrow exception because its lookahead semantics inherently require one shared logical cursor; its cursor operations do not leak back into ordinary `Iter`.

The stable Rust `tryFold`/`tryForEach` family and related fallible terminals remain deferred until Miracle has one library-wide fallible-control-flow abstraction. Cursor-heavy APIs such as `advanceBy`, `nextChunk`, and reverse counterparts are explicitly deferred to Core Revisit, where the future Concepts/Traits and core vocabulary can evaluate them coherently.

Container-extension and mutation-heavy operations such as `collectInto` and `partitionInPlace` are likewise deferred to Core Revisit's Algorithms/container work. `isPartitioned` remains a useful sequence query. Nightly Rust APIs are selected individually rather than adopted mechanically.

### Rust API reconciliation matrix

The final policy is semantic parity, not lexical imitation. Rust's API is classified as follows:

| Classification | Miracle treatment |
| --- | --- |
| Stable applicable Rust iterator operation | Required unless an explicit C++ divergence is documented |
| Selected Rust nightly operation | Adopted when it composes naturally with Miracle/C++ ranges (`intersperseWith`, `mapWindows`, `arrayChunks`, `eqBy`) |
| Standard-C++ equivalent | Use the standard facility rather than wrap it solely to copy Rust's concrete type |
| Intentional C++ divergence | Keep the more truthful C++ range semantics (`repeat`, forward-range `cycle`, zero-work `fuse`) |
| Miracle extension | Preserve C++-native additions such as adaptive invocation, `cacheLatest`, `compare`, slicing, execution policies, and standard-range interoperability |
| Deferred | Move cursor-heavy, fallible-control-flow, container-extension, and mutation-heavy APIs to Core Revisit |

Concrete highlights:

| Rust surface | Miracle status |
| --- | --- |
| `map`, `filter`, `filter_map`, `flat_map`, `flatten`, `enumerate`, `zip`, `chain`, `take`, `skip`, `take_while`, `skip_while`, `step_by`, `rev`, `inspect`, `scan`, `map_while` | Stable parity with C++ spelling/standard lowering where appropriate |
| `cycle`, `copied`, `cloned`, `peekable` | Stable parity through Miracle semantic adaptors/C++ constraints |
| `empty`, `from_fn`, `once`, `once_with`, `repeat`, `repeat_n`, `repeat_with`, `successors` | Constructor parity; `successors` supports both optional and non-empty initial forms |
| `nth_back`, `rfind`, `rfold`, `rposition` | Double-ended useful terminals implemented when capabilities are truthful |
| comparisons, extrema, `find`, `position`, `any`, `all`, `count`, `nth`, `last`, folds/reductions, `partition`, `unzip`, `collect`, `for_each` | Covered, with C++-native ordering/reference semantics where applicable |
| free `chain`, free `zip` | Provided as convenience spellings in addition to member forms |
| `intersperse_with`, `map_windows`, `array_chunks`, `eq_by` | Selected nightly parity |
| `try_fold`, `try_for_each`, broader `try_*` | Deferred to common fallible-control-flow machinery |
| `advance_by`, `next_chunk`, reverse cursor variants | Deferred to Core Revisit cursor evaluation |
| `collect_into`, `partition_in_place` | Deferred to Core Revisit Algorithms/container integration |
| `Fuse` post-exhaustion cursor state | Intentional C++ divergence: ordinary range/sentinel behavior makes a stateful wrapper unnecessary |

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
