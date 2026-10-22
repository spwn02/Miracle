# Miracle Foundation & Structural Programming Contract Pack — Combined Locked Reference

**Version:** 1.0


---

# Miracle Foundation & Structural Programming Design Contract

**Status:** Locked for implementation  
**Version:** 1.0  
**Scope:** Foundational runtime vocabulary, structural programming substrate, and engineering rules for Miracle.  
**Out of scope:** Serde, Clap, networking, RPC, ORM, dependency injection, state-machine compiler, UUID algorithms, Diff/Patch implementation, SoA/SIMD data systems, Nyx-specific ECS/editor property systems.

---

## 1. Mission

Miracle is a general-purpose C++26 foundation library designed to remove repetitive spelling, expose intent directly, and let programmers spend time on design rather than ceremony.

Miracle is not a framework that replaces the standard library. It is a zero/negative-cost ergonomic and structural layer built primarily on top of the standard library, C++26 reflection, ranges, modules, and compile-time programming.

The governing principle is:

> **Complexity belongs behind the API boundary. User code should express intent with the minimum reasonable ceremony, while runtime semantics remain explicit, unsurprising, and standard-accurate.**

Miracle is also an ecosystem substrate. Its core mechanisms—features, traits, reflection, validation, diagnostics, iterators, ownership vocabulary, derive, matching, and transformation—must be reusable by future Miracle modules and third-party extensions without modifying Miracle itself.

---

## 2. Design priority order

When trade-offs conflict, Miracle uses this priority order:

1. Correctness.
2. Caller-facing simplicity.
3. High-quality diagnostics.
4. Runtime performance.
5. Compile-time performance.
6. Generality and extensibility.
7. Implementation simplicity.

Implementation simplicity is deliberately last.

---

## 3. Zero/negative-cost rule

Miracle abstractions must add no avoidable runtime overhead over the standard/native operation they represent.

Miracle may do substantial compile-time work to simplify the API, but compile-time work is also a performance budget and must be optimized aggressively.

### 3.1 No hidden semantic work

Miracle must never silently introduce semantic costs such as:

- heap allocation;
- copying or ownership transfer;
- destructive speculative consumption;
- filesystem or network I/O;
- synchronization;
- background threading;
- implicit parallel execution.

### 3.2 Hidden optimization is encouraged

Miracle may hide implementation optimizations when they preserve observable semantics and capabilities, including:

- cached dereference/results;
- cached reflection metadata;
- range fusion;
- vectorization;
- redundant-allocation elimination;
- internal scratch reuse;
- parallel backend scheduling only after the caller explicitly selected parallel execution.

---

## 4. Modules and visibility

Miracle is module-first.

### 4.1 Public import surface

The default entry point is:

```cpp
import Miracle;
```

Advanced users may import narrower semantic modules such as:

```cpp
import Miracle.Iter;
import Miracle.Meta;
import Miracle.Match;
```

### 4.2 Module naming

Public modules are semantic, not implementation-shaped. Internal implementation partitions are used only when private declarations genuinely need to be shared among multiple module units.

Module partitions use C++ partition syntax rather than fake dotted internal modules.

### 4.3 No fake privacy namespaces

Miracle does not expose or rely on exported namespaces such as:

```text
Miracle::detail
Miracle::internal
Miracle::_impl
```

Private declarations remain unexported in the owning module unit and may stay in the normal `Miracle` namespace. Internal partitions exist only for real cross-unit private sharing.

---

## 5. Naming conventions

- Ordinary Miracle namespaces use `PascalCase` where namespace naming is appropriate.
- User-facing types use `PascalCase`.
- Functions and methods use `camelCase`.
- Rust-inspired feature/annotation/trait façades use concise lowercase/camelCase names such as `debug`, `serde`, `bitflags`.
- API spelling should prefer free functions when they are more readable, while façades may own implementation/cache state underneath.
- CTAD and inference should remove explicit template spelling wherever semantics remain unambiguous.

The general rule is:

> **Infer when meaning is unambiguous; require explicit spelling when inference could change semantics or introduce ambiguity.**

---

## 6. Conversion policy

The project-wide conversion rule is:

> **Non-owning, lossless, unambiguous conversions may be implicit. Owning, allocating, potentially lossy, capability-reducing, or ambiguous conversions are explicit.**

This rule governs `StaticString`, `StringView`, `Ref`, `Result`, `Option`, `std::expected` interoperability, ownership types, and future facilities.

---

## 7. Exception and panic policy

Miracle does not use exceptions as its normal error model.

- Recoverable failures return `Result`.
- Optional values return `Option`.
- Catastrophic/unrecoverable failures call `panic(...)`.
- Exceptions are tolerated only where required by interoperating standard/third-party APIs and are not promoted as Miracle semantics.
- Allocation failure in ordinary infallible-facing container operations follows the underlying STL-style catastrophic model and may panic; explicit `try*` operations expose allocation failure through `Result`.

`panic(...)` is `[[noreturn]]`, emits a rich diagnostic, and terminates. It never throws and cannot be recovered from.

Compile-time failure is separate and uses `meta::require(...)`, `meta::fail<...>()`, and the compile-time diagnostic engine.

---

## 8. Foundational dependency layers

The contract is organized into the following dependency direction:

```text
L0 — bootstrap
  module/capability adaptation
  StaticString
  Feature engine

L1 — diagnostics/runtime foundation
  Diagnostic
  Error
  panic
  Range
  basic aliases / chrono re-exports

L2 — functional substrate
  Iter
  ParallelIter

L3 — reflection substrate
  Meta abstraction
  Reflect façade + internal caches
  Meta Query
  Validation
  Structural Shapes & Traits

──────── Core Revisit Milestone ────────

L4 — redesigned Miracle vocabulary
  Result / Option
  Ref
  Allocator
  Box / Rc / Arc / WeakRc / WeakArc
  Mutex / RwLock and guards
  String / StringView
  Vec / InlineVec / SmallVec and container family
  Path / PathView
  File / OpenOptions
  Bitflags
  review every existing alias/type/API

L5 — structural programming
  Enum++
  Match
  Tagged/algebraic enums
  Derive
  Structural Transform
```

This is dependency direction, not a mandate that every source file physically lives in one monolithic module.

---

## 9. Core Revisit Milestone

After the new foundation is stable, every existing Miracle facility is reviewed rather than only `Result`.

The revisit may break pre-1.0 source compatibility when necessary to produce a cleaner long-term design.

At minimum, review:

- `Result`;
- `Option`;
- `Error`;
- `Diagnostic`;
- `fatal` → `panic`;
- `Ref`;
- strings and views;
- vectors/maps/sets;
- ownership wrappers;
- `Bitflags`;
- `File`, `OpenOptions`, path handling;
- numeric aliases;
- concepts;
- other quickly-written incomplete helpers.

A custom wrapper is introduced only when it adds genuine semantics or ergonomics. Native arithmetic aliases remain aliases.

---

## 10. Phase completeness rule

A facility is not complete merely because its first useful subset exists.

Each facility is implemented incrementally, but the implementation phase continues until the agreed facility surface is complete.

The phase gate requires, where applicable:

1. API agreement.
2. Complete implementation with no production TODO stubs.
3. Positive black-box tests.
4. White-box/internal tests.
5. Compile-fail tests.
6. Adversarial/negative-space tests.
7. `constexpr` tests.
8. Sanitizer/runtime-hardening tests.
9. Compile-time stress tests.
10. Performance benchmarks.
11. Diagnostic snapshot tests.
12. Documentation and examples.
13. CHANGELOG entry.
14. A coherent commit.
15. A pre-release tag at externally meaningful checkpoints.

Temporary workarounds may exist during an implementation slice only if required to make progress; they must not survive the completed phase.

---

## 11. Testing dependency model

Switch is the testing framework for Miracle.

The dependency graph is target-level, not a library cycle:

```text
Miracle library
     ↑
   Switch
     ↑
Miracle test target
```

Miracle never depends on Switch for normal library use. Switch is requested only by Miracle's test build and is pinned to an explicit compatible release/tag.

A released/stable baseline is used to avoid making the exact uncommitted Miracle-under-test a circular bootstrap prerequisite.

---

## 12. Standard accuracy and compiler policy

Miracle supports compilers by **capability**, not vendor-version folklore.

Capabilities are probed for facilities such as:

- reflection core;
- reflection queries;
- annotations;
- expansion statements;
- aggregate synthesis;
- static-storage synthesis;
- user-generated `static_assert` messages;
- other standard features used by a module.

If a capability is missing:

- the dependent facility may be cleanly disabled;
- a standard-accurate compatibility implementation may be used where practical;
- unrelated facilities remain available.

Miracle does not deliberately limit standard-accurate designs to avoid exposing bugs in an experimental/custom compiler. A failure is considered a compiler bug only when the standard requires the behavior and the compiler does not implement it correctly.

Compiler resource use remains a first-class quality metric regardless of compiler maturity.

---

## 13. Documentation and repository rules

All exported public API is documented before a facility is considered complete.

Private/module-local structures and non-obvious internal invariants are also documented to a wgpu-like readability standard. Trivial local implementation variables do not require redundant comments.

Documentation files use lowercase kebab-case, for example:

```text
docs/result.md
docs/iter.md
docs/meta-validation.md
docs/compiler-support.md
```

Commit history follows conventional facility-scoped subjects, for example:

```text
feat(iter): add adaptive tuple invocation
fix(result): preserve error during transition
perf(meta): cache reflected field metadata
```

Commit bodies explain what changed and why, include tests/verification when meaningful, and follow the agreed meaningful-commit-message discipline.

Miracle changes are never pushed automatically to the public repository by the assistant. Instead, implementation work provides the patch/diff, commit subject/body, CHANGELOG changes, and recommended tag for the user to apply.

---

## 14. Deferred first-class facilities

The following are intentionally outside this contract even though this contract must prepare for them:

- Structural Diff/Patch — first intended follow-on facility.
- Serde and domain schemas.
- Clap.
- UUID v1–v8 algorithms.
- Networking.
- RPC.
- ORM/query engine.
- Dependency injection.
- State-machine compiler.
- SoA/SIMD data systems.
- logging / env_logger-style initialization.

Nyx-specific ECS, editor property semantics, and RHI remain outside Miracle unless a later reusable abstraction is proven by multiple consumers.

---

## 15. Locked identity

Miracle is therefore defined as:

> **A module-first, zero/negative-cost C++26 foundation and structural-programming ecosystem that extends standard C++ with ergonomic ownership/error vocabulary, lazy iteration, compile-time feature selection, reflection/query/validation, structural Shapes and Traits, exhaustive matching, derive, and structural type transformation—without hiding semantic costs or sacrificing native interoperability.**


---

# Miracle Core Runtime Vocabulary Contract

**Status:** Locked  
**Parent:** `00-miracle-foundation-contract.md`

---

## 1. `StaticString`

Miracle provides a structural, NTTP-friendly compile-time string family:

```cpp
template<class Char, usize N>
class BasicStaticString;

template<usize N>
using StaticString = BasicStaticString<char, N>;
```

The type is null-terminated when the character type/model permits it and is designed for diagnostics, reflection names, annotations, feature names, generated metadata, and compile-time formatting.

### Required operations

The facility is complete only after it provides the applicable full string vocabulary, including:

- `size`, `empty`;
- iteration;
- `substr`;
- `startsWith`, `endsWith`;
- `contains`, `find`, `rfind`;
- trim operations;
- ASCII/language-appropriate `lower`, `upper` transformations where well-defined;
- replacement/removal;
- concatenation / `operator+`;
- joins/splits where compile-time representation is practical;
- comparison;
- hashing;
- compile-time formatting suitable for generated diagnostics.

### Conversion policy

Non-owning conversions may be implicit:

