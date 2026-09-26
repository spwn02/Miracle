# Meta, reflection, and compile-time query

`Miracle.Meta` is Miracle's C++26 reflection vocabulary. It is a narrow public module and can be imported independently:

```cpp
import Miracle.Meta;
```

The umbrella `import Miracle;` re-exports the same API.

Miracle establishes the reflection vocabulary and source façade. In the future it adds canonical reflection caches, and introduces the dedicated `Query<State>` compile-time algebra. Query remains deliberately independent of `Iter`.

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

Miracle source functions return statically promoted `std::span<const meta::Info>` values in declaration order:

```cpp
static_assert(fields.size() == 3);
constexpr Miracle::meta::Info first = fields.front();
```

This standard carrier is intentionally transitional. Miracle in the future introduces the frozen `Query<State>` surface and the complete transformation/search/ordering/materialization algebra in one coherent pass rather than leaking a partial Query API into Miracle.

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

## Implementation boundary

Miracle intentionally does not introduce reflection caches or Query. Source results use standard C++26 static promotion as a simple temporary carrier. In the future Miracle replaces repeated reflection work with canonical, independently lazy caches without changing the vocabulary; it introduces the fused value-state Query pipeline.
