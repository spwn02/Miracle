# Meta, reflection, and compile-time query

`Miracle.Meta` is Miracle's C++26 reflection vocabulary. It is a narrow public module and can be imported independently:

```cpp
import Miracle.Meta;
```

The umbrella `import Miracle;` re-exports the same API.

The Meta surface establishes a reflection vocabulary, canonical source caches, caller-sensitive access projections, and a dedicated compile-time `Query` algebra. Query remains deliberately independent of runtime `Iter`.

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

This standard carrier remains the source-layer representation underneath Query. Query adds transformation, search, ordering, source-composition, and explicit materialization semantics without changing the canonical reflection identities or their backing storage.

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

Source preconditions are intentional. Record-member sources require a complete class/union; `enumerators()` requires an enumerable enum definition; `parameters()` accepts a concrete function declaration/specialization or a function type, while an unspecialized function-template reflection is rejected because it has no concrete parameter declaration sequence. Namespace aliases and type aliases keep their own reflected identity until `dealias` is requested.

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

Type normalization is deliberately explicit rather than hidden inside a broad canonicalization helper:

```cpp
Miracle::meta::removeCv(^^const volatile int);       // ^^int
Miracle::meta::removeReference(^^const int&);        // ^^const int
Miracle::meta::removeCvref(^^const int&);            // ^^int
Miracle::meta::underlyingType(^^MyEnum);             // reflected enum representation type
```

`dealias` and cv/reference normalization are separate operations: alias identity is preserved until the caller explicitly asks to erase it. These callable projections compose directly in Query, for example `fields<T>() -> map(type) -> map(removeCvref)`.

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

Semantic misuse is reported with `std::meta::exception`. For example, `requireName` rejects a reflection without an identifier and `enumerators` rejects a non-enum reflection. Strict operations are appropriate when failure is itself a semantic error or when a caller is genuinely attempting one compile-time interpretation before falling back to another.

Ordinary Boolean questions do **not** use exceptions as branching syntax. Miracle exposes non-throwing predicates for the common strict-operation preconditions: `hasIdentifier`, `hasParent`, `hasTemplateArguments`, `isFunctionType`, `isType`, `isEnum`, `isComplete`, and `isEnumerable`; substitution has the dedicated `canSubstitute` probe. Use `name(info)` when identifier absence is an expected value rather than an error.

A natural speculative fallback is an operation such as `meta::type(info)`: there is no single standard predicate describing every entity for which `type_of` is meaningful, so a compile-time interpreter may try the projection, catch `std::meta::exception`, and attempt another interpretation. This is distinct from writing `try { requireName(x); } catch (...) { return false; }`, which should simply use `hasIdentifier(x)` or `name(x)`.

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

`Query` is Miracle.Meta's finite, compile-time-only sequence algebra for reflection results and other constant-evaluable static sources. Its public syntax stays deliberately Iter-like while the implementation lowers the fluent chain into immutable normalized value IR rather than encoding every operation as a new C++ stage type.

Every public Query operation is `consteval`. `Query::Value` is the exact logical element type, including references where the pipeline can preserve them. Selection terminals therefore use C++26 `std::optional<T&>` when `Value` is an lvalue reference.

```cpp
consteval auto hasLongRenderFunction() -> bool {
    constexpr std::size_t threshold = 3;
    auto functions = Miracle::meta::query(Miracle::meta::functions<MyType>())
        .filter(Miracle::meta::isFunction)
        .map(Miracle::meta::name)
        .filterMap([](auto name) consteval { return name; })
        .filter([threshold](std::string_view name) consteval {
            return name.size() > threshold;
        });

    return functions.any([](std::string_view name) consteval {
        return name == "render";
    });
}

static_assert(hasLongRenderFunction());
```

Captured lambdas are a supported ordinary case. Structural/captureless callables can be reflected directly into the immutable program; genuinely non-structural captured closures remain in a compact flat capture environment. Users never spell that distinction in the API.

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

`size()` is exposed only where exact cardinality is statically knowable without evaluating a data-dependent predicate. `count()` is the evaluative cardinality terminal.

### Value IR, normalization, and typed segments

A fluent pipeline is represented primarily as immutable structural instruction data. Safe adjacent operations are normalized when doing so preserves evaluation order, short-circuiting, reference categories, overflow behavior, and compile-time observable effects. Examples include removing `skip(0)`, combining compatible `take`/`skip` restrictions, and canonicalizing exact slice restrictions.