```text
StaticString -> Miracle::StringView
StaticString -> std::string_view
StaticString -> const char*   (when representation is compatible)
```

Owning conversions are explicit:

```text
StaticString -> Miracle::String
StaticString -> std::string
```

---

## 2. `Range`

`Range<T>` is a foundational finite half-open interval type integrated across Iter, containers, matching, and future APIs.

```cpp
Range{10}             // [0, 10), deduced default integer policy
Range{2, 10}          // [2, 10)
Range<usize>{10}      // explicitly usize
```

### 2.1 Semantics

- `Range<T>` is templated for integral-like index types.
- It is half-open: `[begin, end)`.
- It is lazy and models the strongest range category the underlying integer type/representation allows.
- `Range` itself is unit-stride only.
- Inclusive/unbounded ranges are distinct types/factories so type-level range capabilities are not weakened by optional runtime endpoints.

### 2.2 Iter integration

```cpp
iter(Range{10})
iter(10)             // shorthand for 0..10
iter(2, 10)          // shorthand for 2..10
iter(2, 10, 2)       // shorthand for Range{2,10}.stepBy(2)
```

Stepping is an Iter adaptor, not stored in `Range`:

```cpp
iter(Range{0, 10}).stepBy(2)
```

### 2.3 Slicing

`slice` means positional slicing and does not replace Rust-style `take(count)`.

```cpp
iter(values).slice(Range{2, 10})
iter(values).slice(10)       // slice 0..10
iter(values).slice(2, 10)
```

The result preserves the strongest observable source capabilities; a filtered/transformed source does not pretend to become a `Span`.

Contiguous containers/views support direct range subscripting:

```cpp
Vec<T> values;
Span<T> middle = values[Range{2, 10}];
Span<const T> cmiddle = constValues[Range{2, 10}];
```

Bounds conventions are Rust-like:

```cpp
values[Range{2, 10}]       // panic on invalid bounds
values.get(Range{2, 10})   // Option<Span<T>>

values[index]              // panic on invalid index
values.get(index)          // Option<Ref<T>>
```

### 2.4 Match integration

`Range` is an interval/value pattern:

```cpp
match(value,
    on(Range{0, 10}, small),
    otherwise(large));
```

Sequence structure remains the responsibility of sequence patterns such as `seq(...)`; `Range` does not ambiguously mean sequence shape.

---

## 3. `Diagnostic`

`Diagnostic` is a structured presentation/explanation object, separate from `Error`.

It represents concepts such as:

- code;
- severity;
- primary message;
- source location/span;
- labels;
- notes;
- help;
- causes/related context;
- renderer metadata.

### 3.1 Construction

The API is fluent and should avoid public post-construction mutation of internal fields.

Preferred shape:

```cpp
auto diagnostic =
    Diagnostic::error(DiagnosticCode::AssertionFailed)
        .at(context.location)
        .message("test returned false")
        .span(statement)
            .label("test return")
            .selection(SpanSelection::Declaration)
        .help("...");
```

The concrete builder may use sub-builders/value composition, but malformed diagnostics should be prevented by construction where practical.

### 3.2 Compile-time and runtime vocabulary

Runtime diagnostics and compile-time validation diagnostics share the same conceptual vocabulary, but storage/rendering implementations may differ where constant-evaluation restrictions require it.

Switch consumes Miracle's base diagnostic model rather than permanently owning a parallel one.

---

## 4. `Error`

`Error` is a compact semantic failure value, not a pre-rendered diagnostic tree.

Default `Result<T>` means:

```cpp
Result<T, Miracle::Error>
```

Typed error alternatives remain first-class:

```cpp
Result<Texture, VulkanError>
Result<User, ParseError>
```

### 4.1 Requirements

`Error` must support:

- compact batch-friendly storage;
- domain/code identity;
- lightweight message/context data;
- optional source information;
- cause/context chaining;
- lazy conversion into a rich `Diagnostic` when display is required.

### 4.2 Cause chains

Context chaining is foundational:

```text
failed to initialize renderer
└─ failed to load material
   └─ failed to load texture
      └─ file not found
```

Ordinary propagation records semantic cause/context; the diagnostic renderer is responsible for presenting the chain cleanly.

---

## 5. `panic`

Current `fatal` is redesigned/renamed to:

```cpp
[[noreturn]]
panic(...);
```

Runtime behavior:

1. adapt the input into a Diagnostic-compatible representation;
2. render/report it using available diagnostic infrastructure;
3. call `std::terminate()`.

`panic` never throws and is not recoverable.

Accepted sources may include strings, `Error`, `Diagnostic`, and types satisfying the corresponding diagnostic/formatting traits.

Compile-time failure does not overload `panic`; use `meta::require` / `meta::fail`.

---

## 6. `Result<T, E = Error>`

`Result` is a real Miracle type with custom discriminated-union semantics, not an alias for `std::expected`.

### 6.1 Core properties

- exactly two observable states: `Ok` and `Err`;
- never observably valueless;
- supports `void` success;
- supports reference success and reference error alternatives;
- propagates triviality where contained alternatives permit it;
- aggressively supports `constexpr`;
- uses conditional `noexcept` truthfully according to contained operations;
- does not use hidden heap storage merely to preserve state;
- state-changing operations that cannot preserve the two-state invariant under required nothrow/rollback properties are constrained out and produce a Miracle diagnostic.

### 6.2 Construction and inference

Factories:

```cpp
Ok(value)
Err(error)
```

Raw success/error construction is allowed when unambiguous:

```cpp
Result<int> f() {
    return 42;
}

Result<Texture> g() {
    Error e = ...;
    return e;             // Err when success construction is impossible
}
```

If a value can construct both `T` and `E`, Miracle refuses to guess and requires `Ok(...)` or `Err(...)`.

`Err(lvalue)` does not silently consume the lvalue merely because it is passed to a function. Ordinary language implicit-move rules apply to eligible local return operands.

### 6.3 Query/access API

Required Rust-inspired surface includes all applicable operations, including:

- `isOk`, `isErr`;
- explicit `operator bool()` delegating to `isOk`;
- `ok`, `err`;
- `value`, `error`;
- `operator*`, `operator->` where the success alternative supports them;
- `map`, `mapErr`, `mapOr`, `mapOrElse`;
- `andThen`, `orElse`;
- `inspect`, `inspectErr`;
- `unwrapOr`, `unwrapOrElse`/equivalent canonical aliases;
- `contains`, `containsErr`;
- `flatten`, `transpose` where semantically valid;
- iteration integration;
- match integration;
- consuming helpers such as `takeOk`/`takeErr` where appropriate.

Wrong-state unchecked convenience access (`value()` on Err, `error()` on Ok, dereference on Err) calls `panic` with a rich diagnostic rather than throwing a Miracle exception.

### 6.4 Value-category-aware projection

Conceptual contract:

```text
Result<T,E>&.ok()          -> Option<Ref<T>>
const Result<T,E>&.ok()    -> Option<Ref<const T>>
Result<T,E>&&.ok()         -> Option<T>
```

with symmetric `err()` behavior.

Const objects are not forcibly consumed. If copy elision does not apply, const correctly prevents ordinary moving.

### 6.5 `std::expected` interoperability

Conversion to/from `std::expected` exists where representable.

- conversion into Miracle may be implicit when lossless and unambiguous;
- conversion out may be implicit for exact representable non-reference alternatives;
- reference Results and other unrepresentable states require explicit/manual conversion or are unavailable;
- potentially lossy/ambiguous conversions are explicit.

### 6.6 Propagation

No `TRY` macro/header is introduced in the module-first contract.

Until a future language mechanism (e.g. future generative/expansion facilities) provides a standard-idiomatic solution, Miracle supports both:

- monadic composition (`andThen`, `map`, etc.);
- ordinary branching and returning the error/value explicitly.

---

## 7. `Option<T>`

`Option` is a first-class Miracle type, with seamless ecosystem behavior rather than exposing `std::optional` as a second-class foreign type.

Factories/tags:

```cpp
Some(value)
None
```

`None` is an object, not a function.

### 7.1 Requirements

- full applicable Rust-inspired Option method surface;
- `Option<T&>` support with rebind-on-assignment semantics;
- complete `constexpr` support where contained operations permit;
- `Iter` integration (`Some` yields one element, `None` yields zero);
- `Match` integration using `.some` / `.none`;
- conversion to/from `std::optional` under the project-wide conversion policy;
- custom public semantics may use standard storage internally where it does not constrain the contract.

---

## 8. `Ref<T>`

`Ref<T>` is a non-owning, non-null reference vocabulary type.

Optional references are represented as:

```cpp
Option<Ref<T>>
```

not nullable `Ref`.

### 8.1 Semantics

- copy assignment rebinds the reference;
- `Ref<T> -> Ref<const T>` is implicit;
- the reverse conversion is forbidden;
- `Ref<T> -> T&` is implicit because it is non-owning/lossless;
- `operator*` and `operator->` exist for ergonomic member access;
- raw references remain allowed in function parameters/returns and user-defined member fields; Miracle does not globally enforce a personal style rule against raw-reference fields.

---

## 9. Allocator model

Allocator is foundational and untyped.

Conceptually:

```cpp
Layout { size, alignment }

allocator.allocate(layout)   -> Result<Block>
allocator.deallocate(block, layout)
allocator.grow(...)
allocator.shrink(...)
```

An allocator is a semantic Trait once Traits are available; bootstrap code uses a primitive structural concept until then.

### 9.1 Requirements

- supports stateful allocators;
- stateless allocator state must optimize away (`[[no_unique_address]]`-style representation where suitable);
- arena/region/custom kernel allocators are first-class;
- ownership types and dynamic containers accept allocator policies from day one;
- allocator failure is fundamentally fallible; infallible-facing container methods panic on failure while `try*` APIs expose `Result`.

---

## 10. Dynamic and inline containers

Storage models remain distinct:

```cpp
Vec<T, Alloc = Global>
InlineVec<T, N>
SmallVec<T, N, Alloc = Global>
```

`Vec<T, Size, Alloc>` is rejected because fixed inline capacity and dynamically allocated capacity are semantically different storage models.

Miracle wrappers use composition rather than public inheritance and preserve the useful semantic/category guarantees of their standard backing types.

### 10.1 Container principles

- Rust-inspired member APIs are the ergonomic primary vocabulary;
- canonical C++ terminology may exist as aliases where useful;
- STL/ranges compatibility is preserved;
- no hidden allocation in operations that look non-allocating;
- normal STL-like operations preserve expected semantics;
- explicit `try*` variants expose recoverable allocation/capacity failure.

Example:

```cpp
vec.reserve(n);       // panic on catastrophic allocation failure
vec.tryReserve(n);    // Result<void>
```

For bounded inline storage, `tryPush` is meaningful for ordinary capacity exhaustion.

### 10.2 Map/set naming

Prefer semantically explicit names such as:

```text
HashMap
HashSet
BTreeMap
BTreeSet
```

rather than ambiguous `Map`/`Set` where ordering/hash semantics are hidden.

---

## 11. `String` and `StringView`

Miracle uses four fundamental text/byte concepts:

```text
String       owning valid UTF-8
StringView   non-owning valid UTF-8
Vec<u8>      owning arbitrary bytes
Span<u8>     non-owning arbitrary bytes
```

This provides strong text safety without reproducing Rust's large collection of string wrapper types.

### 11.1 UTF-8 invariant

`String` and `StringView` guarantee valid UTF-8 and preserve the exact code-point sequence supplied. They do **not** silently normalize NFC/NFD/etc.

Construction from raw bytes is fallible unless explicitly unchecked:

```cpp
String::fromUtf8(bytes)           -> Result<String>
String::fromUtf8Unchecked(bytes)  -> String
```

Unchecked construction has a documented precondition.

### 11.2 Iteration/indexing

