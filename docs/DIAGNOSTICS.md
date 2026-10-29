# Diagnostics and panic

Miracle currently introduces structured diagnostic foundation and the terminal `panic()` boundary. The design deliberately separates diagnostic construction, rendering, error adaptation, contract adaptation, and process termination.

## Modules

`import Miracle;` re-exports:

```text
Miracle:Diagnostic
Miracle:Error
Miracle:Panic
```

The dependency direction is one-way:

```text
Types / StaticString -> Diagnostic -> Error -> Panic
```

## Diagnostic code domains

A diagnostic code is an enum accepted by `DiagnosticCode`. The enum itself carries the domain policy through C++26 annotations:

```cpp
enum class [[
    = diagnostics::prefix("E"),
    = diagnostics::align(3)
]] ParseCode : u8 {
  Unknown = 0,
  UnexpectedToken [[= diagnostics::message("unexpected token")]] = 12,
};
```

Prefix-less domains use the marker directly:

```cpp
enum class [[= diagnostics]] InternalCode : u8 {
  Failed = 1,
};
```

The annotation vocabulary is intentionally lowercase so declarations read as a small diagnostic language:

```text
[[= diagnostics]]
[[= diagnostics::prefix("E")]]
[[= diagnostics::message("...")]]
[[= diagnostics::align(3)]]
```

`diagnostics::align(n)` is a minimum zero-padded numeric width. It never truncates larger values:

```text
7    -> E007
42   -> E042
123  -> E123
1234 -> E1234
```

Alignment requires a prefix. Negative declared values are rejected. Enum aliases are valid and the first declared enumerator with a value is canonical. An enumerator's message annotation is canonical when present; otherwise its identifier is used directly, without a dependency on Debug formatting.

`diagnostics::align(automatic)` remains reserved. `Automatic` is planned as a generic Core vocabulary type and is not introduced locally by Diagnostics.

At runtime each enum code is normalized into `DiagnosticCodeDescriptor`:

```text
domain
prefix
enumerator
canonical message
numeric value
resolved alignment
```

`Diagnostic` itself is therefore not templated on the source enum. A runtime value not matching any declared enumerator is still representable: its numeric value/prefix/alignment are preserved and the normalized enumerator/message are `<unknown>` / `unknown diagnostic code`.

## Severity

```cpp
enum class Severity : u8 {
  Debug,
  Note,
  Warning,
  Error,
};
```

There is intentionally no Trace severity. Runtime event/span tracing belongs to the future `Miracle.Tracing` facility.

## Move-only construction

`Diagnostic` and `DiagnosticSpan` are move-only. Their fluent mutators use explicit-object rvalue receivers, making construction pipelines natural while preventing casual mutation of completed lvalues:

```cpp
auto diagnostic = Diagnostic::error(ParseCode::UnexpectedToken)
    .message("expected expression")
    .span(DiagnosticSpan{"foo + )"}
        .select({.begin = 6, .end = 7})
        .label("unexpected token"))
    .note("the expression started here")
    .help("remove ')' or provide an operand");
```

A code alone is already a complete diagnostic because it supplies the canonical message. `.message(...)` overrides that message only for one occurrence.

## Explicit source spans

`DiagnosticSpan` owns explicitly supplied snippet text, a source location, an optional byte range, label, and primary/secondary role. Rendering never opens `source_location::file_name()` behind the caller's back. Source files are I/O, and diagnostics do not perform hidden I/O.

Range endpoints are clamped to the supplied snippet before any indexing. A reversed range is normalized after clamping. This remains memory-safe even when contracts are configured to ignore invalid caller input.

## Rendering

The free rendering API accepts:

```cpp
enum class ColorMode : u8 { Automatic, Always, Never };
enum class SourceMode : u8 { None, Location, Snippet };
enum class DetailMode : u8 { Compact, Normal, Full };
enum class Presentation : u8 { Plain, Terminal };
```

`Presentation::Plain` is an absolute ANSI boundary: it never emits terminal escape sequences. `ColorMode::Never` also disables color. Full detail includes the normalized diagnostic domain/enumerator/value, while compact detail omits stacktrace and metadata expansion.