Consecutive operations that preserve the exact logical item type compile into one homogeneous execution segment. C++26 expansion statements lower that segment directly into statements. Only genuine type/cardinality boundaries—such as a type-changing `map`, output-changing `filterMap`, or flattening boundary—require typed continuation machinery. Long same-type pipelines therefore do not create one function/class-template specialization per logical operation.

Traversal mutation is evaluation-local data. `take`, `skip`, `enumerate`, `takeWhile`, `skipWhile`, and slices use compact counter/flag pools compiled from the normalized program; mutable traversal state is not stored persistently in the Query object.

### Exact references, values, and captures

Lazy execution preserves exact logical categories whenever the operation permits it: `T`, `T&`, `const T&`, and valid rvalue categories are not silently decayed or copied. A reference-valued selection remains a reference:

```cpp
constexpr auto selected = Miracle::meta::query(objects)
    .filter([](const Item& item) consteval { return item.enabled; })
    .first();

static_assert(std::same_as<decltype(selected), const std::optional<Item const&>>);
```

A non-structural captured callable is stored only because its closure value is semantically required. Captureless/static operations do not consume capture-environment slots, so environment type/storage growth follows the number of irreducible captures rather than total pipeline depth.

### Source composition and lifetime

`flatMap()` and `flatten()` accept another Query or an input-range value. An rvalue nested range yields its value type so a terminal cannot retain a reference into expired temporary storage; lvalue nested ranges preserve their reference semantics. `filterMap()` applies the same rule to optional-like results, including C++26 `std::optional<T&>`.

`chain()` and `zip()` retain their source states as compile-time source composition rather than eagerly converting either input into an intermediate user-visible container. `zip()` terminates at the shorter logical sequence.

### Persistence barriers

Operations that fundamentally require stable reordered/grouped backing are explicit persistence barriers. `materialize()` promotes eligible object values through C++26 `std::define_static_array` and returns an ordinary standard span. `sort`, `reverse`, `partition`, set-like operations, and other persistence-requiring operations evaluate the current program, create the required backing, then continue from that new source.

Ordinary lazy adaptors do not silently create intermediate compile-time containers. Reference-valued queries reject persistence when storing the result would require changing the logical representation, and move-only values are rejected at a barrier when the required persistence operation cannot preserve their semantics.

### Adaptive invocation

Predicates and projections use Meta's local adaptive invocation rules: direct invocation is preferred, then tuple-protocol decomposition, then reflected aggregate-field expansion. Aggregate expansion reuses `Reflect<T>::fields()` so Query does not maintain a parallel reflection path. This machinery is private to Miracle.Meta and does not import runtime `Iter`.

### Compile-time failure

Query is a strict consteval API. Invalid semantic operations use `std::meta::exception` where failure is a genuine semantic error; ordinary Boolean questions use predicates and do not use exceptions as generic branching syntax. Failure-prone source/cache operations validate before entering NTTP cache specializations so catchable semantic exceptions do not escape through constant-initializer machinery.

## Compiler-cost benchmark

`benchmarks/meta-cache` contains isolated translation units for 32, 128, 512, and 1024-field subjects. The benchmark compares one-shot raw `<meta>` reflection, typed/raw Miracle lookups, repeated unchecked queries, and repeated caller-access-filtered queries while charging every variant equally for importing `Miracle.Meta`.

Run the reference Linux benchmark with:

```sh
python3 benchmarks/meta-cache/run.py build/tests --samples 3
```

The driver measures the compiler process directly after module BMIs/module maps exist, reporting front-end wall time and peak RSS without dependency-scanner or link time. One-shot Miracle lookup is required to stay within measurement noise of the raw standard expression; repeated lookups are expected to become cheaper when canonical cache reuse avoids repeated reflection/filter work. The recorded reference snapshot is in `benchmarks/meta-cache/baseline.md`; the executable gate uses same-run ratios rather than absolute machine-specific timings.

`benchmarks/meta-query` separately measures the final reflected value-IR/typed-segment Query architecture against raw and historical architecture controls, including depth scaling, structural allocation, wall time/RSS, generated assembly, and the predicate-versus-`std::meta::exception` control-flow diagnostic. See `benchmarks/meta-query/baseline.md` for the pinned completion measurements and their snapshot/fixture caveats.

## Implementation boundary

The cache layer covers canonical reflection sources and context-safe access-filtered projections. Equivalent structural static Query prefixes rely first on native compiler identity/interning; captured-lambda-heavy arbitrary pipelines are not blindly global-cached. Persistent user values are created only at an explicit semantic barrier.