- no misleading character-by-integer-index semantic is exposed;
- `.bytes()` and `.chars()` are first-class lazy iteration views;
- byte-range slicing requires UTF-8 code-point boundaries;
- invalid direct slicing panics;
- checked `get(Range)` returns `None` for out-of-bounds or invalid UTF-8 boundaries.

`StringView` supports the agreed ergonomic conversion from `std::string_view`, but the UTF-8 invariant is never weakened: the conversion validates and panics on invalid UTF-8. Untrusted input that must remain recoverable uses an explicit fallible `fromUtf8(...) -> Result<StringView>` path; advanced code may use a documented unchecked constructor when it can prove validity. No Miracle-specific `"..."_sv` literal is introduced merely to duplicate standard literal syntax.

---

## 12. Ownership vocabulary

### 12.1 `Box<T, Alloc = Global>`

`Box` is Miracle's unique owning heap/allocation vocabulary type.

It integrates with Allocator and ecosystem Traits/Iter/diagnostics as applicable.

A constrained zero-reallocation transform is supported:

```cpp
box.map<U>(f)
```

only when the allocation layout is exactly reusable:

```cpp
sizeof(T) == sizeof(U)
alignof(T) == alignof(U)
```

The operation reuses the same block and never hides reallocation. Mismatched layout is a compile-time constraint failure with a clear diagnostic. The transition must also satisfy the same never-invalid ownership/exception-safety rules as other Miracle state transitions; unsafe throwing transitions are constrained out. Exact equality is chosen over hidden metadata for smaller-fitting alternatives.

### 12.2 `Rc<T, Alloc = Global>`

- single-threaded/non-atomic strong+weak reference counts;
- custom Miracle control block;
- one-allocation construction where possible;
- `getMut()` returns mutable access only under unique ownership;
- `makeMut()` supports COW when the contained value satisfies the required clone/copy Trait.

### 12.3 `Arc<T, Alloc = Global>`

- atomic/thread-safe reference counts;
- custom Miracle control block sharing internal design with Rc through a counter policy, without exposing policy templates publicly;
- `getMut()` and `makeMut()` semantics analogous to Rc with correct atomic uniqueness checks.

### 12.4 Weak ownership

Public types are separate:

```cpp
WeakRc<T>
WeakArc<T>
```

A generic public `Weak<T, Ownership>` is rejected because it exposes an implementation policy, adds template noise, and makes distinct ownership families appear interchangeable.

---

## 13. Synchronization vocabulary

Miracle provides ownership-safe synchronization wrappers:

```text
Mutex<T>
MutexGuard<T>
RwLock<T>
RwLockReadGuard<T>
RwLockWriteGuard<T>
```

### 13.1 Rules

- the protected value cannot be accessed without an appropriate guard;
- raw underlying mutex state is not part of normal public API;
- no public `unlock()`; guard destruction/move owns release semantics;
- guards are non-copyable and movable;
- no Rust-style poisoning because `panic()` terminates rather than unwinds and continues execution;
- lock errors represent genuine platform/system synchronization failures.

### 13.2 API shape

```cpp
mutex.lock()       -> Result<MutexGuard<T>>
mutex.tryLock()    -> Result<Option<MutexGuard<T>>>
```

Timed APIs belong to the same lock types where the backend supports them:

```cpp
tryLockFor
tryLockUntil
tryReadFor
tryReadUntil
tryWriteFor
tryWriteUntil
```

(Exact spelling will be normalized during implementation; there is no separate `TimedMutex<T>` family solely for timing.)

---

## 14. Chrono vocabulary

Miracle does not wrap `std::chrono` without a concrete semantic deficiency.

It provides aliases/re-exports for common concepts such as:

```text
Duration
TimePoint
Nanoseconds
Microseconds
Milliseconds
Seconds
Minutes
Hours
SystemClock
SteadyClock
HighResolutionClock
```

Standard chrono literals are re-exported through:

```cpp
Miracle::literals
```

No custom implementation is introduced merely for ownership of the API.

---

## 15. `Path`, `PathView`, `File`, `OpenOptions`

### 15.1 Path

```text
Path       owning platform-native path representation
PathView   non-owning path view
```

Path is not required to be valid UTF-8. Conversion between `Path` and UTF-8 `String` is explicit/fallible where platform representation requires it.

Construction is lexical and does not silently perform filesystem I/O or canonicalization.

```cpp
path.normalize()      // lexical normalization
path.canonicalize()   // filesystem operation -> Result<Path>
```

`operator/` is supported for ergonomic path composition.

### 15.2 File API

Rust's File API is the canonical ergonomic reference where it maps cleanly to C++:

- `File::open`;
- `File::create`;
- `File::createNew`;
- read/write/readExact/readToEnd/readToString/writeAll;
- flush/sync;
- metadata/length operations;
- clone/duplicate behavior where meaningful;
- seek/cursor operations (`seek`, `streamPosition`, `rewind`);
- `close() -> Result<void>` for callers that care about close failure;
- destructor closes as fallback and currently ignores/reporting-unavailable close failure until the future logging facility exists.

### 15.3 OpenOptions

Fluent builder:

```cpp
OpenOptions{}
    .read()
    .write()
    .create()
    .truncate(false)
    .open(path);
```

No-argument enabling toggles and optional boolean overloads are both supported.

### 15.4 Native handle

```cpp
file.nativeHandle()
```

returns the actual platform-native handle directly. Backends may delegate to C++26 `native_handle()` internally; Miracle does not expose an underlying stream solely to reach it.

---

## 16. `Bitflags<E>`

`Bitflags<E>` is a real flags-set façade inspired by Rust's `bitflags` crate.

### 16.1 Core surface

Required operations include:

- `empty`, `all`, `bits`;
- `fromBits`, `fromBitsTruncate`, `fromBitsRetain`;
- `contains`, `intersects`;
- `insert`, `remove`, `toggle`, `set`;
- union/intersection/difference/symmetric difference/complement;
- iteration over known flags;
- known/unknown bit preservation semantics.

Composite named enumerators are valid; validation distinguishes atomic flags from composite aliases.

### 16.2 Conversions/operators

- enum -> `Bitflags<E>` may be implicit;
- `E::A | E::B` produces `Bitflags<E>`;
- reverse `Bitflags<E> -> E` is **not** implicit because a flag set may represent no declared enumerator; explicit/raw conversion is required where an external API expects an enum-typed bit pattern.

Both direct façade use and derive/QOL activation are supported:

```cpp
Bitflags<Access>
[[= derive(bitflags) ]]
```

### 16.3 Formatting

Default symbolic formatting preserves unknown bits:

```text
Read | Write
Read | 0x80
```

Formatter specifications support numeric presentations such as binary/hex/octal (`{:b}`, `{:x}`, etc.), with width/alignment based on the underlying enum representation.

---


---

# Miracle Iteration Contract

**Status:** Locked  
**Parent:** `00-miracle-foundation-contract.md`

---

## 1. Purpose

Miracle `Iter` is a Rust-inspired ergonomic layer over the C++ ranges ecosystem.

Its primary goals are:

- replace verbose ranges spelling with compact intent-oriented method chains;
- preserve lazy evaluation;
- preserve range capabilities truthfully;
- provide adaptive callable invocation;
- integrate naturally with `Range`, `Result`, `Option`, tuples, reflected aggregates, and Match;
- reuse standard ranges/algorithms whenever they already provide the correct behavior;
- add custom logic only when the standard facility is absent or does not meet Miracle semantics.

`Iter` is not a new container and does not eagerly materialize values unless a terminal operation requests materialization.

---

## 2. Canonical construction

The ordinary API is one universal `iter(...)` family. Ownership/borrowing details are inferred from the argument and standard viewable/borrowed-range semantics.

Examples:

```cpp
iter(values)
iter(Range{10})
iter(10)
iter(2, 10)
iter(2, 10, 2)
iter(result)
iter(option)
```

Ordinary users do not select `OwnedIter`/`BorrowedIter` types manually.

The returned type adapts its capabilities to the source rather than pretending every source is equally powerful.

---

## 3. Laziness

Intermediate operations are lazy:

```cpp
iter(values)
    .filter(...)
    .map(...)
    .enumerate()
    .take(10);
```

Terminals perform consumption/materialization:

```cpp
.toVec()
.collect<T>()
.fold(...)
.count()
.forEach(...)
```

No intermediate adaptor silently allocates merely to make an operation easier.

---

## 4. Method vocabulary

Rust's `Iterator` API is the canonical naming/reference point where the operation maps cleanly to C++.

The completed Iter phase inventories and implements every applicable operation rather than intentionally leaving holes.

Expected families include, where category constraints make them meaningful:

### Transformation/adaptation

- `map`;
- `filter`;
- `filterMap`;
- `flatMap`;
- `flatten`;
- `enumerate`;
- `zip`;
- `chain`;
- `take`;
- `skip`;
- `takeWhile`;
- `skipWhile`;
- `stepBy`;
- `cycle`;
- `rev`;
- `peekable`;
- `fuse`;
- `inspect`;
- `scan`;
- `mapWhile`;
- chunks/windows-related operations where source semantics permit;
- `intersperse`;
- `slice(Range)`;
- category-preserving standard-range conveniences.

### Search/query terminals

- `find`;
- `findMap`;
- `position`;
- reverse-position where supported;
- `any`;
- `all`;
- `count`;
- `nth`;
- `last`;
- min/max families;
- comparison/key variants.

### Reduction/materialization

- `sum`;
- `product`;
- `fold`;
- `reduce`;
- `partition`;
- `unzip`;
- `collect`;
- `toVec` and other canonical convenience materializers;
- `forEach`.

Canonical C++ names may be provided as aliases when that meaning is common and helps C++ developers discover the API, but the ecosystem remains internally consistent and does not invent alternate semantics for identical operations.

---

## 5. Adaptive callable invocation

Miracle introduces a shared adaptive invocation primitive used by Iter and reusable by other facilities.

Invocation precedence is locked:

1. direct invocation with the item;
2. tuple-protocol decomposition (`tuple_size`/`get`/`apply` semantics);
3. reflected aggregate decomposition;
4. otherwise a clear compile-time diagnostic.

Thus:

```cpp
iter(values | std::views::enumerate)
    .forEach([](auto&& tuple) {
        // direct item form wins when valid
    });
```

and:

```cpp
iter(values | std::views::enumerate)
    .forEach([](usize index, auto&& value) {
        // decomposition fallback
    });
```

both work without ambiguity.

Reflected aggregate decomposition is a genuine Miracle extension, not required for tuple-like standard types.

---

## 6. Capability preservation

Iter never lies about its source.

Examples:

- reverse iteration only when the source supports the needed bidirectional/double-ended capability;
- exact length only when size is available/derivable without destructive work;
- single-pass sources remain single-pass;
- copy/clone-like behavior only when underlying state supports it;
- no hidden buffering solely to provide stronger iteration categories;
- no hidden speculative backtracking.

The API uses concepts/Traits to constrain methods so unavailable operations fail with Miracle diagnostics rather than deep template errors.

---

## 7. Smart caching / `cache_latest`

Miracle may use C++26-style cached dereference/results internally to avoid repeated expensive evaluation when doing so preserves observable semantics and range capabilities.

It must **not** blindly inject a cache adaptor into the outward pipeline if that would degrade a `forward_range`, borrowed range, common range, or other observable capability.

Caching is therefore an optimization strategy inside compound adaptors/terminals or equivalent internal machinery, not an excuse to lie about the public range category.

---

## 8. `Range` integration

`Range<T>` is foundational and works naturally with Iter:

```cpp
iter(10)               // [0,10)
iter(2, 10)            // [2,10)
iter(2, 10, 2)         // [2,10) step 2

iter(Range<usize>{10}).toVec();
```

`take(n)` remains a count limiter.

`slice(...)` is positional slicing:

```cpp
iter(values).slice(10)
iter(values).slice(2, 10)
iter(values).slice(Range{2, 10})
```

The result is a lazy/capability-preserving view unless the source operation naturally returns a contiguous `Span`.

---

## 9. `Result` and `Option` integration

`Result` behaves as a zero-or-one success iterator:

- `Ok(value)` yields one value;
- `Err(error)` yields no success item.

`Option` behaves as:

- `Some(value)` yields one item;
- `None` yields zero items.

These integrations allow ordinary iterator composition without converting through ad hoc containers.

---

## 10. `ParallelIter`

Parallel execution is explicit and returns a distinct adaptor type:

```cpp
iter(values).parallel()
iter(values).parallel(4)
iter(values).parallel(executor)
```

Ordinary `Iter` never silently becomes parallel.

### 10.1 Concurrency value

`parallel(N)` means **maximum concurrency**:

```text
0  -> automatic available concurrency
1  -> serial execution under ParallelIter semantics
N  -> at most N concurrent workers
```

This policy is stored in `ParallelIter` and is not a promise to create exactly N OS threads.

### 10.2 Default runtime

The default parallel backend uses reusable execution state / worker infrastructure rather than creating and destroying OS threads for each terminal operation.

### 10.3 Custom executor

The full facility supports custom executors:

```cpp
iter(values).parallel(executor)
```

Executor is ultimately expressed as a structural Trait so engines, kernels, servers, and other environments can integrate their own scheduling systems without adapting to a Miracle-global thread pool.

### 10.4 Determinism

Parallel execution order may be nondeterministic.

Ordered result-producing terminals preserve deterministic source order where semantically meaningful, for example:

```cpp
iter(values)
    .parallel()
    .map(...)
    .collect<Vec<_>>();
```

Parallel side-effect terminals such as `forEach` do not promise invocation order.

---

## 11. Hidden-work policy for Iter

Iter must never silently:

- allocate;
- copy owned elements merely for convenience;
- consume a source speculatively and then attempt to restore it;
- start threads without `.parallel()`;
- perform I/O;
- add synchronization visible to caller semantics.

It may silently fuse, cache, vectorize, reuse, or otherwise optimize operations when observable semantics/capabilities are unchanged.

---

## 12. Testing requirements

The Iter phase includes:

- every supported range category;
- lvalue/rvalue/const sources;
- move-only values;
- tuple and aggregate adaptive invocation;
- single-pass adversarial tests;
- capability regression tests;
- range/Result/Option integration;
- compile-fail diagnostics for unsupported operations;
- lazy-evaluation tests;
- no-hidden-allocation/copy tests where measurable;
- compile-time/constexpr pipelines where allowed;
- parallel determinism/order tests;
- custom executor tests;
- stress and performance benchmarks against equivalent standard ranges.


---

# Miracle Meta, Validation, Shapes & Traits Contract

**Status:** Locked  
**Parent:** `00-miracle-foundation-contract.md`

---

## 1. Purpose

Miracle.Meta is the sole standard-facing reflection substrate for the rest of the ecosystem.

High-level facilities should not scatter raw `std::meta` calls throughout the codebase. Miracle centralizes reflection adaptation, caching, access handling, queries, validation, diagnostics, Shapes, and Traits.

The goals are:

- minimize redundant compiler work;
- isolate standard/reflection API churn;
- expose a compact fluent API;
- replace template-instantiation explosions with deliberate diagnostics;
- make structural programming reusable by third-party extensions;
- support semantic Traits without inheritance.

---

## 2. Public reflection façade

Ordinary user code should prefer APIs such as:

```cpp
meta::members<T>()
meta::fields<T>()
meta::staticFields<T>()
meta::functions<T>()
meta::constructors<T>()
meta::parameters<F>()
meta::bases<T>()
meta::enumerators<E>()
meta::annotations<T>()

meta::name(info)
meta::type(info)
meta::raw(value)
```

The naming `fields` is used for non-static data members rather than abbreviations such as `nsMembers`.

`std::meta` remains available through explicit escape hatches (`meta::raw(...)`, `.raw()`, or equivalent), not as the default ecosystem vocabulary.

---

## 3. Access model

Miracle aliases:

```cpp
using Access = std::meta::access_context;
```

Public reflection queries take access as a function parameter, defaulting to current access semantics:

```cpp
meta::fields<T>(Access access = Access::current())
```

This preserves ergonomic call-site context while supporting explicit cases such as:

```cpp
meta::fields<T>(Access::unchecked())
```

for frameworks such as Switch that intentionally reflect private member test functions.

Miracle does not add redundant `access::*` wrappers when the standard alias already exposes the needed factories.

---

## 4. `Reflect<T>` façade and caching

`Reflect<T>` is the advanced/public cached reflection façade underneath `meta::*` convenience functions.

Ordinary code should usually call `meta::*`; advanced code may use `Reflect<T>` where direct cached metadata access is useful.

### 4.1 Internal cache privacy

The canonical unrestricted/unchecked reflection universe is module-private implementation state. It is **not** exposed as an access-control bypass.

Public queries always apply the requested `Access`; only explicit `Access::unchecked()` exposes inaccessible members.

### 4.2 Lazy per-category caches

Reflection metadata is cached lazily per category. Requesting fields must not force functions, bases, annotations, etc. to be computed unnecessarily.

Conceptually:

```text
ReflectionCache<T>
  fields        lazy
  functions     lazy
  bases         lazy
  enumerators   lazy
  ...
```

### 4.3 Rich metadata records

Frequently-requested immutable metadata is cached rather than recomputed repeatedly.

Conceptual records may include:

```cpp
FieldMeta {
    std::meta::info raw;
    StaticString name;
    std::meta::info type;
    cached annotations;
    source information;
    other stable category metadata;
};
```

The concrete representation should optimize compile-time/storage cost and need not eagerly compute fields that are never used.

### 4.4 Query caching policy

- base reflection universes are cached;
- lazy query adaptors are not cached merely because they exist;
- materialized/reusable query results may be cached by canonical query identity where profitable;
- ephemeral lambda-heavy pipelines must not explode compiler state by blindly creating caches for every intermediate expression.

---

## 5. Meta Query

Meta Query reuses the same mental vocabulary as runtime `Iter`.

Example:

```cpp
meta::fields<T>()
    .filter(meta::annotated<Serialize>)
    .filter(meta::instanceData)
    .map(meta::type)
    .unique();
```

The completed query facility includes the applicable full sequence/query vocabulary, including:

- `filter`;
- `map`/transform;
- `find`;
- `contains`;
- `all` / `any` / `none`;
- `count`;
- `partition`;
- `unique`;
- sorting/grouping where reflection metadata provides a deterministic ordering/key;
- materialization/caching controls;
- other Iter-like operations that make semantic sense at constant evaluation.

The reflection sequence universe includes all supported standard categories, including:

- members;
- non-static data members;
- static data members;
- functions;
- constructors;
- parameters;
- bases;
- enumerators;
- annotations;
- template arguments/parameters where supported by the standard reflection model.

---

## 6. Validation engine

Miracle uses one general-purpose validation engine for types, members, enumerators, functions, parameters, bases, annotations, and future reflected subjects.

It is not a member-only framework.

Three public modes are locked:

```cpp
meta::valid<T, Policy>()
meta::validate<T, Policy>()
meta::require<T, Policy>()
```

### 6.1 `valid`

- quiet Boolean/capability query;
- aggressively short-circuits;
- optimized for concepts/overload participation;
- performs the minimum work needed to determine validity.

### 6.2 `validate`

- produces structured validation violations;
- collects a bounded useful set rather than only first failure;
- reusable by runtime tooling/tests/compile-time diagnostics as allowed.

### 6.3 `require`

- emits a deliberate compile-time diagnostic;
- reports root context plus useful violations;
- caps diagnostics to avoid enormous compiler output;
- uses `StaticString`/C++26 generated messages where applicable.

### 6.4 Rule diagnostics

Validation rules can provide custom diagnostic text and contextual help, for example:

```text
Debug derivation failed:
Player::transform is not formattable.

Provide std::formatter<Transform>
or derive/implement debug for Transform.
```

Long raw template-instantiation explosions are considered a diagnostic regression.

---

## 7. Shapes

A `Shape` is an automatically satisfied **structural** contract.

Example:

```cpp
inline constexpr auto PositionLike =
    shape()
        .field<Floating>("x")
        .field<Floating>("y")
        .field<Floating>("z");
```

Any type with the required compatible structure satisfies the Shape; no inheritance or explicit registration is required.

### 7.1 Structural requirements

Shapes support, as applicable:

- named fields;
- static fields;
- methods;
- static methods;
- overload/qualifier selection;
- return/parameter constraints;
- `const`, ref, and `noexcept` requirements;
- associated/nested types;
- static constants;
- annotations;
- concept/convertibility requirements;
- method parameter metadata where reflection exposes it.

Constraints can be semantic concepts rather than exact types:

```cpp
.field<Floating>("x")
.field<ConvertibleTo<float>>("x")
.method<Invocable<...>>("update")
```

### 7.2 Structural views

Shapes can produce zero-cost structural views/adapters:

```cpp
auto p = PositionLike.view(value);
```

or through generic Trait/Shape adaptation where appropriate.

A structural view holds only the required reference/value state, resolves member bindings at compile time, and performs no runtime name lookup or dynamic dispatch.

---

## 8. Traits

A `Trait` builds on structural requirements but represents a semantic ecosystem contract.

Traits are Miracle's composition-first alternative to inheritance-heavy protocol design.

### 8.1 Shape vs Trait

```text
Shape
  automatic structural conformance
  describes what a type looks like

Trait
  structural requirements + semantic contract
  can have implementations, defaults, supertraits, derives, adapters
```

### 8.2 Trait contents

Traits support the complete applicable contract vocabulary:

- required/optional associated types;
- required/optional static constants;
- required/optional static functions;
- required/optional member functions;
- overloads/qualifiers/noexcept constraints;
- annotations;
- supertraits / inheritance-like trait requirements;
- default operations implemented in terms of required operations;
- derived implementation strategy when derivable;
- explicit/custom implementation adapters.

### 8.3 Implementation discovery

`[[= impl(trait) ]]` is an optional semantic hint/registration mechanism, not mandatory ceremony for every structurally compatible type.

Miracle supports both:

- intrinsic/annotated implementation for types under user control;
- external implementation adapters/registrations for foreign types.

The trait engine validates conformance and resolves custom/derived/default implementations deterministically.

### 8.4 Adaptation

Generic adaptation uses:

```cpp
as<Trait>(value)
```

when the source satisfies/implements the Trait.

The returned façade is zero-cost: no runtime vtable, string lookup, dynamic registry, or heap allocation. It exposes required and default Trait operations against the original value.

### 8.5 Feature bootstrap boundary

Feature is lower-level than the full Trait engine and cannot depend on Trait during bootstrap.

Feature descriptors initially satisfy a primitive standard-C++ concept. Once Traits are available, Feature's structure may also be described/exposed through the Trait machinery without creating a dependency cycle.

---

## 9. Trait façades and same-name objects

Miracle hard-locks the same-name façade direction, subject to exhaustive standard/name-lookup tests.

Conceptually:

```cpp
struct debug {
    static constexpr auto rename(StringView value) { ... }
    static constexpr auto skip = ...;
};

inline constexpr struct debug debug{};
```

This enables the dream syntax:

```cpp
[[= derive(debug, serde, clap, bitflags, hash) ]]

[[= debug::rename("position") ]]
[[= serde::skip ]]
```

The lower-case object is the Trait/derive/feature identity. Static members on the same-named façade type provide annotation constructors/options.

This is a defining Miracle API pattern rather than an isolated hack.

---

## 10. Derive engine

Derive is an extensible protocol over Traits.

### 10.1 Core syntax

