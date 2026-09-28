# Meta, reflection, and compile-time query

`Miracle.Meta` is Miracle's C++26 reflection vocabulary. It is a narrow public module and can be imported independently:

```cpp
import Miracle.Meta;
```

The umbrella `import Miracle;` re-exports the same API.

The Meta surface establishes a reflection vocabulary, canonical source caches, caller-sensitive access projections, and a dedicated `Query<State>` compile-time algebra. Query remains deliberately independent of `Iter`.

## Core vocabulary

Miracle keeps the standard reflection value instead of wrapping it:

```cpp
using Miracle::Access;      // std::meta::access_context
using Miracle::meta::Info;  // std::meta::info
```

The ordinary free API accepts either a type or a raw reflection subject:

```cpp
constexpr auto fields = Miracle::meta::fields<MyType>();
constexpr auto members = Miracle::meta::members(^^MyNamespace);
constexpr auto parameters = Miracle::meta::parameters(^^myFunction);
constexpr auto annotations = Miracle::meta::annotations(^^someDeclaration);
```

Reflection source functions return statically promoted `std::span<const meta::Info>` values in declaration order:

```cpp
static_assert(fields.size() == 3);
constexpr Miracle::meta::Info first = fields.front();
```

This standard carrier remains the source-layer representation underneath the frozen `Query<State>` surface. Query adds transformation/search/ordering/materialization semantics without changing the canonical reflection identities or their backing storage.

## `Reflect<T>`

`Reflect<T>` is the sole advanced type-oriented façade and always represents exactly `^^T`:

```cpp
constexpr auto reflected = Miracle::reflect<MyType>();

static_assert(reflected.raw() == ^^MyType);
constexpr auto fields = reflected.fields();
constexpr auto functions = reflected.functions();
constexpr auto annotations = reflected.annotations();
```

Enums additionally expose `enumerators()`:

```cpp
constexpr auto values = Miracle::reflect<MyEnum>().enumerators();
```

The stored reflection is private and arbitrary `Info` construction is intentionally forbidden. Non-type subjects such as namespaces and functions use the free `meta::*` API instead of a second reflection façade.

## Reflection sources

The Meta source layer provides declaration-ordered source queries for:

- `members`
- `fields` (non-static data members)
- `staticFields`
- `functions`
- `constructors`
- `bases`
- `enumerators`
- `parameters`
- `annotations` and `annotations<A>`
- `templateArguments`

Access-sensitive APIs default to the caller's `Access::current()`. Callers may explicitly request another standard access context, including `Access::unchecked()`.

## Functional metadata vocabulary

Reflection operations are small callable values where composition benefits from it:

```cpp
Miracle::meta::name(info);         // optional identifier
Miracle::meta::requireName(info);  // strict identifier
Miracle::meta::displayName(info);  // diagnostic spelling

Miracle::meta::type(info);
Miracle::meta::returnType(info);
Miracle::meta::parent(info);
Miracle::meta::dealias(info);
Miracle::meta::templateOf(info);
Miracle::meta::constant(info);
Miracle::meta::extract<MyAnnotation>(info);
Miracle::meta::sourceLocation(info);
```

Template formation uses:

```cpp
Miracle::meta::canSubstitute(^^Template, arguments);
Miracle::meta::substitute(^^Template, arguments);
```

`raw` is the explicit escape hatch and leaves `std::meta::info` available for standard facilities that Miracle does not wrap.

## Predicate DSL

Primitive predicates are positive callable values and compose without a class hierarchy:

```cpp
constexpr auto interesting =
    Miracle::meta::isFunction &&
    !Miracle::meta::annotated<Hidden>;

static_assert((Miracle::meta::isType || Miracle::meta::isNamespace)(^^MyType));
```

The initial vocabulary includes type/class/union/enum, function/function-template, field/static/instance-data, constructor, namespace, base, enumerator, variable, template, type-alias, concept, annotation, and access predicates.

## Strict consteval failure

Semantic misuse is reported with `std::meta::exception`. For example, `requireName` rejects a reflection without an identifier and `enumerators` rejects a non-enum reflection. This is also the foundation for the speculative compile-time control-flow model: callers may deliberately try one semantic interpretation, catch `std::meta::exception`, and attempt another.

Boolean questions should still use predicates rather than exceptions.

## Reflection cache semantics

The cache layer makes source reflection canonical without changing the public vocabulary. Every reflected `(subject, category)` pair owns one independently lazy unchecked universe in static storage. Asking for fields does not instantiate functions, bases, annotations, or another unrelated source category.

Access-sensitive sources derive visible subsets from that canonical universe. `Access::unchecked()` returns the canonical span directly; other structural access contexts are cached independently after one `std::meta::is_accessible` filtering pass. Declaration order is never changed. The unchecked universe remains module-private and is not an access-control bypass.

The three equivalent type paths converge on the same backing storage:

```cpp
constexpr auto typed = Miracle::meta::fields<MyType>(Miracle::Access::unchecked());
constexpr auto raw = Miracle::meta::fields(^^MyType, Miracle::Access::unchecked());
constexpr auto facade = Miracle::reflect<MyType>().fields(Miracle::Access::unchecked());

static_assert(typed.data() == raw.data());
static_assert(typed.data() == facade.data());
```

