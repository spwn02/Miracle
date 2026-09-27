# Meta, reflection, and compile-time query

`Miracle.Meta` is Miracle's C++26 reflection vocabulary. It is a narrow public module and can be imported independently:

```cpp
import Miracle.Meta;
```

The umbrella `import Miracle;` re-exports the same API.

Miracle establishes the reflection vocabulary and source façade. It adds canonical reflection caches and caller-sensitive access projections. In future Miracle introduces the dedicated `Query<State>` compile-time algebra. Query remains deliberately independent of `Iter`.

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

This standard carrier remains the source-layer representation underneath Miracle's frozen `Query<State>` surface. Query adds transformation/search/ordering/materialization semantics without changing the canonical reflection identities or their backing storage.

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

Miracle provides declaration-ordered source queries for:

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

Semantic misuse is reported with `std::meta::exception`. For example, `requireName` rejects a reflection without an identifier and `enumerators` rejects a non-enum reflection. This is also the foundation for Miracle's speculative compile-time control-flow model: callers may deliberately try one semantic interpretation, catch `std::meta::exception`, and attempt another.

Boolean questions should still use predicates rather than exceptions.

## Reflection cache semantics

Miracle makes source reflection canonical without changing the public vocabulary. Every reflected `(subject, category)` pair owns one independently lazy unchecked universe in static storage. Asking for fields does not instantiate functions, bases, annotations, or another unrelated source category.

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

## Compiler-cost benchmark

`benchmarks/meta-cache` contains isolated translation units for 32, 128, 512, and 1024-field subjects. The benchmark compares one-shot raw `<meta>` reflection, typed/raw Miracle lookups, repeated unchecked queries, and repeated caller-access-filtered queries while charging every variant equally for importing `Miracle.Meta`.

Run the reference Linux benchmark with:

```sh
python3 benchmarks/meta-cache/run.py build/tests --samples 3
```

The driver measures the compiler process directly after module BMIs/module maps exist, reporting front-end wall time and peak RSS without dependency-scanner or link time. One-shot Miracle lookup is required to stay within measurement noise of the raw standard expression; repeated lookups are expected to become cheaper when canonical cache reuse avoids repeated reflection/filter work. The recorded reference snapshot is in `benchmarks/meta-cache/baseline.md`; the executable gate uses same-run ratios rather than absolute machine-specific timings.

## Implementation boundary

Miracle caches only canonical reflection sources and access-filtered source projections. Arbitrary user Query pipelines are deliberately not cached: Future Miracle will keep them as fused value-state expressions and materialize persistent results only at the explicit boundary required by the frozen contract.