```cpp
[[= derive(debug, hash, bitflags) ]]
```

Later modules can contribute their own traits such as `serde`/`clap` without changing the Derive core.

### 10.2 Core derives in this contract

The initial structural-programming contract includes core derives such as:

- `debug`;
- equality/comparison support;
- `hash`;
- `visit`;
- `bitflags` activation where appropriate.

Serde/Clap/RPC/database derives belong to their future facility contracts.

### 10.3 Custom implementations

Implementation precedence is locked:

```text
explicit user/custom implementation
  > explicit derived implementation
  > generic structural fallback where that Trait permits one
```

Miracle never fights hand-written customization.

### 10.4 Third-party derives

Third parties can define their own Traits and derivation strategies and use them in `derive(...)` without modifying Miracle.

Derives may declare dependencies/conflicts and can reuse the Feature engine for those relationships.

### 10.5 Trait derivation vs implementation

A Trait may be derivable when it defines a mechanical derivation strategy. Traits that are not mechanically derivable remain explicitly implementable through `impl`/external adapters.

For example, `debug` itself is a Trait. Users may provide a custom debug implementation satisfying its required parse/format-related contract instead of using the generated structural implementation.

---

## 11. Feature engine

Feature is a foundational compile-time ecosystem mechanism, independent of UUID/Serde/etc.

It separates:

```text
build features
  affect compiled modules/dependencies/capability universe

semantic features
  affect compile-time behavior/configuration inside the available universe
```

### 11.1 Feature descriptor

Features are first-class structural values where practical.

A descriptor can express:

- name;
- version;
- description;
- dependencies;
- optional dependencies;
- conflicts;
- implications;
- capability requirements;
- default-enabled state;
- feature groups;
- future metadata required by ecosystem tooling.

Third-party code can define equivalent descriptors.

### 11.2 Resolution

Feature resolution is:

- transitive;
- deterministic;
- canonicalized;
- cycle-checked;
- conflict-checked;
- compile-time diagnosed.

Groups expand into underlying features.

Versioned feature families may imply a parent feature, e.g. a future `uuid.v7` implies base `uuid` support.

### 11.3 Build universe and local sets

```text
BuildFeatureSet
  maximum capabilities compiled into this Miracle build

Local FeatureSet
  subset/configuration drawn from BuildFeatureSet
```

A local set can never enable a feature whose heavy implementation was not compiled into the build.

Global/default feature state and explicit local FeatureSets are both supported.

### 11.4 Disabled feature visibility

Disabled feature APIs may retain lightweight façade declarations sufficient to produce a Miracle diagnostic such as:

```text
feature `uuid.v4` is not enabled

help:
  enable `uuid.v4` in the Miracle feature set
```

Heavy implementation partitions/dependencies remain absent so compile work/code generation is not paid for disabled features.

### 11.5 Build bridge

Build systems configure features through a portable bridge:

```text
CMake/Meson/xmake/custom build
  -> compile definitions / generated configuration
  -> Miracle configuration partition
  -> typed BuildFeatureSet
```

Macros stop at the configuration boundary and are not the user-facing feature API.

A generated configuration module unit/partition is preferred when supported; compile definitions remain the portable lowest-common-denominator input.

The configured feature set is part of Miracle's build/BMI identity and incompatible configurations must not be silently mixed.

### 11.6 Annotation integration

Feature requirements use concise syntax such as:

```cpp
[[= requires(serde::json) ]]
```

when the future facility exists.

---

## 12. Compiler-work optimization

Miracle Meta/Traits treat compiler work as a performance budget.

Hard practices include:

- cache repeated reflection universes;
- cache commonly-requested immutable metadata;
- avoid repeated `define_static_array`/reflection enumeration where reusable storage exists;
- prefer expansion statements/flat constant-evaluation flows over recursive template machinery where possible;
- bound diagnostic collection;
- preserve lazy query behavior;
- benchmark compile time/memory on large reflected types;
- retain crash/regression tests when compiler bugs are found and fixed.

The goal is not to avoid sophisticated standard-accurate C++26 code; the goal is to avoid **redundant** work.

---

## 13. Module-local implementation policy

Private Meta caches, helpers, validation records, and Trait machinery remain unexported in their owning module units.

They do not require `detail`/`internal` namespaces.

Private partitions are introduced only when multiple private module units genuinely need to share an implementation contract.

---


---

# Miracle Structural Programming Contract

**Status:** Locked  
**Parent:** `00-miracle-foundation-contract.md`

---

## 1. Scope

This document locks the structural-programming layer built on top of Miracle.Meta, Validation, Shapes, Traits, Iter, Result/Option, and the redesigned core vocabulary.

Included:

- Enum++;
- exhaustive Match;
- tagged/algebraic enums;
- core Derive behavior;
- Structural Transform.

Excluded from this contract:

- Diff/Patch implementation;
- Serde/Clap/RPC/ORM-specific derives;
- state-machine compiler;
- SoA/SIMD storage systems.

---

## 2. Enum++

Enum++ makes ordinary enums first-class reflected values without forcing wrapper types into every call site.

### 2.1 Public surface

Miracle supports both free functions and an advanced façade/cache form.

Examples:

```cpp
enumValues<Color>()
enumNames<Color>()
enumEntries<Color>()

enumName(Color::Red)
enumNames(Color::Red)     // all aliases for duplicate value
enumParse<Color>("Red")
enumIndex(Color::Red)
enumContains(Color::Red)
enumNext(Color::Red)
enumPrevious(Color::Red)
```

An `Enum<E>`/equivalent façade may expose the same operations and own cached implementation metadata underneath. Free functions remain the lightweight ordinary API.

### 2.2 Ordering

Declaration order is canonical.

For:

```cpp
enum class E {
    A = 50,
    B = -3,
    C = 900,
};
```

`enumIndex(E::B) == 1`, and next/previous semantics follow declaration order, never sorted underlying values.

### 2.3 Duplicate underlying values

Aliases are preserved in metadata:

```cpp
enum class E {
    A = 1,
    AliasA = 1,
};
```

The first declaration is the canonical single-name answer for `enumName(value)`. `enumNames(value)` returns all declared aliases in declaration order.

### 2.4 Annotation metadata

Enumerator annotations are exposed through the common Meta/Reflect cache and query system rather than a separate Enum metadata engine.

### 2.5 Formatting/parsing

Formatting/parsing integrate with Miracle Traits and diagnostics. Invalid parse returns `Result`/`Option` according to the exact API's failure semantics rather than raw sentinel values.

---

## 3. Match architecture

`match()` is domain/adapter-driven, not enum-specific.

The backend abstraction is a `MatchAdapter`/equivalent matchable-domain protocol. `define_aggregate` and synthesized handler aggregates are frontend tools, not the architecture itself.

### 3.1 Match domains

The matcher supports the relevant domain families:

- named closed sums — enums, Option, Result/expected-like values;
- typed sums — variant/tagged algebraic enums;
- products — tuples, pairs, reflected aggregates;
- sequences — ranges, cursors, Iter;
- user-defined custom adapters.

### 3.2 Exhaustiveness

`match(...)` is exhaustive by default.

A match expression must cover every possible alternative/pattern space recognized by the domain or provide an explicit default/otherwise branch.

Non-exhaustive matches fail at compile time with a Miracle diagnostic describing missing coverage.

Any deliberately partial matching facility must be explicit and differently named/configured; ordinary `match` never silently accepts missing cases.

---

## 4. Named sum syntax

For finite named alternatives, Miracle targets designated-member-like ergonomics:

```cpp
match(result, {
    .ok = [](auto&& value) { ... },
    .err = [](auto&& error) { ... },
});

match(option, {
    .some = [](auto&& value) { ... },
    .none = [] { ... },
});
```

For enums:

```cpp
match(state, {
    .idle = [] { ... },
    .running = [] { ... },
    .stopped = [] { ... },
});
```

`.ok/.err` and `.some/.none` are locked semantic spellings.

---

## 5. General pattern syntax

Arbitrary patterns use an `on(pattern, handler)`-style frontend plus explicit default/otherwise handling.

The complete Match phase includes all applicable families:

- named closed-sum alternatives;
- typed sum alternatives;
- literal/equality patterns;
- comparison/range patterns;
- type patterns;
- logical and/or/not composition;
- guards;
- tuple/product patterns;
- reflected aggregate patterns;
- sequence exact/prefix/suffix/rest patterns;
- bindings;
- nested patterns;
- custom adapters.

The implementation is incremental, but the phase is not complete until the contracted pattern families are delivered and tested.

---

## 6. Value-category and ownership preservation

Match never copies/owns a subject implicitly.

Conceptual behavior:

```cpp
match(value, ...)              // lvalue projections
match(constValue, ...)         // const projections
match(std::move(value), ...)   // ownership/move projections
```

Nested bindings preserve references/value category whenever safely representable.

Ownership transfer requires an explicit move or equivalent consuming operation.

---

## 7. Branch result typing

- if every branch returns `void`, Match returns `void`;
- otherwise Miracle derives one unambiguous compatible result type;
- references are preserved where safe;
- incompatible branch result types produce a Miracle diagnostic describing each relevant branch/type;
- Match does not silently erase results into `std::any`, a variant, or another dynamic container merely to make incompatible branches compile.

---

## 8. Sequence matching

Sequence matching respects actual range capabilities.

### 8.1 Safe structural decomposition

Canonical finite-sequence structural cases include concepts equivalent to:

```text
empty
head + tail
```

and richer `seq(...)` patterns.

### 8.2 Backtracking rule

Miracle never silently buffers or destructively consumes a single-pass range in order to pretend backtracking is possible.

Backtracking patterns require an appropriate multipass range or explicit buffering chosen by the caller.

This is a hard no-hidden-semantic-work rule.

### 8.3 `Range` patterns

`Range{a,b}` is a scalar/comparable interval pattern, not sequence-shape syntax.

Sequence shape remains explicit through sequence patterns such as `seq(...)`.

---

## 9. Tagged/algebraic enums

Miracle's preferred declaration model is annotation-driven enum-oriented ADTs.

Conceptual form:

```cpp
enum class TokenKind {
    [[= payload(^^String) ]]
    Identifier,

    [[= payload(^^double) ]]
    Number,

    Plus,
};

using Token = TaggedEnum<TokenKind>;
```

This is preferred over a purely type-list DSL because the declaration remains immediately recognizable as an enum domain and integrates directly with Enum++ metadata and named matching.

### 9.1 Representation

The public semantic model is stable while the storage strategy is replaceable/optimizable.

Version one may lower to a standard discriminated union where it satisfies the contract; later generated storage may be used when beneficial.

`Result` and `Option` are **not** implemented through the generic ADT engine merely for purity. They share Match/Iter/trait semantics but remain direct optimized foundational types.

---

## 10. Derive semantics

Derive is Trait-based.

Core syntax:

```cpp
[[= derive(debug, hash, bitflags) ]]
```

The object supplied to `derive(...)` is the same semantic Trait identity whose façade type exposes options:

```cpp
[[= debug::rename("position") ]]
```

### 10.1 Explicit opt-in

Semantic derives are explicit opt-ins unless a particular Trait explicitly defines a safe generic structural fallback.

Miracle does not assume that every reflectable aggregate wants every public semantic protocol.

### 10.2 Recursive capability

A derived operation may recursively require the corresponding capability on member types, but does not magically inject annotations into nested types.

### 10.3 Override precedence

Locked precedence:

```text
explicit/custom user implementation
  > requested derived implementation
  > generic structural fallback where permitted
```

### 10.4 Core derive set

This contract includes the core structural derives:

- debug;
- equality/comparison support;
- hash;
- visit;
- bitflags activation.

Future facilities contribute their own Traits/derives without changing the Derive engine.

---

## 11. Structural Transform engine