The public APIs remain value-oriented. Internally, raw `Info`/`Access` values cross a C++26 reflection/substitution bridge only at the private cache boundary so callers do not inherit NTTP-heavy API state. Typed annotation queries use the same policy with `(subject, annotation type)` as the cache key. Public APIs validate subjects before crossing that bridge: an exception escaping a `constexpr` cache-variable initializer would become a hard constant-expression failure, so cache specialization is never used as Miracle's semantic-error channel.

## Query algebra

`Query<State>` is the finite, compile-time-only sequence algebra used to compose reflection results and other static metadata sources. `State` is the only public template parameter; the implementation keeps one source plus a flat tuple of normalized operation state rather than recursively nesting expression-template nodes.

Every public Query operation is `consteval`. `Query::Value` is the exact logical element type, including lvalue-reference and other reference qualifiers where the pipeline can preserve them. Selection terminals therefore use the C++26 `std::optional<T&>` facility when `Value` is a reference.

```cpp
constexpr auto functions = Miracle::meta::query(Miracle::meta::functions<MyType>())
    .filter(Miracle::meta::isFunction)
    .map(Miracle::meta::name)
    .filterMap([](auto name) consteval { return name; });

static_assert(functions.any([](std::string_view name) consteval {
    return name == "render";
}));
```

The transformation surface is:

```text
map       filter       filterMap       flatMap       flatten       inspect
```

Selection/composition is:

```text
take      skip         takeWhile       skipWhile     slice         enumerate
zip       chain
```

Search/cardinality and terminals are:

```text
find      findMap      contains        position      rposition
all       any          none            count         isEmpty
size      first        last            nth           forEach
```

Ordering/set/group operations are:

```text
reverse   sort         sortBy          sortByKey
min       max          minBy           maxBy          minByKey       maxByKey
unique    uniqueBy     partition
```

`size()` is exposed only where the implementation can prove exact cardinality without evaluating a data-dependent predicate. `count()` is the evaluative cardinality terminal.

### Fusion and state

Element-wise transformations, predicates, inspection, slicing, enumeration, and cardinality-limiting operations are represented as one flat normalized stage tuple. Evaluation walks the source once and propagates each logical value through the stages, so compatible pipelines do not create intermediate compile-time containers.

Operations that require persistent reordered/grouped storage use a static-storage barrier only when the result element representation is eligible for `std::define_static_array`. Reference-valued queries retain reference semantics through selection and other non-persistent operations; `materialize()` rejects a result when persistence would require silently changing that logical representation.

`flatMap()` and `flatten()` accept another Query or an input-range value. An rvalue nested range yields its value type so that a terminal cannot accidentally retain a reference into a temporary; lvalue nested ranges preserve their reference semantics. `filterMap()` follows the same rule for optional-like results and recognizes the C++26 `std::optional<T&>` specialization explicitly, preserving `T&` as the logical Query value instead of decaying it to `T`.

`chain()` and `zip()` retain their source states as compile-time source composition rather than eagerly converting either input into a user-visible container. `zip()` terminates at the shorter logical sequence.

### Persistence

`materialize()` is the explicit persistence boundary. Eligible object values are promoted through C++26 `std::define_static_array` and returned as an ordinary standard span. The query abstraction itself never becomes a runtime range and has no runtime storage or iterator hierarchy.

### Adaptive invocation

Predicates and projections use Meta's local adaptive invocation rules: direct invocation is preferred, then tuple protocol decomposition, then reflected aggregate-field expansion. Aggregate expansion reuses `Reflect<T>::fields()` so the Query engine does not maintain a parallel reflection path. This machinery is private to Meta Query and does not import the runtime `Iter` implementation.

### Compile-time failure

Query remains a strict consteval API. Invalid semantic operations use `std::meta::exception` where a meaningful semantic failure can be represented that way; ordinary Boolean questions continue to use predicates and do not use exceptions as branching syntax.

## Compiler-cost benchmark

`benchmarks/meta-cache` contains isolated translation units for 32, 128, 512, and 1024-field subjects. The benchmark compares one-shot raw `<meta>` reflection, typed/raw Miracle lookups, repeated unchecked queries, and repeated caller-access-filtered queries while charging every variant equally for importing `Miracle.Meta`.

Run the reference Linux benchmark with:

```sh
python3 benchmarks/meta-cache/run.py build/tests --samples 3
```

The driver measures the compiler process directly after module BMIs/module maps exist, reporting front-end wall time and peak RSS without dependency-scanner or link time. One-shot Miracle lookup is required to stay within measurement noise of the raw standard expression; repeated lookups are expected to become cheaper when canonical cache reuse avoids repeated reflection/filter work. The recorded reference snapshot is in `benchmarks/meta-cache/baseline.md`; the executable gate uses same-run ratios rather than absolute machine-specific timings.

`benchmarks/meta-query` separately compares direct raw `<meta>` multi-stage evaluation with the fused Query state machine and measures the consteval predicate-versus-`std::meta::exception` control-flow forms on the reference compiler. These measurements are diagnostic rather than absolute-time gates.

## Implementation boundary

The cache layer covers only canonical reflection sources and access-filtered source projections. Arbitrary user Query pipelines are deliberately not cached: Query keeps them as fused value-state expressions and materializes persistent results only at the explicit boundary required by the frozen contract.