Rendering is implemented by an unexported stateful `DiagnosticRenderer` so the public surface remains small and output state stays local to one operation.

## Cause trees

A diagnostic may own child causes. Rendering uses parent/index DFS frames rather than native recursion or a sibling queue:

```text
time                O(number of diagnostics rendered)
aux traversal space O(cause-tree depth)
native recursion    O(1)
```

The frame retains only the current parent, next child index, and depth. Wide cause sets therefore do not create an O(width) pending queue. This traversal choice is deliberate because diagnostics may be produced by failed systems where deep or wide error graphs should not amplify failure through call-stack exhaustion.

## Error bridge

The existing Error representation is retained for now. `diagnose(error)` converts its ordered messages into a causal Diagnostic chain. The newest outer message becomes the root and earlier messages become nested causes in order.

Legacy `fatal(...)` is removed completely; there is no compatibility alias. `std::formatter<Error>` formats through a const reference so a move-only Error is never copied.

## C++26 contracts

Contract violations adapt to this domain:

```cpp
enum class [[
    = diagnostics::prefix("CTR"),
    = diagnostics::align(3)
]] ContractCode : u8 {
  Precondition = 1,
  Postcondition = 2,
  Assertion = 3,
};
```

`diagnose(const std::contracts::contract_violation&)` maps:

```text
kind()
semantic()
detection_mode()
location()
comment()
is_terminating()
```

A non-empty contract comment overrides the canonical code message. Semantic, detection mode, and terminating state are retained as diagnostic notes.

Miracle currently does not install the global `::handle_contract_violation`. Final handler replaceability waits for the Trait Engine because that customization boundary is implementation-defined.

## Panic

The terminal API is:

```cpp
enum class TraceMode : u8 {
  None,
  Current,
};

struct PanicOptions {
  RenderOptions render{};
  TraceMode trace{TraceMode::Current};
  bool breakpoint{true};
};

[[noreturn]] auto panic(Diagnostic, PanicOptions = {}) noexcept -> void;
[[noreturn]] auto panic(const Error&, PanicOptions = {}) noexcept -> void;
[[noreturn]] auto panic(StringView,
    PanicOptions = {},
    std::source_location = std::source_location::current()) noexcept -> void;
```

The terminal pipeline is fixed:

```text
build/adapt Diagnostic
capture std::stacktrace when requested
render to stderr and flush
std::breakpoint_if_debugging()
std::terminate()
```

Rendering happens before the debugger breakpoint so diagnostic text survives when a debugger changes control flow. Stacktrace capture and structured rendering are the only exception-catching boundary in this terminal path. If either throws, panic attempts a minimal no-formatting stderr fallback and still terminates.

## Capabilities

Miracle currently requires executable probes for:

```text
contracts
  contract_violation field inspection

debugging
  is_debugger_present + breakpoint_if_debugging

stacktrace
  std::stacktrace::current through import std

literal_utf8
  std::text_encoding::literal() == std::text_encoding::UTF8
```

The reflection annotation probe additionally exercises `annotations_of_with_type`. The generated `Miracle::capabilities` partition exposes the resulting booleans; no compiler version checks participate in feature detection.

## Complexity and allocation

Normalized descriptors own strings so diagnostics do not retain reflected enum types at runtime. Diagnostics own their spans, notes, help text, causes, and optional stacktrace. Rendering is streaming except when the String-returning free overload explicitly accumulates output.

Cause traversal allocates only its depth-bounded frame vector. Source rendering operates exclusively on explicitly supplied snippet bytes owned by the span and does not allocate or perform filesystem access merely to resolve a location.

## Non-goals

Miracle currently intentionally does not provide:

- a global contract-handler customization mechanism;
- general tracing/event spans;
- a local Diagnostics `Automatic` vocabulary type;
- Trait/Validation integration;
- a comprehensive Error redesign;
- hidden source-file loading;
- legacy `fatal` compatibility.

Those boundaries keep Diagnostics usable immediately without pulling later Core facilities forward in the implementation order.