Transform is a generic structural type-transformation engine, not a collection of unrelated aliases.

It operates over reflected structural metadata and synthesized canonical types.

### 11.1 Core operations

The complete Transform engine supports applicable operations equivalent to:

- select fields;
- reject/remove fields;
- map field types;
- map/rename names;
- add fields;
- flatten;
- zip/merge compatible structures;
- qualifier/storage-policy transformations;
- annotation-aware transformations;
- composable transformation pipelines.

Conveniences such as:

```text
Optionalized<T>
Selected<T, ...>
Without<T, ...>
ReferenceView<T>
```

are compositions over this engine rather than one-off implementations.

### 11.2 Annotation propagation

Names/semantic identity may be preserved when the transformation semantics say they remain meaningful.

Arbitrary annotations are **not blindly copied**.

An annotation or transformation must explicitly define propagation semantics where the meaning may change. This avoids carrying `transient`, field IDs, DB metadata, serialization instructions, etc. into contexts where they become false or ambiguous.

### 11.3 Canonical generated types

Equivalent deterministic transformations must produce the same canonical C++ type rather than merely equivalent anonymous layouts.

For example, repeated identical selections of the same source fields must resolve to the same generated type identity.

Canonicalization is required for composability, caching, concepts, diagnostics, and compile-time performance.

---

## 12. Structural Shapes/Traits integration

Transform, Match, Enum, and Derive consume the common Shape/Trait system rather than inventing parallel customization protocols.

Examples of integration:

- `debug` is a Trait with a derivation strategy and custom implementation path;
- allocator/executor Traits can be structurally validated and adapted;
- Match adapters may themselves satisfy a Matchable Trait;
- transformations can require Shapes/Traits on selected fields;
- third-party Traits can participate in `derive(...)` when they provide derivation logic.

No inheritance/vtable is required for any of these protocols unless a future facility explicitly chooses runtime polymorphism for separate reasons.

---

## 13. Testing requirements

Structural programming tests include:

- exhaustive/non-exhaustive enum matching;
- duplicate enum values;
- weird underlying enum values;
- move-only/const/reference subjects;
- Result/Option Match integration;
- variant/tagged enum payloads;
- nested patterns;
- aggregate decomposition;
- single-pass sequence rejection/backtracking tests;
- custom adapter tests;
- incompatible branch-return diagnostics;
- derive override precedence;
- third-party Trait/derive extension tests;
- transform canonical type identity;
- annotation propagation correctness;
- very large reflected aggregate stress tests;
- compile-time diagnostic snapshots.


---

# Miracle Engineering & Delivery Contract

**Status:** Locked  
**Parent:** `00-miracle-foundation-contract.md`

---

## 1. Implementation method

Miracle is implemented facility-by-facility, but each facility is itself developed in small tested slices:

```text
small coherent subset
  -> implementation
  -> black-box tests
  -> white-box tests
  -> adversarial/negative-space tests
  -> compile-fail/diagnostic tests
  -> performance/compile-time checks
  -> next subset
```

A facility phase ends only when the agreed facility is complete. "Useful for now" is not a reason to intentionally omit contracted operations.

---

## 2. Testing philosophy

Miracle treats attempts to break the library as a first-class engineering activity.

Required categories include, where relevant:

- happy path;
- boundaries;
- compile-fail;
- invalid reflection;
- ownership/lifetime abuse;
- single-pass misuse;
- overload ambiguity;
- huge-type/compile-time stress;
- recursive/nested structures;
- unusual enum layouts/aliases;
- move-only/non-copyable types;
- throwing third-party types;
- constexpr execution;
- allocator failures;
- concurrency misuse;
- compiler-crash regressions;
- performance regressions;
- diagnostic regressions.

Property/fuzz-style testing should be used for large state-space facilities such as Iter, Result/Option, enum parsing, Match, containers, and Transform where practical.

---

## 3. Diagnostic snapshot testing

Good diagnostics are a contractual product feature.

Compile-fail tests should snapshot/check the meaningful diagnostic output/structure so a regression from:

```text
Player::transform is not formattable
```

into a long raw template-instantiation explosion is treated as a failing regression even though compilation still fails.

---

## 4. Switch integration

Switch powers Miracle tests but is not a runtime/library dependency of Miracle.

The test target pins a specific Switch release/tag rather than depending on a moving branch.

Switch itself gradually adopts Miracle facilities, but released baselines preserve an acyclic target dependency graph.

---

## 5. Compiler support

Miracle is written against the official latest C++26 standard contract, not against arbitrary conservative assumptions about specific compiler maturity.

When a compiler deviates from required standard behavior:

- record a minimal regression test;
- fix/report the compiler;
- do not redesign standard-accurate Miracle semantics merely to preserve the compiler bug.

When a desired behavior is genuinely not standardized and will not be portable to conforming GCC/Clang implementations, Miracle must redesign toward standard C++ rather than calling the limitation a compiler bug.

---

## 6. Compile-time performance

Compile time and compiler memory are first-class performance metrics.

Every reflection-heavy facility must actively reduce redundant work through:

- cached base reflection queries;
- lazy category initialization;
- cached common metadata;
- canonical generated types;
- bounded diagnostic collection;
- materialized-query caching only when profitable;
- avoiding redundant template recursion;
- reuse of expansion/static-storage results.

Stress benchmarks should exercise large types and large query/derive graphs.

---

## 7. Runtime performance

Runtime abstractions target zero or negative cost relative to the equivalent hand-written standard/native implementation.

Performance work includes:

- preserving laziness;
- fusing range operations;
- avoiding hidden allocation;
- allocator-aware storage;
- exact ownership/control-block semantics;
- no dynamic dispatch in structural Traits/Shapes;
- direct Match dispatch;
- cached metadata never leaking into runtime hot paths unnecessarily;
- explicit parallelism only.

---

## 8. Documentation quality

Every exported symbol is documented before phase completion:

- types/classes;
- concepts/Traits/Shapes;
- functions;
- methods;
- overload families;
- enums/enumerators;
- public data members;
- annotations;
- customization points;
- feature descriptors.

Private/module-local data structures and non-obvious invariants are also documented to a wgpu-like readability standard.

Comments explain purpose, semantic constraints, ownership, invariants, and non-obvious algorithms rather than merely restating syntax.

Complex internal chunks receive focused explanatory comments.

---

## 9. Design documentation

Each completed facility receives a dedicated lower-case kebab-case design document, for example:

```text
docs/result.md
docs/option.md
docs/iter.md
docs/range.md
docs/meta.md
docs/meta-validation.md
docs/shapes-traits.md
docs/match.md
docs/transform.md
docs/compiler-support.md
```

Each design document covers:

- purpose and non-goals;
- public API;
- every public method/structure/function/annotation;
- ownership/error semantics;
- compile-time semantics;
- backend implementation model where useful to users/contributors;
- quirks/constraints;
- examples;
- performance considerations;
- extension/customization points.

---

## 10. Changelog

`CHANGELOG.md` is updated with every externally meaningful coherent change.

Recommended categories:

```text
Added
Changed
Fixed
Removed
Deprecated
Performance
Diagnostics
```

`Performance` and `Diagnostics` are first-class because they are core Miracle quality metrics.

---

## 11. Commit discipline

Commits use meaningful Conventional Commit-style subjects with facility scopes:

```text
feat(iter): add adaptive tuple invocation
fix(result): preserve error during transition
perf(meta): cache reflected field metadata
docs(match): document sequence backtracking
```

Commit subjects are concise, imperative, and describe one coherent change. The body explains **what** changed and **why**, not just a diff narration, and includes verification/testing context where helpful.

After each coherent patch, provide the user:

- exact commit subject;
- commit body;
- CHANGELOG changes;
- relevant documentation changes;
- recommended pre-release tag when the checkpoint is externally meaningful.

Never automatically push to the public Miracle repository.

---

## 12. Tag/version strategy

Use SemVer-compatible prerelease progression, e.g.:

```text
v0.1.0-alpha.1
v0.1.0-alpha.2
v0.1.0-beta.1
v0.1.0-rc.1
v0.1.0
```

Not every tiny commit receives a tag.

Tags mark externally meaningful, fully passing checkpoints. Completed facilities are natural version milestones.

---

## 13. Module source layout policy

Prefer declaring and defining private/constexpr-friendly implementation directly in the owning module unit when sharing is unnecessary.

Do not mechanically split declaration and implementation merely to imitate header/source layouts.

Private partitions are justified only when multiple private module units need to share declarations/implementation.

This policy improves locality, constant evaluation visibility, and compile-time reasoning.

---

## 14. No production workarounds

Temporary implementation scaffolding is acceptable during an in-progress slice only when unavoidable.

Before a facility is declared complete:

- remove temporary hacks;
- replace provisional diagnostics;
- remove test-only semantic compromises;
- close TODO behavior;
- verify standard accuracy;
- document intentional limitations that are genuinely outside the contract rather than unfinished work.

---

## 15. Phase ordering

The recommended implementation sequence is:

1. module/capability adaptation + `StaticString`;
2. Feature engine bootstrap;
3. diagnostics + `panic` foundation;
4. `Range`;
5. Iter + adaptive invocation + ParallelIter groundwork;
6. Meta abstraction + Reflect caching + Meta Query;
7. Validation;
8. Structural Shapes & Traits;
9. **Core Revisit milestone** — redesign existing Miracle vocabulary comprehensively;
10. Enum++;
11. Match v1 through complete Match phase slices;
12. tagged/algebraic enums;
13. Derive core;
14. Structural Transform;
15. final integrated stress/performance/diagnostic validation of the contract.

Within each numbered phase, implementation proceeds through small tested subsets; a phase is complete only when its contracted surface is complete.

---

## 16. First deferred follow-on contract

After this contract is fully implemented, the first planned follow-on facility is Structural Diff/Patch because it immediately exercises Transform, Traits, Result/Option, diagnostics, and future Serde/config/network use cases without pulling general facilities into this contract prematurely.


---

# Miracle Design Decision Ledger — Q1–Q238

**Purpose:** Audit trail for the three design-question rounds that produced the locked Foundation & Structural Programming contract. Later answers supersede earlier uncertainty where noted.

---

## Q1–Q78 — Scope and first architecture round

1. **Contract boundary:** Foundation + Iter/Meta/Validation/Structural Concepts/Interfaces + Enum/Match/ADT/Derive/Transform. Diff/Patch is the first deferred facility; Serde/Clap/etc. are later contracts.
2. **UUID:** Deferred; full v1–v8 support is a future facility. The generic Feature engine is foundational instead.
3. **Phase completeness:** A phase is not done until its contracted feature surface, tests, diagnostics, docs, stress/performance work, and production cleanup are complete.
4. **Umbrella module:** `import Miracle;` remains the batteries-included entry point; narrower modules are also available.
5. **Module granularity:** Semantic public modules; use C++ module partitions (`:Partition`) rather than fake `.Internal` modules.
6. **`Miracle.Core`:** Rejected; avoid dumping-ground modules.
7. **Naming:** PascalCase ordinary namespaces/types; camelCase functions; Rust-inspired annotation/feature façades use concise lowercase/camelCase naming.
8. **Inference/QOL:** Aggressive CTAD/inference where unambiguous. Error propagation must avoid repetitive ownership-transfer spelling; ordinary return should naturally transfer eligible locals.
9. **Core revisit:** Formal mid-contract redesign of the existing Miracle library, not just Result/Error.
10. **Compatibility:** Pre-1.0 APIs may be broken when necessary for a better architecture.
11. **Review breadth:** Revisit aliases/types such as Result/Option/String/Vec/maps/ownership/File/OpenOptions/Concepts/Error; wrappers are introduced only when they add real semantics. This later expands into Rust-like core vocabulary.
12. **Result genericity:** `Result<T, E = Error>`; custom typed errors first-class.
13. **Void Result:** `Result<void, E>` supported.
14. **Reference success:** `Result<T&, E>` supported.
15. **Result storage:** Own Miracle discriminated-union implementation rather than being limited by `std::expected` reference restrictions.
16. **Factories:** `Ok(...)` and `Err(...)`; implicit construction where unambiguous.
17. **Result methods:** Full applicable Rust-like monadic/query surface, not a deliberately small subset.
18. **Unwrap/expect failure:** Runtime `panic`/terminate path, not exceptions.
19. **Boolean QOL:** explicit `operator bool()` delegates to `isOk()`.
20. **`std::expected` interop:** Ergonomic conversion when lossless/unambiguous; later refined to conditional implicit/explicit conversion.
21. **Result + Iter:** Ok yields one success item; Err yields none.
22. **Result Match names:** `.ok` / `.err` locked.
23. **Option:** Real first-class `Option<T>` with `Some(...)`, `None`, Iter and Match (`.some/.none`) integration.
24. **Option implementation:** Public Miracle semantics may use `std::optional` internally where it does not constrain the contract.
25. **Default Error:** Universal `Error` remains convenient default while typed Result errors remain first-class.
26. **Error/Diagnostic:** Error stays compact and produces rich diagnostics lazily; diagnostic creation itself must be redesigned.
27. **Error context:** Cause/context chaining is foundational.
28. **TRY macro:** Macro solution rejected for module ergonomics; use monadic/branching now and wait for a future language-native/generative mechanism.
29. **Diagnostic vocabulary:** One Miracle diagnostic vocabulary shared by runtime facilities and compile-time validation conceptually.
30. **Error vs Diagnostic:** Separate semantic failure (`Error`) from presentation/explanation (`Diagnostic`).
31. **Validation surfaces:** `valid`, `validate`, `require` all adopted.
32. **Violation count:** `valid` short-circuits; validation collects a bounded useful set; require emits root + bounded violations.
33. **Validation generality:** One engine for types, members, enumerators, functions, parameters, bases, annotations, etc.
34. **Rule messages:** Rules may provide rich custom compile-time diagnostics/help text.
35. **Switch bootstrap:** Miracle library never depends on Switch; Miracle tests depend on a pinned Switch, yielding an acyclic target graph.
36. **StaticString generality:** `BasicStaticString<Char,N>` plus `StaticString<N>` alias.
37. **Conversion rule:** non-owning conversions implicit; owning conversions explicit, subject to ambiguity/safety.
38. **StaticString completeness:** Full useful compile-time string vocabulary, not only lower/upper.
39. **Iter laziness:** Iter pipelines are lazy by default.
40. **Iter vocabulary:** Rust Iterator is canonical reference; standard C++ aliases may exist when helpful.
41. **Meta Query vocabulary:** Reuse Iter vocabulary for compile-time reflected sequences.
42. **Adaptive invocation:** Direct callable invocation wins; decomposition is fallback.
43. **Decomposition order:** tuple protocol first, reflected aggregate decomposition afterward.
44. **Capability truthfulness:** Iter never lies about sized/forward/bidirectional/etc. capabilities and never buffers merely to fake them.
45. **Iter construction:** One universal `iter(...)` surface adapts ownership/borrowing automatically.
46. **Meta abstraction/cache:** Hide most raw `std::meta`; use cached `Reflect<T>` underneath `meta::*`; keep a raw escape hatch.
47. **Meta syntax:** Fluent value-style query API; optimize redundancy rather than avoiding sophisticated standard C++26.
48. **Reflection universe:** fields/members/functions/constructors/parameters/bases/enumerators/annotations/template metadata where standardized.
49. **Access:** Access is a function parameter defaulting to current context; explicit unchecked contexts remain available; `fields` preferred over `nsMembers`.
50. **Compiler cost:** Compiler resource use is a first-class metric; do not avoid standard-accurate designs merely because they expose custom-compiler bugs.
51. **Structural concepts/interfaces:** One shared substrate; ergonomic Shape-like DSL strongly favored.
52. **Structural constraints:** Requirements support concepts/convertibility/method constraints, not only exact field types.
53. **Enum API:** Support both free functions and a façade/cache implementation.
54. **Enum ordering:** Declaration order is canonical.
55. **Duplicate enum values:** Preserve aliases; first declaration is canonical single name; expose all names separately.
56. **Match exhaustiveness:** `match()` is exhaustive by default or fails at compile time.
57. **Branch types:** derive an unambiguous common result; incompatible branches are a Miracle diagnostic; no dynamic erasure fallback.
58. **Value categories:** Match preserves lvalue/const/rvalue ownership/reference semantics; no implicit copying.
59. **Match completeness:** Named sums, typed sums, values/ranges/types/logical patterns/guards/products/aggregates/sequences/bindings/nesting/custom adapters all belong to completed Match.
60. **Single-pass matching:** No implicit buffering/backtracking/destructive speculative consumption.
61. **ADT declaration:** Prefer annotation-driven enum-oriented tagged/algebraic enums.
62. **Result/Option vs ADT:** Foundational Result/Option remain direct optimized types; do not route them through generic ADT machinery.
63. **Derive QOL:** Desired syntax is `[[= derive(debug, serde, clap, bitflags, hash) ]]` plus feature-specific annotations. Later solved by same-name Trait façade type + constexpr object.
64. **Recursive derive capability:** Nested types must satisfy the required capability; no magical transitive annotation injection.
65. **Customization precedence:** explicit user implementation > requested derive > generic structural fallback where allowed.
66. **Core derives:** Debug, Eq/comparison, Hash, Visit; later facilities contribute their own derives.
67. **Transform:** One generic structural transformation engine, not unrelated helper aliases.
68. **Annotation propagation:** Use strict explicit propagation semantics; do not blindly copy arbitrary annotations into transformed types.
69. **Generated identity:** Equivalent transforms must yield canonical identical generated types.
70. **Negative-space testing:** Formal categories for misuse, lifetime, invalid reflection, huge types, weird enums, move-only types, compiler regressions, etc.
71. **Diagnostic snapshots:** Compile-fail diagnostics are regression-tested.
72. **Property/fuzz tests:** Use where practical for broad state-space generic facilities.
73. **Compiler support:** Capability-based, not vendor-version based.
74. **Missing capabilities:** clean module disablement plus compatibility implementations where standard-accurate/practical.
75. **Compiler bugs:** Do not weaken standard-accurate design to avoid discovering bugs; fix/regress them instead.
76. **Zero/negative cost:** Hard runtime abstraction principle; Miracle should often outperform naïve hand-spelling through safe optimization.
77. **Compile-time performance:** Equal importance as a performance budget; reflection/query caching is central.
78. **Priority order:** correctness > simple caller API > diagnostics > runtime performance > compile-time performance > generality/extensibility > implementation simplicity.

---

## Q79–Q164 — Feature engine and core-vocabulary round

79. **Feature engine scope:** Foundational and included now; UUID remains future. It allows compile-time selected capabilities/modules and third-party features.
80. **Feature levels:** Separate build features from semantic/local features.
81. **Feature sets:** Support global/default and explicit local FeatureSets.
82. **Feature representation:** Prefer first-class structural values/descriptors over specialization-heavy marker types where practical.
83. **Feature descriptor:** Rich metadata: name/version/dependencies/optional dependencies/conflicts/implies/capabilities/default/description/groups and future ecosystem metadata.
84. **Versioned features:** Version-specific features imply a parent feature (future UUID versions included).
85. **Conflicts:** Mutually-exclusive features resolved/diagnosed at compile time.
86. **Groups:** Feature groups expand to useful sets.
87. **Resolution:** Transitive, deterministic, canonical dependency resolution.
88. **Annotation requirement:** concise `[[= requires(serde::json) ]]`-style feature requirements.
89. **Build bridge:** Build definitions/generated configuration module feeds typed FeatureSet; macros stay at boundary.
90. **Build identity:** Feature configuration is part of module/BMI/build identity.
91. **Raw success construction:** `Result<int>` may `return 42;` when unambiguous.
92. **Raw error construction:** `return error;` becomes Err when it cannot be success; ambiguity requires explicit Ok/Err.
93. **Reference errors:** `Result<T, E&>` supported as general-purpose case.
94. **Never valueless:** Hard Result invariant.
95. **Triviality:** Propagate trivial copy/move/destruction where alternatives permit.
96. **Constexpr:** Aggressively `constexpr` APIs even when definitions must stay visible in module interfaces.
97. **Wrong-state Result access:** rich `panic`, never Miracle exception.
98. **Value-category `ok/err`:** refs for lvalues/const refs, owned value for rvalues; const is intentionally non-consumable if elision is unavailable.
99. **Taking:** Rust-style/consuming take operations supported appropriately.
100. **Result dereference:** `operator*`/`operator->` QOL supported.
101. **Propagation style:** Both monadic chains and ordinary branching; choose the clearest for each case.
102. **`Err(lvalue)`:** normal function-call semantics; do not mysteriously consume lvalue. Eligible return operands still benefit from language implicit move.
103. **None:** `None` is an object; `Some(value)` is a factory.
104. **Option references:** `Option<T&>` with rebind-on-assignment semantics.
105. **Exceptions:** Forbidden as Miracle's expected-error model; `Result` for recovery, `panic` for unrecoverable. Exceptions tolerated only where unavoidable interop requires them.
106. **Allocation:** Normal container operations preserve STL-like behavior; `try*` operations expose Result. Allocator customization must allow arena/kernel-like environments.
107. **`constexpr`/`noexcept`:** Use aggressively when semantically truthful; avoid exceptions as Miracle control flow.
108. **String/Vec wrappers:** Real composition-based Miracle types, not public inheritance.
109. **STL compatibility:** Preserve range/contiguous/category semantics and interoperability of underlying standard-like containers.
110. **String invariant:** Initially debated; later Q168 resolves `String` as valid UTF-8.
111. **Text iteration:** `bytes()` and `chars()` are first-class; no misleading character indexing.
112. **StringView invariant:** Aligned with valid UTF-8 String model; explicit unsafe/unchecked escape where necessary.
113. **Container names:** Prefer explicit `HashMap`, `HashSet`, `BTreeMap`, `BTreeSet` over ambiguous Map/Set.
114. **Vocabulary consistency:** Rust-inspired member naming across String/Vec/maps/Option/Result/File/Mutex with useful C++ aliases.
115. **Wrapped integers:** Debated; later Q171 decisively rejects wrapper and keeps native aliases/free numeric helpers.
116. **Ref non-null:** `Ref<T>` is always non-null; optionality uses `Option<Ref<T>>`.
117. **Ref assignment:** rebinds rather than assigns through referent.
118. **Ref ergonomics:** `operator*` and `operator->` accepted because C++ lacks overloadable `operator.`; avoid mandatory `.get()` ceremony.
119. **Raw-reference fields:** Personal Miracle style, not globally enforced on developers.
120. **Box:** becomes Miracle unique-owner vocabulary; map idea explored and later constrained by exact layout reuse.
121. **Rc:** custom non-atomic refcount, not `shared_ptr`.
122. **Arc:** choose custom Miracle control-block implementation too for exact semantics/performance/allocator control.
123. **Weak ownership:** choose distinct `WeakRc<T>` and `WeakArc<T>` rather than exposing an Ownership policy template.
124. **Mutex encapsulation:** protected state cannot be obtained without locking; underlying raw mutex not normal API.
125. **Poisoning:** discussed; later locked out because Miracle panic terminates rather than unwinds.
126. **Unlock:** no public unlock; guards own release.
127. **tryLock:** `Result<Option<MutexGuard<T>>>` distinguishes system failure, busy, acquired.
128. **RwLock:** one `RwLock<T>` with read/write guard types; not separate ownership-parameterized lock types.
129. **Reflect API:** `meta::*` lightweight adapters over cached `Reflect<T>` façade.
130. **Lazy reflection caches:** hard requirement.
131. **Derived query cache:** smart materialization/canonical-query caching, not blind cache of every intermediate.
132. **Rich metadata:** cache commonly requested immutable names/types/annotations/source data.
133. **Raw reflection escape:** both `meta::raw(x)` and `.raw()` may exist.
134. **Access alias:** `Access` preferred over `AccessContext`.
135. **Access wrappers:** no redundant wrapper namespace; standard alias already provides needed factories.
136. **Reusable Shapes:** named reusable structural contracts are foundational and should be pushed far.
137. **Structural adapters:** Shape can produce zero-cost structural view/adaptor.
138. **Method requirements:** full overload/qualifier/noexcept/return/parameter/annotation/static/member support.
139. **Derive syntax:** dream syntax kept unresolved until later same-name façade solution.
140. **Derive extensibility:** third-party custom derives/extensions are first-class.
141. **Derive dependencies:** derive dependencies/conflicts can reuse Feature engine.
142. **Bitflags API:** full Rust-like flag set operations.
143. **Unknown bits:** preserve/retain/truncate semantics are distinct and supported.
144. **Composite flags:** named composite enumerators allowed and validated distinctly from atomic flags.
145. **Flag conversion:** enum -> flags convenient; later Q188 reverses earlier idea and rejects implicit flags -> enum to preserve closed-enum semantics.
146. **Bitflags façade/functions:** both façade methods and free-function conveniences point to common implementation.
147. **panic runtime:** `[[noreturn]]`, Diagnostic then terminate, never throw.
148. **panic inputs:** adapt strings/Error/Diagnostic/diagnostic-formattable sources into one sink.
149. **Compile-time failure:** keep runtime panic separate from `meta::require`/`meta::fail`.
150. **Diagnostic builder:** fluent builder API hard-locked; no awkward public field mutation.
151. **Typed preset diagnostics:** optional presets can exist, but do not over-engineer strict schema templates as the primary model.
152. **Cause rendering:** Error cause chain rendered into Diagnostic rather than manually nesting diagnostics during propagation.
153. **File API:** Rust File is canonical ergonomic reference; complete read/write/sync/metadata surface.
154. **OpenOptions:** no-argument fluent toggles plus optional bool overloads.
155. **Path:** redesign with Path/PathView in the same core IO vocabulary.
156. **Close:** explicit `close() -> Result<void>`; destructor closes fallback and ignores/reporting-unavailable failure until future logging exists.
157. **Native handle:** `file.nativeHandle()` directly returns platform handle; backend may use C++26 `native_handle()` internally.
158. **Parallel Iter syntax:** `iter(...).parallel()` chosen over separate `parIter`; returns distinct adaptor.
159. **Parallel result determinism:** preserve ordered final results when source/order semantics make sense.
160. **Commit/tag granularity:** coherent slices commit; externally meaningful fully passing checkpoints tag; completed facility is a natural milestone.
161. **Versioning:** SemVer-compatible alpha/beta/rc progression.
162. **CHANGELOG:** Added/Changed/Fixed/Removed/Deprecated/Performance/Diagnostics categories.
163. **Docs naming:** later resolved to lowercase kebab-case for consistency with existing docs.
164. **Documentation completeness:** every exported API and meaningful private invariant documented before completion.

---

## Q165–Q238 — Final architecture round

165. **Allocator shape:** foundational untyped allocator model using layout/block semantics; future Trait expression.
166. **Container storage types:** separate `Vec`, `InlineVec`, `SmallVec`; do not overload Vec with fixed storage semantics.
167. **Fallible containers:** normal operations preserve familiar behavior; `try*` variants expose allocation/capacity errors.
168. **Text/byte model:** lock `String`/`StringView` as valid UTF-8; arbitrary bytes are `Vec<u8>`/`Span<u8>`. Introduce ecosystem `Range` and slicing integration.
169. **UTF-8 creation:** fallible validated construction plus explicit unchecked escape.
170. **Path names:** `Path` owning, `PathView` borrowed; C++ View naming preferred over Rust PathBuf terminology.
171. **Primitive integers:** lock native aliases (`u32 = std::uint32_t` etc.); add free numeric convenience functions instead of wrapper primitives.
172. **Ref const conversion:** `Ref<T> -> Ref<const T>` and `Ref<T> -> T&` implicit; reverse const removal forbidden.
173. **Box mapping:** allowed when exact size/alignment permit same allocation reuse; later final spelling chosen as constrained `map<U>` for simplicity.
174. **Rc/Arc COW:** include `getMut` and `makeMut`/copy-on-write semantics where value Traits permit.
175. **Allocator + Traits:** ownership types allocator-aware; Shape/Trait can describe Allocator structure instead of OOP inheritance.
176. **Weak names:** `WeakRc`/`WeakArc` preferred.
177. **Timed locks + chrono:** timed lock vocabulary included; chrono itself stays aliases/re-exports; standard literals re-exported through Miracle literals.
178. **Guards:** movable, non-copyable hard lock.
179. **Disabled feature API:** keep lightweight diagnostic façade visible; heavy disabled implementation does not compile/import.
180. **Build/local feature invariant:** local FeatureSet cannot enable unavailable build feature.
181. **Feature cycles:** compile-time cycle diagnostic including dependency cycle.
182. **Third-party feature syntax:** same structural descriptor/façade mechanism as built-ins; later can integrate with Traits.
183. **Module-private cache:** unchecked reflection universe/cache stays private; no exported `detail/internal`; module export boundary is privacy mechanism.
184. **Reflection metadata cache:** aggressively cache common immutable metadata.
185. **Query cache:** base always cached; materialized canonical results cached intelligently, not every lazy intermediate.
186. **Result transition safety:** use nothrow/rollback constraints; operations unable to preserve two-state invariant are constrained out rather than expose valueless state.
187. **`noexcept`:** conditional/truthful according to contained operations; no dishonest blanket specification.
188. **Bitflags reverse conversion:** enum -> flags implicit, flags -> enum explicit because combinations may not be declared enumerators.
189. **Bitflags activation:** both `Bitflags<E>` direct use and `[[= derive(bitflags) ]]` QOL.
190. **Same-name façade:** hard-lock type/object lookup trick subject to exhaustive standard/compiler tests.
191. **Trait façade split:** lower-case object is trait/derive identity; static type members expose annotations/options.
192. **Associated items:** Shapes/Traits support associated types/constants/functions.
193. **Trait view:** `as<Trait>(value)`/equivalent zero-cost reference façade; no runtime dynamic dispatch.
194. **File completeness:** cursor/seek/read/write surface included.
195. **Path semantics:** lexical construction, explicit normalize/canonicalize, `/` QOL composition.
196. **ParallelIter policy:** distinct adaptor stores max concurrency; custom number allowed.
197. **Parallel determinism:** execution interleaving unspecified, ordered final result deterministic where semantically meaningful.
198. **No automatic parallelism:** only explicit `.parallel()` starts parallel semantics.
199. **Docs naming:** lowercase kebab-case locked.
200. **Commit writing:** follow the agreed meaningful Conventional Commit discipline/article guidance.
201. **No hidden work rule:** no hidden semantic cost; hidden semantics-preserving optimization encouraged.
202. **Range family:** `Range<T>` supports integral index types; separate inclusive/unbounded forms preserve type-level capabilities; integrate Range broadly across ecosystem.
203. **Range step:** Range is unit-stride; `stepBy`/`iter(start,end,step)` provide stepping.
204. **Iter slicing:** `.slice(Range)` plus raw-integer overloads; `.take(n)` remains count-limiting.
205. **Container slicing:** contiguous containers/views support `operator[](Range) -> Span` with const correctness.
206. **Checked indexing/slicing:** `[]` panics on invalid bounds; `.get(...)` returns Option.
207. **UTF-8 slicing:** String range boundaries must be UTF-8 code-point boundaries; checked get returns None when invalid.
208. **Range Match semantics:** scalar/comparable interval pattern; sequence shape stays `seq(...)`.
209. **Shape vs Trait:** Shape = automatic structure; Trait = semantic protocol built on structural requirements.
210. **Trait impl marker:** `[[= impl(...) ]]` is optional hint/registration; not mandatory boilerplate for every valid implementation. External adapters also supported.
211. **Trait defaults/supertraits:** Rust-like default methods and inheritance-like supertrait requirements supported without OOP.
212. **Trait completeness:** associated types/constants/static/member methods/qualifiers/annotations/defaults all included.
213. **Trait adaptation:** generic `as<Trait>(value)` accepted when constraints are satisfied.
214. **Feature bootstrap:** Feature bootstrap remains independent; Trait enhancement occurs after bootstrap without creating dependency cycle.
215. **Allocator failure:** `allocate` fundamentally returns Result; infallible container façade panics, `try*` propagates.
216. **Allocator storage:** stateful allowed; stateless storage optimized away.
217. **Box map spelling:** use simpler constrained `box.map<U>(...)`; docs clearly state exact layout requirement.
218. **Box layout:** exact `sizeof` and `alignof` equality; no hidden original-layout metadata.
219. **Ownership COW completeness:** `getMut` and `makeMut` included in ownership phase.
220. **Lock errors:** only actual platform/system failures, no poisoning.
221. **Timed locks:** same Mutex/RwLock types expose timed operations where supported; no TimedMutex family solely for this.
222. **Chrono:** aliases/re-exports only until a real deficiency appears.
223. **Literals:** Miracle-owned/re-exported literals live under `Miracle::literals`.
224. **StringView literal:** no custom `_sv`; rely on compatible/validated conversion from standard string-view forms.
225. **Unicode normalization:** valid UTF-8 invariant only; preserve exact code points; normalization explicit/later.
226. **Path encoding:** native platform representation; String conversion explicit/fallible where necessary.
227. **Result never-valueless strategy:** expected-like constraint/rollback philosophy; unsafe transitions unavailable with good diagnostics.
228. **Third-party exceptions:** preserve conditional exception specification; do not catch arbitrary contained-type exceptions and pretend they are Miracle errors.
229. **Parallel N:** maximum concurrency (`0` auto, `1` serial, `N` at most N workers).
230. **Custom executor:** full ParallelIter supports `.parallel(executor)`; Executor eventually a Trait.
231. **Default executor:** reusable runtime/worker state, never per-terminal OS-thread creation.
232. **Bitflags formatting:** symbolic known flags + retained unknown bits; numeric formatter specs support binary/hex/octal with underlying-width behavior.
233. **Trait/derive identity:** Traits can provide derive strategies; traits such as debug support custom implementations analogous to `impl Debug for T`; ecosystem extensibility is a defining goal.
234. **Façade hard lock:** same-name trait type + constexpr identity object enables `derive(debug, ...)` and `debug::rename(...)`.
235. **Module privacy:** private definitions stay unexported in owning module; internal partitions only for genuine private sharing; never export implementation namespaces.
236. **Private docs:** document private invariants/structures with the same readability philosophy as public code when non-trivial.
237. **Commit scopes:** facility-scoped Conventional Commit subjects such as `feat(iter)`, `fix(result)`, `perf(meta)`.
238. **Structural layer name:** formally becomes **Structural Shapes & Traits** and is part of this contract/foundation.

---

## Final superseding decisions

Where early questions remained uncertain, the later lock is authoritative. In particular:

- no TRY macro/header now;
- String/StringView are valid UTF-8;
- native integer aliases remain aliases, not wrapper classes;
- flags -> enum is explicit;
- same-name Trait façade/object syntax is the hard direction;
- Weak ownership types are `WeakRc`/`WeakArc`;
- no lock poisoning;
- docs use lowercase kebab-case;
- `Box::map<U>` is constrained to exact allocation-layout reuse;
- Structural Interfaces are finalized as Structural Shapes & Traits.

