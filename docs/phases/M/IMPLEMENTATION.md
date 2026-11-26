# Miracle — Phase M: Module Migration
## Software Design & Engineering Specification, v2.2 (Author-Approved Architecture; Compiler Verification Pending)

**Document class:** Blueprint design, migration plan, API reconciliation, implementation and verification specification.
**Source provenance:** v2.1 recovery baseline, 2026-10-30. Original v2.0 source and 109-page PDF preserved unchanged. Appendix D supplies recovered continuity, post-interruption corrections, detailed event-writer and type-deduction seams, additional narrow host probes and an explicit readiness checkpoint.
**Revision status:** Original historical approvals preserved; the seven explicit 2026-10-30 decisions are approved and incorporated in the normative reconciliation.
**Authority:** Miracle Development Protocol v1.0; binding `docs/phases/M/DESIGN.md` v2.0; expressly approved owner decisions. API conformance still requires reference-toolchain evidence.
**Current remote source baseline:** `spwn02/Miracle@e2da473e6dd5637f1f70625738f8378e211a7478`, observed 2026-10-30. **This is already a partially migrated tree.**
**Prior Phase M contract:** v1.0 at commit `469083ed1b30e3770667e1fe35473e9f96ed1e7a`; superseded by restored, approved v2.0 `docs/phases/M/DESIGN.md` in this documentation adoption.
**Previous implementation guidance:** `Miracle-Phase-M-Implementation-Guide-v1.1.md`; its duplicated Builder proposal is superseded by the current unified Format design.
**Repository mutation:** documentation-only adoption proposed; no production source mutation by the reviewer. **Implementation and reference-compiler execution:** NONE performed by this specification; independently identified narrow host-language probes appear in the evidence appendix.
**Status:** **ARCHITECTURE LOCKED — DEC-COL-001–003 and DEC-FMT-001–004 approved; reference-compiler API verification still NOT RUN.**
**Authoritative implementation owner:** Miracle project author.
**Reference compiler:** `spwn02/clang-cxx26`, immutable snapshot `cxx26-2026.10.27`, source revision `b56ae4e61cf1f46495f0fb08f2b9fc270aad162c`, matching libc++ and ABI runtime; C++26; CMake ≥4.4, Ninja. Source: `docs/COMPILER_SUPPORT.md`, `docs/REFERENCE_TOOLCHAIN.md`.
**Available analysis host:** Clang 17.0.0 (Swift build) and CMake 3.31.6; **not** a compliant substitute for the reference toolchain.

> **READER SAFETY.** Every code block marked `PROPOSED` describes an illustrative declaration needing reference-toolchain verification; approved semantic decisions are locked by DESIGN.md and the normative reconciliation notice. A block marked `EXISTING` is a source-derived description, not an instruction to change behavior. A `PROBE` has an explicit environment and observed outcome. Nothing is compiler-confirmed merely because it appears in plausible C++.

---


## Normative reconciliation notice — 2026-10-30

**Approval:** The author accepted all seven final recommendations DEC-COL-001–003 and DEC-FMT-001–004 and approved restoring canonical phase files at `docs/phases/M/`. The binding architectural contract is `DESIGN.md` v2.0. The present document is the implementation-grade companion; descriptions of *earlier pending owner decisions* in the preserved original audit appendix are historical provenance, **not live approval gates**. This notice and the approved decision table below prevail over pre-reconciliation text. Compiler API verification is **NOT RUN** on the pinned reference snapshot unless a separately cited CI fixture proves otherwise. Historical source baseline CI success cannot prove newly introduced Format/Color APIs.

| Identifier | Final author-approved outcome |
|---|---|
| DEC-COL-001 | `Color::rgb(uint8_t,uint8_t,uint8_t)` plus `checkedRgb` for arbitrary integral inputs; range-check before narrowing; `std::optional<Color>` on invalid. |
| DEC-COL-002 | `Color::hex(uint32_t) -> std::optional<Color>`; reject bits outside 24-bit RGB. |
| DEC-COL-003 | Include `toHsv` and both `#RGB` and `#RRGGBB` parsing in Phase M. |
| DEC-FMT-001 | Owned fixed-extent Fragment overload for context and other diagnostic sections; at most one context. |
| DEC-FMT-002 | One public, lightweight, statically dispatched `Event`/`TextSink` rendering protocol, non-owning event views, explicit `SinkFailed` status and invalid failed-state reuse. |
| DEC-FMT-003 | Exactly one final LF, one separating blank line between nonempty sections, no duplicate LF on finish. |
| DEC-FMT-004 | Escape unsafe C0/DEL bytes in single-line diagnostic fields using C-style escape spelling; explicitly structured multiline snippets are separate events. |

**Other approved invariants:** single `Miracle::Severity` owned by `Miracle.Format`; no nested diagnostic namespace or duplicate Builder; constructor-created `Message` with plain `data()`/`view()`, style runs and inferred type-level capacity; initial contiguous storage; independent `Miracle.Color` with Format as its only permitted Miracle dependency. Deferred broad general-purpose formatting belongs to Core Revisit.

**Technical refinements necessary to implement the decisions (not additional user approvals):** use `CodeView` solely as an immediate-use presentation view, never a second code authority; publish read-only `StyleRun` introspection so fragment ranges can be validated; `RenderState` includes an irreversible failure status; input sanitization happens centrally in Format's layout encoder, not in each Diagnostics caller. The exact C++ signature of every new declaration is a reference-compiler conformance target—not a claim that those signatures have compiled.

### Concrete required implementation notes for newly approved decisions

**COL checked factory:**

```cpp
// Contractual API candidate; C++26 module and overload tests NOT RUN.
template <std::integral R, std::integral G, std::integral B>
  requires (!std::same_as<std::remove_cv_t<R>, bool> &&
            !std::same_as<std::remove_cv_t<G>, bool> &&
            !std::same_as<std::remove_cv_t<B>, bool>)
static constexpr auto checkedRgb(R r, G g, B b) noexcept -> std::optional<Color> {
    if (!std::in_range<std::uint8_t>(r) || !std::in_range<std::uint8_t>(g) ||
        !std::in_range<std::uint8_t>(b)) return std::nullopt;
    return Color::rgb(static_cast<std::uint8_t>(r),
                      static_cast<std::uint8_t>(g),
                      static_cast<std::uint8_t>(b));
}
```

**FMT state and event semantics:** `EventKind` = `Text|Heading|Context|Field|Note|Help`; only Heading may initiate a diagnostic stream. `RenderState` begins `Plain`, transitions monotonically `Heading → Context → Fields → Notes → Help`, rejects a second Context and all backward moves; `Text` is only valid in ordinary/plain mode. A failed sink write transitions to `Failed` and later `emit`/`finish` must return `SinkFailed` without pretending to recover or consuming new content. `finish` may emit terminal reset, but it does **not** add another LF when the last successful event already ended with LF. Empty layout fields are legal values; empty *keys* are rejected. Style metadata is applied over the emitted escaped representation; escapes are not ANSI commands.

**FMT control encoding:** treat the one-line content as a byte stream. Encode LF as `\\n`, CR as `\\r`, HT as `\\t`, NUL as `\\0`, ESC as `\\x1B`; all remaining bytes in ranges 0x01–0x1F and 0x7F as `\\xNN` uppercase hexadecimal, except covered special cases. All bytes >=0x80 pass through unchanged. Raw input line breaks are permitted only via a separate explicit multiline/source layout path, not by embedding a control in a one-line summary/key/value. The normal `Message::append` path remains byte-preserving. Avoid under-reserving compile-time space when a source byte expands to four output bytes; preflight from known input extents or use capacity conservatively and precisely update run offsets after normalization.

**FMT sink protocol:** `TextSink` requires `bool write(std::string_view)` (no virtual function); `emit(Sink&, RenderState&, const Event&)` returns `WriteResult::Written|InvalidOrder|SinkFailed`; `finish` returns the same status type. `Event` and `CodeView` are borrowed immediate-use values. No `Event` may outlive its input strings, and no pointer/view may be retained by the writer after `emit` returns. The encoder must not output any bytes for a deterministically invalid event. Sink failure may leave partially written bytes; rollback is not required. Stream adapters may propagate standard-library exceptions configured through the underlying stream's mask—do not mark potentially allocating or stream-writing functions unconditionally `noexcept`.

**FMT test additions:** verify `checkedRgb` with wide signed/unsigned integers including 0, 255, −1, 256, `INT_MIN`, `UINT64_MAX`; invalid high-HEX bits; all `#RGB` nibbles; HSV sectors/gray/NaN/inf; owned style-fragment lifetimes; 1-context ordering; `emit`/`finish` invalid ordering, partial sink failure, reset failure and no writer reuse; exact plain-vs-ANSI bytes; C0/DEL escaping with golden snapshots; final LF; and existing `Miracle::Diagnostic` public API unchanged except module ownership.


# Part I — Authority, terminology and document control

## 1.1 Authority and precedence

**REQ-GOV-001 [REQUIRED].** The repo's designated local checkout prevails over an unverified remote snapshot. Its accepted contracts prevail over this draft. `docs/DEVELOPMENT_PROTOCOL.md` §6.3 governs conflicts in this order: explicitly approved newer amendment; locked feature/subsystem contract; locked phase contract; development protocol/general conventions. Unknown approval is not presumed. An architectural departure identified below must be recorded as a proposed amendment/ADR; implementation of an unapproved signature is at the author's discretion and cannot subsequently be marked contract-compliant without ratification.

**REQ-GOV-002 [REQUIRED].** The user implements production code; the architectural partner supplies specifications, verification methodology, reviews and corrective design constraints. Review may be **batched after individually committed M.1–M.8**, as expressly agreed, provided each milestone remains separately auditable and none is marked *Accepted* before its review and evidence. Implementation-complete, verification-complete and accepted are separate statuses.

**REQ-GOV-003 [REQUIRED].** Final source of truth is a committed, owner-approved repo document. This draft is a handoff artifact, not a stealth amendment to earlier contracts, historical frozen archives, tags or main branch. If its `DESIGN.md` authority cannot be recovered on current `master`, reconcile via Git history and explicit owner action before asserting contractual lock.

**REQ-GOV-004 [REQUIRED].** Every mandatory statement has an identifier. This document uses `REQ-*` for requirements, `INV-*` for invariants, `ALG-*` for algorithms, `API-*` for proposed interfaces, `TEST-*` for tests, `DEC-*` for decisions and `RISK-*` for risk findings. Normative categories are `REQUIRED`, `RECOMMENDED`, `ILLUSTRATIVE`, `DEFERRED` as defined by Development Protocol §5.3. Where requirements restate prior approved architecture, the authority column says *Approved intent*. The newly reconciled DEC-COL-001–003 / DEC-FMT-001–004 selections are author-approved; all signatures remain subject to reference-compiler evidence.

### Document-revision discipline

- **v1.0 historical:** original Phase M Blueprint Contract, repository file later removed from `master` by source-history changes; preserve as historical evidence, not current binding Git content without reconciliation.
- **v1.1 historical guidance:** source-oriented implementation sketch that proposed a separate `format::diagnostic::Builder` and duplicate `Level`. This design was expressly rejected by the author; **do not implement that proposed public API**.
- **v2.0 draft:** this specification proposes one content/presentation foundation, canonical severity ownership, Color as an independent named module, append-only inferred-size compile-time messages, styled fragments and a Diagnostics adapter. It supplies provisional exact interfaces and identifies approval/verification gates.
- **After approval:** record owner decision and reconcile `DESIGN.md` before relabeling v2.0 locked; update this document's status and record a precise source revision for the implementation review.
- **v2.1 recovery supplement:** Appendix D preserves and extends the recovered draft, records executed host-language probes P-HOST-003/004, and sets explicit contract and reference-compiler approval gates. Its API choices remain proposed.

## 1.2 Essential vocabulary

**Named module:** a C++20+ separately named ownership/import unit (`Miracle.Iter`); not a separate binary or a source directory. **Primary interface:** one `export module M;` definition per named module. **Interface partition:** `export module M:P;`, importable only within M and re-exported where required. **Implementation unit:** `module M;` and normal `.cpp`, with no exported interface. **Reachability:** definitions and dependencies reachable for template instantiation in downstream translation units; not equivalent to textual visibility. **BMI:** compiler-produced module interface artifact, sensitive to compiler and build configuration.

**Color value:** RGB8 sRGB-coded triplet independent of terminal transport. **Terminal color selection:** default, indexed ANSI, or RGB24, owned by Format, not a second standalone color model. **Style:** semantic coloring/attributes on one text fragment, with no embedded ANSI byte sequences. **Fragment:** text plus presentation metadata. **Message:** owned plain-text content plus fragment-style runs and (in diagnostic mode) monotonic section state. **Presentation sink:** statically selected output destination receiving text/ANSI sequences; **not** a virtual base interface. **Diagnostic:** a domain-semantic object carrying code, source locations/spans, causes, notes and optional traceback; its renderer uses Format's output grammar.

**Exact-size capacity:** capacity derived from compile-time-known text extent, not caller-supplied arbitrary buffer size. **Runtime-sized input:** length that is not a template-time constant; cannot magically change the return type of a `constexpr` function. **Plain representation:** text from `data()` / `view()` absent ANSI escapes, independent of rendering policy.

## 1.3 Source and evidence rules

The following evidence categories are distinct:

- **S — inspected source:** concrete paths under the pinned repository revision; verified reading of source alone, not executed behavior.
- **C — approved contract:** historical Phase M design and broader frozen contracts, subject to precedence and current-tree reconciliation.
- **D — explicit design approval:** user approved standalone Module Migration, .cppm/.cpp, shared Format architecture, one canonical severity, constructors, section ordering, named Color and F7–F10.
- **L — language reference:** relevant C++26 normative draft and compiler/CMake documentation; compiler implementation may diverge.
- **P — executed probe:** isolated test with recorded host/compiler/command/result; host probe is *not* reference-toolchain confirmation.
- **N — not run:** no test outcome claimed.

## 1.4 Baseline identity and non-assumptions

**REQ-BASE-001 [REQUIRED].** The baseline record must capture Git SHA, toolchain snapshot/manifest, build configuration, relevant source-tree inventory, tests, current packaging state and at least one reproducible before/after compile performance comparison. No new benchmark number is invented here.

At `e2da473e6dd5637f1f70625738f8378e211a7478`, source review establishes: project v0.5.28; `.cppm`/`.cpp` conversions exist; `src/public/numeric/Numeric.cppm`, `types/Types.cppm`, `static_string/StaticString.cppm`, `range/Range.cppm`, `feature/Feature.cppm` exist; Feature has `Types`, `Descriptor`, `Set`, `Catalog`, `Requirement` partitions; `Miracle` remains mixed umbrella with old `:Iter`, `:Diagnostic`, `:Error` etc.; `Miracle.Meta` remains independently named; no observed `src/public/color/Color.cppm` or `src/public/format/Format.cppm`. The prior Phase M `docs/phases/M/DESIGN.md` file was removed from the current tree, so the authoritative status of that file must be reconciled. A remote commit proves only its own source state; the author's checkout may differ.

**REQ-BASE-002 [REQUIRED].** Never repeat M.2/M.3 source migrations that the author has already committed. Audit by functionality and accepted contract, not milestone label. Existing commits are preliminary evidence of implementation, not automatic proof of a passed verification gate.

---

# Part II — Product boundary and desired end state

## 2.1 Problem

Miracle's initial architecture attached broadly independent facilities to partitions of the umbrella module `Miracle`, creating unnecessary coupling in public import identity and in runtime implementation ownership. Independently named modules correct granularity and future package consumption, but are not in themselves a proof of lower build time. Independently, compile-time hard diagnostics are generated in StaticString, Feature, Iter and Meta with inconsistent presentation and some duplicated buffering machinery. A complete high-level Diagnostics module cannot be imported by its own foundational dependencies. The approved solution is a small `Miracle.Color` value module and a unified `Miracle.Format` presentation foundation, with `Miracle.Diagnostics` responsible for domain-specific adapters and source-aware semantics.

**REQ-SCOPE-001 [REQUIRED].** Phase M MUST migrate module ownership, source conventions, source organization, packaging and common diagnostic presentation without redesigning existing algorithms or user APIs. Introduce only narrowly approved new Format and Color foundational APIs.

**REQ-SCOPE-002 [DEFERRED].** Validation Engine rule/policy semantics; Shapes, Traits, Derive; replacement String/Vec/Option; global formatting language; generic `Formatter<T>` ecosystem; localization; gradient palettes; RGBA/blending; color-space management; full `std::format` replacement; theme framework; standalone linked library targets per named module. These belong to an explicitly approved subsequent phase, normally Core Revisit for formatting/color extensions.

**REQ-SCOPE-003 [REQUIRED].** Preserve consumer `import Miracle;` and `Miracle::Miracle` CMake alias. Existing public namespaces and type/function names survive source ownership changes unless a separate accepted amendment documents an exception. Existing `Miracle.Meta` reflection/Query semantics are invariant; compiler ABI/BMI compatibility is not guaranteed across named-module ownership changes.

**REQ-SCOPE-004 [REQUIRED].** API additions MUST avoid a second parallel diagnostic hierarchy. `Miracle::Severity` has a single definition, owned by Format and used by Diagnostics. Normalized code identity may reside in Format as one presentation value, while reflected semantic code descriptors remain in Diagnostics. There SHALL NOT be both `format::diagnostic::Level` and `Miracle::Severity`, nor an independent `format::diagnostic::Builder` shadowing `Miracle::Diagnostic`.

## 2.2 Completion conditions

Phase M succeeds only when source modules are independently importable and acyclic, installed consumers compile, compile-time errors have standardized authored-message bodies where feasible, runtime Diagnostic rendering uses shared Format presentation primitives without semantic regressions, the new Color module is dependency-free relative to Miracle, and all test/benchmark evidence and deviations are documented. The phase is accepted **only** after the author's explicit final review decision, distinct from both implementation-complete and any release tag.

## 2.3 Proposed target import graph

```text
C++ standard library
   ├─ Miracle.Numeric
   ├─ Miracle.Color
   ├─ Miracle.Build / Miracle.Capabilities (generated facts)
   └─ other independent standard-only facilities
             │
             └─ Miracle.Format ──────┐
                  │                  │
             StaticString            ├─ Diagnostics ── Error ── Panic / Fs
                  │                  │
             Feature               Iter (also Range, Types as needed)

Miracle.Meta ── own reflection partitions; no import of umbrella
Miracle.Debug / Bitflags / Profiling ── true dependencies only
Miracle (umbrella) ── export-import accepted public module interfaces
```

This is an **illustration of direction**. The actual DAG must be generated from source imports, including transitive internal partitions, and checked independently. Format may import Color (a newly approved exception to the former 'no Miracle imports' invariant) but no higher-level facilities. If `Numeric` is not required for Format, it must not be imported. Color may import only standard facilities and MUST NOT import Format, Numeric or StaticString absent a later explicit revision.

**INV-ARCH-001 [REQUIRED].** Production import graph is acyclic. Each named public module has exactly one primary interface owner. No production module imports umbrella `Miracle`.

**INV-ARCH-002 [REQUIRED].** Exactly one authoritative owner exists for every exported entity. Moving a definition out of a `Miracle:` partition removes the old ownership; no alias/re-export trick may create two distinct class identities under the same exposed spelling.

**INV-ARCH-003 [REQUIRED].** `Miracle.Format` may import only `std` and `Miracle.Color`. `Miracle.Color` imports no Miracle module. `Miracle.StaticString` does not import Diagnostics, Meta or Validation. `Miracle.Diagnostics` does not import umbrella or Validation.

**INV-ARCH-004 [REQUIRED].** Test packages use the exact installed module metadata. An interface partition required transitively by an exported primary interface must remain available to the installed consumer even if its declaration is not itself exported.

## 2.4 Architectural alternatives rejected

A formatting-only *style guideline* with independent implementations would duplicate ANSI, padding, indentation and fragment logic; rejected. A second diagnostic builder and second severity type were explicitly rejected. A generic runtime Format framework would prematurely duplicate `std::format` and complicate compile-time lifetimes; deferred. A virtual base `Message` or type-erased diagnostic hierarchy adds allocations and dynamic dispatch without need; rejected. A complete rewrite of Meta Query / Iter while rearranging source files risks breaking reflection observation points and lazy semantics; rejected. Independent CMake library targets per module are not required; no performance result justifies them.

---

# Part III — Relevant C++26 theory and compiler constraints

## 3.1 Module attachment and import rules

A named module has one primary module interface unit. A declaration attached to module `Miracle:Iter` is owned by module `Miracle`; changing it to `Miracle.Iter` changes module attachment and potentially mangled identity and compilation requirements. Source suffixes alone do **not** change the declared module. Partitions `M:P` are not standalone consumer-importable modules; a consumer imports `M`. Module partition spelling is flat: `M:Sources.Views` does not form a nested subpartition. The primary interface must make interface partitions reachable/re-exported as required; module implementation units may import the primary module and need correct ownership.

```cpp
// PROPOSED: primary interface
export module Miracle.Iter;
export import :Sources;
export import :Views;
export import :Core;

// PROPOSED: interface partition
export module Miracle.Iter:Sources;
import std;
// exported declarations here

// PROPOSED: implementation unit
module Miracle.Iter;
import std;
// non-template definitions here
```

**REQ-MOD-001 [REQUIRED].** Verify every changed `export module`, `module`, `import`, `export import` and CMake path against the actual reference compiler. A `module Miracle;` implementation left over after migrating its interface to `Miracle.Diagnostics` is a build defect, not an intentional backward-compatibility unit.

**REQ-MOD-002 [REQUIRED].** Do not move a generic exported template's required definition into an inaccessible `.cpp` unless explicit instantiation coverage or language rules demonstrably preserve every supported consumer instantiation. A test using an arbitrary third-party type in a separately built installed consumer is mandatory for each high-risk decomposed template.

**REQ-MOD-003 [REQUIRED].** Do not use `PRIVATE FILE_SET` to conceal interface partitions needed by exported module interfaces in installed packages. Verify CMake collation/transitive BMI maps using `add_subdirectory`, `FetchContent`, and `find_package` tests, not merely in-tree compilation.

## 3.2 Constant evaluation and type-level capacities

C++ template arguments must themselves satisfy constant-expression requirements. A function parameter of type `std::string_view` does not become an admissible template argument merely because the function is `consteval` or called with a literal. Therefore, a return type depending on `text.size()` cannot generally be instantiated from arbitrary parameter values:

```cpp
// ILLUSTRATIVE INVALID PATTERN — DO NOT USE
consteval auto f(std::string_view text) {
    return std::array<char, text.size()>{};
}
```

A literal passed by array reference provides its **extent** as a template argument:

```cpp
// PROPOSED representative mechanism; reference-compiler verification pending
template <std::size_t N>
consteval auto fromLiteral(const char (&text)[N]) {
    std::array<char, N> bytes{};
    for (std::size_t i = 0; i < N; ++i) bytes[i] = text[i];
    return bytes;
}
```

For reflection-derived type names, a provider can form `constexpr auto name = std::meta::display_string_of(^^T)` locally and use its constant length in a *separate template-instantiation context* to materialize an owned `std::array<char, name.size()>`. Format itself need not import `<meta>` or know reflection semantics. A `std::string_view` passed through a generic function parameter does not preserve a type-level length; adapters must explicitly establish an extent.

**INV-FMT-001 [REQUIRED].** Every constexpr-stored character array has capacity derived from statically known extents, with `0 <= size <= capacity` and a valid `data()` range. No arbitrary user-provided capacity is required for supported compile-time inputs.

**INV-FMT-002 [REQUIRED].** Style metadata is stored separately from text bytes. `data()` and `view()` return plain text, irrespective of presentation/rendering options, and remain valid for the lifetime of the referenced Message value.

## 3.3 Generated `static_assert` messages

C++26 permits a constant-expression message object in `static_assert`. The object must expose `size()` convertible to `std::size_t`, and `data()` convertible to `const char*` with readable constant-expression characters for that extent. The compiler owns its wrapper, colors, source excerpts and instantiation-trace printing. Contractual testing therefore normalizes compiler wrappers and compares only Miracle-authored text. `static_assert` messages are **not a substitute** for concepts or recoverable `std::meta::exception` paths. If an expression fails to form before reaching the authored assertion, a standardized message may not be available.

```cpp
// PROPOSED example; NOT reference-compiler verified
constexpr auto msg = format::Message{"invalid configuration"};
static_assert(/* constexpr predicate */, msg);
```

**REQ-FMT-001 [REQUIRED].** An accepted `Message` representation must support C++26 generated static assertion messages without temporary allocation escaping constant evaluation, dangling views, or ANSI bytes.

## 3.4 Styles, color space and terminal transport

RGB8 channels in Color represent sRGB-coded values. HSV conversion here operates on normalized **encoded** RGB values, not linear-light radiometric values; HSV is not a substitute for color-management transforms. `#RRGGBB` is syntax for numeric RGB, not a separate color model. Terminal ANSI palette indexes are *logical palette selections*, not exact RGB colors; an indexed entry must retain its index rather than be converted to a fabricated RGB triplet. True-color ANSI sequences are produced only by Format during explicit rendering, while Color knows nothing about ANSI.

**REQ-COL-001 [REQUIRED].** Color conversion must validate NaN, infinities and out-of-range components without exceptions. Hue wrap and boundary behavior must be specified; parsing must not silently truncate invalid hexadecimal digits. Float rounding must be deterministic under the reference toolchain and documented in the test matrix.

## 3.5 Compile-time complexity and instantiation structure

If a `Message<C,R>` stores contiguous text of capacity C and R style runs, each append returns a **different type** with larger C/R. The append copies existing characters and runs into a new constexpr object: for K successive fragments of total length N, naive repeated copies cost O(KN) work and can be quadratic when K≈N. This is acceptable only for small diagnostic paths; a bulk-compose operation is recommended for long messages and must be benchmarked. A persistent typelist of fragments delays copying but can cause deep instantiation chains and hard-to-inspect error traces; not selected for Phase M by default.

**REQ-PERF-001 [REQUIRED].** Do not instantiate expensive reflected diagnostic-message builders on successful `if constexpr` branches if a failing branch can be selected. Ensure ill-formed code does not trigger an irrelevant compiler-generated error before the intended `static_assert`.

**REQ-PERF-002 [REQUIRED].** Record compiler wall time, peak memory, BMI footprint, umbrella-import and granular-import consumer costs before/after on identical toolchain and build flags. A material unexplained regression is a review finding, not silently accepted.

## 3.6 Runtime adaptation without a second diagnostic engine

Compile-time message storage and runtime Diagnostic storage have different requirements. It is impossible to infer a distinct C++ class template argument from a runtime-only string length. This does **not** justify two competing public presentation systems. The shared architecture is a single **Format event grammar and style policy** with two ways to supply text: an owned compile-time Message (statically sized) or a runtime Diagnostics traversal that emits the same fragment/layout events directly to the Format sink. The runtime `Miracle::Diagnostic` remains move-only with owned dynamic strings, source spans, causes and stacktrace; it does **not** need to instantiate `Message<guessed-size>`.

**INV-DIAG-001 [REQUIRED].** The semantic object may own runtime strings, but all terminal style encoding and common diagnostic layout rules have one authoritative implementation in Format. Diagnostics adapts its data without adding an independent severity/color/padding renderer.


---

# Part IV — Normative requirements and interface-approval register

## 4.1 Classification of existing and new surface

The migration **does not** entitle the author to delete existing API declarations. The current public interfaces of `StaticString`, `Range`, `Iter`, Feature, Diagnostics, Error, Panic, Fs, Meta, Debug, Bitflags, Memory, Profiling, Build and Capabilities remain governed by their current declarations and locked contracts. For these existing components, the complete API inventory is the inspected source declaration inventory plus relevant frozen contracts; the migration changes ownership and import paths. This document provides **complete proposed new APIs for Color and Format**, and specifies module/partition relocation for the rest. It does not pretend to recatalog every method of 3,700-line Iter and 2,191-line Meta Query in lieu of the existing normative code/contracts.

**REQ-API-001 [REQUIRED].** All added public declarations MUST appear in the API inventory below. No undocumented public overload, implicit conversion, new global configuration or nonessential customization hook may be introduced during Phase M. Any example that calls an unlisted public API is illustrative until reconciled.

**REQ-API-002 [REQUIRED].** Every existing public symbol's namespace, constraints, return category, exception specification, evaluation mode and generic participation MUST be preserved. Module identity may change under the approved migration; compatibility of direct legacy partition imports is not promised. A difference found in tests or API snapshots is a review finding, not an excuse to redefine the contract silently.

**REQ-API-003 [REQUIRED].** New APIs compile under `-std=c++26` with pinned `clang-cxx26` and matching libc++, in standalone and installed consumer contexts. Until then their verification status is **NOT RUN**.

## 4.2 Approved architecture versus provisional signatures

| Design topic | Authority | Implementation implication |
|---|---|---|
| Standalone Phase M, `.cppm`/`.cpp`, named-module boundaries | Approved intent | Migrate without semantic rewrite |
| `Miracle.Format` is the shared presentation foundation | Approved intent | Do not introduce parallel diagnostic formatter |
| `Miracle.Color` is a standalone tiny foundation | Approved intent | No Color dependency on Format or StaticString |
| One `Miracle::Severity` type | Approved F7 | Move its owner to Format; preserve enumerators |
| Ordinary and diagnostic message construction | Approved F8 | One representation, diagnostic mode has constrained grammar |
| `data()/view()` plain; ANSI only via explicit rendering | Approved F9 | No escape bytes in message storage |
| Inferred contiguous compile-time storage | Approved F10 | No caller-provided arbitrary buffer size |
| Constructor-based initialization | Approved F2 | No half-initialized diagnostic builder |
| Enforced diagnostic ordering | Approved F3 | Type-level or other statically enforceable stage |
| Diagnostic code for authored errors | Approved F4 | Prefer stable codes; allow documented bootstraps |
| Style fragments with color subranges | Approved intent | Text + metadata, not embedded ANSI |
| Exact `Color::hex`, `Color::fromHsv` signatures | **Proposed** | Owner must approve API form and invalid-input semantics |
| Exact `Message` template/factory/member signatures | **Proposed** | No ratification yet; compile probes required |
| Exact `TerminalColor`/`Style` representation | **Proposed** | Owner must approve exposed value semantics |
| Runtime Diagnostics-to-Format rendering adapter contract | Approved intent; detailed signatures **proposed** | Source ranges/causes remain in Diagnostics |

## 4.3 Globally applicable error categories

**REQ-ERR-001 [REQUIRED].** Invalid template constraints lead to normal substitution rejection or deliberate hard diagnostics according to the original API contract; the migration MUST NOT turn SFINAE-friendly probes into unconditional hard assertion failures.

**REQ-ERR-002 [REQUIRED].** Invalid parseable user input (hex strings, HSV values) yields a documented non-throwing result; malformed data must not overflow, wrap silently, or produce arbitrary colors.

**REQ-ERR-003 [REQUIRED].** Internal compile-time impossible state violates a locally checked invariant or produces an explicit compiler failure; no unchecked write to a compile-time message array is permitted. Runtime I/O errors follow the selected sink's observable status; Format does not invent or throw Miracle exceptions.

**REQ-ERR-004 [REQUIRED].** `std::meta::exception` remains a recoverable constant-evaluation mechanism where existing Meta contracts require it; Format MUST NOT convert it to `static_assert` merely for layout uniformity.

## 4.4 Style and encoding decisions

**REQ-STYLE-001 [REQUIRED].** Styled fragments contain plain bytes plus attributes. Plain rendering reproduces exactly the concatenated plain bytes (including normalized newlines), ignoring styles. ANSI rendering may add escape sequences but must leave the ordered plain-text payload unchanged.

**REQ-STYLE-002 [REQUIRED].** ANSI emission has explicit user selection, and transitions must not allow styling to leak to subsequent output. If one span is bold and the next is plain, the latter MUST be rendered under the plain/default style. Terminal palette indexes and true-color RGB MUST retain distinct semantics.

**REQ-STYLE-003 [REQUIRED].** The library's authored diagnostic heading is one severity, optional code, summary, optional context, ordered fields, notes and help. Existing runtime Diagnostics remains free to add source snippets, spans, causes and traces through the same presentation primitives. Format MUST NOT own source-location resolution or reflection-based code-domain validation.

## 4.5 Concurrency and ownership

**REQ-LIFE-001 [REQUIRED].** A compile-time `Message` owns its bytes and style records; it MUST NOT retain a view into a temporary literal adapter, reflection scratch buffer, or builder parameter. Plain `view()` and `data()` borrow the value and are invalid after its destruction/move as normal object-lifetime rules dictate.

**REQ-LIFE-002 [REQUIRED].** Each call to a message-extension operation returns a distinct value. The original message value is unchanged. No hidden global mutable state, terminal state singleton, cache, dynamic allocation, or thread synchronization is permitted for compile-time message creation. Independent immutable messages are safe to read concurrently.

**REQ-LIFE-003 [REQUIRED].** Runtime rendering receives a sink by reference and may mutate it. Concurrent writes to the same sink are the caller's synchronization responsibility; the formatter itself must not race via shared mutable global state. Nested runtime diagnostic causes traverse without recursion-depth-dependent stack overflow in the existing renderer.

---

# Part V — Full proposed new public API

> **Approval and verification gate for Part V.** The author ratified the architecture and seven DEC-COL/DEC-FMT interface decisions on 2026-10-30. The detailed declaration spelling in this section is the targeted implementation interface; it is **NOT RUN** on the pinned C++26 reference toolchain. Compiler-sensitive declarations, CTAD, modules and constraints must pass the specified verification matrix before their implementation can be called verified. Only explicitly approved contract amendments may change required semantics.

## 5.1 `Miracle.Color`: types and exact interface proposal

```cpp
// PROPOSED — src/public/color/Color.cppm
export module Miracle.Color;
import std;

export namespace Miracle {

struct Hsv final {
    double hueDegrees{};  // [0,360) for chromatic color
    double saturation{};  // [0,1]
    double value{};       // [0,1]
    constexpr auto operator==(const Hsv&) const noexcept -> bool = default;
};

struct Color final {
    std::uint8_t red{};
    std::uint8_t green{};
    std::uint8_t blue{};

    [[nodiscard]] static constexpr auto rgb(
        std::uint8_t red,
        std::uint8_t green,
        std::uint8_t blue) noexcept -> Color;

    template <std::integral R, std::integral G, std::integral B>
      requires (!std::same_as<std::remove_cv_t<R>, bool> &&
                !std::same_as<std::remove_cv_t<G>, bool> &&
                !std::same_as<std::remove_cv_t<B>, bool>)
    [[nodiscard]] static constexpr auto checkedRgb(R red, G green, B blue) noexcept
        -> std::optional<Color>;

    // A valid 0xRRGGBB value yields a Color; high bits yield nullopt.
    [[nodiscard]] static constexpr auto hex(std::uint32_t rgb24) noexcept
        -> std::optional<Color>;

    // Accept exactly #RRGGBB or #RGB; ASCII hex digits case-insensitive.
    [[nodiscard]] static constexpr auto parseHex(std::string_view text) noexcept
        -> std::optional<Color>;

    // Reject NaN, infinity, negative/out-of-range S/V and out-of-range hue.
    [[nodiscard]] static constexpr auto fromHsv(Hsv hsv) noexcept
        -> std::optional<Color>;

    // Guaranteed valid; achromatic colors have canonical hue 0.
    [[nodiscard]] constexpr auto toHsv() const noexcept -> Hsv;
    [[nodiscard]] constexpr auto toHex() const noexcept -> std::uint32_t;
    constexpr auto operator==(const Color&) const noexcept -> bool = default;
};

} // namespace Miracle
```

**API-COL-001: `Miracle::Color`.** One canonical RGB8 sRGB-coded value type; exactly 3 unsigned 8-bit channels in the proposed public aggregate representation; no alpha. This type is trivially copyable, standard-layout and valid as a structural NTTP (subject to actual compiler check). Default value is black. No implicit arithmetic or string conversion. `operator==` compares all three encoded bytes. Construction via `Color{r,g,b}` uses ordinary aggregate initialization with narrowing diagnostics for non-representable constant arguments; `Color::rgb` takes already typed `uint8_t` channels and does not validate implicit conversions occurring before the call. **Risk:** accepting `Color::rgb(300,0,0)` may silently convert at call boundary; the proposed checked-vs-exact-type policy is an approval-dependent choice (`DEC-COL-001`).

**API-COL-002: `Color::hex(u32) -> optional<Color>`.** Valid input domain is `0x000000..0xFFFFFF`; outside yields `nullopt`, never strips high bits. Extraction: red `(x>>16)&0xFF`, green `(x>>8)&0xFF`, blue `x&0xFF`. Complexity O(1), allocation 0, constexpr. **Alternative needing approval:** `Color::hex()` returning a Color directly for known-good values, paired with a different checked parse API. A single `optional` checked signature minimizes unsafe runtime assumptions but changes convenience ergonomics.

**API-COL-003: `Color::parseHex(string_view) -> optional<Color>`.** Accept exactly 4- or 7-byte string `#RGB` or `#RRGGBB`; each code unit must be ASCII `0-9`, `a-f` or `A-F`. For short syntax, expand nibble `v` to `17*v`; never interpret `0x` prefix, whitespace, alpha or bare unprefixed text. Invalid size/character returns `nullopt`. No allocation. O(text length), bounded by 7. Parsing empty string, multi-byte non-ASCII, embedded NUL or overlong representation yields failure. Explicit `std::string_view` means only the view's lifetime matters during the call; result owns channels.

**API-COL-004: `Color::fromHsv(Hsv) -> optional<Color>`.** HSV fields are double in fixed ranges. Reject nonfinite components, hue <0 or >=360, saturation <0 or >1, value <0 or >1. For S==0, output grayscale; hue is ignored only **after input validity checks**. Quantization uses nearest integer with half-up rule `floor(x*255+0.5)` on x in `[0,1]`, clamped to `[0,255]` as a defensive final step. Use an integer cast after adding `0.5` (for bounded nonnegative x) if constexpr `<cmath>` support is inadequate; no library-dependent rounding mode. O(1), allocation 0, constexpr where compiler implements required arithmetic.

**API-COL-005: `Color::toHsv() -> Hsv`.** Convert 8-bit channels to `[0,1]`, determine M=max and m=min, Δ=M−m. V=M, S=(M==0?0:Δ/M). If Δ==0, return canonical H=0; otherwise select dominant channel with deterministic tie priority R then G then B and convert sector offset to degrees, adding 360 if negative. Round-trip HSV→RGB is lossy at most one RGB code value per channel under the proposed nearest quantizer (candidate target; verify numerically before locking that bound). O(1), allocation 0, constexpr.

**API-COL-006: `Color::toHex()`.** Pack `(u32(red)<<16)|(u32(green)<<8)|blue`. Always six-digit RGB payload in numeric form (leading zeros implicit in hex numeric representation). No `#` or text allocation.

**API-COL-007: `Hsv`.** Conversion carrier only, not competing canonical Color storage. Not an independently importable module, no arithmetic operations and no assumption that floating-point round trips preserve exactly original HSV inputs. Equality is exact field comparison for diagnostic/testing convenience, not a perceptual metric.

**DEC-COL-001 [APPROVED 2026-10-30]:** Keep `Color::rgb(std::uint8_t,...)` as the direct typed factory **and** provide a separate `checkedRgb` template for arbitrary integral inputs, returning `std::optional<Color>` after validating all channels via `std::in_range<std::uint8_t>`. Bool does not participate. Callers needing validation must use `checkedRgb`; `rgb` does not validate implicit narrowing at the call boundary.

**DEC-COL-002 [APPROVED 2026-10-30]:** `Color::hex(std::uint32_t) -> std::optional<Color>` is checked; high bits reject with `nullopt`.

**DEC-COL-003 [APPROVED 2026-10-30]:** Phase M includes `toHsv()` and `parseHex("#RGB")` as well as 6-digit HEX parsing.

### Algorithm ALG-COL-001 — Hex parse

```text
parseHex(s):
  if length is not 4 or 7: return empty
  if s[0] != '#': return empty
  if length == 4:
      for each of 3 digits d:
          n = hexDigit(d); if invalid return empty
          channel = 17*n
  else:
      for each of 3 channel pairs (a,b):
          high = hexDigit(a); low = hexDigit(b)
          if invalid return empty
          channel = 16*high + low
  return RGB(channel[0], channel[1], channel[2])
```

**Correctness:** hexDigit maps precisely sixteen allowed ASCII characters to 0..15; short expansion duplicates the nibble, long pair composes exactly one byte, and every input byte is consumed. No shifting overflow (maximum 255 per channel), no locale use and no dangling result.

### Algorithm ALG-COL-002 — HSV to RGB

```text
validate finite(H), finite(S), finite(V), 0<=H<360, 0<=S<=1, 0<=V<=1
C = V*S
Hprime = H/60
sector = integer_part(Hprime)  // 0..5, nonnegative
f = Hprime - sector
X = C*(sector even ? f : 1-f)
m = V-C
switch sector:
  0: (rp,gp,bp)=(C,X,0)
  1: (rp,gp,bp)=(X,C,0)
  2: (rp,gp,bp)=(0,C,X)
  3: (rp,gp,bp)=(0,X,C)
  4: (rp,gp,bp)=(X,0,C)
  5: (rp,gp,bp)=(C,0,X)
for each channel y in (rp+m,gp+m,bp+m):
  q = int(y*255 + 0.5)  // y bounded [0,1]
  clamp q to 0..255, convert to uint8
return RGB
```

**Proof sketch:** in each hue sector, exactly one component equals C, one equals zero and the third linearly interpolates 0..C, preserving nonnegativity and V maximum after m. Sector transitions share endpoint outputs. Saturation-zero yields C==0 and all channels V. Quantization is monotonic, preserves endpoints 0/255, and does not allocate. `std::isfinite` constexpr support is reference-compiler-dependent; an explicit finite test using comparisons with `numeric_limits<double>::infinity()` may be used after verifying it on pinned C++26.

### Algorithm ALG-COL-003 — RGB to HSV

```text
r=red/255.0, g=green/255.0, b=blue/255.0
maxv=max(r,g,b), minv=min(r,g,b), delta=maxv-minv
v=maxv; s=(maxv==0 ? 0 : delta/maxv)
if delta==0: return (0, s, v)
if r is selected maximum: h=60*((g-b)/delta)
else if g is selected maximum: h=60*(2+(b-r)/delta)
else: h=60*(4+(r-g)/delta)
if h<0: h += 360
if h>=360 due to roundoff: h -= 360
return (h,s,v)
```

**Numerical risks:** near-achromatic Δ is a multiple of `1/255`, so no division by a zero delta after the explicit branch. Tie selection is deterministic. The reverse output is approximate and test tolerances must reflect float arithmetic and 8-bit quantization; do not promise byte-identical HSV input reproduction.

## 5.2 `Miracle.Format`: unified primitive vocabulary

The agreed target surface avoids a nested namespace and avoids a second `Level` type. It has **one owner for severity, colors and common layout**. Diagnostics remains a separate module that imports the foundation.

```cpp
// PROPOSED — src/public/format/Format.cppm and interface partitions
export module Miracle.Format;
import std;
import Miracle.Color;

export namespace Miracle {
    // Same original public identity and values; ownership moves to Format.
    enum class Severity : std::uint8_t { Debug, Note, Warning, Error };

    // Existing names retain public identity; see §5.6 for migration.
    enum class ColorMode : std::uint8_t { Automatic, Always, Never };
    enum class Presentation : std::uint8_t { Plain, Terminal };
}

export namespace Miracle::format {
    enum class ColorKind : std::uint8_t { Default, Indexed, Rgb };

    struct TerminalColor final {
        ColorKind kind{ColorKind::Default};
        std::uint8_t index{};
        Miracle::Color rgb{};
        [[nodiscard]] static constexpr auto defaultColor() noexcept -> TerminalColor;
        [[nodiscard]] static constexpr auto indexed(std::uint8_t index) noexcept -> TerminalColor;
        [[nodiscard]] static constexpr auto trueColor(Miracle::Color color) noexcept -> TerminalColor;
        constexpr auto operator==(const TerminalColor&) const noexcept -> bool = default;
    };

    struct Style final {
        TerminalColor foreground{};
        TerminalColor background{};
        bool bold{};
        bool dim{};
        bool italic{};
        bool underline{};
        constexpr auto operator==(const Style&) const noexcept -> bool = default;
    };

    template <std::size_t Characters>
    struct Fragment; // Owns Characters bytes + Style; no ANSI embedded.

    template <std::size_t N>
    [[nodiscard]] constexpr auto fragment(const char (&text)[N], Style style = {})
        -> Fragment<N - 1>;

    template <std::size_t N>
    [[nodiscard]] constexpr auto styled(const char (&text)[N], Style style)
        -> Fragment<N - 1>;

    template <std::size_t N>
    [[nodiscard]] constexpr auto fragment(const std::array<char, N>& text, Style style = {})
        -> Fragment<N>;

    template <std::size_t PrefixCharacters, std::size_t MinDigits>
    struct Code;

    // Public factory's width is a template argument, avoiding runtime-sized return type.
    template <std::size_t Width = 3, std::size_t N>
    [[nodiscard]] constexpr auto code(const char (&prefix)[N], std::uint64_t value)
        -> Code<N - 1, Width>;

    enum class Section : std::uint8_t { Plain, Heading, Context, Fields, Notes, Help };

    template <std::size_t Capacity, std::size_t Runs,
              bool Diagnostic = false, Section Current = Section::Plain>
    class Message;

    // See §5.3–5.5 for complete member signatures, CTAD guides and render APIs.
}
```

**Declaration-order guarantee:** `Code` is forward-declared before the returning `code` factory. Actual public definition/export/reachability and CTAD behavior remain reference-compiler verification targets `TEST-API-DECL-001`. The sketch is not yet a compiled API.

### API-FMT-001 — `Miracle::Severity`

Preserve existing four enumerators, order, underlying type's 8-bit width and semantic meaning. No new Info level, no duplicate `format::Level`. Move definition to `Miracle.Format` and remove it from `Diagnostic.cppm`; `import Miracle.Diagnostics;` must expose the name via its exported interface dependencies or the equivalent approved re-export arrangement. The code owner is Format; Diagnostics' `Severity` references refer to the same type. Transition is ABI/BMI-breaking but source spelling stays `Miracle::Severity`.

### API-FMT-002 — `Miracle::ColorMode` / `Miracle::Presentation`

Preserve `Automatic`, `Always`, `Never`, and `Plain`, `Terminal` respectively, with current enumerator ordinals. Format becomes the single owner of this shared presentation vocabulary. Existing `SourceMode`, `DetailMode`, `RenderOptions` stay owned by Diagnostics because they govern domain-specific source/trace content; those structs may reference the Format-owned enums after import. **Automatic** retains the observable baseline policy: `Presentation::Plain` suppresses ANSI; `ColorMode::Never` suppresses ANSI; `Always` permits ANSI in Terminal mode; `Automatic` follows existing renderer policy unless explicitly amended with terminal-detection semantics. Do not invent TTY detection during migration.

### API-FMT-003 — `TerminalColor`

A small trivially copyable tagged value: default terminal color, ANSI palette index 0..255, or exact RGB24 `Miracle::Color`. The tag determines which field is semantically active. `defaultColor()` returns default; `indexed(i)` retains exact index; `trueColor(c)` retains exact Color. Equality compares active meaning; either mandate canonicalize inactive fields to zero (proposed) or implement tag-aware comparison; the proposed implementation canonicalizes them to zero, allowing defaulted equality. No direct ANSI escape strings, no Hsv inside Format and no attempt to claim ANSI indexed palette equals a specific RGB.

### API-FMT-004 — `Style`

Trivial value containing foreground/background TerminalColor plus four text-attribute flags. Default style means terminal default foreground/background, with no attributes. Styling **never modifies** fragment text. Formatting can compute style transitions; no user-defined ANSI bytes, stack of styles, global mutable current style or theme registry during Phase M. `bold`/`dim` may share ANSI reset semantics; the renderer can safely emit a full reset then complete new style at each transition rather than use clever partial diffs. Complexity O(fragment runs × constant escape overhead). Underlining and italic may be unsupported by a terminal; Style represents intent and ANSI renders conventional SGR codes.

### API-FMT-005 — `Fragment<N>`

Own exactly N text bytes and one Style. Its proposed public observers are `size() -> size_t`, `data() -> const char*`, `view() -> string_view`, and `style() -> const Style&`. Storage is a fixed `std::array<char,N>`; no allocation; `Fragment<0>` is valid and empty. Two fragments with different extents have distinct C++ types but identical semantic behavior. Constructing from a literal copies N-1 bytes and excludes terminating `\0`. Constructing from `std::array` copies exactly N bytes, including embedded null characters, which remain in plain text; no implicit C-string truncation. A fragment must own the text; not a dangling `string_view` into an ephemeral object.

### API-FMT-006 — `fragment` / `styled` factories

`fragment(literal, style)` and `fragment(std::array, style)` deduce their capacities; `styled(literal, style)` is a clarity alias for the first. Distinct Style values are stored even if text is equal. Overloads for dynamic `std::string_view` are **not** exposed as compile-time-owned Fragment factories in Phase M, because the result type cannot be made exact-size from a runtime value. Dynamic text is passed to the runtime sink using `std::string_view` with guaranteed consumption before return.

### API-FMT-007 — `Code<P,Width>`

Presentation identity for a compile-time diagnostic heading: owning prefix exactly P bytes and runtime/nontype value `u64`, static minimum digit width Width. Its template parameters are **inferred from prefix literal** and (for Width) a default/template argument, not an arbitrary message-buffer allocation. `code<3>("ITR", 4)` represents ITR004; `code<5>("CTR", 7)` represents CTR00007. Width==0 means no extra zero padding, but no truncation of the actual number. If P==0, omit `[code]` altogether. Minimum width uses `max(Width,decimalDigits(number))`; `Width` is known before determining output type. To prevent enormous invalid types, a Phase M implementation may impose a documented implementation-supported maximum Width only if the owner ratifies a limit; no arbitrary limit is currently approved. `Code` is a **formatting value**, not a second code-domain registry or enum. Existing semantic code identity is sourced from Diagnostics, including annotations and enumerator values.

### API-FMT-008 — `Message<C,R,D,S>`

One class template representing owned plain text plus an array of style runs and compile-time diagnostic grammar state. `C` is static capacity derived from constructed text extents and conservative known-value maxima, not supplied manually by ordinary users. `R` is a compile-time upper bound on run count. `D` selects ordinary versus diagnostic grammar, `S` the last completed section. A `Message` is immutable from the consumer's perspective: extending it returns a new value/type, no mutation of prior values. `Message` is not an exception, not a dynamic string and not a generic formatting language. No public `Builder` is added.

Proposed observers:

```cpp
// PROPOSED member signatures; enclosing class template and N-dependent
[[nodiscard]] constexpr auto size() const noexcept -> std::size_t;
[[nodiscard]] constexpr auto data() const noexcept -> const char*;
[[nodiscard]] constexpr auto view() const noexcept -> std::string_view;
[[nodiscard]] constexpr auto styles() const noexcept -> std::span<const StyleRun>;

// Ordinary text mode; append returns a NEW deduced Message type.
template <std::size_t N>
[[nodiscard]] constexpr auto append(const char (&text)[N]) const
    requires (!Diagnostic);
template <std::size_t N>
[[nodiscard]] constexpr auto append(Fragment<N> text) const
    requires (!Diagnostic);
template <std::integral Integer>
[[nodiscard]] constexpr auto append(Integer number) const
    requires (!Diagnostic);

// Diagnostic grammar; all return a NEW deduced Message type.
template <std::size_t N>
[[nodiscard]] constexpr auto context(const char (&text)[N]) const
    requires (Diagnostic && Current <= Section::Context);
template <std::size_t K, std::size_t V>
[[nodiscard]] constexpr auto field(
    const char (&key)[K], Fragment<V> value) const
    requires (Diagnostic && Current <= Section::Fields);
template <std::size_t K, std::size_t V>
[[nodiscard]] constexpr auto field(
    const char (&key)[K], const char (&value)[V]) const
    requires (Diagnostic && Current <= Section::Fields);
template <std::size_t N>
[[nodiscard]] constexpr auto note(Fragment<N> value) const
    requires (Diagnostic && Current <= Section::Notes);
template <std::size_t N>
[[nodiscard]] constexpr auto note(const char (&text)[N]) const
    requires (Diagnostic && Current <= Section::Notes);
template <std::size_t N>
[[nodiscard]] constexpr auto help(Fragment<N> value) const
    requires (Diagnostic && Current <= Section::Help);
template <std::size_t N>
[[nodiscard]] constexpr auto help(const char (&text)[N]) const
    requires (Diagnostic && Current <= Section::Help);
```

**StyleRun** is a proposed exported trivial record of `{size_t begin, size_t end, Style style}` over the plain-text buffer, representing half-open byte ranges; `begin<end<=size`, ordered and nonoverlapping, optionally adjacent. Unstyled bytes require no run. Public `styles()` permits renderers and tests to inspect style metadata but not mutate it. A normalized run sequence must be produced deterministically.

**API-FMT-009 — constructors/deduction.** Direct ordinary construction from a literal is the preferred entry point:

```cpp
// PROPOSED shape (CTAD/constructor overload verification NOT RUN)
auto simple = Miracle::format::Message{"processed "};
// deduces Message<N-1, ... , false, Plain>
```

A diagnostic constructor is required to receive a severity, one typed Code, and summary literal or Fragment. There is no headerless diagnostic builder. Precisely deducing the `Message` template's C and R from the Code prefix extent, Width and summary fragment length requires an explicit C++ class template deduction guide or factory return type. The proposed *consumer spelling* is:

```cpp
// PROPOSED: requires compiler probe and API ratification
auto diagnostic = Miracle::format::Message{
    Miracle::Severity::Error,
    Miracle::format::code<3>("ITR", 4),
    "callable cannot accept sequence element"
};
```

The corresponding CTAD must compute a worst-case header capacity of `severityMaxBytes + prefixExtent + max(Width,20) + punctuation + summaryExtent` (or an exact constexpr size when code value is encoded as NTTP). The number is an **internal conservative type bound**, not caller-supplied capacity. `max(Width,20)` reserves enough for any `uint64_t`, so the active length may be shorter. A design using a code-value NTTP can compute exact decimal digits but increases template identity churn; **RECOMMENDED** use the conservative constant bound unless measured overhead justifies the more complex specialization. Literal `char[N]` argument extent drives the summary bound. Runtime spans/summaries cannot construct an inferred-capacity `Message` object through this constructor; they use the shared sink grammar (§5.6).

### API-FMT-010 — decimal append

Only non-bool standard integral types participate; bool uses explicit `"true"` / `"false"` fragment construction if needed, and char uses a one-byte fragment, avoiding surprising numeric/character overload selection. To render signed minima safely, compute magnitude in an unsigned type through modulo arithmetic instead of `-INT_MIN`. Worst-case capacity is `numeric_limits<Unsigned>::digits10 + 3`, sufficient for sign and rounding safety. Algorithm repeated `%10`, reverse collected digits. O(number of decimal digits); allocation 0. No floating-point formatter in Phase M.

### API-FMT-011 — section methods

Diagnostic state follows `Heading -> Context -> Fields -> Notes -> Help` with repeated fields, notes, help allowed. A transition from an earlier section to a later one is allowed; backward movement is rejected at constraint-checking time where expressible. `context` may be called at most once in a given diagnostic (additional constraints/one-bit flag required; **proposed** single context per diagnostic). `field` requires nonempty key, preserves call order; values may be styled fragments. `note`/`help` preserve insertion order and use consistent labels. The first item in a new section emits a separating blank line; repeated items within a section emit one new line. Invalid mixed ordering must not silently rearrange entries or require allocations.

**Signature decision `DEC-FMT-001` (APPROVED 2026-10-30):** A literal-only `context` cannot carry a reflected `std::string_view` directly. The exact interface therefore ALSO has `template<size_t N> constexpr auto context(Fragment<N> text) const requires(Diagnostic && Current == Section::Heading);`. `context` is one-shot; `note`/`help` already accept Fragment. `field` accepts both Fragment and literal; a reflected value must first be materialized as an owned sized Fragment. These overloads are part of the approved target surface and require compiler verification before implementation acceptance.

### API-FMT-012 — rendering

```cpp
// PROPOSED, independent of Diagnostic module
namespace Miracle::format {
    struct RenderPolicy final {
        Miracle::Presentation presentation{Miracle::Presentation::Plain};
        Miracle::ColorMode color{Miracle::ColorMode::Never};
    };

    // Renders a compile-time-owned message into an output sink.
    template <class MessageType, class OutputIterator>
      requires /* output iterator writable with char */
    [[nodiscard]] auto renderTo(const MessageType& message,
                                OutputIterator out,
                                RenderPolicy policy = {}) -> OutputIterator;

    template <class MessageType>
    [[nodiscard]] auto render(const MessageType& message,
                              RenderPolicy policy = {}) -> std::string;
}
```

**API-FMT-013 — runtime semantic adapter boundary.** To avoid a second Format message class for dynamic Diagnostics, Format must expose a small **non-owning, immediate-consumption rendering primitive** usable with arbitrary text `std::string_view`, Style and a caller-provided output sink. The proposed interface is a function-family `format::writeFragment(out, text, style, policy)` and `format::writeHeader(out, Severity, codePrefix, codeNumber, codeWidth, summary, policy)`; these emit common layout/style semantics and never retain views. They are **formatting operations, not a second object model**. The precise overload set must be ratified and reference-compiler verified; `std::ostream` and generic output iterators must share one core encoding algorithm, not two separately maintained SGR implementations.

**Important possibility for simplification:** instead of exposing `renderTo`/`writeFragment`/`writeHeader` individually as public APIs, they can be internal importable module-local functions used by Diagnostics, with a single documented `format::render` facade. However, a separate named `Miracle.Diagnostics` cannot import a private partition of `Miracle.Format` unless the required symbols are reachable through its exported interface. If they are not part of the public export, an externally accessible header/module import approach must be verified. This is approval-dependent `DEC-FMT-002`. A *public minimal event-sink API* is the safe default for independent module consumption.

### API-FMT-014 — plain and terminal semantics

`Message::data()` and `view()` always return pure concatenated content, including newline characters; no ANSI; no semantic markup delimiters. `format::render(message,{Plain,Never})` returns exactly `message.view()` as a string. For terminal emission, `TerminalColor::indexed` uses ANSI SGR 38;5;i foreground / 48;5;i background; `trueColor` uses 38;2;r;g;b and 48;2;r;g;b; standard palette indexes 0..15 also use indexed sequences or equivalent short SGR forms, but the chosen canonical encoding must be stable in snapshot tests. Text style codes: bold 1; dim 2; italic 3; underline 4; resetting all relevant attributes uses 0. ANSI sequences emitted only for non-default style runs, with reset after runs and before leaving the renderer; a normalized plain run after a styled one cannot inherit styling. No interpolation, no terminal detection beyond existing policy, no embedded raw escape injection as a styling API. Text itself may contain ESC bytes if callers explicitly append them; the formatter does not sanitize arbitrary text (security boundary documented); **RECOMMENDED** diagnostic-domain providers escape untrusted control characters before display.

### API-FMT-015 — runtime style and stream failure

`RenderPolicy` is only about plain versus terminal styling and uses already-existing `Presentation`/`ColorMode`, not new redundant enums. A sink is expected to support writing bytes and signaling failure. For `std::string` output, allocation is permitted, but no user-defined exceptions may be thrown by Miracle's own logic; standard allocation failure or stream exception mask behavior remains governed by standard library operations. The library's *no custom exceptions* policy cannot guarantee that every standard library operation is `noexcept`. Public signatures therefore must **not** assert `noexcept` on potentially allocating render paths. A rendering implementation targeting a nonthrowing sink can provide `bool`/iterator status, but this policy must be explicit and tested.

### API-FMT-016 — no generic formatting syntax in Phase M

No public `{}`-pattern grammar, `Formatter<T>` customization point, floating-point formatting, type-erased runtime `Message`, localization, user-defined terminal command language or `std::format` clone. Existing `Miracle::staticFormat` remains in `Miracle.StaticString` untouched unless a separate semantic amendment is approved. One small integral conversion routine inside Format is not an attempt to replace it.

## 5.3 Canonical diagnostic grammar

Header (mandatory):

```text
<severity>[<optional-code>]: <summary>\n
```

Context (optional, at most one):

```text
  --> <context>\n
```

Fields (optional; fields in insertion order, one per line):

```text
  <key>: <value>\n
```

Notes (optional; insertion order):

```text
  = note: <note text>\n
```

Help (optional; insertion order):

```text
  = help: <help text>\n
```

A blank line separates **nonempty sections** after the header; optional sections that contain no items leave no gap. Every diagnostic message ends with **exactly one** line-feed after its last line (**DEC-FMT-003 APPROVED**). Severity spelling is lowercase `debug`, `note`, `warning`, `error`. Code omission means no brackets. Padding is left-zero-fill to max(minimumDigits, actual decimal count), without truncation. Context is **author-provided logical operation**, not a fake compiler file location. Non-ASCII characters are preserved bytewise; tab is escaped as literal `\\t` in single-line fields, while explicitly structured multiline snippets may retain tabs under Diagnostics control. Visual column alignment with Unicode graphemes is deferred. Embedded `\n` in user-supplied field text is currently **a layout risk**; the proposed grammar requires escaping or rejecting raw control characters in domain-owned summary/key/context and encoding multiline values as separate fragment events. `DEC-FMT-004` is **APPROVED**: escape unsafe C0/DEL controls in single-line diagnostic fields using *literal* escape spellings (`\\n`, `\\r`, `\\t`, `\\0`, `\\x1B`, `\\xNN`) and preserve structured multiline snippet/layout events separately.

Illustrative richly-styled diagnostic:

```text
error[ITR004]: callable cannot accept sequence element
  --> Iter::map

  callable: MoveOnlyCallable
  element:  std::tuple<int, float>

  = note: direct call, tuple expansion and reflected aggregate expansion failed
  = help: define a callable matching one supported invocation shape
```

A runtime Diagnostic may additionally contain a real `file:line:column`, source snippet, selection, secondary spans, causes, stack traces and code metadata. These belong to Diagnostics, but severity/code/header and fragment style emission are governed by Format. Do not erase the richer runtime structure to force it into this minimal compiler-error grammar.

## 5.4 Format examples and negative space

```cpp
// PROPOSED — compile-time plain content
constexpr auto msg = Miracle::format::Message{"Hello "}
    .append(Miracle::format::styled("world", accentStyle));
static_assert(msg.view() == "Hello world");

// PROPOSED — diagnostic mode
constexpr auto msg2 = Miracle::format::Message{
    Miracle::Severity::Error,
    Miracle::format::code<3>("STR", 1),
    "invalid character type"
}.context("BasicStaticString")
 .field("actual", Miracle::format::fragment("UnstableChar"))
 .note("the character type is not standard-layout")
 .help("use a trivially-copyable, standard-layout character type");

// PROPOSED — C++26 generated message
static_assert(/* desired predicate */, msg2);
```

Do **not** treat these snippets as reference-compiler successes. In particular, `Message` direct construction/deduction guides with diagnostic Code types are not implemented yet; formal `constexpr` object usability and `static_assert` expression syntax must be tested with the pinned C++26 snapshot. The examples are intended to expose signature questions before approval, rather than to disguise them.

**Negative cases:** context after note; second context; field after help; message with `std::string_view` unknown extent being returned as an exact-size compile-time type; `styled()` with non-owning temporary `string_view`; arbitrary UTF-8 column alignment; `rgb(300,...)` implicit narrowing; invalid hex; NaN HSV; ANSI coloring leaking into plain view; `static_assert` requiring a text pointer to a destroyed object. Every case maps to a named test later.

## 5.5 Lifetime, allocation and performance matrix

| Operation | Input ownership | Output ownership | Allocation | Expected cost | Failure |
|---|---|---|---|---|---|
| Color::rgb | value channels | 3 bytes | none | O(1) | caller-conversion risk |
| Color::hex | u32 | optional Color | none | O(1) | nullopt on high bits |
| Color::parseHex | borrowed view, consumed | optional Color | none | O(length<=7) | nullopt |
| Color::fromHsv | Hsv value | optional Color | none | O(1) | nullopt invalid |
| Fragment literal | borrowed array, copied | owned array+Style | none | O(N) | ill-formed unsupported type |
| Message initial | literal fragments, copied | owned plain+styles | none | O(N) | compile-time ill-formed bad input |
| Message append | prior owned object + new extent | new owned object | none | O(current length + added) | static capacity requirement |
| Message context/field/note/help | sized text/fragments | new owned object | none | O(current+added) | constrained ordering violation |
| Message data/view | owned Message | borrowed view/pointer | none | O(1) | dangling if Message lifetime ends |
| Format plain render | borrowed Message | std::string | may allocate | O(text bytes) | std allocator failure |
| Format ANSI render | borrowed Message | sink/string | sink-dependent | O(bytes + runs×escapes) | sink-dependent |
| Diagnostics adapter | borrowed/move-only Diagnostic | output sink | existing behavior | O(total diagnostic bytes/spans/causes) | stream policy |

**REQ-PERF-003 [REQUIRED].** Compiler complexity and runtime behavior must be compared to the old implementation. The approved contract did not set fixed percentage thresholds; report the actual observed regression/benefit. Large message or many-fragment pathological cases must be measured separately from ordinary diagnostic use.


---

# Part VI — Module ownership and internal decomposition

## 6.1 Target source tree and named-module inventory

The following layout is the proposed **end state**, not a claim that these files exist. Where a component has no independently justified subdivision, a single primary interface is preferred. The actual current tree is reflected in §1.4. Implementation-unit placement is driven by template reachability and build overhead, not aesthetic line counts.

```text
src/
  public/
    Miracle.cppm                      export module Miracle; // reexports only
    numeric/Numeric.cppm              export module Miracle.Numeric;
    color/Color.cppm                  export module Miracle.Color;
    format/Format.cppm                export module Miracle.Format;
    format/Message.cppm               export module Miracle.Format:Message; [if helpful]
    format/Render.cppm                export module Miracle.Format:Render;  [if helpful]
    types/Types.cppm                  export module Miracle.Types;
    static_string/StaticString.cppm   export module Miracle.StaticString;
    range/Range.cppm                  export module Miracle.Range;
    iter/Iter.cppm                    export module Miracle.Iter;
    iter/Detail.cppm                  export module Miracle.Iter:Detail;   [optional]
    iter/Sources.cppm                 export module Miracle.Iter:Sources;
    iter/Views.cppm                   export module Miracle.Iter:Views;
    iter/Core.cppm                    export module Miracle.Iter:Core;
    iter/Consumers.cppm               export module Miracle.Iter:Consumers; [only if coherent]
    diagnostics/Diagnostics.cppm      export module Miracle.Diagnostics;
    feature/Feature.cppm              export module Miracle.Feature;
    feature/Types.cppm                export module Miracle.Feature:Types;
    feature/Descriptor.cppm           export module Miracle.Feature:Descriptor;
    feature/Set.cppm                  export module Miracle.Feature:Set;
    feature/Catalog.cppm              export module Miracle.Feature:Catalog;
    feature/Requirement.cppm          export module Miracle.Feature:Requirement;
    error/Error.cppm                  export module Miracle.Error;
    panic/Panic.cppm                  export module Miracle.Panic;
    fs/Fs.cppm                        export module Miracle.Fs;
    concepts/Concepts.cppm            export module Miracle.Concepts;
    debug/Debug.cppm                  export module Miracle.Debug;
    bitflags/Bitflags.cppm            export module Miracle.Bitflags;
    memory/Memory.cppm                export module Miracle.Memory;
    profiling/Profiling.cppm          export module Miracle.Profiling;
    build/Build.cppm.in               produces Miracle.Build;
    build/Capabilities.cppm.in        produces Miracle.Capabilities;
    build/FeatureConfiguration.cppm.in produces Miracle.FeatureConfiguration;
    meta/Meta.cppm                    export module Miracle.Meta;
    meta/{Types,Storage,Operators,Names,Predicates,Access,Cache,Members,
          Layout,Annotations,Values,Substitution,Generation,TypeQueries,
          TypeTransforms,Reflect,Query}.cppm
  private/
    diagnostics/Diagnostic.cpp        module Miracle.Diagnostics;
    error/Error.cpp                  module Miracle.Error;
    panic/Panic.cpp                  module Miracle.Panic;
    fs/Fs.cpp                        module Miracle.Fs;
    profiling/Profiling.cpp          module Miracle.Profiling;
    memory/{linux,windows,unsupported}/Memory.cpp  module Miracle.Memory;
```

`format/Message.cppm` and `format/Render.cppm` are **not automatically required**: the naming is illustrative, and an initial single `Format.cppm` may be simpler. Conversely, a generic template whose definition is needed for consumer instantiation cannot be hidden in an implementation `.cpp` to reduce interface size. `Iter:Consumers` is only justified after resolving dependencies with `Iter:Core`; it is not a mandate to cut a class definition into arbitrary files.

### Complete source-to-owner reconciliation table

| Existing location / old owner at inspected revision | Target primary owner | Required semantic preservation |
|---|---|---|
| `src/public/numeric/Numeric.cppm`: `Miracle.Numeric` | unchanged | `u8...u64`, `i8...i64`, `f32/f64`, `usize/isize` exact alias definitions |
| `src/public/types/Types.cppm`: `Miracle.Types` | unchanged | String, StringView, Vec, Array, Option, Span, Ref, Path, FlatSet, FlatMap, InplaceVec, Hive, etc. |
| `src/public/static_string/StaticString.cppm`: `Miracle.StaticString` | unchanged | `BasicStaticString`, `StaticString`, `staticFormat`, parser, adapters, NTTP semantics |
| `src/public/range/Range.cppm`: `Miracle.Range` | unchanged | range endpoint and stride rules, overflow and signedness behavior |
| `src/public/feature/*`: `Miracle.Feature` | unchanged, improve only if evidence requires | Descriptor/Set/Catalog/Requirement constraints and resolution order |
| `src/public/Iter.cppm`: `Miracle:Iter` | `Miracle.Iter` | lazy projection, sources, adaptors, terminals, ownership, move-only behavior |
| `src/public/Diagnostic.cppm`: `Miracle:Diagnostic` | `Miracle.Diagnostics` | source ranges, enum annotations, severities, code alignment, notes/help, causes, errors |
| `src/public/Error.cppm`: `Miracle:Error` | `Miracle.Error` | `Result`, error conversion/ownership and public API |
| `src/public/Panic.cppm`: `Miracle:Panic` | `Miracle.Panic` | panic semantics, contract integration |
| `src/public/Fs.cppm`: `Miracle:Fs` | `Miracle.Fs` | `OpenOptions` and filesystem behavior |
| `src/public/Concepts.cppm`: `Miracle:Concepts` | `Miracle.Concepts` | existing concepts and constraints |
| `src/public/Memory.cppm`: `Miracle:Memory` | `Miracle.Memory` | platform declarations, unsupported platform behavior |
| `src/public/Profiling.cppm`: `Miracle:Profiling` | `Miracle.Profiling` | existing precision/ownership, Debug imports only if genuinely needed |
| `src/public/meta/Bitflags.cppm`: `Miracle:Bitflags` | `Miracle.Bitflags` | reflected bitflags behavior, Meta dependency, move file to `bitflags/` |
| `src/public/meta/Debug.cppm`: `Miracle:Debug` | `Miracle.Debug` | debugging/formatting behavior, move to `debug/` |
| `src/public/meta/Meta.cppm`: `Miracle.Meta` | unchanged | existing façade and reflection partition graph |
| `src/public/meta/Query.cppm`: `Miracle.Meta:Query` | unchanged named module, optionally split partitions | query IR, barrier semantics, evaluation, caching, materialization |
| generated `Build/Capabilities/FeatureConfiguration.cppm` | standalone existing named modules | immutable facts, build identity and selectable feature gates |
| `src/private/*.cpp` with `module Miracle;` | corresponding individual named modules | runtime linkage and behavior, no leftover legacy owner |
| `src/public/Miracle.cppm` | `Miracle` | only `export import` directives, preserve top-level consumer usability |

**REQ-MOD-004 [REQUIRED].** `Miracle.Numeric` owns **only** existing primitive numeric aliases. Do not add arithmetic wrappers, new integer types, unit systems or formatting helpers during Phase M. `Miracle.Types` retains std-backed aliases; whether it re-exports Numeric must be explicitly verified against existing direct-import expectations (currently source imports Numeric but not `export import` it). A direct consumer expecting `Miracle::usize` after only `import Miracle.Types` may fail unless visibility is preserved; add a corresponding test before finalizing the transitional import contract.

**REQ-MOD-005 [REQUIRED].** `Miracle.Color` is imported by Format, and neither imports StaticString, Diagnostics, Meta or umbrella. Introduce this edge into module graph analysis and update any earlier statement claiming Format imports zero Miracle modules. That prior statement is superseded by the user's explicit Color approval and requires repository reconciliation.

## 6.2 CMake source set and generated module rules

The current root target remains `add_library(Miracle STATIC)` with alias `Miracle::Miracle`. Maintain **one** target initially. `target_sources` lists all importable module interface units in `PUBLIC FILE_SET miracle_modules TYPE CXX_MODULES`. Implementation `.cpp` files belong to `PRIVATE` target sources and must carry the migrated owning `module ...;` declaration. Generated module templates end `.cppm.in`; generated files are emitted into the **build tree** with `.cppm` and added to the file set. Preserve generated build-feature identity and capability probe semantics. Do not list feature-gated source files unconditionally if the feature engine deliberately excludes them. There must be no duplicate provider of the same module name.

```cmake
# PROPOSED excerpt only — existing CMake settings remain authoritative
# This shows ownership/file-set semantics, not complete replacement CMakeLists.
target_sources(Miracle
  PUBLIC FILE_SET miracle_modules TYPE CXX_MODULES
    BASE_DIRS "${CMAKE_CURRENT_SOURCE_DIR}/src/public"
              "${miracle_generated_dir}"
    FILES
      src/public/Miracle.cppm
      src/public/color/Color.cppm
      src/public/format/Format.cppm
      src/public/diagnostics/Diagnostics.cppm
      # ... every publicly necessary module/partition, including generated ...
  PRIVATE
      src/private/diagnostics/Diagnostic.cpp
      # ... all implementation units, plus selected platform source ...
)
```

**REQ-BLD-001 [REQUIRED].** CMake must provide a clean `-std=c++26` and `import std;` graph for the pinned Clang fork. The source file extension `.cppm` must be detected as CXX and scanned for module dependencies. A CMake configure success alone is insufficient: build and install a downstream import consumer.

**REQ-BLD-002 [REQUIRED].** Retain supported source/subdir, `FetchContent`, and installed `find_package` consumers with the same `Miracle::Miracle` target. Every new direct import must compile in at least one *independent* installed consumer not relying on source tree include paths or accidental BMI lookup.

**REQ-BLD-003 [REQUIRED].** Scripts and docs must recognize `.cppm`/`.cpp`; build-generation probes, benchmark source templates and test discovery may not silently omit renamed files. A historical code scanner's unsupported C++26 module extraction is not fixed by renaming an extension; keep CodeQL coverage claims evidence-based.

**REQ-BLD-004 [REQUIRED].** Incompatible BMI and object artifacts from prior named-module ownership must not be reused. Use a clean build directory for acceptance and a fresh installed prefix. Check that the exported package provides all primary interfaces and required partitions, even though consumers cannot directly import partitions.

## 6.3 Implementation-unit and platform ownership

Current source observation at `e2da473e` includes `src/private/windows/Memory.cpp` declaring `module Miracle.Core; import :Memory;` while its public memory partition is still `Miracle:Memory`. This mismatch is a **source-level defect candidate**, not a verified Windows build failure. The migration target is `module Miracle.Memory;` and the corresponding import is `import Miracle.Memory;` if needed, or merely definitions with primary-interface reachability consistent with reference compiler. Linux and unsupported implementations also require matching ownership after migration.

**TEST-MOD-PLATFORM-001 [REQUIRED].** Configure at least one source-unit compile test for the Windows implementation with the correct reference target if a Windows toolchain is available; otherwise record Windows compatibility **NOT RUN**, and statically assert that no source retains `module Miracle.Core` or `module Miracle;` for a migrated standalone module. Do not claim Windows support from a Linux build.

## 6.4 Iter: component-level split specification

`src/public/Iter.cppm` is approximately 3,701 lines at the inspected revision, owned by `Miracle:Iter`, with imports of std, Numeric, Types and Range. Its sections are discoverable in source: forward declarations and `std::ranges::enable_view` customizations near the beginning; adaptive invocation helpers, `MovableBox`, `AdaptiveCallable`, `ComposedProjection`, projected iterator/sentinel; source adapters `MaybeRefView`, `MaybeValueView`, `OnceWith`, `RepeatWith`, `FromFn`, `Successors`, `Cycle`; projection/filter/scan/intersperse/window/chunk classes; `Infinity`, `SizeHint`; `Peekable`; core `Iter<View,Projection>`; `ParallelIter`; and `iter()/empty()/once()/repeat()/fromFn()` source factory overloads near the end. This evidence supports a semantic split, not a mechanical line split.

### Proposed partition responsibilities

**`Miracle.Iter:Detail`.** Exported *within the module* for cross-partition template reachability, but public consumer names remain hidden: `HasTupleProtocol`, aggregate reflection caching, adaptive invocation selection, `MovableBox`, `AdaptiveCallable`, `AdaptiveBinaryCallable`, `ComposedProjection`, projected iterator and sentinel primitives, terminal reference/value helper traits and local concepts. Place cross-partition helper definitions **before** their consumers in the dependency order. Do not introduce an import of `Miracle.Meta` merely to simplify reflection; Iter currently uses `std::meta` directly, and replacing it would alter dependency direction and potentially expensive module graphs.

**`Miracle.Iter:Sources`.** Own the source view classes `MaybeRefView`, `MaybeValueView`, `OnceWith`, `RepeatWith`, `FromFn`, `Successors`, `Cycle` together with public factory functions `iter`, `empty`, `once`, `onceWith`, `repeat`, `repeatN`, `repeatWith`, `fromFn`, `successors`, and other exact source factories present in the authoritative checkout. **Dependency trap:** the factories return `Iter`, while `Iter` uses source view types. To avoid `Sources ↔ Core`, split into `SourceViews` (underlying view classes), `Core` (Iter façade), and `SourceFactories` (factory functions) if the compiler graph requires it. This variant is explicitly **RECOMMENDED** over mutually importing interface partitions.

**`Miracle.Iter:Views`.** Own `ProjectedFilterView`, `Scan`, `Intersperse`, `IntersperseWith`, `MapWindows`, `ArrayChunks`, and their range/view behavior; `Peekable` can be placed here if dependencies are only on SourceViews/Detail, or in its own `Peekable` partition if it imports Core. A view's iterator/sentinel nested definitions must remain reachable for arbitrary consumer template instantiation; `std::ranges::enable_view`/`enable_borrowed_range` specializations must stay attached and visible consistently with the C++ ranges constraints.

**`Miracle.Iter:Core`.** Own `Iter<View,Projection>`, `Infinity`, `SizeHint`, `ParallelIter` if logically tightly coupled, and core member method templates (map, filter, reduce, collection/consumption and reference-preserving operations). If physically decomposing the large `Iter` class requires out-of-class method definitions, preserve exact deducing-this syntax, template parameters, overload sets, requires clauses and instantiation reachability. A multi-interface class declaration spread across independent partitions is **not legal C++**; only its function definitions may be separated where language semantics permit.

**`Miracle.Iter:Consumers` (conditional).** May own terminal helper functions and out-of-class definitions that do not require breaking declaration ownership. Do **not** introduce an artificial dependency cycle by moving `Iter` member declarations to one partition while requiring method bodies as re-imported peers. If Core must use a helper, move the helper to Detail; if Consumers must know Core, Consumers imports Core, never the reverse.

### Required ordering

```text
Std / Numeric / Types / Range
             │
             ▼
           Detail
             │
      ┌──────┴────────┐
      ▼               ▼
  SourceViews        Views
      └──────┬────────┘
             ▼
            Core
             │
      ┌──────┴────────┐
      ▼               ▼
 SourceFactories   Consumers (if justified)
      └──────┬────────┘
             ▼
          Iter.cppm (re-export-only primary)
```

**INV-ITER-001 [REQUIRED].** The original adaptive invocation precedence remains **direct call → tuple decomposition → reflected aggregate decomposition**, with the original ill-formed cases and diagnostics. Moving helpers may not change overload resolution, associated constraints or `std::meta::access_context` behavior.

**INV-ITER-002 [REQUIRED].** The one-pending-map-projection invariant, lazy views, reference preservation, move-only source handling, once/terminal evaluation rules, borrow behavior and `Peekable` cursor state remain unchanged. `ParallelIter` dispatch remains driven by the chosen standard execution policy, not a new Miracle scheduler.

**INV-ITER-003 [REQUIRED].** `std::ranges` variable-template customizations remain valid declarations with the required reachability. They must not silently become different specializations, duplicate definitions or unreachable imported declarations. Test `std::ranges::view`, `borrowed_range`, and iterator category values for each affected type **from outside the library**.

**INV-ITER-004 [REQUIRED].** `Iter` template method bodies remain reachable at consumer instantiation. Test custom user types, move-only callables, lvalue/rvalue ranges, non-common sentinel sources, and reflected aggregates from both umbrella and direct imports.

### Concrete extraction procedure (author-owned)

1. Record `git show BASE:src/public/Iter.cppm` and current test list; export a machine-readable list of declared symbols/constraints, not just line numbers.
2. Introduce `Miracle.Iter` primary interface skeleton and `Detail` containing helper declarations/definitions; build using *no changed APIs*. Assert no original `Miracle:Iter` owner remains once extraction completes.
3. Move source view classes into SourceViews, then view/adaptor classes into Views. Preserve public export markers and `std::ranges` specializations; compile a direct import consumer after each move.
4. Move `Iter` core class and `ParallelIter` together initially. Only later move standalone terminal helpers when an acyclic graph and consumer reachability have been demonstrated.
5. Move factory overloads last into SourceFactories to eliminate the `Iter`-returns-`Iter` dependency cycle.
6. Re-export exact required interface partitions from primary `Miracle.Iter`. Verify negative adaptive invocation tests fail for the expected reason; verify randomized range and ownership tests.
7. Measure cold/incremental compilation with unchanged runtime behavior; record class and BMI ownership differences. Do not claim a smaller BMI or faster build without evidence.

### Examples of unacceptable Iter regressions

A `std::ranges::enable_view` specialization added after a consumer has already instantiated the primary trait can be too late, changing constraints or being ill-formed. Moving `adaptiveInvoke` into a nonreachable `.cpp` can change member template substitution from user-defined types. Making source factory import Core while Core imports SourceFactories creates an actual module cycle. Turning `MaybeRefView` into a borrowed temporary is a dangling-lifetime bug. Flattening iterator references into values to simplify partition boundaries is a semantic rewrite. All are **Blocker** or **Major** findings based on observed behavior.

## 6.5 Feature: preserve already extracted partitions

At the inspected revision, `src/public/feature/Feature.cppm` is the primary façade, re-exporting `Types`, `Descriptor`, `Set`, `Catalog`, `Requirement`. `Types` imports Numeric and StaticString; `Descriptor` imports Numeric, Capabilities, and Types; `Set` imports Descriptor; `Catalog` imports Set; `Requirement` imports Descriptor. The source is already decomposed. The earlier guideline to split a 1,365-line `Feature.ixx` is now obsolete at this revision and must not be followed mechanically.

**REQ-FEA-001 [REQUIRED].** Preserve Feature's descriptor option categories, default selection, build-versus-semantic constraint, canonical graph, deterministic cycle detection, name validation, `ResolutionReport`, structural feature-set values, build/capability resolution and generated feature-configuration module. The exact existing public API and its tests remain authoritative.

**REQ-FEA-002 [REQUIRED].** Existing internal feature reports use `StaticString<MessageCapacity>` and derive bounds from feature/catalog names. They may be adapted to the shared Format diagnostic layout in a failing branch, but no release-time or success-path resolution behavior may be changed by a formatting refactor. `Catalog::analyze*` must remain non-emitting; `Catalog::resolve*` may emit the authored `static_assert` failure.

**REQ-FEA-003 [REQUIRED].** A Feature resolution failure should identify the exact feature and failing relation (missing feature/build capability, duplicate, cycle, conflict) and, where already available, the deterministic cycle chain. Codes should be allocated in a module-owned domain; numerical codes should be stable after the author assigns them. Format supplies layout and does not own resolver diagnostics.

**TEST-FEA-001..007 [REQUIRED].** Compare current to migrated outputs for empty catalogs, duplicate names, missing dependency, missing build feature, missing capability, conflicting features, cycle ordering. Successful catalogs must not instantiate failure-only builders; rejected catalogs must preserve the intended semantic error and useful context. Use generated Build and FeatureConfiguration modules with enabled/disabled feature settings.

## 6.6 StaticString: preserve its role and solve bootstrap cleanly

At this revision `Miracle.StaticString` already owns the 945-line interface, imports `std` and `Miracle.Numeric`, and provides `BasicStaticString<Char,Capacity>`, structural NTTP support, `StaticString` alias, split results and `staticFormat`. Format must not import it. During Phase M it may import Format to produce rich authored diagnostic messages. Its existing methods for substring, trim, find, split, formatting and structural storage do not change.

**REQ-STR-001 [REQUIRED].** Keep `BasicStaticString` structural, fixed-capacity, null-terminated within its current public representation, preserving `length`, `storage`, deduction guides and public aliases used as NTTPs. Do not silently replace it with `format::Message`, which has a distinct purpose and does not require structural NTTP semantics.

**REQ-STR-002 [REQUIRED].** For static format grammar failures, report the earliest supported failure condition with consistent Format-authored text: `char`-format requirement, unmatched or unsupported brace grammar, and placeholder-count mismatch. Reuse the existing formatting parser as the semantic authority; Format owns message presentation, not format-string parsing. Avoid recursively calling `staticFormat` to format its own errors if doing so risks a bootstrap path or circular dependency.

**REQ-STR-003 [REQUIRED].** The `BasicStaticString` character-type requirement must remain queryable or at least reject invalid types at the same instantiation boundary. A separately exported `StaticStringCharacter<T>` concept is an optional façade only if it doesn't change substitution or constraint semantics; do not introduce it solely for Phase 6.

**REQ-STR-004 [REQUIRED].** Preserve exact active-length and zero terminator semantics. Format Message's `size()` and `view()` intentionally differ in purpose: they are presentation strings; they need not expose StaticString's modification, split and search methods.

## 6.7 Diagnostics: separate semantic model from presentation

Current `Miracle::Diagnostic` is move-only and stores severity, enum-derived code descriptor, canonical message, source spans, notes, help, nested causes, optional stacktrace. It uses `Severity` and source/render option vocabulary defined in its current interface, and `src/private/Diagnostic.cpp` contains a runtime renderer with hardcoded severity ANSI, code width, snippet highlighting, source path normalization, notes, metadata and causes.

**REQ-DIAG-001 [REQUIRED].** Extract the named module `Miracle.Diagnostics` from `Miracle:Diagnostic` without changing `Miracle::Diagnostic` public factory/member spelling (`Diagnostic::error`, `::warning`, `::note`, `::debug`, `.message`, `.note`, `.help`, `.span`, `.cause`, `.trace`, etc.). Preserve `diagnostics::prefix(...)`, `message(...)`, `align(...)` annotations and the `DiagnosticCode` concept.

**REQ-DIAG-002 [REQUIRED].** Move the **definition** of `Miracle::Severity`, `ColorMode`, `Presentation` to Format and remove their old definitions from Diagnostics. Keep one source of truth. Keep `SourceMode`, `DetailMode` and Diagnostics-specific `RenderOptions` in Diagnostics; do not create competing equivalents inside Format. Cross-module exported name reachability must be verified with a consumer importing only `Miracle.Diagnostics`.

**REQ-DIAG-003 [REQUIRED].** Diagnostics owns enum-domain metadata and reflected annotation parsing; `DiagnosticCodeDescriptor` may own strings, normalized code number and alignment. It passes a view of the relevant data into Format's common header/fragment renderer. Format does not reflect arbitrary diagnostic enums, allocate a code registry, import Diagnostics, or interpret a `std::meta::info` independently.

**REQ-DIAG-004 [REQUIRED].** Runtime rendering semantics of existing `DiagnosticSpan`, `RenderOptions`, `SourceMode`, `DetailMode`, cause traversal and contract-violation conversion remain unchanged. Source path normalization belongs in Diagnostics (because it depends on filesystem and source_location); Format handles byte layout and styling. Existing source snippet caret selection, including out-of-range ranges and overlapping roles, must not regress.

### Runtime renderer decomposition

The existing `DiagnosticRenderer` performs distinct responsibilities:

1. Extract and validate runtime metadata in Diagnostics (`DiagnosticCodeDescriptor`, `SourceRange`, location, trace and cause list).
2. Produce a traversal of diagnostic nodes, preserving established order; a stack of traversal frames already avoids recursion across deep causes.
3. Convert each header, context, snippet, note/help or metadata field into **Format output events** (text fragments with Style and layout directives), not an independently ANSI-coded string.
4. Invoke Format's one SGR encoding engine on those events, or plain bypass; preserve `RenderOptions` as the policy source.
5. Emit to provided stream/string and propagate underlying sink failure according to documented semantics; never retain a view beyond immediate consumption.

**ALG-DIAG-001 — Render diagnostic tree.** Use iterative DFS with a stack of frames `{const Diagnostic* parent, size_t nextChild, size_t depth}`, as the baseline does. On entry render node header/body before children, then push first child frame; after child completion continue siblings in original insertion order. All source/notes are rendered through shared Format rules. Time O(total nodes + total authored bytes + total span bytes), stack O(maximum nested depth), not O(nodes) recursion stack. The maximum number of causes can still cause large output; this is expected and should be benchmarked. Node ownership stays in `Diagnostic`; stack holds non-owning pointers valid for the duration of the immutable render call.

**ALG-DIAG-002 — Render source selection.** Clamp half-open `SourceRange` endpoints to snippet length; if endpoints are reversed, preserve the existing normalization behavior. Derive logical start/end line boundaries, mark primary span using caret and secondary using a distinct marker/style. Use Format's styled fragments, not raw pre-colored text. Keep byte-offset semantics (not Unicode grapheme columns) unless the prior contract expressly promises otherwise. Test empty snippet, zero-length span, span crossing line boundary, out-of-bounds, and absent location.

**ALG-DIAG-003 — Code display.** Extract domain prefix, nonnegative numeric value and width from Diagnostics; call Format's shared code encoder with `minWidth=alignment`, no truncation. Preserve prefix-less behavior and enum alias canonical messages. Runtime and compile-time paths must produce matching plain heading grammar for equivalent semantic inputs. Duplicate code enums are domain-semantic policy and remain in Diagnostics.

### Diagnostic fragments and Switch precedent

Switch's existing `DiagnosticFragment { String text; bool highlighted{}; }` demonstrates the need for selective styling, but it is **not** a new Miracle API. Miracle Format generalizes the concept to `Fragment<N>` with explicit Style and RGB/indexed choices, while Diagnostics may own dynamic source strings and use immediate-consumption fragment events. No dependency on Switch is introduced. Do not adopt Switch's temporary parallel note/attachment layout or duplicate color flags.

**INV-DIAG-002 [REQUIRED].** An identical semantic Diagnostic rendered with `Presentation::Plain` contains no ANSI regardless of `ColorMode`. Rendered colors must not change `Diagnostic::messageText()`, its code identity, source location or causes. Source rendering and cause order must be deterministic under identical inputs.

## 6.8 Error, Panic, Fs, Concepts, Memory, Profiling, Debug and Bitflags

**REQ-REST-001 [REQUIRED].** Move interfaces to exact named modules `Miracle.Error`, `Miracle.Panic`, `Miracle.Fs`, `Miracle.Concepts`, `Miracle.Memory`, `Miracle.Profiling`, `Miracle.Debug`, `Miracle.Bitflags`. Each retains its original namespace, semantics, constraints and relevant standard-library dependencies. Move Debug and Bitflags source files out of the `meta/` directory for architectural clarity without renaming their public symbols.

**REQ-REST-002 [REQUIRED].** For Error/Panic/Fs, update implementation unit declaration and imports together with the exported interface. Preserve error types, `std::expected` aliases, conversion behavior, panic error report and filesystem options. Generic API behavior cannot be inferred solely from a successful module import; run actual tests with errors, path handling and contract/panic cases.

**REQ-REST-003 [REQUIRED].** For Memory, use one platform implementation unit at a time, all owned by `Miracle.Memory`; unsupported platforms retain explicit unsupported semantics. Do not leave `module Miracle.Core;` in Windows implementation, and do not claim cross-platform verification without compiling on that platform/toolchain.

**REQ-REST-004 [REQUIRED].** For Debug/Bitflags, preserve reflection imports and constraints. Moving declarations outside the umbrella must not alter the reflection access context or the order of enumeration members/flag derivation. Preserve source-level `import Miracle;` and add `import Miracle.Debug;` / `Miracle.Bitflags;` consumer tests.

**REQ-REST-005 [REQUIRED].** Profiling may import Debug only when actually needed; its declaration and platform lifetime semantics remain unchanged. Avoid introducing Format imports solely for convenience if no Format-authored diagnostic is required.

## 6.9 Meta and Query: conservative decomposition

`Miracle.Meta` already is a standalone named module with interface partitions: Types, Storage, Operators, Names, Predicates, Access, Cache, Members, Layout, Annotations, Values, Substitution, Generation, TypeQueries, TypeTransforms, Reflect, Query. Query remains roughly 2,191 lines in one interface; size alone is not a defect. Meta Query's reflection observation points, `std::define_static_array` promotion, constant-evaluation exception behavior, lazy evaluation and cache semantics are fundamental to the completed Phase 5 contract.

**REQ-META-001 [REQUIRED].** Do not use Phase M to redesign Query IR, evaluation semantics, materialization rules, reference preservation, cache key identity, reflection access or declaration-point sensitivity. Split only if actual dependencies yield independently cohesive partitions and evidence shows semantic equivalence.

### Proposed optional Query partition split

```text
Miracle.Meta:QueryIR        — Instruction / ProgramNode / SegmentNode / SegmentPlan
Miracle.Meta:QueryState     — capture environment, source wrappers, State construction
Miracle.Meta:QueryPlan      — graph of compiled evaluation stages / slot planning
Miracle.Meta:QueryEval      — evaluator and sinks, preserving stage order
Miracle.Meta:Query         — public Query<T> façade, query(...) factories
```

The order of these pieces is a proposed mapping, not authorization to change IR representation or compiler-visible reflection point. If `QueryPlan` imports `QueryEval` and `QueryEval` imports `QueryPlan`, combine them; the graph must be acyclic. Keep externally instantiated template bodies reachable. Preserve `std::meta::exception` control flow for recoverable constexpr errors. A Query operation may query a reflected scope that changes at its declaration point, so shifting a consteval call's instantiation context may change behavior even if the API looks identical. Detect these with existing late-declaration and cross-translation-unit hardening tests.

**INV-META-001 [REQUIRED].** For a fixed source revision and compiler snapshot, observable Query results, value/reference categories, evaluation order, cache behavior and error paths match the pre-migration version. No code movement may turn a lazy evaluator into eager materialization, or change whether a borrow escapes permitted lifetime boundaries.

**INV-META-002 [REQUIRED].** Meta partitions must form an acyclic subgraph. No imported reliance on Phase 6 Validation or on the `Miracle` umbrella. The existing `Cache` partition must preserve its promoted static lifetime and reflect identity constraints where applicable.

### Decision policy for M.6

If the Query split increases BMI count and compiler work without an import-boundary or maintenance benefit—or risks changing declaration-point semantics—**do not split it**. A documented decision to retain one large cohesive interface is compliant with the original Phase M contract. Independently named modules, not minimum file size, are the architectural requirement. For Iter, a split has stronger justification because it contains separable sources, views and core algorithms; still validate the actual graph.

---

# Part VII — Internal algorithms, representations and invariants

## 7.1 Message representation: exact capacity and style runs

The proposed representation is a contiguous owned byte array and style-run array:

```cpp
// PROPOSED internals — not necessarily exported literal source
struct StyleRun { std::size_t begin, end; Style style; };

template<std::size_t Capacity, std::size_t RunCapacity, bool Diagnostic, Section Stage>
class Message {
    std::array<char,Capacity+1> text_{};
    std::array<StyleRun,RunCapacity> runs_{};
    std::size_t activeBytes_{};
    std::size_t activeRuns_{};
};
```

The +1 byte allows NUL termination for interoperable APIs; plain text may contain embedded NUL and `view()` must still report the full active byte count. `Capacity` is a compile-time **upper bound** deduced from input literal extents, known decimal bounds, and layout punctuation. `RunCapacity` is a compile-time bound from fragment count, not a runtime heap allocation. A normalized output may have fewer active runs because unstyled or adjacent compatible fragments are coalesced; unused entries are inert.

**INV-MSG-001 [REQUIRED].** `0 <= activeBytes_ <= Capacity`. `text_[activeBytes_]=='\0'`. The observable range is exactly `[data(), data()+activeBytes_)` even when an interior byte is NUL.

**INV-MSG-002 [REQUIRED].** `0<=activeRuns_<=RunCapacity`; each active run `[begin,end)` has `0<=begin<end<=activeBytes_`; runs are ordered by begin and nonoverlapping. Adjacent runs with equal Style SHOULD be coalesced; canonical tests may require exact coalescing after approval.

**INV-MSG-003 [REQUIRED].** Any accessor returning a view into message storage borrows the message's lifetime. Extension operations copy all semantic bytes and style records, offset new runs by old active size and return a new value, leaving original unchanged. No self-referential pointer is stored in the message.

**INV-MSG-004 [REQUIRED].** `StyleRun` indexes operate in bytes, not UTF-8 code points or terminal display columns. Format never splits an input fragment by byte without caller direction, so styled UTF-8 sequence boundaries remain intact if caller supplied valid UTF-8 fragments. The module does not perform Unicode validation during Phase M.

### Algorithm ALG-FMT-001 — append one sized fragment

```text
inputs: old Message<C,R,D,S>, Fragment<N> F, allowed new Section S'
newC = checked_static_add(C, N + layout_overhead)
newR = checked_static_add(R, newStyleRunPossible ? 1 : 0)
construct Message<newC,newR,D,S'> result with zero-initialized storage
copy old.activeBytes characters to result.text
copy old.activeRuns runs unchanged
append any layout separator bytes (known from old stage -> S')
append exactly N fragment bytes
if style != default and N>0:
    append run [offset,offset+N) with style
    optionally coalesce with immediate preceding same-style adjacent run
set result.activeBytes and sentinel NUL
return result; original is unchanged
```

For compile-time literal input, `N` is known. For decimal input, `N` is replaced by the statically known maximum representable decimal width; `activeBytes` records the actual digits. All type-level additions must be checked for wrap before array instantiation. The implementation must not trust an unchecked `C+N` that wraps `size_t` to a small value. For unrealistic extents beyond compiler resource limits, compilation may fail with a documented compile-time resource diagnostic; no arbitrary small hard cap may be silently inserted.

**Correctness:** old bytes and runs are copied without modification and remain within newC/newR by construction; the new fragment's offset is old active byte count plus exact separator length; its run does not overlap prior runs; sentinel is placed exactly after resulting active bytes. No dynamic allocation or dangling input survives return. Termination follows finite bounded loops over static extents.

**Complexity:** O(C_active + R_active + N + separator) time and O(C_new + R_new) compile-time result storage; K chained appends can yield O(KN) total copying. Coalescing adjacent style runs reduces resulting R without changing text.

### Algorithm ALG-FMT-002 — diagnostic section transitions

Section state is monotonic and encoded in a template parameter or conceptually equivalent compile-time state. Proposed progression:

```text
Heading(0) -> Context(1) -> Fields(2) -> Notes(3) -> Help(4)
```

Calling a method with stage less than current stage is ill-formed via constraints. Context may be entered only once; the cleanest approach is stage semantics where context is only available from Heading, and calls to context at stage Context are excluded. Subsequent Fields may repeat; Notes may repeat; Help may repeat. A transition omitting stages is allowed. The formatter inserts exactly one blank line before the first item of any non-heading section and one newline before each additional item in the same section; the header ends in one newline. To enforce one trailing newline total, never add an extra newline after a terminal build; append lines already terminated by `\n`.

**INV-SEC-001 [REQUIRED].** Diagnostic messages contain one heading and at most one context; field/note/help groups are ordered; no transition to an earlier group is accepted. Repeated members inside the same group preserve caller order. Plain messages have no section-state restriction.

**INV-SEC-002 [REQUIRED].** Diagnostic mode's high-level `append` is unavailable (or available only internally for renderer-controlled layout); caller cannot bypass section ordering by appending arbitrary raw text after a `help` section. Arbitrary-output use cases use ordinary Message mode or explicit Format render events.

**Correctness:** constraints reject reverse transitions without runtime state; each stage knows whether it is entering a new group. The resulting separator sequence is determined by stage pair, making formatting deterministic and allocation-free. **Caveat:** computed runtime code prefix/number and unknown-length reflected names require conservative typed bounds or pre-sized fragment conversion before creating final Message types.

### Algorithm ALG-FMT-003 — decimal encoding

For unsigned input, repeatedly extract `digit=x%10` and set `x/=10` until x==0, ensuring zero emits one digit. Store reversed digits in a local fixed array whose capacity is bounded by `numeric_limits<T>::digits10 + 1`. For signed input, determine sign before magnitude; compute `U magnitude = value < 0 ? U{0}-static_cast<U>(value) : static_cast<U>(value)`, never `-value` in signed type. Emit minus sign then reverse digits. For code padding, emit `max(minDigits-digitCount,0)` zeros first. Complexity proportional to decimal digits, no locale, no allocation, no overflow under correct U type.

**INV-NUMTXT-001 [REQUIRED].** Decimal renderer covers zero, unsigned maximum, signed minimum, sign and code width without truncation, and renders identical normalized bytes through compile-time and runtime paths. Width is a minimum; no maximum truncation.

### Algorithm ALG-FMT-004 — ANSI encoding and transitions

The renderer processes an ordered stream of plain fragments with Style metadata. Plain mode writes only text. ANSI terminal mode maintains a *local* current Style (initialized to default). Before outputting a fragment, if style differs from current style, emit the canonical transition sequence. Recommended simple strategy for Phase M: emit `ESC[0m` if current style was nondefault, then emit the complete new style as one or more SGR sequences. For nondefault foreground/background indexed color use SGR `38;5;<index>` / `48;5;<index>`; for RGB use `38;2;<r>;<g>;<b>` / `48;2;<r>;<g>;<b>`; attributes use 1,2,3,4. Empty fragments do not change style; style transitions only occur before nonempty output. At end, if nondefault, emit `ESC[0m`. Consecutive same-style fragments skip redundant transitions. All encoder state is local to the render call.

```text
current = defaultStyle
for fragment in order:
    if fragment.empty: continue
    if policy is Plain or Never:
        write(fragment.text)
        continue
    if fragment.style != current:
        if current != default: write(ESC[0m)
        if fragment.style != default: writeSGR(complete fragment.style)
        current = fragment.style
    write(fragment.text)
if terminal_enabled and current != default: write(ESC[0m)
```

**INV-ANSI-001 [REQUIRED].** Removing ANSI control sequences emitted by the renderer from an ANSI presentation yields the same byte sequence as plain rendering, unless caller text itself contains ESC bytes; such caller-supplied escapes are not safely distinguishable by a generic stripper. Every render that emitted nondefault styles terminates in a reset, and no state leaks to another render call.

**INV-ANSI-002 [REQUIRED].** The encoder MUST NOT treat ANSI index 1 as a hardcoded RGB red. Actual indexed palettes are terminal-configurable. Color RGB/HSV conversion must not change the stored index tag. RGB24 terminal encoding uses exact channel values from Miracle.Color.

**Complexity:** O(total text bytes + number of style runs × maximum SGR output) and O(1) encoder state, excluding sink storage. Output may be slightly larger than plain text due to escapes; this is expected and testable. Avoid a persistent global output color state: it would become unsound across independent consumers and threads.

### Algorithm ALG-FMT-005 — runtime Diagnostics event adaptation

Diagnostics iterates semantic objects; it does not instantiate `Message<C,R>` for runtime length. For each section, it obtains a borrowed text view, semantic style, and logical layout event; it calls Format's statically dispatched writer, which applies the same SGR encoder/grammar. The sink must consume borrowed data before the event returns. For source snippets, Diagnostics computes source offset/line markers then emits those formatted fragments. For causes, iterative DFS maintains the existing parent/nextChild/depth stack. No duplicate severity-to-string, code-padding, or ANSI encoder in Diagnostics.

**Proof of lifetime safety:** Diagnostic owns all runtime string/spans during immutable render; each view is consumed immediately; no view escapes to asynchronous storage or returned Message. Frame pointers refer to child Diagnostics owned by the unchanged parent object, whose lifetime covers the rendering call. Renderer and sink do not take ownership of the Diagnostic.

**Performance:** O(total rendered bytes plus cause nodes and spans), allocations remain those implied by the output sink/string and existing Diagnostic-owned dynamic containers; no intermediary rendered-string per node is required. Source-span location resolution may perform filesystem work per current renderer; Phase M must not introduce new calls for identical inputs.

## 7.2 Module graph validation algorithm

**ALG-MOD-001 [REQUIRED].** Enumerate all module-providing and module-consuming source units from actual source (including generated interfaces) and normalize each import to a named-module or module-partition edge. An edge `A -> B` means module unit A imports B; `import std` is external leaf. Construct adjacency lists, run DFS coloring or Kahn topological sort, and reject a cycle. Verify exactly one primary interface provider for each named module; no `module Miracle;` implementation remains for a facility whose owner has changed. Match CMake file-set entries against the discovered provider list, accounting for feature gating. A missing provider/import is not an isolated incidental compile error—it is an architectural inventory mismatch.

```text
providers = map<module_name, [source units]>
for every .cppm/.cpp unit:
   parse declared module identity
   record partition/primary/implementation kind
   parse imports; normalize :P to current named module M:P
   add directed dependency edges
for each named module:
   assert exactly one primary interface
   assert interface partitions referenced are provided
assert each required public interface appears in PUBLIC CMake module file set
assert no reachable PRIVATE interface dependency breaks installed consumer
order = Kahn_sort(graph)
if number_ordered != number_nodes: reject with explicit cycle path
```

**Complexity:** O(V+E) once source metadata is obtained; file parsing complexity O(total source bytes). Real C++ syntax parsing may require compiler/CMake scanner data rather than fragile regex; a regex-only import inventory is suitable as a cross-check, **not** definitive proof because imports can appear after global module fragments or under preprocessor conditions. CMake-generated `.ddi` scan outputs and reference compiler module dependency logs are preferred verification evidence.

**INV-MOD-001 [REQUIRED].** The graph's provider set matches source/CMake/installed metadata. There are no omitted generated outputs, duplicate named module primaries or unresolved consumer imports. The `Miracle` umbrella has only re-export declarations and no hidden module implementation dependency.

## 7.3 Header/source availability and build visibility

**ALG-MOD-002 — Consumer proof.** Configure a fresh installed package in a location without the source tree on module search paths. For each accepted public named module, build a distinct translation unit importing that module alone and using its characteristic public API; build an umbrella-only consumer. Check source-level API visibility, template instantiation on custom types, and link runtime operations where applicable. The returned module metadata must include all transitive interface partitions. This procedure distinguishes merely building the library from actually delivering importable public modules.

**INV-MOD-002 [REQUIRED].** No public API requires accidental import of unrelated umbrella modules or private source paths. A transitive interface dependency must be importable for installed consumers even if its declarations are not exported as user-facing names.

## 7.4 Error propagation and resource handling

Compiled `Message`/Fragment/Color operations are value-based, use no allocation, retain no borrowed text and throw no exceptions. Dynamic output may allocate via standard strings/streams; the project does not itself throw a new exception type, but cannot unilaterally declare STL allocation or stream operations `noexcept`. If a public helper promises status-returning nonthrowing output, it must use a nonthrowing sink contract with explicit failure returns; otherwise document the underlying C++ standard-library behavior and keep exception specifications honest. Do not misuse `noexcept` to encode a policy the implementation cannot meet.

**INV-RES-001 [REQUIRED].** No success-path diagnostic construction triggers a recoverable or unrecoverable constexpr failure. Semantic compile-time failure should identify the intended invariant, not a formatter buffer error. No buffer overrun/underflow can occur in any accepted constant expression; representational growth uses checked compile-time arithmetic.

---

# Part VIII — Failure-mode and risk analysis

The following table gives specific trigger, consequence, detection, prevention and verification relationships. Where a legacy failure is already covered by earlier contracts, Phase M must retain that behavior rather than redefine it.

| Risk ID | Trigger | Internal consequence | Observable outcome | Detection/prevention | Test |
|---|---|---|---|---|---|
| RISK-001 | `Color::hex(0x1000000)` | high bit outside RGB24 | checked `nullopt` | range check before shifts | TEST-COL-HEX-003 |
| RISK-002 | `parseHex("#12G456")` | invalid ASCII digit | checked `nullopt` | reject before accumulation | TEST-COL-PARSE-005 |
| RISK-003 | `fromHsv(NaN,...)` | undefined numeric sector | checked `nullopt` | explicit finite guard before cast | TEST-COL-HSV-004 |
| RISK-004 | hue==360 or S>1 | invalid domain | checked `nullopt` | boundary validation | TEST-COL-HSV-005 |
| RISK-005 | `Color::rgb(300,...)` implicit conversion | wraps before body | wrong color possible | resolve DEC-COL-001; brace-site test | TEST-COL-RGB-004 |
| RISK-006 | append `string_view` runtime length | return type cannot encode length | ill-formed template/API ambiguity | supported literal/array adapter only | TEST-FMT-TYPE-006 |
| RISK-007 | `Capacity+N` overflow | undersized constexpr array | invalid expression / compiler crash | compile-time checked add | TEST-FMT-OVERFLOW-001 |
| RISK-008 | interior `\0` in Fragment | C-string truncation if misused | content lost | use size-bounded view/write | TEST-FMT-BINARY-002 |
| RISK-009 | style run offset wrong | color shifts to unrelated bytes | wrong highlighted text | half-open ordered run invariants | TEST-FMT-STYLE-006 |
| RISK-010 | `help` then `field` | noncanonical layout | inconsistent diagnostics | constraints on stage transitions | TEST-FMT-ORDER-003 |
| RISK-011 | `context` repeated | duplicate logical location | inconsistent message | one context only | TEST-FMT-ORDER-005 |
| RISK-012 | escaping ANSI leaks | subsequent terminal text colored | terminal state corruption | reset after last styled run | TEST-FMT-ANSI-008 |
| RISK-013 | terminal index interpreted as RGB | wrong user palette color | incorrect visual emphasis | preserve index tag | TEST-FMT-COLOR-004 |
| RISK-014 | reflect name temporary borrowed | dangling view in Message | invalid static_assert bytes | fragment copies bytes | TEST-FMT-LIFE-003 |
| RISK-015 | std::ostream failures hidden | partial result reported as success | missing diagnostic text | sink status/stream policy | TEST-FMT-SINK-004 |
| RISK-016 | Format imports StaticString | dependency cycle | module build failure | graph invariant | TEST-MOD-DAG-001 |
| RISK-017 | Severity defined twice | distinct C++ types / ODR/visibility error | consumer compile/link failures | one owner Format | TEST-DIAG-API-003 |
| RISK-018 | Diagnostics duplicates ANSI encoder | output divergence | inconsistent compile/runtime reports | single Format encoder | TEST-DIAG-PARITY-001 |
| RISK-019 | `Iter` source factories import Core circularly | partition cycle | failed module collation | SourceViews/Core/Factories split | TEST-ITER-MOD-001 |
| RISK-020 | `enable_view` moved too late | incorrect view concept | constraint-regression | early customization reachability | TEST-ITER-RANGE-004 |
| RISK-021 | Meta Query reflection point moves | changed observed members | silent different result | declaration-point parity test | TEST-META-SNAP-003 |
| RISK-022 | Feature success path formats failures | heavy constexpr work | compile regression | compile-time branch selection | TEST-FEA-PERF-002 |
| RISK-023 | private partition imported publicly | installed BMI unavailable | consumer build failure | PUBLIC file set and install test | TEST-BLD-INSTALL-004 |
| RISK-024 | generated module not migrated | missing provider | compile graph break | generated file inventory equality | TEST-BLD-GEN-002 |
| RISK-025 | old `module Miracle.Core` on Windows | wrong module owner | Windows compile/link error | correct declaration and platform test | TEST-MOD-PLATFORM-001 |
| RISK-026 | CodeQL exclusion treated as scan success | false safety conclusion | reporting error | preserve historic extraction record | TEST-AUDIT-COVER-002 |
| RISK-027 | repeated fragment append enormous K | O(KN) compile-time cost | compile resources exhausted | bulk-compose path, benchmark | TEST-PERF-FMT-003 |
| RISK-028 | old .pcm reused after module moves | stale type identities | ODR/ABI/hard-to-debug compile | fresh build/installed prefix | TEST-BLD-CLEAN-001 |
| RISK-029 | arbitrary text includes ESC/control codes | terminal injection by caller bytes | misleading output | documented trust boundary; escape untrusted content | TEST-FMT-CTRL-002 |
| RISK-030 | `noexcept` on allocating render | terminate upon allocation failure | process termination | honest exception spec | TEST-FMT-SINK-006 |

## 8.1 Failure-class policy

**Contractually invalid program:** a template fails a required concept/constraint or deliberate `static_assert`; tests must verify the intended reason rather than any unrelated compiler error. **Recoverable invalid input:** Color parse/conversion yields optional-empty. **Runtime output failure:** follows sink/stream contract; no silently fabricated complete output. **Compiler limitation:** mark BLOCKED and distinguish from invalid C++; do not rewrite approved language semantics to appease incomplete reference compiler. **Internal invariant violation:** compile-time compile failure or explicit defensive check, not undefined behavior. **Unsupported dynamic exact-size type:** no such public overload is promised; callers must use runtime sink/event path. **Resource exhaustion:** compiler may reject huge template instantiation; record as implementation/compiler limit, not claim of unbounded finite resources.

## 8.2 Security and control-character boundary

When a source snippet, file path, template name or runtime diagnostic contains untrusted control bytes (e.g. newline, ESC or terminal carriage return), the renderer could produce a misleading display if it emits text verbatim. A formatter that promises arbitrary text formatting cannot silently reinterpret or sanitize bytes; treating untrusted diagnostic text as terminal-safe is a **consumer/presentation decision**, not a guaranteed property of `Message`. The proposed safe policy is: *domain-specific Diagnostics renderer sanitizes or escapes disallowed terminal control characters before emitting them through Format; the core Format engine does not treat caller-provided ANSI as style metadata.* This is a recommended mitigation requiring explicit documented policy to become a mandatory safety guarantee. Test malicious input as bytes, not only as visible Unicode rendering.

## 8.3 Regression and compatibility distinctions

Changing source suffixes, module ownership and BMI outputs is a build/ABI compatibility break and should force rebuild; it does not authorize source API signature changes, different exception guarantees, changed reference categories or different reflection semantics. A direct `import Miracle.Meta;` client must remain valid. A direct legacy partition import is not a promised public client API; user code importing `Miracle` umbrella is. A new Color/Format direct-import user has no historical behavior, so tests establish its newly approved contract only after ratification.


---

# Part IX — Author-owned implementation blueprint, M.1–M.8

This plan specifies concrete files, dependencies, quality gates and technical techniques while preserving implementation ownership. The milestones are independently committable. As explicitly agreed, **intermediate architectural reviews may be batched after M.8**, but the author must retain sufficient evidence and checkpoint boundaries to evaluate every milestone distinctly. An earlier commit is not proof of acceptance.

## 9.1 Common preparation and workspace constraints

Create an implementation branch from the authoritative checkout; record original SHA and working-tree status. Prefer one commit per milestone (additional focused fix commits permissible). Preserve historical frozen contracts and keep owner-approved Design Contract separate from this proposed technical guide. A new `docs/phases/M/PROGRESS.md` should track source revision, current milestone, implementation status, verification status, open findings, compiler identity, and next action; do not use conversational history as the progress ledger. Before moving a module, obtain a complete build/test snapshot with the pinned compiler, configured feature set and current test counts. Keep a manifest of source files and their target module identities, updated after each milestone. Do not work against stale BMI folders; use a fresh build after a named-module ownership change.

### Source change guardrails

- Source modifications stay the owner's responsibility. This document supplies focused sketches, not a production patch.
- Local refactoring freedoms include variable naming, helper layout, file ordering and equivalent algorithmic implementation. The fixed boundaries—module identity, public API, original semantics, presentation grammar, Color/Format dependency direction—are not implementation freedoms.
- Do not mix an unrelated Core Revisit redesign into a structural migration commit. A semantic change encountered during migration should be recorded separately and explicitly approved.
- Use source-aware `git mv` where moving whole files; for decomposed interfaces, use separate focused commits to preserve attribution and simplify bisecting.
- Every milestone commit message should identify the module/architecture area; an example convention is `refactor(module): ...`, `feat(format): ...`, `test(module): ...`, aligned with the project's own style.

## 9.2 M.1 — Baseline, contract reconciliation and dependency graph

**Entry:** Current authoritative checkout, historic Phase M contract available for provenance, current Development Protocol, compiler snapshot policy, and existing CI/test presets. **Status at inspected remote:** partial audit exists; implementation already advanced beyond historic M.1 without a completed measured baseline record. **No inference of acceptance.**

### Required engineering work

1. Record `git rev-parse HEAD`, `git status --short`, branch, version, CMake preset, system ABI target and compiler manifest. The remote baseline cited above is a cross-check; use local authoritative revision for implementation evidence.
2. Inventory all `.cppm`, `.cppm.in`, `.cpp`, optional feature sources, Meta partitions, platform implementations, tests/examples/benchmarks, and generated CMake outputs. Determine exactly which names are exported primary, exported partitions, internal partitions, or implementations.
3. Parse source imports and CMake source-list ownership to construct the current dependency DAG; record provider counts, graph topological order and transitive reachability. Include `import std;` as external dependency, but do not pretend to control std BMI ownership.
4. Compare exported API symbols from the current checkout against earlier accepted public contracts (Foundation, Phase 5 Meta, Phase 5.4/5.6 where authoritative). Differences require explanation, not silent preservation of an accidental bug or accidental override of a frozen contract.
5. Recover provenance of `docs/phases/M/DESIGN.md` from Git history. The file existed in commit `469083e...` but is absent in the inspected current master after a docs cleanup. Decide explicitly whether to restore it as the current phase's binding contract or maintain a new approved successor. Add an ADR for the novel Color→Format dependency if the restored contract formerly barred all Miracle imports from Format.
6. Measure (or recover from authoritative CI/benchmark records) representative cold and incremental build time, peak RSS/BMI count/size, and import-only consumer times under a fully described toolchain. Use repeated runs and record raw data, not only summary.
7. Record all pre-existing source-level anomalies, particularly `src/private/windows/Memory.cpp` declaring `module Miracle.Core;` and any missing exported-import visibility for Numeric aliases through Types.

### Technical shell sketch

```sh
# ILLUSTRATIVE commands; adapt to current checkout and pinned toolchain.
git rev-parse HEAD
git status --short
git ls-files 'src/**/*.cppm' 'src/**/*.cpp' 'src/**/*.cppm.in'
grep -R -n -E '^(export )?module |^(export )?import ' src/public src/private
cmake --preset tests --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset tests --parallel 2
ctest --preset tests --output-on-failure
```

The `grep` is an initial audit only; the definitive import graph must include CMake scan/collation outputs because C++ module grammar and generated imports can exceed regex-based detection. Do not run `cmake --preset tests` with the host CMake 3.31 expecting a valid reference result.

### Exit evidence

`M1-SOURCE-INVENTORY.json`; `M1-DEPENDENCIES.md` or DOT graph; `M1-API-BASELINE.md`; `M1-BUILD-BASELINE.csv` with exact flags and raw durations; source revision and compiler manifest; list of open approval-dependent design changes. A topologically sorted graph with no missing providers is the acceptance target. If the historical pre-rename performance baseline was not captured before work began, clearly mark it **UNRECOVERABLE** unless valid prior measurements exist; do not fabricate a before/after comparison. This is a verification issue requiring explicit owner risk disposition.

## 9.3 M.2 — Source suffix, templates and tooling migration

**Entry:** M.1 source manifest and verified current compiler/CI configuration. **Current remote status:** `.cppm`/`.cpp` migration substantially exists, but full integration is not assumed verified. **Primary risk:** scripts and generated modules silently missed during rename.

### Exact change inventory

- Convert all importable interfaces and partitions `.ixx` → `.cppm`; all ordinary/module implementations, tests, examples, benchmark sources and compiler probes `.cxx` → `.cpp` unless a verified compiler-specific exception is recorded.
- Convert generated `src/public/build/{Build,Capabilities,FeatureConfiguration}.cppm.in`, and configured output paths; ensure generated directories are created before `configure_file` and registered module outputs appear in public module file sets.
- Update CMakeLists, nested CMake tests, feature-registration `SOURCES`, source glob assumptions, Docker scripts and compiler capability checks. Preserve `import std;` and capability detection semantics.
- Update benchmark generation `.cpp.in` and generated test sources, scripts parsing compile commands, docs examples, formatting/lint selection and Neovim/clangd test paths.
- Update CodeQL coverage inventory scanning: recognize `.cpp` implementation units and `.cppm` module interfaces, including `.cppm` partitions, rather than reporting a false increase/decrease in scanned files. Keep historical extraction evidence separate from new scan evidence.
- Ensure `docs/SUMMARY.md` is updated whenever new docs chapters appear, because the existing mdBook navigation verifier fails on unlisted pages. A documentation-only commit previously failed CI with `Not linked in SUMMARY.md`—a concrete historical integration risk.

### Verification and rollback

For each category, compare tracked-file counts before/after and require no unexpected omission. Build the three CMake consumers, run tests and check generated module source names. Before changing named-module ownership, retain the old declarations; M.2 should be a file extension/toolchain change only, so a failure can be isolated from C++ semantics. Acceptance requires a clean build and package consumer on the new suffix layout. A break only in a portability branch is separately reported, not allowed to silently dictate master’s feature baseline.

**Exit evidence:** rename manifest; tooling path matrix; CMake generated-output inventory; successful CI job/run IDs; source-build and installed-consumer results; documented CodeQL coverage status. **Non-goal:** changing `Miracle:Iter` to `Miracle.Iter` during this milestone.

## 9.4 M.3 — Foundational extraction: Numeric, Types, Color, Format, StaticString

**Entry:** M.2 source mapping stable, ratified exact Color/Format API, reference compiler available, no unresolved graph cycle. **Current remote:** Numeric/Types/StaticString already extracted; Color/Format not yet present. **Primary risk:** introducing a high-level dependency cycle through foundational diagnostics.

### Numeric and Types

`Miracle.Numeric` owns exactly `u8,u16,u32,u64,i8,i16,i32,i64,f32,f64,usize,isize` as standard-backed aliases. Current `src/public/numeric/Numeric.cppm` already matches this conceptual inventory. `Miracle.Types` owns the remaining std-backed aliases as currently defined. Verify direct `import Miracle.Numeric;` and `import Miracle.Types;` consumers compile with expected aliases and that umbrella consumers see the same names; no forward declaration of a std-backed alias should create a new type identity. Do not modify project style to replace every `usize` with `size_t`; preserve public API spelling.

### Color implementation order

1. Create `src/public/color/Color.cppm` as an independent primary interface, importing only standard facilities.
2. Implement `Color::rgb` and `toHex` with trivially copyable storage and compile-time tests. Resolve typed-channel conversion policy before claiming safe invalid-input behavior.
3. Implement checked `hex(u32)` and `parseHex(string_view)` with explicit ASCII decoding; verify short and long forms, invalid cases and no truncation.
4. Implement `fromHsv(Hsv)` and `toHsv()` using the specified sectors, finite checks and rounding. Verify primaries, grayscale, boundaries, interior hues, stable canonical hue for achromatic colors, and random property tests.
5. Add direct module import consumer; check no Miracle imports; capture exported API declarations in the API matrix.

### Format implementation order

1. Establish single owner `Miracle::Severity`, `ColorMode`, `Presentation` with source-compatible names and enumerators. During transition, avoid defining these enums simultaneously in Diagnostics and Format; they may temporarily be referenced by imported declarations but never duplicated.
2. Implement `TerminalColor` and `Style` as small value types, distinguishing indexed palette selection from true-color RGB. Include constexpr equality and default initialization semantics.
3. Implement sized owning `Fragment<N>` and CTAD/helper factories. Test copying from string literal and `std::array` with embedded NUL, zero length and Styled flags.
4. Implement owned contiguous `Message<C,R,D,S>` storage with checkable invariants. Begin with ordinary `Message{"literal"}` and `append(Fragment)`; verify correct type growth and borrowed `view()` lifetime. Add decimal append only after signed-minimum correctness is established.
5. Implement diagnostic constructor and typed `Code` width model; ensure capacity inference works without unbounded template depth or user guesses. Add context, field, note and help section methods with compile-time ordering constraints. Ensure a diagnostic body is never blank due to builder overrun.
6. Implement one renderer core for plain and ANSI using local style state and a statically dispatched sink. Verify color foreground/background for default/indexed/RGB, attributes, resets and concatenated-text equality.
7. Implement a minimal runtime event interface or equivalent exportable presentation primitive for Diagnostics. Do not make Format import Diagnostics. Prove a fake Diagnostics adapter can send runtime-sized text without allocating guessed static buffers.
8. Write negative compile tests for unavailable methods in wrong section order; write one generated `static_assert` probe using the actual reference compiler.
9. Record API signature verification results (PASS/FAIL/NOT RUN/BLOCKED) for every exported type/factory and at least one installed direct-import test. If a signature fails C++26, fix the sketch and document the reason rather than introducing an undocumented implementation workaround.

### StaticString integration

`BasicStaticString` and `staticFormat` remain in their already extracted module. Only after the low-level Format tests pass, migrate selected authored hard assertions to Format-based messages. Keep local bootstrap assertions simple where dependency ordering or language constraints make full formatting inappropriate. `StaticString` must never import Diagnostics. Test false/true constraint behavior, compile-fail text, and unchanged NTTP structural types.

**Exit evidence:** complete Color/Format API matrix; Color/Format golden output fixtures; compiler-verified import + constexpr API probes; StaticString regression results; zero direct Format imports beyond std/Color; one owned Severity type; no changes to existing semantic predicates. **Ratification prerequisite:** exact public signatures and unresolved `DEC-COL-*` / `DEC-FMT-*` decisions.

## 9.5 M.4 — Diagnostics extraction and renderer unification

**Entry:** Format/Color primitives and render sink verified; existing Diagnostics tests and compile-fail fixtures captured; ownership conversion path planned. **Primary risks:** duplicate enums, different runtime rendering order, source-span styling regressions and imported interface reachability.

### Implementation sequence

1. Move `src/public/Diagnostic.cppm` into `src/public/diagnostics/Diagnostics.cppm`, with `export module Miracle.Diagnostics;`. Preserve namespace and existing user APIs. Avoid renaming public `Miracle::Diagnostic` to plural.
2. Make Diagnostics import `Miracle.Format` and required lower-level modules; remove local Severity, ColorMode, Presentation declarations after verifying their single definitions remain visible via correct re-export strategy. Keep `SourceMode`, `DetailMode`, and current `RenderOptions` as Diagnostics semantic policy.
3. Update `src/private/Diagnostic.cpp` to `module Miracle.Diagnostics;`, adjusting all imports. **Do not** let it retain `module Miracle;` while the public interface moves ownership.
4. Extract the ANSI encoding and severity/code header formatting from the old `DiagnosticRenderer`. Use Format's `writeFragment`/`writeHeader` or the approved equivalent shared primitives. Remove duplicate implementations, not merely wrap them.
5. Adapt runtime `DiagnosticCodeDescriptor` to normalized code formatting without copying/owning duplicate registries. Preserve reflective prefix, annotations, alignment, enum alias message selection, and numeric width behavior.
6. Preserve span/snippet/path/trace/cause handling in Diagnostics. For each text span, pass a style and plain bytes to Format. Retain existing half-open selection clamping and source path relativization.
7. Compare captured plain and terminal golden outputs against the intended shared grammar; deliberate formatting changes must be documented separately from semantic preservation. Validate `Presentation::Plain` always suppresses ANSI, even when `ColorMode::Always` is selected.
8. Add installed consumer that imports only `Miracle.Diagnostics` and constructs a real `Diagnostic` with code, span, note, help and cause; build/render it. Add cross-module color and severity compile-time identity checks.

### Specific negative cases

A broken `diagnostics::prefix` annotation produces an intended compile failure; nonexistent diagnostic enum domain must remain outside the `DiagnosticCode` concept. Source snippet with no selection, empty snippet, large offsets, negative-underlying enumerator, duplicate message annotation and zero width should match existing semantics. Deep cause trees must not overflow call stack if existing traversal was iterative. Console style must reset after each diagnostic tree.

**Exit evidence:** source/ABI identity report; no duplicate Severity; exact renderer-output snapshot changes explained; import and installed-consumer results; all runtime and compile-time Diagnostics tests; no Format → Diagnostics edge; old `Diagnostic.cpp` module ownership removed.

## 9.6 M.5 — Remaining named-module extractions

**Entry:** verified foundation, clean Diagnostics dependency, target graph and current owner map. **Primary risk:** moving template declarations into incorrectly visible implementation units.

### Dependency-guided order

1. `Miracle.Range` (already extracted at inspected revision), then `Miracle.Iter` after Range/Types and Format are stable.
2. `Miracle.Feature` (already extracted) with generated `Miracle.Capabilities` and `Miracle.FeatureConfiguration` as separate named modules; verify feature option gating and resolution.
3. `Miracle.Concepts`, `Miracle.Memory` (standalone, platform unit), then `Miracle.Error`, `Miracle.Panic`, `Miracle.Fs` after Diagnostics.
4. `Miracle.Debug` and `Miracle.Bitflags` after independent `Miracle.Meta` and their actual dependencies are available; do not pull Meta into Format/Color.
5. `Miracle.Profiling` after Debug only if necessary.
6. Update all `.cpp` implementation `module` declarations and corresponding imports. Re-run source inventory and CMake provider scan.

### Per-component checklist

For each component: declare exactly one primary `export module Miracle.Component;`; transfer declarations from old `Miracle:Component`, remove old owner; import actual dependencies only; update CMake fileset; use existing tests with direct import; build an external consumer; compare public declarations and constraints; check runtime linkage for non-template methods; inspect exported-source and compiled BMI boundary. If an API type is still owned by another named module, test that its exported declaration is actually reachable without umbrella import.

**Exit evidence:** complete target module inventory, no legacy public partitions (except Meta's own proper partitions), no `module Miracle;` runtime implementation units for extracted components, source compatibility and consumer tests for each, compiled per-platform units or documented untested platforms.

## 9.7 M.6 — Cohesive decomposition of oversized units

**Entry:** named ownership correct and direct imports working. **Primary risk:** instantiation reachability and inadvertently changing reflection observations/algorithm order.

### Iter

Follow §6.4: Detail/SourceViews/Views/Core/SourceFactories with acyclic imports; preserve all forward declarations, std::ranges variable-template specializations, view contracts, referenced helper definitions and method templates. Do **not** attempt to move a class member declaration alone to another partition. If a proposed `Consumers` split would create a cycle or make the out-of-class definition invisible, retain it in Core.

### Feature

At observed revision Feature is already decomposed into Types/Descriptor/Set/Catalog/Requirement. Use this milestone to audit rather than re-split. Confirm no source-symbol ownership duplication, and direct `Miracle.Feature` consumer sees all expected APIs. Any optimization to the resolver should be deferred unless required to repair an identified migration regression.

### Meta Query

Evaluate the optional split described in §6.9. A decision *not* to split Query is legitimate if the correct implementation would require fragile reflection observation changes or negligible modular benefit. If split, independently verify value-IR, state capture, tuple/aggregate invocation, compiled program slots, lazy evaluation, reference-materialization barriers, end-of-scope behavior, and cross-TU visibility. Do not move hard static storage promotions or observation-sensitive declarations without measurement.

### Format

A separate `Message` and `Render` partition is appropriate only if it gives an acyclic boundary and reduces rebuild dependencies while keeping all consumer-instantiated templates reachable. A compact single `Format.cppm` is acceptable; splitting for aesthetics does not count as architecture improvement.

**Exit evidence:** partition graph and source mapping; direct/imported consumer tests; exact template instantiation and constexpr stress tests; before/after build critical-path measurements. No mandatory line-count threshold.

## 9.8 M.7 — Umbrella, CMake package and ecosystem integration

**Entry:** every accepted named module directly imports and linked source units compile. **Primary risk:** umbrella appears correct but installed package lacks partitions or feature-gated modules.

### Required umbrella shape

```cpp
// PROPOSED representative; exact re-export list equals accepted public inventory.
export module Miracle;
export import Miracle.Numeric;
export import Miracle.Color;
export import Miracle.Format;
export import Miracle.Types;
export import Miracle.StaticString;
export import Miracle.Range;
export import Miracle.Iter;
export import Miracle.Diagnostics;
export import Miracle.Feature;
export import Miracle.Error;
export import Miracle.Panic;
export import Miracle.Fs;
export import Miracle.Meta;
export import Miracle.Bitflags;
export import Miracle.Debug;
export import Miracle.Build;
export import Miracle.Capabilities;
export import Miracle.FeatureConfiguration;
export import Miracle.Concepts;
export import Miracle.Profiling;
export import Miracle.Memory;
```

`export import` conveys the public module's declarations; it doesn't add another definition/owner. The umbrella must not import an unexported incompatible partition to make missing declarations reachable. Verify FeatureConfiguration is exported only where build-selection policy requires it, preserving currently visible user semantics. `Miracle::Miracle` remains the CMake target; package version comes from approved release policy and must not automatically be bumped as an incidental migration step.

### Consumer matrix

**Source-subdirectory:** configure consumer with local source tree via `add_subdirectory`; build direct and umbrella imports. **FetchContent:** use the same repository checkout without assuming extra network availability; validate exact target reuse. **Installed:** perform clean Release install into isolated prefix; consumer uses `find_package(Miracle CONFIG REQUIRED)` and target link; import each named module and instantiate a representative template. **Feature options:** repeat with default and disabled optional features; missing heavy feature source must not break formatting diagnostic imports. **Editor:** run clangd/Neovim smoke against final compile database. **Documentation:** mdBook navigation `docs/SUMMARY.md` must list newly added tracked chapters or the CI verifier will fail.

**Exit evidence:** all three consumers, full umbrella/direct-import list, exported module metadata, successful package rebuild, source inventory matching module provider list and configured optional features.

## 9.9 M.8 — Integrated verification, final engineering review and acceptance

**Entry:** implementation-complete code with separate milestone commits and reproducible evidence. **Primary risk:** interpreting a green main CI job as full architecture correctness, especially unsupported C++26 CodeQL extraction.

### Required activities

1. Run pinned reference toolchain Debug/Release, warnings-as-errors, tests and compile-fail fixtures; record raw job IDs/commands and counts. Repeat with fresh BMI/build paths to detect stale artifacts.
2. Run code-style/static checks and module import graph analyzer. Compare effective imports to the approved dependency matrix and ensure no Format↔Diagnostics, Color↔Format or Iter internal partition cycles.
3. Execute all Color conversion and Format fragment/ANSI/style tests; evaluate exact inferred-capacity and C++26 generated `static_assert` cases, including bad type names and long fragments.
4. Run all old subsystem behavioral tests plus direct-import consumers and installed package matrix; compare API snapshots and runtime diagnostics semantics.
5. Run benchmark suite (cold full build, incremental public/private edits, import-only consumers, Meta Query compile stress, Feature/Iter constexpr cases, long diagnostic messages) using same pinned toolchain/hardware/configuration as baseline. Report median and range, not just one timing; identify regressions and plausible causes without inventing thresholds.
6. Check source/CodeQL extraction coverage explicitly. `.cppm` rename may improve recognition but does not prove C++26 semantic analysis. Note unsupported CodeQL module coverage as a known limitation unless a new official supported run provides evidence.
7. Review every milestone's commit range against its exit condition. Consolidate findings under Development Protocol classifications: Blocker/Major/Minor/Observation and outcomes Accepted/Changes requested/Verification pending/Contract revision required.
8. Prepare `docs/phases/M/ACCEPTANCE.md` referencing exact revision, compiler manifest, test results, performance data and explicit waivers. Mark phase Accepted only after author sign-off; no automatic release/tag.

**Exit evidence:** completed verification matrix, requirement traceability, source/contract alignment, accepted resolutions of Blocker/Major findings, clear untested-platform notes, direct author acceptance. A review performed on one head revision does not automatically cover later commits; rerun affected gates or record a bounded review delta.

## 9.10 Batched-review artifact package

The implementing engineer should provide, at final handoff, a reproducible archive or Git range containing: `base_sha`, `head_sha`, ordered milestone commits and their intent; authoritative Design Contract revision; source/dependency inventory; build scripts and compiler manifests; all test commands/result logs; compile-fail snapshot normalization method; benchmark raw CSV + environment; installed-consumer proof; deviations and amended ADRs; known compiler limitations; CodeQL extraction audit; explicit acceptance request. The reviewer should inspect source rather than trusting an automatically generated checklist. A full test suite may be run once at M.8, but high-risk changes should receive targeted local testing while being implemented so failures remain attributable.


---

# Part X — Complete verification and validation specification

## 10.1 Verification environment and status discipline

A verification record contains: test ID; exact API or invariant; compiler executable and manifest; stdlib/ABI revision; language mode; relevant compiler/linker flags; target/OS; test source content/path; build command; expected outcome; actual outcome; logs/exit status; verdict PASS/FAIL/NOT RUN/BLOCKED; scope limitations. **NOT RUN is not equivalent to FAIL.** A GitHub workflow marked success validates only its actual committed source and configured jobs. New Color/Format APIs did not exist in the inspected baseline and therefore were not compiled by that workflow.

**Observed baseline evidence:** GitHub Actions CI run `40987672889` for commit `e2da473e6dd5637f1f70625738f8378e211a7478` completed successfully with job `Reference C++26 (cxx26-2026.10.27)`. This confirms the repository's configured reference CI at that commit, **not** future Format/Color API conformance, unrelated platforms, or milestone acceptance. CI link: `https://github.com/spwn02/Miracle/actions/runs/40987672889`.

**Host probe P-HOST-001:** On this analysis host, Clang 17.0.0 in `-std=c++23 -Wall -Wextra -Wpedantic -Werror` compiled, linked and executed `miracle_phase_m_evidence/format_core_host_probe.cpp` with process exit 0. It verifies only a miniature ordinary literal-to-array append pipeline and style-run offsets, **not** the intended Format module, C++26 `static_assert`, CMake integration or any actual Miracle source. Reference-toolchain status for this probe: **NOT RUN**.

**REQ-VER-001 [REQUIRED].** Every API classified `PROPOSED` must have a reference-toolchain compile/consumer test before it can be called VERIFIED and before final signature ratification. A status matrix is required even for tests that cannot currently run.

### Reference build commands (not executed here)

```sh
# PROPOSED reference procedure, to run in the author's properly activated
# clang-cxx26-2026.10.27 environment with coherent matching libc++.
source "$CXX26_TOOLCHAIN_ROOT/share/clang-cxx26/activate.sh"
"$CXX" --version
cmake --version
ninja --version
cmake --preset tests --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset tests --parallel 2
ctest --preset tests --output-on-failure
cmake --preset release --fresh -DCMAKE_TOOLCHAIN_FILE="$CXX26_CMAKE_TOOLCHAIN_FILE"
cmake --build --preset release --parallel 2
```

Record the entire toolchain manifest. The exact source revision must be captured; do not point to mutable `cxx26` HEAD or ambiguous `latest` releases. Compiler options such as `-stdlib=libc++`, std module scanning and linker selection should come from validated project toolchain configuration, not separately guessed flags.

## 10.2 New Color test matrix

Each scenario is a concrete fixture with a precise expected result. `constexpr` and runtime variants should be run where applicable; assertions against Color contents must inspect raw encoded channels. The IDs are stable intended test names, not claims that corresponding files presently exist.

| Test ID | Requirements | Exact scenario | Expected result / verification level |
|---|---|---|---|
| TEST-COL-RGB-001 | API-COL-001 | `Color{0,0,0}` | channels exactly 0; `toHex()==0`; compile-time PASS expected |
| TEST-COL-RGB-002 | API-COL-001 | `Color::rgb(255,255,255)` | channels each 255; `toHex()==0xFFFFFF`; constexpr |
| TEST-COL-RGB-003 | API-COL-001 | `Color::rgb(12,34,56)` | red 12, green 34, blue 56, copy/equality stable; constexpr |
| TEST-COL-RGB-004 | DEC-COL-001 | `Color::rgb(300,0,0)` | **decision-dependent**: rejected/checked; must not silently claim invalid-input protection for current uint8 signature; API review gate |
| TEST-COL-TYPE-001 | API-COL-001 | `is_trivially_copyable_v<Color>` | true on reference compiler; constexpr compile test |
| TEST-COL-TYPE-002 | API-COL-001 | structural template argument `template<Color C>` | compiles with `Color{1,2,3}` if intended structural guarantee ratified |
| TEST-COL-HEX-001 | API-COL-002 | `Color::hex(0x123456)` | value 0x12/0x34/0x56; `toHex()==0x123456` |
| TEST-COL-HEX-002 | API-COL-002 | `Color::hex(0)` | black; has_value true |
| TEST-COL-HEX-003 | API-COL-002 | `Color::hex(0x1000000)` | nullopt; never discards high bits |
| TEST-COL-HEX-004 | API-COL-002 | `Color::hex(0xFFFFFFFF)` | nullopt; no truncation |
| TEST-COL-PARSE-001 | API-COL-003 | `parseHex("#FF0000")` | red=255, green=blue=0 |
| TEST-COL-PARSE-002 | API-COL-003 | `parseHex("#fF9aC0")` | 255,154,192; case-insensitive hex |
| TEST-COL-PARSE-003 | API-COL-003 | `parseHex("#abc")` | 0xAA,0xBB,0xCC; nibble replication |
| TEST-COL-PARSE-004 | API-COL-003 | `parseHex("#000")`, `"#FFF"` | black, white respectively |
| TEST-COL-PARSE-005 | API-COL-003 | `parseHex("#12G456")` | nullopt for nonhex G |
| TEST-COL-PARSE-006 | API-COL-003 | `parseHex("123456")` | nullopt, required leading `#` |
| TEST-COL-PARSE-007 | API-COL-003 | `parseHex("")`, `"#"`, `"#12"`, `"#12345"`, `"#1234567"` | all nullopt for unsupported lengths |
| TEST-COL-PARSE-008 | API-COL-003 | `parseHex("#12\0ABC"sv)` with explicit length 7 | nullopt, NUL is not valid hex digit |
| TEST-COL-PARSE-009 | API-COL-003 | `parseHex(" #ABC")` | nullopt, no whitespace trimming |
| TEST-COL-HSV-001 | API-COL-004 | fromHsv({0,1,1}) | red 255/0/0; constexpr |
| TEST-COL-HSV-002 | API-COL-004 | fromHsv({120,1,1}) | green 0/255/0 |
| TEST-COL-HSV-003 | API-COL-004 | fromHsv({240,1,1}) | blue 0/0/255 |
| TEST-COL-HSV-004 | API-COL-004 | NaN or infinity in each HSV field | nullopt, no float-to-int cast attempted |
| TEST-COL-HSV-005 | API-COL-004 | H=-0.01, H=360, S=-.01, S=1.01, V=-.01, V=1.01 | each nullopt as invalid domain |
| TEST-COL-HSV-006 | API-COL-004 | H=0, S=0, V=0.5 | RGB grayscale 128,128,128 using half-up quantizer |
| TEST-COL-HSV-007 | API-COL-004 | H=60,180,300 with S=1,V=1 | yellow/cyan/magenta primaries |
| TEST-COL-HSV-008 | ALG-COL-002 | each sector boundary ±small epsilon within range | continuous channel behavior, no sector overflow |
| TEST-COL-HSV-009 | ALG-COL-002 | S=0, V=0 and S=0,V=1 | exact black and white |
| TEST-COL-HSV-010 | ALG-COL-002 | H=359.999999,S=1,V=1 | valid near-red, no invalid sector index |
| TEST-COL-REV-001 | API-COL-005 | `Color::rgb(0,0,0).toHsv()` | H=0,S=0,V=0 |
| TEST-COL-REV-002 | API-COL-005 | `Color::rgb(255,255,255).toHsv()` | H=0,S=0,V=1 |
| TEST-COL-REV-003 | API-COL-005 | RGB primaries to HSV | H=0,120,240 respectively; S=1,V=1 |
| TEST-COL-REV-004 | API-COL-005 | equal-channel grayscale 127/127/127 | H=0,S=0,V=127/255 within float tolerance |
| TEST-COL-PROP-001 | ALG-COL-003 | 10,000 deterministic pseudorandom RGB8 samples, seed recorded | `fromHsv(c.toHsv())` yields channels within agreed rounding bound; track maxima, not unapproved PASS |
| TEST-COL-PROP-002 | ALG-COL-001 | 256×256 selected boundaries and random colors | hex parse/toHex invariant for `#RRGGBB` generated from packed bytes |
| TEST-COL-IMPORT-001 | INV-ARCH-003 | consumer `import Miracle.Color;` only | Color/Hsv compile without Numeric, Format, StaticString or umbrella imports |

**Property strategy:** use deterministic seeds; cover every channel boundary 0,1,127,128,254,255; use uniform random RGB interior samples and adversarial equal/tied channels. Shrink any mismatch to a minimal triplet and report both original and transformed values. No property test substitutes for the exact domain validity proofs above.

## 10.3 New Format API, correctness and style tests

| Test ID | Requirements/invariants | Scenario | Exact property / level |
|---|---|---|---|
| TEST-FMT-TYPE-001 | API-FMT-005 | `fragment("abc")` | owns 3 bytes, size 3, view `abc`, default style |
| TEST-FMT-TYPE-002 | API-FMT-005 | `fragment("")` | owns zero visible bytes; view empty |
| TEST-FMT-TYPE-003 | API-FMT-006 | `styled("x", redBold)` | text remains `x`, style metadata exactly redBold |
| TEST-FMT-TYPE-004 | INV-MSG-001 | `Message{"abc"}` | size 3, `data()[3]=='\0'`, view `abc` |
| TEST-FMT-TYPE-005 | INV-MSG-003 | `auto b=a.append("x")` | a remains `abc`, b becomes `abcx`, independent lifetime |
| TEST-FMT-TYPE-006 | §3.2 | attempt exact-sized `fromStringView(dynamic)` | no misleading compile-time inferred-length API participates |
| TEST-FMT-OWN-001 | REQ-LIFE-001 | create Message from local array, return by value | returned Message owns exact bytes after local array lifetime ends |
| TEST-FMT-OWN-002 | REQ-LIFE-002 | copy/move Message then inspect view | surviving object's data/view refers to own array, no self pointer |
| TEST-FMT-LIFE-003 | REQ-LIFE-001 | reflection name adapter copies locally constexpr view to owned Fragment | static assertion characters remain constant-evaluation valid |
| TEST-FMT-BINARY-001 | INV-MSG-001 | zero-length `std::array<char,0>` Fragment | append is no-op on visible bytes, no phantom style run |
| TEST-FMT-BINARY-002 | INV-MSG-001 | `array{'a','\0','b'}` Fragment | Message size==3; view byte1 NUL; renderer uses explicit length, not `strlen` |
| TEST-FMT-NUM-001 | ALG-FMT-003 | decimal append 0 | exactly character `0` |
| TEST-FMT-NUM-002 | ALG-FMT-003 | decimal append uint64 maximum | exactly 18446744073709551615; no truncation |
| TEST-FMT-NUM-003 | ALG-FMT-003 | decimal append int64 minimum | exactly -9223372036854775808; no signed overflow |
| TEST-FMT-NUM-004 | ALG-FMT-003 | `char` and `bool` append | prohibited/explicit, not surprising implicit numeric overload |
| TEST-FMT-CODE-001 | API-FMT-007 | `code<3>("ITR",4)` heading | text contains `[ITR004]` |
| TEST-FMT-CODE-002 | API-FMT-007 | `code<5>("CTR",7)` | text `[CTR00007]` |
| TEST-FMT-CODE-003 | API-FMT-007 | `code<2>("E",12345)` | text `[E12345]`; width never truncates |
| TEST-FMT-CODE-004 | API-FMT-007 | empty prefix code | no brackets, header `error: summary` |
| TEST-FMT-CODE-005 | API-FMT-007 | width 0 with number 0 | code has one digit `0`, no empty number |
| TEST-FMT-CODE-006 | ALG-FMT-003 | runtime code number/width parity | exact same plain header bytes as compile-time case |
| TEST-FMT-HEAD-001 | API-FMT-009 | construct Error diagnostic with code/summary | one header; lowercase `error`, code and colon; exactly one final newline |
| TEST-FMT-HEAD-002 | API-FMT-009 | each of four Severity values | exact `debug/note/warning/error` spelling, no second Level type |
| TEST-FMT-ORDER-001 | INV-SEC-001 | heading→context→field→note→help | exact section separators, stable order |
| TEST-FMT-ORDER-002 | INV-SEC-001 | heading→note with no context/field | no extra blank lines for omitted sections |
| TEST-FMT-ORDER-003 | INV-SEC-001 | help then field | `field` overload does not participate (compile-fail for intended constraint) |
| TEST-FMT-ORDER-004 | INV-SEC-001 | note then context | `context` rejects backwards section transition |
| TEST-FMT-ORDER-005 | INV-SEC-001 | context twice | second context rejected at compile time |
| TEST-FMT-ORDER-006 | INV-SEC-001 | two consecutive notes, two helps | insertion order, no unwanted blank line between same-section entries |
| TEST-FMT-ORDER-007 | INV-SEC-001 | field key empty literal | rejected or documented; no `  : value` malformed output |
| TEST-FMT-ORDER-008 | INV-SEC-001 | diagnostic Message `.append(...)` | unavailable to public user; grammar not bypassable |
| TEST-FMT-STYLE-001 | INV-MSG-002 | styled fragment then unstyled | exact styled range excludes subsequent text |
| TEST-FMT-STYLE-002 | INV-MSG-002 | adjacent equal-style fragments | canonical coalescing or documented two-run equivalence; output same |
| TEST-FMT-STYLE-003 | INV-MSG-002 | styled empty fragment | no zero-length style record |
| TEST-FMT-STYLE-004 | INV-MSG-002 | multiple styled fields | offsets correctly include punctuation/newlines |
| TEST-FMT-STYLE-005 | API-FMT-004 | default Style and default TerminalColor | no ANSI on terminal even in Always mode |
| TEST-FMT-STYLE-006 | INV-MSG-002 | insert 20 runs alternating styles | every run in ascending nonoverlapping byte intervals, correct colored spans |
| TEST-FMT-COLOR-001 | API-FMT-003 | `indexed(1)` | preserves index 1, not auto converted to Color(255,0,0) |
| TEST-FMT-COLOR-002 | API-FMT-003 | `trueColor(Color{255,159,67})` | exact RGB retained, no indexed interpretation |
| TEST-FMT-COLOR-003 | API-FMT-003 | foreground and background simultaneously | emits 38;2 and 48;2 with correct channel ordering |
| TEST-FMT-COLOR-004 | INV-ANSI-002 | 0..255 indexed selection | emits 38;5;i / 48;5;i and does not assume palette RGB |
| TEST-FMT-ANSI-001 | INV-ANSI-001 | Plain render of heavily styled message | output == view() byte-for-byte, zero generated ESC |
| TEST-FMT-ANSI-002 | INV-ANSI-001 | Terminal/Never | output == view(), zero generated ESC |
| TEST-FMT-ANSI-003 | API-FMT-014 | Terminal/Always, red indexed bold | exact SGR color/attribute components, followed by text and reset |
| TEST-FMT-ANSI-004 | API-FMT-014 | Terminal/Always, RGB24 fg | contains `\x1b[38;2;255;159;67m` or agreed canonical equivalent |
| TEST-FMT-ANSI-005 | API-FMT-014 | Terminal/Always, RGB24 bg | contains `\x1b[48;2;...m` with exact channels |
| TEST-FMT-ANSI-006 | API-FMT-014 | italic, dim, underline separately | SGR 3,2,4 present, reset consistent |
| TEST-FMT-ANSI-007 | API-FMT-014 | alternating bold/plain/bold fragments | styles do not leak into plain middle region |
| TEST-FMT-ANSI-008 | INV-ANSI-001 | last run styled, output following another message | first output terminates reset; next begins default state |
| TEST-FMT-ANSI-009 | INV-ANSI-001 | empty message with style metadata | no redundant escapes; style state default |
| TEST-FMT-ANSI-010 | API-FMT-014 | remove renderer-generated escape spans | remainder equals plain byte content, excluding caller-provided ESC |
| TEST-FMT-SINK-001 | API-FMT-012 | render to std::string | content identical to iterator/sink output in Plain policy |
| TEST-FMT-SINK-002 | API-FMT-013 | stream adapter with multiple runtime fragments | same ANSI state machine across fragments, no extra resets |
| TEST-FMT-SINK-003 | API-FMT-013 | writer consumes temporary runtime view immediately | no retained pointer, verified by lifetime tests |
| TEST-FMT-SINK-004 | API-FMT-015 | simulated writer fails after K bytes | failure observed by appropriate status/stream state; no false success report |
| TEST-FMT-SINK-005 | API-FMT-012 | output iterator not writable with char | constraints reject unsupported sink rather than obscure template backtrace |
| TEST-FMT-SINK-006 | API-FMT-015 | allocating string path | not incorrectly marked `noexcept` when STL can throw |
| TEST-FMT-OVERFLOW-001 | INV-MSG-001 | synthetically excessive type-level capacity addition | compile-time rejection, not wrapped size_t creating tiny buffer |
| TEST-FMT-CTRL-002 | §8.2 | runtime diagnostic includes ESC/control and newline | documented escaping policy verified, or explicit unsanitized trust limitation recorded |
| TEST-FMT-STATIC-001 | REQ-FMT-001 | `static_assert(false, constexpr Message)` in pinned compiler | correct rejection with authored body; no unrelated format failure |
| TEST-FMT-STATIC-002 | REQ-FMT-001 | `static_assert(true, message)` | accepted; compiler need not evaluate/print message in true case |
| TEST-FMT-STATIC-003 | REQ-FMT-001 | wrong consteval pointer lifetime | test intentionally invalid, compiler rejects invalid message storage |
| TEST-FMT-STATIC-004 | REQ-FMT-001 | inferred Message from long reflected type name | no guessed cap, full name preserved within compiler resource limits |
| TEST-FMT-PERF-001 | REQ-PERF-001 | 10K successful generic instantiations with/without failed-message provider | no unexpected message builder instantiations (trace evidence) |
| TEST-FMT-PERF-002 | REQ-PERF-003 | 100 small fragments chained | measured compile time/RSS compared to bulk compose variant |
| TEST-PERF-FMT-003 | REQ-PERF-003 | 1,000 fragments / long 64KiB text | record compiler cost and potential resource failure, no unsupported guarantee |
| TEST-FMT-IMPORT-001 | INV-ARCH-003 | `import Miracle.Format;` only | direct import succeeds with transitive Color, no StaticString/Diagnostics import |
| TEST-FMT-IMPORT-002 | API-FMT-001 | `static_assert(same_as<decltype(Diagnostic{}.severity()), Severity>)` adapted to accessible factory | same one Severity type across imports |

### Negative test harness requirement

A compile-fail fixture is **not** counted as passing because `clang++` returns nonzero from a missing header or syntax error. The harness must assert that compilation fails under the pinned reference compiler and that the normalized diagnostic output contains the **intended Miracle code or diagnostic summary**. When the failure is a concept-substitution rejection without a Miracle-authored message, inspect the intended constraint and match a stable compiler-specific feature (for the pinned snapshot only). Template-instantiation backtraces, filesystem prefixes, colored console wrappers and exact reflected spelling may be normalized; the user's authored message bytes must not be indiscriminately removed by a loose regular expression.

### Example compile-fail CMake approach

```cmake
# ILLUSTRATIVE only; exact project helper is already present under tests/cmake/.
add_test(NAME format_invalid_order
  COMMAND ${CMAKE_COMMAND}
    -DSOURCE=${CMAKE_CURRENT_SOURCE_DIR}/compile-fail/format-invalid-order.cpp
    -DEXPECTED_FRAGMENT="no matching member function"
    -P ${CMAKE_CURRENT_SOURCE_DIR}/cmake/expect-build-failure.cmake)
```

The actual invocation contract of the existing helper must be read before use; the above shows the intended *pattern*, **not** verified helper argument names. Preferred action: extend the existing tested compile-fail machinery, rather than creating a second independent harness.

## 10.4 Old subsystem regression and module integration tests

| Test ID | Requirement/invariant | Scenario | Expected result |
|---|---|---|---|
| TEST-MOD-DAG-001 | INV-ARCH-001 | scan every module provider/import including generated | no cycle, no duplicate primary provider |
| TEST-MOD-DAG-002 | INV-ARCH-003 | collect Format and Color direct imports | Color standard-only; Format standard/Color-only |
| TEST-MOD-DAG-003 | INV-ARCH-003 | inspect StaticString/Diagnostics/Iter imports | no upward Validation/umbrella cycle |
| TEST-MOD-OWNER-001 | INV-ARCH-002 | source search for `export module Miracle:` | no leftover moved public partitions outside Meta's own named module |
| TEST-MOD-OWNER-002 | INV-ARCH-002 | source search for `module Miracle;` | only intended owner (if any), no legacy implementation files for extracted modules |
| TEST-MOD-PLATFORM-001 | REQ-REST-003 | inspect/compile Windows Memory implementation | proper `Miracle.Memory` ownership; Windows compile PASS or NOT RUN explicitly |
| TEST-BLD-GEN-001 | REQ-BLD-001 | configure generated Build, Capabilities, FeatureConfiguration | each `.cppm` exists in build tree and is in module file set |
| TEST-BLD-GEN-002 | REQ-BLD-003 | compare generated provider inventory to CMake scan | no missing or duplicate generated module providers |
| TEST-BLD-CLEAN-001 | REQ-BLD-004 | clean isolated build/install prefix | no stale BMI, all modules rebuild and installed consumer succeeds |
| TEST-BLD-INSTALL-001 | REQ-BLD-002 | add_subdirectory consumer with `import Miracle;` | build, link, representative runtime operations PASS |
| TEST-BLD-INSTALL-002 | REQ-BLD-002 | FetchContent consumer of local checkout | build/link succeeds, target reused |
| TEST-BLD-INSTALL-003 | REQ-BLD-002 | installed find_package consumer | direct named imports and public templates work |
| TEST-BLD-INSTALL-004 | REQ-MOD-003 | installed consumer depends on exported partitions | no unavailable `PRIVATE FILE_SET` provider |
| TEST-BLD-INSTALL-005 | REQ-BLD-002 | import every accepted public module individually | symbols available without umbrella import |
| TEST-BLD-FEATURE-001 | REQ-FEA-001 | enabled/default optional feature sources | linked/imported features exactly match generated configuration |
| TEST-BLD-FEATURE-002 | REQ-FEA-001 | disabled optional feature | no accidental import/undefined link, helpful disable diagnostic preserved |
| TEST-AUDIT-COVER-001 | REQ-BLD-003 | source inventory `.cpp` and `.cppm` | every tracked C++ source classified, none silently dropped |
| TEST-AUDIT-COVER-002 | §8.3 | compare CodeQL extraction to modules | explicit extracted/not-extracted count; no false unsupported language claim |
| TEST-STR-API-001 | REQ-STR-001 | old `BasicStaticString` construction/NTTP | unchanged type layout and constexpr behavior |
| TEST-STR-API-002 | REQ-STR-001 | substr/trim/split/find | exact current behavior, including empty/capacity cases |
| TEST-STR-API-003 | REQ-STR-002 | `staticFormat` valid 0/1/N substitutions | unchanged output and supported grammar |
| TEST-STR-NEG-001 | REQ-STR-002 | unmatched `{`, `}`, bad placeholder | deliberate failure with intended authored message where feasible |
| TEST-STR-NEG-002 | REQ-STR-002 | placeholder count mismatch | intended error, no internal buffer failure |
| TEST-STR-NEG-003 | REQ-STR-003 | nontrivial Char | rejection at original compatible boundary |
| TEST-ITER-MOD-001 | INV-ITER-004 | direct-import external custom template types | no uninstantiable hidden helper or partition cycle |
| TEST-ITER-RANGE-001 | INV-ITER-002 | iter(lvalue vector) | borrowed lvalue behavior preserved |
| TEST-ITER-RANGE-002 | INV-ITER-002 | iter(move-only rvalue sequence) | source moved once, no unintended copy |
| TEST-ITER-RANGE-003 | INV-ITER-002 | Peekable next/peek interactions | cached cursor state and no duplicate pull |
| TEST-ITER-RANGE-004 | INV-ITER-003 | `std::ranges::view` and `borrowed_range` on each view | traits unchanged after specialization relocation |
| TEST-ITER-RANGE-005 | INV-ITER-002 | map/filter/scan/chunks/windows/intersperse on edge sizes | same elements and cardinality as baseline |
| TEST-ITER-INV-001 | INV-ITER-001 | direct callable succeeds | direct precedence selected, no forced tuple expansion |
| TEST-ITER-INV-002 | INV-ITER-001 | tuple-like only callable succeeds | tuple expansion selected and preserves forwarding |
| TEST-ITER-INV-003 | INV-ITER-001 | reflected aggregate callable succeeds | member order and access context stable |
| TEST-ITER-INV-004 | INV-ITER-001 | none of three shapes invocable | intended compile-fail with Format-authored diagnostic where feasible |
| TEST-ITER-INV-005 | INV-ITER-001 | move-only callable / reference-sensitive member | value categories not changed by module moves |
| TEST-ITER-PAR-001 | INV-ITER-002 | policy-backed terminal on eligible range | correct algorithm/policy behavior, no new scheduler |
| TEST-ITER-PAR-002 | INV-ITER-002 | ineligible non-random-access range | original constraint rejection preserved |
| TEST-FEA-001 | REQ-FEA-001 | empty catalog | successful empty result and deterministic fingerprint |
| TEST-FEA-002 | REQ-FEA-001 | duplicate names | intended failure and identifies duplicate stable name |
| TEST-FEA-003 | REQ-FEA-001 | missing dependency | intended missing feature, correct relation |
| TEST-FEA-004 | REQ-FEA-001 | unavailable capability | intended capability name in error |
| TEST-FEA-005 | REQ-FEA-001 | conflict | intended conflicting identity |
| TEST-FEA-006 | REQ-FEA-001 | required dependency cycle | deterministic full cycle explanation |
| TEST-FEA-007 | REQ-FEA-001 | build feature selected but not built | intended unavailable build feature message |
| TEST-FEA-PERF-002 | REQ-FEA-002 | success catalog repeated instantiations | no failing diagnostic formatting on success |
| TEST-DIAG-API-001 | REQ-DIAG-001 | annotated enum with prefix + message | same code and canonical message |
| TEST-DIAG-API-002 | REQ-DIAG-001 | prefix-less domain | heading behavior matches existing contract |
| TEST-DIAG-API-003 | REQ-DIAG-002 | only `import Miracle.Diagnostics` | Severity/ColorMode/Presentation visible, one type identity |
| TEST-DIAG-API-004 | REQ-DIAG-001 | `align(n)` with n below/above enumerator digit count | minimum zero width; no truncation |
| TEST-DIAG-NEG-001 | REQ-DIAG-001 | negative enum value | intended DiagnosticCode rejection |
| TEST-DIAG-NEG-002 | REQ-DIAG-001 | invalid align without prefix | intended rejection as baseline |
| TEST-DIAG-NEG-003 | REQ-DIAG-001 | duplicate message annotation | intended rejection |
| TEST-DIAG-PARITY-001 | INV-DIAG-001 | equivalent compile-time and runtime header/code | identical plain severity, code, width, summary layout |
| TEST-DIAG-RENDER-001 | REQ-DIAG-004 | Terminal/Always | styled output and trailing reset |
| TEST-DIAG-RENDER-002 | REQ-DIAG-004 | Plain/Always | no SGR escapes, same plain data |
| TEST-DIAG-SPAN-001 | ALG-DIAG-002 | half-open single-line primary span | correct caret count/offset |
| TEST-DIAG-SPAN-002 | ALG-DIAG-002 | out-of-bounds and reversed span | baseline clamp/normalize behavior |
| TEST-DIAG-SPAN-003 | ALG-DIAG-002 | secondary span | distinct marker/style, no corrupt primary |
| TEST-DIAG-TREE-001 | ALG-DIAG-001 | chain of 2,000 nested causes | iterative traversal; no recursion stack overflow |
| TEST-DIAG-TREE-002 | ALG-DIAG-001 | multiple siblings with notes and trace | deterministic insertion order and depth indentation |
| TEST-META-SNAP-001 | INV-META-001 | public query source / map / filter / terminal | same results/value categories as old baseline |
| TEST-META-SNAP-002 | INV-META-001 | borrowed local escape negative | rejected at intended barrier, no accidental materialization |
| TEST-META-SNAP-003 | INV-META-001 | late-declared members and cross-TU snapshots | same reflected observations at declaration point |
| TEST-META-SNAP-004 | INV-META-002 | cache/extract of repeated reflected types | same access and storage lifetime semantics |
| TEST-META-SNAP-005 | INV-META-001 | move-only captured callable in Query | preserved evaluation and rejection barriers |
| TEST-META-SNAP-006 | INV-META-001 | flattening temporary range and nested Query | no dangling reference regression |
| TEST-REST-001 | REQ-REST-002 | Error expected propagation | old `Result` semantics, no changed allocation/throwing |
| TEST-REST-002 | REQ-REST-002 | Panic behavior / contract violation | established error report and termination semantics |
| TEST-REST-003 | REQ-REST-002 | Fs OpenOptions create/truncate/read/write | existing filesystems contracts preserved |
| TEST-REST-004 | REQ-REST-004 | Debug and Bitflags across direct imports | same reflection labels and flag behavior |
| TEST-REST-005 | REQ-REST-003 | Memory supported/unsupported paths | same optional snapshot semantics |
| TEST-REST-006 | REQ-REST-005 | Profiling direct import and runtime output | same measurement APIs and content semantics |

## 10.5 Performance experiments — complete methodology

Performance is a separate verification axis; there is no preapproved numerical budget. Select one fixed hardware machine with recorded CPU model/cores, RAM, OS, filesystem, cache constraints, reference compiler manifest and exact CMake/Ninja versions. Run three or more repetitions of each configuration; report medians, min/max, and raw observations. Cold builds must remove build artifact and module/BMI caches as appropriate; warm incremental edits must use the same primed build state and edit scope. Do not compare a Release build on one commit to a Debug build on another.

**PERF-01 Cold build:** freshly configure and compile complete library and self-tests under pinned flags; capture wall time, peak RSS and generated BMI count/bytes. **PERF-02 Private edit:** touch an implementation unit in Diagnostics, Error or selected runtime implementation; rebuild; capture recompilation file count and critical path. **PERF-03 Public edit:** alter a public interface in a controlled no-semantic-change way; rebuild downstream; restore file; measure invalidation costs. **PERF-04 Granular imports:** separately compile an otherwise empty consumer importing Color, Format, Numeric, StaticString, Iter, Diagnostics, Meta and umbrella; compare wall/RSS/BMI mapping. **PERF-05 Template stress:** compile 100/1,000 representative Iter/Feature/Meta Query instantiations, preserving code semantics. **PERF-06 Diagnostic stress:** build/compile styled Message chains at lengths 100, 1K, 10K and fragment counts 10, 100, 1K; compare chained versus bulk construction. **PERF-07 Runtime render:** render identical diagnostic trees with plain/ANSI options and large source snippets; measure bytes/sec and allocation count if instrumentation available. **PERF-08 Installation:** configure and build independent installed consumers from cold and warm state to detect exported BMI overhead.

For each experiment specify baseline SHA and target SHA, exact commands, target name, repeated observations and observed regression or improvement. A performance regression is **material** if it meaningfully affects supported build workflows or violates an independently approved target; absent such approval the reviewer must report numbers and seek explicit disposition rather than invent an arbitrary 5% gate. Variance/thermal throttling/background jobs must be recorded. The known O(KN) chained Message strategy is a potential design tradeoff, not automatically a failure; it becomes a review issue if representative compile-time use suffers meaningful cost.

## 10.6 API compiler verification matrix

**Status of proposed interfaces as of this document:** the actual pinned reference compiler is unavailable in this environment. No Phase M Color/Format API has been implemented and compiled here. The matrix records the correct unverified status; only the isolated host P-HOST-001 is a PASS, explicitly scoped to its miniature C++23 snippet.

| API verification ID | Exact public surface | Required reference experiment | Reference status | Notes |
|---|---|---|---|---|
| API-V-COL-01 | `Miracle.Color`, Color/Hsv declarations | standalone import + constexpr construction | NOT RUN | module does not exist in inspected source |
| API-V-COL-02 | RGB / hex / parseHex overloads | constexpr positive/negative + constraints | NOT RUN | checked invalid-input policy proposed |
| API-V-COL-03 | HSV conversions | constexpr sector + finite + round-trip tests | NOT RUN | compiler constexpr finite checks need proof |
| API-V-FMT-01 | `Miracle.Format` import | direct+umbrella+installed CMake consumers | NOT RUN | module does not exist in inspected source |
| API-V-FMT-02 | single Severity/ColorMode/Presentation | cross-module identity + direct-import | NOT RUN | ABI/BMI owner migration pending |
| API-V-FMT-03 | TerminalColor/Style/Fragment | aggregate/constexpr/CTAD tests | NOT RUN | exact interface signatures provisional |
| API-V-FMT-04 | Code prefix/width helper | CTAD/templates/invalid width tests | NOT RUN | declaration ordering must be fixed |
| API-V-FMT-05 | ordinary Message CTAD | C++26 constexpr append consumer | NOT RUN | host P-HOST-001 partial analogue only |
| API-V-FMT-06 | diagnostic Message constructor | C++26 class template deduction + methods | NOT RUN | type-level bounds and stage rules require proof |
| API-V-FMT-07 | `static_assert(false, Message)` | pinned C++26 compile-fail snapshot | NOT RUN | cannot test on Clang17 C++23 |
| API-V-FMT-08 | fragment style offsets | constexpr ranges + ANSI renderer | NOT RUN | host P-HOST-001 checks miniature copy logic only |
| API-V-FMT-09 | runtime rendering/event interface | stream/string/iterator consumers | NOT RUN | final writer API pending DEC-FMT-002 |
| API-V-DIAG-01 | Diagnostics adapts to Format | source span/cause/full rendering | NOT RUN | still old renderer at baseline |
| API-V-ITER-01 | Iter independent module and partitions | custom type import/instantiation | NOT RUN | currently old umbrella partition |
| API-V-BLD-01 | exported package module set | clean installed consumer after extraction | NOT RUN | baseline CI proves current source only |
| P-HOST-001 | miniature typed growing fragment Message | Clang17/C++23, compile/link/run | **PASS (host only)** | no import/std module/C++26 guarantee |

### Reference probe fixture layout

```text
tests/format/
  import-color.cpp
  import-format.cpp
  literal-append.cpp
  styled-runs.cpp
  message-section-order.cpp
  constexpr-code-width.cpp
  cxx26-static-assert.cpp
  compile-fail/
    invalid-section.cpp
    second-context.cpp
    bad-message-lifetime.cpp
    impossible-static-storage.cpp
  rendered/
    plain.golden
    ansi.golden
  diagnostics-adapter.cpp

tests/color/
  constexpr-rgb.cpp
  constexpr-hsv.cpp
  hex-parser.cpp
  roundtrip.cpp
```

These are proposed fixture names and assertions; the exact test harness must align with repository conventions. No files in this layout are claimed to exist. Negative tests should compile in an environment containing the accepted module providers, not in isolation with unresolved imports.


---

# Part XI — Engineering reference, worked transformations and debugging

## 11.1 Worked example A: StaticString hard assertion without a dependency cycle

### Context and design objective

The existing BasicStaticString template rejects nontrivially copyable/default-constructible/standard-layout character types, and the existing `staticFormat` parser rejects invalid format grammars. These requirements arise **below** Diagnostics and cannot use a high-level `Miracle::Diagnostic` object without a cycle. The approved architecture allows `Miracle.StaticString → Miracle.Format → Miracle.Color`, with no reverse arrow. Local semantic predicates remain in StaticString; only the presentation of a hard failure moves to Format.

### Data and dispatch path

1. A consumer instantiates `BasicStaticString<Char,N>` or invokes `staticFormat<FormatString>(args...)` after `import Miracle.StaticString;`.
2. The original template constraint or locally computed format parser determines valid/invalid state. The source module retains ownership of the predicate.
3. For a deliberate hard failure, a source-private diagnostic provider selects code and summary, and optionally a reflected type string. It uses Format's `code<Width>` and compile-time Message construction on the failing path.
4. Format constructs plain bytes and style range metadata using sized literals or a reflection adapter with a known compile-time extent. No Diagnostics/Meta module is imported to build the message.
5. The final `constexpr` Message exposes `size()` and `data()` to C++26 generated `static_assert`, whose diagnostic body is reported by the compiler. No ANSI bytes appear because `Message::data()` is plain.
6. For success, the expensive provider is not instantiated if the language permits placing it within a discarded `if constexpr` branch. Existing `staticFormat` runtime/consteval behavior remains unchanged.

### Illustrative source-private provider

```cpp
// ILLUSTRATIVE — intentionally not a complete definition or tested API.
// Placed in Miracle.StaticString's internal implementation region.
template<class Char>
consteval auto invalidCharacterMessage() {
    // If a type-name fragment is desired, first materialize an owned
    // size-known fragment from std::meta::display_string_of(^^Char).
    return format::Message{
        Miracle::Severity::Error,
        format::code<3>("STR", 1),
        "invalid StaticString character type"
    }
        .context("BasicStaticString")
        .help("use a trivially copyable, trivially default constructible, "
              "standard-layout character type");
}
```

The adjacent string literals concatenate into one help message. This is **illustrative** and remains unverified against the approved reference compiler; the source-private provider must be reconciled to the accepted Message constructor and type-level capacity rules.

### Correctness argument

Import graph cannot cycle because Format imports only Color/std. Source-local format parser is the single authority for brace grammar, and the display builder cannot make an invalid input valid or vice versa. The only semantic risk is that constructing an intended diagnostic triggers a more fundamental substitution error before the `static_assert`, so the test suite includes intentionally malformed format strings and invalid template types with expected fragments. The user-visible symptom of a regression is an unrelated compiler trace from the formatting machinery rather than an actionable StaticString error.

### Failure debugging

If the diagnostic cannot instantiate, reduce to a minimal `import Miracle.Format;` fixture and separate literal Message from reflected-name adapter. Verify owner imports, constant-expression status of `name.size()`, and any pointer lifetime. Do not automatically increase guessed message buffer capacity; the design requires deduced sufficient storage. If a constraint rejects the template before entering the authored failure path, decide whether that is a contract-permitted concepts diagnostic rather than forcing a rich hard error.

## 11.2 Worked example B: from Switch's highlighted fragments to typed styles

Switch's `DiagnosticFragment` stores text with a boolean highlighted flag. Imagine it produces a note:

```text
expected a value of type MyType; observed OtherType
```

where only `MyType` and `OtherType` are emphasized. This cannot be faithfully represented by styling the entire note uniformly. Miracle Format represents the same text through three or more fragments:

```text
F1("expected a value of type ", default)
F2("MyType", highlighted Style)
F3("; observed ", default)
F4("OtherType", emphasized-error Style)
```

The owned Message concatenates bytes, recording style runs only for F2 and F4, with byte offsets. `view()` returns exactly the plain sentence. ANSI rendering emits a reset/reapplication around highlighted fragments, whereas plain rendering does not emit any terminal control bytes. Runtime Diagnostics can yield the same fragments with `std::string_view` data; the output sink consumes them immediately without copying them into an arbitrary fixed-size buffer.

**Why this is one system:** both the compiled Message and runtime adapter use one Style type, one TerminalColor type, one severity vocabulary, and one Format SGR encoder. The difference is how content is *supplied* (owned constexpr vs borrowed immediate runtime); it is not two independent diagnostic implementations.

**Counterexample:** designing a second `RuntimeDiagnosticFragment` with another color enum and an ad-hoc `highlighted` flag would create semantic duplication. The runtime adapter should use the same `Style` value and immediate text views.

## 11.3 Worked example C: without an invented exact-size dynamic return type

Suppose a runtime Diagnostic contains `std::string message` with length known only after execution. A function returning `format::Message<message.size()>` cannot form that return type from the runtime length. Attempting to run the whole Diagnostics renderer through a compile-time class template would require guessed caps, dynamic memory disguised as static, or a type-erased second public Message. Instead:

```text
Diagnostic owns std::string
        │
        ▼
Diagnostics adapter obtains string_view
        │
        ▼
Format receives (text view, Style, Section event)
        │
        ▼
Format streams to sink under one formatting grammar
```

This preserves runtime ownership and compile-time API purity. It is a sound design consequence of C++ type/value separation, not a workaround for missing implementation effort. Source/contract authoring must still settle the exact writer overload set before the API becomes locked. The need for a small runtime event interface is an **architectural fact** under these requirements, not optional scope creep.

## 11.4 Worked example D: why Iter's factories cannot simply live with sources

An `iter(range)` factory constructs and returns `Iter<decltype(view)>`. The core Iter class itself uses view class types and may depend on helpers supplied by Sources. If `Sources` imports `Core` for the return type and `Core` imports `Sources` for its view classes, CMake's module graph contains a cycle. Merely moving definitions to different files does not solve a C++ module cycle.

The correct decomposition is:

```text
SourceViews (class declarations/definitions)
          │
          ▼
         Core (Iter class, consumes SourceViews)
          │
          ▼
SourceFactories (iter/once/fromFn... constructs Core)
```

`Views` and `Detail` feed Core similarly, without reverse imports. `Iter.cppm` re-exports all three from the primary interface; consumers still see the same factory spellings after `import Miracle.Iter;`.

### C++ semantics to preserve

If a view's nested iterator is a dependent type, its full definition may need to be reachable when a consumer evaluates a range concept. A declaration without a reachable template body will compile in the library's own translation unit but fail downstream with a user-defined type. Test within an installed consumer to detect this. The `std::ranges::enable_view` specialization placement matters because specialization visibility must precede trait instantiation in a TU; moving it below an earlier dependent use can change overload participation. `std::ranges::borrowed_range` likewise affects whether temporary owners leak references. The partition split must be reverified against the reference C++26 modules implementation, not inferred from header-only C++ rules alone.

## 11.5 Worked example E: why Feature's split should not be redone

An older guide described a single large Feature partition; the current source already has `Miracle.Feature:Types`, `:Descriptor`, `:Set`, `:Catalog`, `:Requirement` and a re-exporting primary interface. M.6 should validate the decomposition that **already exists**. Its current order is types → descriptor → set → catalog, with requirement importing descriptor. If a helper in Types uses StaticString, that's an allowed downward dependency. If Catalog imports Set and Set imports Catalog, that is not allowed; retain the existing forward declaration structure rather than introduce a cycle. The compile-time resolver's `ResolutionReport` and `Catalog` should remain unchanged except presentation when possible.

This case illustrates a broader implementation-planning rule: **the specification must follow source evolution**. A previously reasonable migration task can become a no-op plus verification at a later HEAD. Reapplying it mechanically is not diligence; it is unnecessary churn and may introduce regressions.

## 11.6 Worked example F: source-aware runtime diagnostics remain separate from formatting

`DiagnosticSpan` owns explicit source snippets, half-open selection and optional location metadata. Format doesn't know how to relativize a filesystem path or classify a source selection as primary/secondary; those are Diagnostics decisions. The adapter first computes a normalized span:

```text
source = stored snippet
selection = clamp(begin,end,0,len(source))
lineStart = byte index after prior '\n' or 0
lineEnd = next '\n' or len(source)
markerStart = begin - lineStart
markerLength = max(1, end-begin) under existing span policy
```

Then it emits the heading/location/snippet/marker through shared Format fragments and styles. The terminal choice only changes escape bytes, not selection logic. Untrusted control bytes need an explicit escaping policy, particularly when a source snippet itself contains ANSI escapes. If source line/column semantics ever become Unicode-aware, that belongs to a separately approved domain requirement; Phase M preserves byte/code-unit behavior.

## 11.7 Worked example G: CMake module visibility is not lexical `export`

A public interface might `import :Detail;` and use a type or function defined there in exported template bodies. C++ treats the declaration's export status and reachability separately; CMake treats the module interface file-set visibility as an installation/usage requirement. Marking `:Detail` as PRIVATE solely because its C++ names aren't exported can break downstream installations, since consumers may still need its BMI. The correct review question is whether the consuming public module interface requires the partition for compilation and instantiation. If so, keep it in a public-installable module file set or use an alternative internal layout that demonstrably works with the reference compiler and CMake.

Test through `find_package` after a fresh install, then compile an arbitrary custom-type `Iter` instantiation. An in-tree build uses favorable BMI paths and can conceal a broken install, so it is insufficient evidence.

## 11.8 Worked example H: color conversion boundary details

Consider `Color::fromHsv({60.0, 1.0, 1.0})`. The sector index is 1; chroma is 1; fractional position is zero, so the yellow transition yields R=1,G=1,B=0; quantization returns `(255,255,0)`. At H slightly below 60 the sector is 0 with G approaching 1; slightly above 60 the sector is 1 with R approaching 1. The function is continuous at this boundary before quantization, which may introduce at most a one-code-value jump. For saturation zero, chroma vanishes and all three outputs equal value; hue remains an input-domain check but does not affect channel result. Invalid NaN must be rejected **before** floating-to-int sector conversion.

`parseHex("#abc")` expands digits `a`, `b`, `c` to `0xAA`, `0xBB`, `0xCC`; `toHex` yields `0xAABBCC`. Parsing an invalid seventh byte returns `nullopt` without partially exposing the first two parsed channels. The parser's result is one value with a valid-or-invalid state, not an exception or mutable half-parsed object.

## 11.9 Debugging module migration failures systematically

A module migration failure should be classified before applying a fix:

**Provider not found:** inspect CMake file sets, actual `export module` spelling, generated output existence, Ninja scan graph and BMI map. **Partition cycle:** generate import edges and print the cycle; split dependent factories from lower-level views rather than inserting textual includes. **ODR/ABI error:** inspect whether both old `Miracle:` and new `Miracle.Component` declare the same exported entity; clean BMI cache. **Template substitution failure in consumer:** verify definitions reachable and public imports sufficient, especially after moving a helper to `.cpp`. **C++26 reflection discrepancy:** compare declaration point, access context, captured `std::meta::info` and compiler snapshot; do not change semantics to silence a diagnostic. **Formatted diagnostic error:** isolate owning provider, convert to one literal Message, add styled fragment gradually; check type-level bounds and constant-expression lifetime. **ANSI mismatch:** compare plain bytes first; if equal, inspect style runs then encoder state transitions. **Windows-only failure:** check platform-selected module implementation and declarations, especially old `Miracle.Core` ownership. **Doc CI failure:** inspect `docs/SUMMARY.md` navigation before changing mdBook build code.

## 11.10 Incorrect implementation patterns to reject in review

```cpp
// BAD: coupled to higher-level module
export module Miracle.Format;
import Miracle.StaticString;   // creates potential bootstrap cycle
import Miracle.Diagnostics;    // direct forbidden upward dependency

// BAD: duplicated semantic severity
namespace Miracle::format { enum class Level { Error, Warning }; }
namespace Miracle { enum class Severity { Error, Warning }; }

// BAD: arbitrary guessed storage and silent truncation
std::array<char,256> buffer{};
// append without checked bounds; silently cut off error summary

// BAD: runtime value as template argument
consteval auto make(std::string_view s) { return Fragment<s.size()>{}; }

// BAD: ANSI embedded in source message
constexpr auto error = Message{"\x1b[31merror\x1b[0m"};

// BAD: incorrect module ownership after extraction
module Miracle;               // in Diagnostics.cpp for Miracle.Diagnostics

// BAD: pretending runtime string length has type-level capacity
auto runtimeMessage(std::string_view text) -> Message<text.size(),1>;

// BAD: two independent renderers with independent severity/color tables
Diagnostics::renderAnsi(...);  // ad-hoc duplicate Format SGR engine
```

These examples are **deliberately invalid architectural patterns**, not code to compile. A review should cite the actual invariant they violate rather than merely label them 'unidiomatic'. In particular, the `consteval` runtime template-size example is an example of a language-rule violation; it cannot be repaired by changing `constexpr` to `consteval`.

## 11.11 Independent implementation exercise

An experienced implementing engineer should be able to use this specification to perform the following without designing a new abstraction: create Color's RGB8/HSV and parser, prove conversion correctness, implement typed Fragment ownership, derive contiguous Message capacity from extents, enforce monotonic diagnostic section ordering, write plain/ANSI output through one encoder, adapt existing move-only Diagnostics into the same rendering grammar, migrate source ownership into named modules and verify installed-package imports. For every step, the engineer has a named invariant and test. Direct HEX return type, runtime writer architecture, and control-character normalization were all approved on 2026-10-30; they are no longer open questions. Any new design-changing deviation still requires owner approval.


---

# Part XII — Traceability, decisions, audit and readiness

## 12.1 Decision register: explicit approval state

A new public API becomes locked only after the author approves the signatures and the governing contract is updated in the authoritative repository. Prior design discussions approved architectural direction, not every exact spelling selected here. The register explicitly distinguishes the two.

| Decision | Problem and alternatives | Selected/proposed approach | Authority | Consequence/approval |
|---|---|---|---|---|
| DEC-M-001 | Independent Module Migration versus Phase 5.x extension | standalone Phase M between Phase 5 and 6 | **APPROVED owner** | current roadmap must preserve identity |
| DEC-M-002 | MSVC-influenced vs Clang source suffix | `.cppm` interface, `.cpp` implementation | **APPROVED owner** | source/CMake/tooling migration |
| DEC-M-003 | Independent named module vs umbrella partitions | primary `Miracle.<Component>` per cohesive facility | **APPROVED owner** | changed module ownership/BMI |
| DEC-M-004 | Many CMake linked targets vs one | retain `Miracle::Miracle` target | **APPROVED contract intent** | no target proliferation |
| DEC-FMT-ARCH-001 | Format as guidelines only vs shared machinery | shared presentation foundation used by Diagnostics | **APPROVED owner** | one renderer grammar, no duplicate severity |
| DEC-FMT-ARCH-002 | Nested `format::diagnostic` namespace vs one namespace | single `Miracle::format` public namespace | **APPROVED owner** | no nested diagnostic hierarchy |
| DEC-FMT-ARCH-003 | Builder/factories vs constructors | constructor-initialized Message, no public duplicate Builder | **APPROVED owner** | no invalid headerless diagnostic state |
| DEC-FMT-ARCH-004 | Caller guessed capacity vs type inference | statically typed append-only growing contiguous Message | **APPROVED owner** | no fixed arbitrary user size |
| DEC-FMT-ARCH-005 | Embedded ANSI vs style metadata | plain bytes plus style runs; ANSI only render | **APPROVED owner** | deterministic plain static_assert |
| DEC-FMT-ARCH-006 | Free-form diagnostic order vs enforced | monotonic Heading/Context/Fields/Notes/Help | **APPROVED owner** | compile-time stage semantics |
| DEC-FMT-ARCH-007 | Separate Level and Severity vs one owner | move existing `Miracle::Severity` ownership to Format | **APPROVED owner (F7)** | no duplicate enums |
| DEC-FMT-ARCH-008 | Compiled-only vs ordinary and diagnostic messages | one Message model, different constructor/grammar modes | **APPROVED owner (F8)** | code uses same fragments |
| DEC-FMT-ARCH-009 | Plain data/view or ANSI contents | always plain data/view; explicit ANSI rendering | **APPROVED owner (F9)** | static assert stable |
| DEC-FMT-ARCH-010 | Persistent fragment chain vs contiguous | contiguous typed storage initially, benchmarks | **APPROVED owner (F10)** | O(KN) chaining risk quantified |
| DEC-COL-ARCH-001 | Color inside Format vs standalone | `Miracle.Color`, std-only; Format imports it | **APPROVED owner** | new exception to old Format import ban |
| DEC-COL-ARCH-002 | RGB/HSV/HEX mini primitive vs color framework | minimal sRGB8, HSV/hex conversions, no alpha/theme | **APPROVED; reference checks NOT RUN** | Phase M only minimal capability |
| DEC-COL-001 | `rgb(uint8_t,uint8_t,uint8_t)` vs checked integer channel overloads | typed uint8 plus checkedRgb for integral inputs | **APPROVED 2026-10-30** | checked factory validates full range |
| DEC-COL-002 | `Color::hex(u32)` Color vs optional return | proposed optional checked return | **APPROVED 2026-10-30** | changes ergonomic call sites |
| DEC-COL-003 | include reverse HSV and #RGB shorthand now? | proposed both | **APPROVED 2026-10-30** | small extension to Color scope |
| DEC-FMT-001 | context accepting type-sized Fragment; one context | proposed literal+Fragment overload, one-shot | **APPROVED 2026-10-30** | compile-time generic/reflection context |
| DEC-FMT-002 | public sink/event API vs internal-only primitives | proposed minimal public statically dispatched writer/event primitives | **APPROVED 2026-10-30** | public event/sink seam agreed; syntax tests NOT RUN |
| DEC-FMT-003 | final diagnostic newline grammar | proposed exactly one `\n` after final line | **APPROVED 2026-10-30** | snapshot stability |
| DEC-FMT-004 | embedded control characters and multiline values | Format core escapes one-line fields; structured multiline controlled by Diagnostics | **APPROVED 2026-10-30** | terminal safety/diagnostic layout |
| DEC-FMT-005 | whether `StyleRun` is public and normalized | proposed public read-only run descriptor, adjacent equal styles coalesced | **DERIVED from approved FMT-2; implementation target** | observable style representation |
| DEC-FMT-006 | how to carry runtime code prefix/width into Format event writer | proposed borrowed Code-like view with no registry | **DERIVED from approved FMT-2; implementation target** | no second code-domain hierarchy |
| DEC-META-001 | always split Query or only if useful | split optional, preserve observation points | **APPROVED contract direction** | no pointless line-cutting |
| DEC-REVIEW-001 | review every commit immediately or batch final | one milestone commit each; batched final review allowed | **APPROVED owner** | evidence and status remain per milestone |

**Approval rule:** pending decisions block a claim that the *exact new API* is locked. They do not negate the already approved architectural direction. Resolution may adopt a different precise representation so long as it preserves the approved invariants and updates the table, API declarations and test cases consistently. A material deviation from approved architecture requires an ADR and explicit approval; a minor equivalent implementation choice does not.

## 12.2 Contract collision and historical-reconciliation report

**Finding AUDIT-C-001 [MAJOR / contract status].** The original Phase M `DESIGN.md` existed at historical `469083e...`, but is absent from current master after a commit whose message describes removing docs contracts. The Development Protocol states the authoritative source is repository-tracked accepted contracts. Resolution: owner selects current canonical document location, restores/adopts the contract with provenance and explicit approval, or records an accepted successor. This specification must not claim the deleted document automatically remains active as current Git authority.

**Finding AUDIT-C-002 [MAJOR / new dependency].** The historic Phase M contract prohibited all Miracle imports from Format; the owner subsequently approved standalone Color and Format consuming Color. This is an **approved direction change** but the contract text must be amended. Resolution: replace `Format imports no Miracle modules` with `Format imports only std and Miracle.Color`, and preserve the ban on StaticString, Diagnostics, Meta, Iter, umbrella and Validation.

**Finding AUDIT-C-003 [MAJOR / API duplication].** The previous implementation guide proposed `format::diagnostic::Builder` and `Level`, separately from existing `Diagnostic` and `Severity`. The owner explicitly rejected these. Resolution: remove the rejected public API from accepted guidance; document one Message/Style/Severity foundation and Diagnostics adapter. The old guide remains historical, not implementable authority.

**Finding AUDIT-C-004 [MAJOR / size semantics].** User-provided fixed `Message<512>` buffers and silent truncation contradict the owner's inferred-size, append-only decision. Resolution: compile-time extents drive resulting types; runtime-sized data streams through shared presentation sink; no guessed capacity or second dynamic Message hierarchy.

**Finding AUDIT-C-005 [MAJOR / source evolution].** Current master already migrated suffixes and several modules and split Feature. A guide written for v0.5.27 can no longer be executed step-for-step as an implementation queue. Resolution: audit milestones against current source state, mark already implemented work for verification, do not reapply it mechanically. Incorporate the new Color/Format scope only after signature approval.

**Finding AUDIT-C-006 [MAJOR / unverified compiler].** C++26 generated static assertions, CTAD from typed Code and modules require the pinned compiler. Only host C++23 core probes were executed. Resolution: perform exact reference tests before declaring API compiler-verified or milestone complete; record unavailable tests NOT RUN.

**Finding AUDIT-C-007 [SOURCE DEFECT CANDIDATE].** `src/private/windows/Memory.cpp` declares `module Miracle.Core;` whereas the current public Memory owner is `Miracle:Memory` pending standalone extraction. Resolution: correct to final named owner `Miracle.Memory`, test platform when available; do not claim a proven Windows build failure without compilation.

**Finding AUDIT-C-008 [SCOPE].** Existing runtime Diagnostics source renderer includes advanced source/snippet/cause handling that must not be reduced to Format's minimal compile-time grammar. Resolution: keep domain traversal/position logic in Diagnostics and delegate common style/header encoding to Format.

## 12.3 Requirement-to-component-to-test traceability matrix

The table traces stable requirements and invariants to implementation and verification gates. Some requirements have source/static inspection rather than executable tests, and are explicitly classified. When the exact Format runtime writer API changes during ratification, revise affected matrix rows before declaring API coverage complete.

| Requirement / invariant | Component/algorithm | Milestone | Evidence / specific tests | Gate |
|---|---|---|---|---|
| REQ-GOV-001,003,004 | contract status/precedence | M.1,M.8 | contract reconciliation report; owner approval | governance audit |
| REQ-GOV-002 | milestone ownership/reviews | M.1–M.8 | commit range + ledger; review outcomes | phase acceptance |
| REQ-BASE-001,002 | source/baseline inventory | M.1 | M1-SOURCE-INVENTORY; baseline CSV, HEAD | no invented metrics |
| REQ-SCOPE-001,003,004 | migration boundaries | M.1–M.8 | source API diff + direct imports, TEST-MOD-OWNER-001 | source compatibility |
| REQ-SCOPE-002 | deferred features | M.3–M.8 | source review for absent unrelated framework | scope audit |
| INV-ARCH-001,002 | module DAG/ownership | M.1,M.3–M.7 | TEST-MOD-DAG-001, TEST-MOD-OWNER-001/002 | acyclic, one owner |
| INV-ARCH-003 | Color/Format foundation | M.3,M.4 | TEST-MOD-DAG-002/003, TEST-COL-IMPORT-001, TEST-FMT-IMPORT-001 | no upward imports |
| INV-ARCH-004 | package partitions | M.7 | TEST-BLD-INSTALL-003/004 | install consumer PASS |
| REQ-MOD-001..003 | file declarations/template reachability | M.2,M.5–M.7 | TEST-ITER-MOD-001, TEST-BLD-INSTALL-004 | provider/consumer match |
| REQ-MOD-004 | Numeric/Types mapping | M.3 | direct Numeric/Types consumers + source inventory | alias continuity |
| REQ-MOD-005 | Color import edge | M.3 | TEST-COL-IMPORT-001, TEST-FMT-IMPORT-001 | no cycle |
| REQ-BLD-001..004 | CMake/export/install/tooling | M.2,M.7 | TEST-BLD-GEN-001/002, TEST-BLD-CLEAN-001, TEST-BLD-INSTALL-001..005 | reproducible consumer PASS |
| REQ-API-001..003 | added/existing APIs and compiler | M.3–M.8 | Part V inventory, API-V-* and subsystem tests | signatures verified |
| REQ-COL-001 | sRGB8/HSV/hex safety | M.3 | TEST-COL-HSV-001..010, TEST-COL-PARSE-001..009 | checked invalid inputs |
| API-COL-001..007 | Color/Hsv | M.3 | TEST-COL-RGB/TYPE/HEX/PARSE/HSV/REV/PROP-* | compiler+properties |
| REQ-FMT-001 | C++26 generated message | M.3,M.4 | TEST-FMT-STATIC-001..004, API-V-FMT-07 | no ANSI/valid lifetime |
| API-FMT-001,002 | Severity/presentation ownership | M.3,M.4 | TEST-FMT-HEAD-002, TEST-DIAG-API-003 | one type identity |
| API-FMT-003,004 | Color/style semantics | M.3 | TEST-FMT-COLOR-001..004, TEST-FMT-STYLE-001..006 | visual metadata correct |
| API-FMT-005,006 | sized Fragment factories | M.3 | TEST-FMT-TYPE-001..003, TEST-FMT-BINARY-001/002 | owned bytes |
| API-FMT-007 | code prefix/width | M.3,M.4 | TEST-FMT-CODE-001..006 | no truncation |
| API-FMT-008..011 | owned Message/sections | M.3 | TEST-FMT-TYPE-004..006, TEST-FMT-ORDER-001..008 | inferred capacity/order |
| API-FMT-012..015 | runtime/ANSI render | M.3,M.4 | TEST-FMT-SINK-001..006, TEST-FMT-ANSI-001..010 | one encoder |
| API-FMT-016 | no generic formatter | M.3 | public API inventory and source inspection | no scope creep |
| INV-MSG-001..004 | Message storage/lifetime | M.3 | TEST-FMT-BINARY/OWN/STYLE/LIFE, host P-HOST-001 | storage proofs |
| INV-SEC-001,002 | diagnostic state machine | M.3 | TEST-FMT-ORDER-001..008 | no illegal section order |
| INV-NUMTXT-001 | decimal safety | M.3 | TEST-FMT-NUM-001..004, CODE-001..006 | full numeric range |
| INV-ANSI-001,002 | terminal state/color | M.3,M.4 | TEST-FMT-ANSI-001..010, COLOR-001..004 | plain parity/reset |
| REQ-STYLE-001..003 | styling/layout | M.3,M.4 | TEST-FMT-STYLE-001..006, TEST-DIAG-PARITY-001 | common grammar |
| REQ-ERR-001..004 | error classification | M.3–M.8 | compile-fail harness, Color invalid tests, Meta barriers | intended failure |
| REQ-LIFE-001..003 | ownership and sink lifetime | M.3,M.4 | TEST-FMT-OWN-001/002, LIFE-003, SINK-003 | no dangling |
| REQ-PERF-001..003 | compile/runtime resource | M.1,M.3,M.6,M.8 | PERF-01..08, TEST-FMT-PERF-* | measured regressions |
| REQ-STR-001..004 | StaticString old API | M.3 | TEST-STR-API-001..003, TEST-STR-NEG-001..003 | no semantic change |
| INV-ITER-001..004 | Iter constraints/lifetimes | M.5,M.6 | TEST-ITER-INV/RANGE/MOD/PAR-* | reference preservation |
| REQ-FEA-001..003 | Feature resolver semantics | M.3,M.5,M.6 | TEST-FEA-001..007, TEST-FEA-PERF-002 | deterministic reports |
| REQ-DIAG-001..004 | Diagnostics extraction/adapter | M.4 | TEST-DIAG-API/NEG/SPAN/RENDER/TREE/PARITY-* | current semantics retained |
| INV-DIAG-001,002 | shared render and tree stability | M.4 | TEST-DIAG-PARITY-001, RENDER-001/002, TREE-* | parity, no duplication |
| REQ-REST-001..005 | remaining modules | M.5 | TEST-REST-001..006, TEST-MOD-PLATFORM-001 | import/runtime parity |
| REQ-META-001, INV-META-001,002 | Meta cache/Query semantics | M.5,M.6 | TEST-META-SNAP-001..006 + existing parity/hardening | preserved reflection |
| REQ-VER-001 | compiler verification discipline | M.3–M.8 | API Verification Matrix with logs | no fake PASS |
| INV-RES-001 | no invalid constexpr capacity | M.3,M.8 | TEST-FMT-OVERFLOW-001, STATIC-001..004 | intended failure |
| INV-MOD-001,002 | provider/install equivalence | M.2,M.5–M.7 | TEST-MOD-DAG/OWNER, TEST-BLD-* | matches manifest |

**Known traceability limitation:** the table groups related requirements and test ranges; it is a *design-level map*, not evidence that the corresponding tests exist or have executed. Before formal acceptance, expand each grouped range into exact test records with source filename, command, result and commit SHA. A generated trace validator should fail on missing ID definitions, duplicate IDs and orphan tests.

## 12.4 Invariant preservation matrix

| Invariant | Establishment | Preservation points | Detection | Status |
|---|---|---|---|---|
| INV-ARCH-001 graph acyclic | source ownership and import design | every new module/partition edge | graph sort + installed test | NOT RUN against target |
| INV-ARCH-002 single owner | primary/partition declarations | every move/removal of old owner | provider enumeration and source API diff | NOT RUN against target |
| INV-ARCH-003 foundational imports | Color/Format interfaces | every API helper import | direct-import scan | NOT RUN against target |
| INV-FMT-001 capacity | template extent inference | append, section separators, decimal conversion | bounds/static_assert tests | HOST PARTIAL only |
| INV-FMT-002 no ANSI in data | owned bytes and style-run separation | append/ANSI output | plain byte equality | HOST PARTIAL only |
| INV-MSG-001 buffer/sentinel | Message constructor | every append/extension | constexpr size and NUL tests | HOST PARTIAL only |
| INV-MSG-002 run intervals | Fragment append | style offset/coalesce | ordered span property | HOST PARTIAL only |
| INV-MSG-003 ownership | value construction | copy/move/return | lifetime tests | HOST PARTIAL only |
| INV-SEC-001 monotonic stages | diagnostic constructor | context/field/note/help | compile-fail constraints | NOT RUN |
| INV-ANSI-001 reset/plain parity | renderer state initialization | every styled write/finalization | ANSI snapshot tests | NOT RUN |
| INV-NUMTXT-001 decimal range | unsigned magnitude conversion | integer/header padding | min/max boundary tests | NOT RUN |
| INV-ITER-001 invocation precedence | existing Iter helpers | partition moves/diagnostic providers | custom-type tests | existing source only |
| INV-ITER-002 lazy/reference lifetime | existing Iter architecture | view moves/member refactors | ownership/functional tests | baseline CI only |
| INV-DIAG-001 shared presentation | one Format encoder | Diagnostics runtime adapter | parity and implementation review | NOT RUN |
| INV-META-001 Query semantics | existing Meta contract | any Query split | full parity/hardening | baseline CI only |
| INV-MOD-001 provider set | CMake/exported module inventory | every source move | source scan/consumer | NOT RUN final |

## 12.5 Stage A–H execution evidence

| Requested stage | Performed in this drafting turn | Outstanding |
|---|---|---|
| A — Reconnaissance | Read current Development Protocol, architecture/compiler docs, CMake and representative source modules; inspected tree/commits/CI | Authoritative local checkout and all frozen contracts remain to reconcile in full |
| B — Gap analysis | Identified duplicate diagnostic APIs, guessed capacities, unapproved Color/Format signatures, source drift, missing verification | Complete API symbol-by-symbol inventory of all existing Miracle modules relies on source/contracts |
| C — Design resolution | Proposed Color/Format exact APIs, representations, algorithms, module graph and adapter; classified approvals | DEC-COL-001..003 and DEC-FMT-001..004 approved; FMT-005/006 derived technical implementation details, reference verification pending |
| D — API compiler verification | Executed two host C++23 probes: one positive typed buffer, one negative dynamic parameter extent | Pinned clang-cxx26 unavailable; all novel C++26 API probes NOT RUN |
| E — Authoring | Produced this structured multi-part engineering specification with stable IDs | Final ratification pending; adjust signatures after compiler results |
| F — Implementation planning | M.1–M.8 entry/steps/exit evidence; current source reconciliation | Update plan if author changes public API or target branch |
| G — Adversarial review | Identified and repaired obvious declaration-order and literal-spelling defects; reviewed cycle and type-level sizing issues | Full compiler/back-end verification and post-ratification consistency pass needed |
| H — Final readiness | This section states evidence and approval gaps explicitly | No claim of locked/fully implementation-ready exact API |

## 12.6 Adversarial review findings and disposition

**AUDIT-A-001 — duplicate Level/Severity:** resolved architecturally; only one Severity in approved design. **AUDIT-A-002 — Type-level size from consteval parameter:** corrected by requiring sized literals/arrays or separately materialized reflected name; runtime uses sink. Host compiler negative probe confirms the invalid pattern on Clang17. **AUDIT-A-003 — fixed arbitrary Message capacity:** removed; type-level growth specified. **AUDIT-A-004 — inline raw ANSI:** replaced by style runs and explicit renderer. **AUDIT-A-005 — copied dynamic string views in constexpr Message:** lifetime and return-type restrictions identified. **AUDIT-A-006 — bad return type Code before declaration:** declaration order corrected in written API sketch; actual module API uncompiled. **AUDIT-A-007 — one-shot context constraints:** proposed `Current==Heading`, replacing earlier permissive `<=Context`. **AUDIT-A-008 — Color::rgb implicit conversion:** open owner decision; document must not claim rejection of arbitrary ints. **AUDIT-A-009 — Color hex direct/optional:** open owner choice. **AUDIT-A-010 — runtime Format writer exact API:** open approval/verification. **AUDIT-A-011 — installed partition reachability:** requires CMake installed proof. **AUDIT-A-012 — Meta declaration-point sensitivity:** protected via conservative split and parity fixtures. **AUDIT-A-013 — old source tree assumptions:** updated to v0.5.28 partial migration. **AUDIT-A-014 — deleted Design Contract on master:** explicit reconciliation gate. **AUDIT-A-015 — impossible universal `noexcept` with standard streams:** narrowed to no Miracle-authored exceptions and honest std operation specs.

**Known unresolved technical issue:** The exact C++26 class template deduction and type growth for `Message{Severity,Code,summary}` is **not yet compiler-verified**. It is feasible in principle to derive an upper bound from literal extents, Code prefix length and fixed integral width, but the exact template signature must be completed, compiled and potentially revised before compiler-verification acceptance. A reference-toolchain incompatibility is a finding requiring diagnosis, not an excuse to silently return to user-supplied `Message<512>`.

**Resolved `Color::rgb` ergonomic trade-off:** typed `rgb(uint8_t,...)` preserves convenient access but may narrow implicit integral arguments; the separately approved checked `checkedRgb(...) -> optional<Color>` must validate whole input ranges first. Tests must document the distinction rather than promise that `rgb` prevents implicit conversion.

## 12.7 Final implementation-readiness assessment

### Achieved

- The architectural direction for standalone Phase M, Color, Format, unified Severity, styled fragments, inferred append-only storage, plain/ANSI rendering separation, and independent Diagnostics has been recorded from explicit owner approvals.
- Current source evidence has been reconciled sufficiently to avoid recommending that already migrated Numeric/Types/StaticString/Range/Feature work be blindly repeated.
- New Color/Format modules have proposed signatures, input/output semantics, ownership, algorithms, complexity and negative-space analysis, subject to pending decisions.
- An acyclic target module graph, concrete Iter/Feature/Meta migration boundaries, CMake integration rules, M.1–M.8 implementation tasks, test matrices and traceability IDs are present.
- Two narrow host compiler probes have factual outcomes: typed contiguous append PASS and invalid `consteval` parameter-size template pattern correctly rejected. Neither validates C++26.

### Not achieved / must not be claimed

- **No reference-compiler verification of new APIs.** The required pinned Clang fork is absent, and reference build/consumer tests must be run in the author's environment.
- **No owner ratification of the exact signature inventory.** The architecture is approved; precise signatures have pending decisions.
- **No authoritative Git reconciliation of the deleted Phase M Design Contract on current master.** Historical file provenance is known, but current binding status requires the author's decision.
- **No full source implementation or performance results of Color/Format.** This is a specification, not production code or benchmark report.
- **No claim that the number of document pages proves engineering completeness.** The audit favors actionable requirements; any formatted page count is an output-layout metric, not a certification.

### Readiness verdict

**Architectural intent: APPROVED.**
**Technical design specification: substantial DRAFT, subject to decisions and C++26 verification.**
**Exact Color/Format public API: PROPOSED — NOT LOCKED.**
**Reference-compiler validation: NOT RUN (novel APIs).**
**Owner acceptance: PENDING.**
**New-foundation implementation readiness: CONDITIONAL / NOT YET LOCKED.**
**Existing in-progress source migration: may proceed only within previously approved boundaries; do not claim the novel API as frozen.**

The correct next action is to resolve the pending API decisions in §12.1, execute the reference-compiler probe matrix, amend the canonical repo Design Contract for Color and Format, and only then mark the exact new public API locked. The owner remains free to implement already-approved existing migration tasks on milestone commits while those decisions are being reviewed.

## 12.8 Normative and inspected source references

**Project sources, pinned remote commit `e2da473e6dd5637f1f70625738f8378e211a7478`:**

- `docs/DEVELOPMENT_PROTOCOL.md` — approved process, review outcomes, contract precedence and author ownership.
- `docs/ARCHITECTURE.md`, `docs/COMPILER_SUPPORT.md`, `docs/REFERENCE_TOOLCHAIN.md` — C++26-first policy, compiler pin and package identity.
- `CMakeLists.txt`, `CMakePresets.json`, `cmake/Capabilities.cmake`, `cmake/Features.cmake` — build filesets, compiler probes, generated modules and consumers.
- `src/public/numeric/Numeric.cppm`, `types/Types.cppm`, `static_string/StaticString.cppm`, `range/Range.cppm`, `feature/*.cppm` — current extracted foundation/Feature structure.
- `src/public/Miracle.cppm`, `src/public/Iter.cppm`, `src/public/Diagnostic.cppm`, `src/private/Diagnostic.cpp`, `src/private/windows/Memory.cpp`, `src/public/meta/*.cppm` — remaining module migration and runtime renderer evidence.
- `tests/`, `tests/meta/`, `tests/compile-fail/`, `tests/consumer/`, `benchmarks/` — existing test and benchmark infrastructure (their semantics should be inspected in the authoritative checkout).
- GitHub Actions CI run `40987672889` at this exact commit — baseline CI success, not future API verification.

**Historic project-source references:**

- Phase M v1.0 Design Contract from prior Git commit `469083ed1b30e3770667e1fe35473e9f96ed1e7a` (available as provided local file), **absent from inspected current master**.
- `Miracle-Phase-M-Implementation-Guide-v1.1.md` provided as prior proposal, superseded only in its *unapproved* API examples by this new proposal.
- Frozen Foundation and Phase 5 design records, where current authoritative checkout retains them; no frozen historical text is modified by this draft.

**Language and build references (not replacements for compiler testing):**

- C++ working draft `[dcl.pre]` generated `static_assert` message object requirements: `https://timsong-cpp.github.io/cppwp/dcl.pre`.
- C++ modules and partition rules: `https://en.cppreference.com/w/cpp/language/modules`.
- Clang Standard C++ Modules documentation: `https://clang.llvm.org/docs/StandardCPlusPlusModules.html`.
- CMake C++ Modules guide (CMake 4.4+): `https://cmake.org/cmake/help/latest/manual/cmake-cxxmodules.7.html`.
- C++26 generated static assertions proposal P2741 and implementation documentation: `https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2023/p2741r0.pdf`.
- C++ reflection and constant-evaluation support: consult current standard draft and the pinned compiler's implemented feature set before interpreting advanced Meta behavior.

**Host evidence file paths:** `miracle_phase_m_evidence/format_core_host_probe.cpp` (Clang17/C++23 PASS); `miracle_phase_m_evidence/consteval_size_host_negative.cpp` and associated diagnostic log (expected rejection). These are deliberately tiny isolated probes and not a substitute for Miracle source builds.


---

# Appendix A — Concrete runtime presentation writer proposal (for approval)

This appendix preserves the detailed architecture analysis of the event/sink seam, subsequently approved in DEC-FMT-002 on 2026-10-30. The approved premise is that Format has one rendering grammar used both by owned constexpr Message values and by the existing runtime Diagnostics object. The technical problem is different size ownership: an arbitrary `std::string_view` cannot determine a distinct return type from its runtime length. Therefore the Format core needs **one immediate-consumption event writer**. It is not a second Diagnostic class, and no extra Severity or `format::diagnostic` namespace is introduced.

## A.1 Chosen representation and why it is necessary

Use one non-owning `Event` value describing a single presentation action and one short-lived renderer state owned by the caller. The event is a *command to emit*, not a stored Message and not a semantic Diagnostic. It carries references to already-owned strings; the writer consumes them synchronously. A diagnostic-source adapter can generate events during its existing traversal without allocating a parallel event list. An ordinary compile-time Message's constructors/section methods should invoke the **same layout operation** against a fixed-array sink, producing exactly the same plain bytes and style-run boundaries. Reuse at the operation level is a mandatory architectural intent, even if the final API type names differ.

```cpp
// PROPOSED — exact names and signatures still require author ratification
namespace Miracle::format {

struct CodeView final {
    std::string_view prefix{};
    std::uint64_t numeric{};
    std::size_t minimumDigits{3};
};

enum class EventKind : std::uint8_t {
    Text, Heading, Context, Field, Note, Help
};

struct Event final {
    EventKind kind{EventKind::Text};
    std::string_view text{};
    std::string_view key{};       // used for Field
    Style style{};
    Miracle::Severity severity{Miracle::Severity::Note};
    CodeView code{};             // used for Heading
};

enum class WriteResult : std::uint8_t {
    Written, InvalidOrder, SinkFailed
};

struct RenderPolicy final {
    Miracle::Presentation presentation{Miracle::Presentation::Plain};
    Miracle::ColorMode color{Miracle::ColorMode::Never};
};

struct RenderState final {
    RenderPolicy policy{};
    Style activeStyle{};
    Section section{Section::Plain};
    bool started{};
};

template <class Sink>
concept TextSink = requires(Sink& sink, std::string_view text) {
    { sink.write(text) } -> std::same_as<bool>;
};

template <TextSink Sink>
[[nodiscard]] constexpr auto emit(
    Sink& sink, RenderState& state, const Event& event)
    -> WriteResult;

template <TextSink Sink>
[[nodiscard]] constexpr auto finish(Sink& sink, RenderState& state)
    -> WriteResult;
}
```

`TextSink` is a compile-time *structural protocol*. It is not a polymorphic hierarchy, owns no vtable and does not require allocation. A C++ `std::ostream` doesn't directly satisfy it; Diagnostics can create a tiny local adapter with `write(view) -> bool` that performs `output.write(view.data(), streamsize(view.size()))`, checks stream status and forwards the failure bit. A string sink appends bytes to `std::string`. A bounded constexpr message sink copies bytes into statically deduced capacity, recording styled range metadata. All use the same event encoder.

**Critical exception-specification choice.** The concept requires `bool`, not `noexcept`, because a `std::string` adapter or an ostream with throwing exception mask may throw from the standard library. The Format layer introduces no explicit `throw`; `emit` must not be marked unconditionally `noexcept` unless all accepted sinks meet that requirement. If a strict non-throwing API is required for a particular embedded/error path, define a separate constrained `NothrowTextSink` or use a fixed-capacity status-returning sink. Do not convert library allocation failure to silent success or `std::terminate` through an unjustified `noexcept`.

**CodeView is not a second code-domain type.** It is a view of prefix/number/width attributes already owned by the producing Diagnostics descriptor or statically typed `Code`. The producer remains the code authority; no code registration or semantic annotation interpretation occurs in Format. The view may be ephemeral because the `emit` call cannot retain it. With `CodeView` the runtime and compile-time header encoder share exactly the same decimal padding algorithm.

## A.2 Exact state machine and Event dispatch

`RenderState::started` distinguishes an unused renderer from a renderer that has emitted a heading. For ordinary text mode, text events may be written in any order. For diagnostic mode, the first heading event sets section Heading; subsequent context/field/note/help events enforce monotonic order. A second heading must either start a new logical message after `finish`/state reset or return InvalidOrder. An adapter building a diagnostic tree should initialize one RenderState per diagnostic node; do not allow header ordering to leak from parent to child.

```text
emit(sink, state, event):
    if event.kind == Text:
        if state.section != Plain: return InvalidOrder
        return writeStyledFragment(event.text, event.style)
    if event.kind == Heading:
        if state.started: return InvalidOrder
        state.started = true
        state.section = Heading
        return writeHeading(severityName, code, summary)  // ends in LF
    if not state.started: return InvalidOrder
    target = stageOf(event.kind)
    if target < state.section: return InvalidOrder
    if event.kind == Context and state.section == Context:
        return InvalidOrder
    if target > state.section:
        write LF if a nonempty new section needs a blank separator
    write canonical prefix/field label/fragment bytes/LF
    state.section = target
    return Written
finish(...):
    if state.activeStyle != default: write ESC[0m
    reset renderer-local style state to default
    return Written or SinkFailed
```

The algorithm must not partially advance `RenderState` when sink writing fails halfway. Since emitted bytes cannot be retracted from an arbitrary sink, a failure may leave partial output; the renderer must return SinkFailed and document that retries require a fresh sink/state. The easiest invariant is to update `section` after all fragments for an event have successfully been emitted. Style state can still be affected if a sink fails after an ANSI prefix; `finish` should attempt a final reset only when the sink remains writable, but cannot guarantee recovery from an already failing output device. This is an I/O property, not a defect in inferred compile-time storage.

**INV-EVENT-001 [PROPOSED REQUIRED].** For successful output, `section` is monotonic, each consumed view is read only during `emit`, and all style transitions remain local to `RenderState`. For failed output, `SinkFailed` is returned and the state is invalid for further emission until reinitialized.

**INV-EVENT-002 [PROPOSED REQUIRED].** The byte sequence produced by emitting a well-formed diagnostic sequence into a plain runtime sink equals that produced by constructing equivalent Format-owned compile-time Message sections, modulo only data that the runtime model adds (file paths, source caret spans, stacktrace). Equal semantic inputs must not produce different whitespace, severity spellings or numeric padding.

## A.3 Why `Event` is not over-engineering

The alternative is exposing six independently authored `writeHeading`/`writeContext`/`writeField`/`writeNote`/`writeHelp`/`writeText` functions, each capable of implementing its own line separation and failure handling. That is precisely the duplication the project wants to eliminate. One tagged event and one emitter centralize the grammar. A `std::variant` of six event subclasses is possible but adds type machinery, visitor dispatch and template instantiations without useful capability here; a single lightweight aggregate with inactive fields is simpler. A virtual `IEventSink` is rejected because no dynamic polymorphism is necessary. A global string builder is rejected because runtime Diagnostic already owns data and appending to one dynamic buffer would duplicate storage.

**Trade-off:** `Event` contains fields unused for most event kinds. This is a transient stack value with fixed-size, trivially-copyable metadata; storage cost per event is bounded and no event list is required. If benchmarks reveal that all events are emitted from hot paths, a specialized overload family can be considered later without changing formatting semantics. For Phase M the correctness and shared grammar benefit outweighs the few unused bytes.

## A.4 Public API impact and minimum exact test package

If the owner approves this interface, extend Part V's public API inventory with `CodeView`, `EventKind`, `Event`, `WriteResult`, `RenderState`, `TextSink`, `emit`, and `finish`, and replace the provisional `writeFragment`/`writeHeader` family in API-FMT-013 with this single event API. Keep `Message`, `Fragment`, `Style`, `Code`, `Severity`, `ColorMode`, and `Presentation` unchanged. Test that a direct `import Miracle.Format;` consumer can implement a minimal custom sink, emit a header+field+note+help, and compare its output to a compile-time-owned Message's plain text. Validate a failing sink returns SinkFailed; invalid ordering returns InvalidOrder and emits no event body; `finish` resets ANSI after a styled fragment; an `Event` whose string_view points to a local string does not survive the immediate call. An installed-package consumer must compile the concept and the calls; importing only Format should not expose Diagnostics-specific types.

**Owner approval required:** This event API is a concrete reconciliation proposal, not an approved extension. If the owner chooses internal-only routing, prove that Diagnostics in a distinct named module can access the necessary presentation primitives without leaking an uninstalled PRIVATE interface unit into the public graph.

---

# Appendix B — C++26 compiler verification execution pack

## B.1 One-reference-snapshot rule

Verify all novel public APIs with the exact `cxx26-2026.10.27` snapshot and matching libc++/libc++abi, using the repo's configured CMake toolchain and `-std=c++26`. The previous Phase M design may have referenced a different baseline; the current `docs/COMPILER_SUPPORT.md` is the authority for the current build. Record the compiler manifest hash and actual `clang++ --version` output, plus the compiler target triple. Never mix a dynamic `cxx26` development HEAD with the pinned released libc++ in the same proof. A secondary compiler's success can be useful, but must be labeled secondary support rather than changing the reference contract.

## B.2 Representative module consumer fixtures

`module-consumer-01`: single consumer `import Miracle.Color;` and `constexpr auto c = Color::rgb(255,159,67); static_assert(c.toHex()==0xFF9F43);`. Expected: compile/link PASS and the Numeric/Format umbrella is not required.
`module-consumer-02`: `import Miracle.Format;` only; constructs `Fragment` and Message; asserts plain content and styled run counts. Expected: compile PASS and no import cycles.
`module-consumer-03`: `import Miracle.Diagnostics;` only; creates a reflected domain enum, calls `Diagnostic::error`, formats plain and terminal output; uses `Miracle::Severity` without importing Format explicitly if Diagnostics re-exports the owner. Expected: one type identity and correct linked renderer.
`module-consumer-04`: `import Miracle.Iter;` only; uses custom tuple-like and reflected aggregate elements with map/filter; proves templates can instantiate outside library without invisible Detail partition.
`module-consumer-05`: `import Miracle;` only; old representative example compiles with same namespace/public signatures.
`module-consumer-06`: `import Miracle.Meta;` only; runs reflection and Query source/terminal operations, proving no umbrella dependency.

Each consumer fixture is built in three contexts: linked source subdirectory; FetchContent using local checkout; isolated installed find_package. A library-only success is insufficient. Store the actual compiler command from `compile_commands.json` and the return code/log, not just a screenshot of a green CI job.

## B.3 Generated `static_assert` probe

A module-import fixture should build a `constexpr` Format Message and supply it as the C++26 static_assert message argument. The negative case intentionally fails the predicate, and the build harness must inspect the authored summary fragment in the diagnostics. A positive case should compile with the same Message expression and true predicate. A third case uses a long reflected type-name fragment materialized from a `constexpr` reflection view; the copy into the message must retain a valid pointer/size after the temporary adapter's scope. A fourth case intentionally supplies an object whose `data()` points to nonconstant storage to confirm the compiler rejects invalid message bytes rather than falsely accepting them.

**Known language rule:** C++26 `[dcl.pre]` requires the message object to have size and data members convertible to the specified types and constant-expression-readable characters at each position. This does not require the formatting representation to be structurally usable as an NTTP; private arrays can be sufficient for constexpr Message creation. Do not accidentally make structural Message an API promise unless another consumer need arises. A string view borrowed from a runtime `std::string` cannot form a literal message simply because the formatter is `consteval`.

## B.4 Compiler diagnostic normalization

Normalize only known compiler-owned frames: absolute working-directory prefixes, ephemeral BMI paths, terminal color escapes from the compiler wrapper, source line numbers that naturally move after file splitting, and template-instantiation frame spelling that the reference compiler doesn't guarantee stable. Do **not** normalize the Miracle severity, bracketed code, summary, context, field labels, note/help prefixes, or intended line-break sequence. Require a positive substring match for the exact authored code and summary, and count no successful negative test unless the expected failure condition was reached. If a diagnostic is emitted only by a concepts substitution rejection, document why no Miracle-authored message can be portably injected without changing the constraints contract.

Store both raw stderr and normalized golden fixture. The normalization algorithm itself should have tests proving it doesn't accidentally delete `error[STR001]` or the malformed-content cause. Where the compiler emits ambiguous extra messages, verify the authored failure dominates and no unrelated buffer/ODR error appears before it. If the compiler fails solely because a module BMI cannot be found, the test is BLOCKED by environment, not PASS for rejecting invalid user code.

## B.5 API correctness through isolated probes

Compile small probes first, then full module consumers. Suggested order: (a) C++23 sized literal deduction and array copying (host-compatible); (b) C++26 `static_assert` object syntax and generated text; (c) C++26 class template deduction guides for `Message` from `Code<P,W>` and summary literal; (d) return type changes through `.append` and section methods; (e) constrained ill-formed reverse section operations; (f) std module imports of Color and Format; (g) pass ANSI Style/Color metadata into a runtime sink; (h) exported template instantiation from installed consumer; (i) C++26 reflection name materialization; (j) Meta Query declaration-point parity after any optional source split.

For each probe, print exact source revision/compiler/toolchain versions. If a test fails, isolate one language-rule question at a time. Do not patch `std` declarations, ignore `-Werror`, switch to unrelated libc++, or change approved behavior to work around an incomplete compiler unless a separate compiler-support decision authorizes it.

## B.6 Host probe records in this handoff

**P-HOST-001 — `format_core_host_probe.cpp`.** Clang 17.0.0 with system GCC 14 libstdc++ headers, `-std=c++23 -Wall -Wextra -Wpedantic -Werror`; compiled, linked and ran successfully with process exit 0. Test created an owning `Message<C,R>` type deduced from a string literal, appended a styled `Fragment<N>` to produce `Message<C+N,R+1>`, and checked plain length/view, style interval and trivially-copyable property. This establishes the *core language feasibility of type-inferred contiguous storage* under one host setup; no Miracle module or C++26 feature participated.

**P-HOST-002 — `consteval_size_host_negative.cpp`.** Same Clang17/C++23 mode; compilation returned exit 1 as expected. Main diagnostic: `non-type template argument is not a constant expression` at `std::array<char,text.size()>`, with note that parameter `text` has unknown value. The follow-on incomplete-type error is a consequence of the deliberate invalid definition. This substantiates why Format cannot use arbitrary function-parameter `string_view` length as a return-type template argument. This is a host-language negative probe, **not** a substitute for the pinned reference fork.

---

# Appendix C — Design verification checklists for independent reviewers

## C.1 Color review

Inspect the public Color layout and note whether any public constructor accepts unchecked wide integers. Verify all conversions are constexpr without relying on non-constexpr standard functions in the pinned compiler. Inspect numeric sector logic for H=0,60,120,180,240,300,359.999; inspect grayscale path and NaN/infinite handling **before** integer conversion. Confirm HEX parsing uses only ASCII byte decoding with fixed accepted lengths and no locale or allocation. Confirm `toHex` never exposes an alpha value. Confirm `Hsv` is only a conversion carrier, and no color-management/gamma framework has been introduced. Review owner decisions for reverse conversion and shorthand support; if approved, verify corresponding tests and baseline preservation.

## C.2 Format public interface review

Compare header exports against complete API inventory, including factory overloads and deduction guides. Confirm one Severity, one ColorMode and one Presentation type definition. Verify direct import of Format does not require any unrelated module. Inspect Fragment ownership for local array copies, temporary lifetime and embedded NUL. Inspect Message's type-level capacity additions for overflow and inferability. Check that plain data/view is independent of style, null terminated at active length, and that style ranges never overlap. Verify Section ordering is enforced through compile-time constraints rather than runtime exceptions. Verify a diagnostic code's minimum digits and no-truncation rule. Examine how dynamic runtime data is rendered: it must not be copied into guessed fixed-length Message arrays or use a duplicate SGR encoder.

## C.3 Diagnostics adapter review

Walk one output from `Diagnostic::error` factory through reflected code descriptor to Format header event. Verify numeric code and annotation message remain single-source-of-truth. Walk a note with two selectively highlighted fragments through the sink and compare plain and ANSI results. Walk a selected source span containing multiple lines; ensure byte offsets and markers agree with baseline. Walk two nested causes and multiple siblings; confirm output order and indentation. Search for duplicated `severityName`, `severityAnsi`, `renderCode`, manual SGR constants in Diagnostics; any remaining copies should be justified as semantic adaptation rather than competing format implementation. Check that `RenderOptions` retains existing source/trace modes and that Plain/Always still emits no ANSI.

## C.4 Module migration review

Search every translation unit for a module declaration; each new named module has one primary provider. Verify implementation units use the exact owning named module. Check the current Feature partitions are re-exported and that the oversized Query split (if performed) preserves reflection observation semantics and instantiation reachability. Check generated Build/Capabilities/FeatureConfiguration outputs remain in build tree and registered. Build installed consumers outside source tree, including direct imports of Color, Format, Iter and Diagnostics. Review `module Miracle.Core` source-level mismatch in Windows Memory, document platform compile result. Check CMake fileset visibility for any interface partition that a public interface imports, and verify old BMIs are not re-used in a clean acceptance run.

## C.5 Evidence review

Every claimed PASS must contain exact compiler environment and logs/result. If a test was performed only on host Clang17 C++23, it cannot prove the pinned C++26 compiler and matching libc++ combination. If CI passed on a commit before Color/Format existed, it cannot verify those new APIs. Every Blocker and Major finding must be resolved or explicitly waived with scope and rationale by the author before final phase acceptance. Every milestone must have a distinct source revision, exit evidence and review outcome; the batch-review process may consolidate timing but not erase milestone traceability. The acceptance record must distinguish code implementation-complete, verification-complete, accepted and released.


---


# Appendix D — Recovery reconciliation and executable interface seams (2026-10-30)

> **Status:** Technical completion supplement to the v2.0 design, not an approved amendment. The owner has approved the architectural direction of unified Format, standalone Color, styled fragments, inferred growth, constructor-based diagnostic construction, and one Severity. **The exact declarations below are proposed** until the owner accepts a corresponding repository contract revision and the pinned reference compiler verifies them. This appendix records the recovered work and clarifies implementation-critical seams rather than starting another competing API.

## D.1 Recovery provenance and amendment boundary

**REC-001 [SOURCE-EVIDENCED].** The recovered original includes the 109-page v2.0 PDF, the corresponding Markdown source, source/tree reconnaissance against `spwn02/Miracle@e2da473e6dd5637f1f70625738f8378e211a7478`, existing host probes, and appendices A–C. The source describes an already partially migrated v0.5.28 checkout. The old Phase M contract is recoverable from `469083ed1b30e3770667e1fe35473e9f96ed1e7a`, but its historical path is not present on the inspected newer `master`. Its deletion is **not** authority to disregard its previously agreed design; neither is the historical file automatically a current locked version. Preserve provenance and require an explicit owner-approved reconciliation.

**REC-002 [REQUIRED].** Distinguish the following four axes in every new implementation claim: (a) *architectural owner approval*, (b) *exact API signature approval*, (c) *compiler/toolchain validation*, and (d) *repository integration and milestone acceptance*. Owner now approved the seven specified new API behavior choices (b); no pre-Color/Format CI satisfies (c) or (d) for the new APIs. Host probes validate only the language subset they exercise.

**REC-003 [SOURCE-EVIDENCED].** The recovered repository state already contains `.cppm`/`.cpp` migration, standalone Numeric/Types/Range/StaticString, and separately named Feature partitions. Avoid repeating those changes as if they have not been made. The remaining old `Miracle:...` partitions—Iter, Diagnostic, Error, Panic, Fs, Concepts, Memory, Profiling, Bitflags, Debug—still require ownership review. `Miracle.Meta` is already an independent named module with its own partitions. The Windows implementation source `src/private/windows/Memory.cpp` still contains `module Miracle.Core;` in the inspected baseline: a **source-level ownership mismatch**, not a verified Windows compilation failure.

**REC-004 [RECOMMENDED].** Persist the next owner-approved canonical specification at an owner-selected tracked location, preferably `docs/phases/M/DESIGN.md` for normative API/architecture and `docs/phases/M/IMPLEMENTATION.md` for teaching-oriented implementation procedures, with an explicit historical-contract amendment/ADR. Until then this long document is a supporting proposed specification, not a replacement for the repository's source-of-truth policy. No Git write is implied by creating this artifact.

## D.2 The smallest viable single formatting core

A single Format foundation must serve two **representations of input**, not two semantic engines:

1. **Compile-time-owned value:** `format::Message` contains contiguous owned UTF-8/code-unit bytes plus an ordered style-run table. Every append/section method returns a fresh value with type-derived upper bounds. It is suitable for C++26 user-generated `static_assert` message objects and for explicit ANSI or plain rendering.
2. **Runtime semantic adapter:** existing `Miracle::Diagnostic` owns dynamically sized strings, source spans, causes, trace and provenance. It enumerates presentation **events** through Format's one non-owning, immediate-consumption writer. The event path does not introduce a second `Message` class, Severity, terminal-color implementation, diagnostic registry, or ANSI renderer.

A writer emits bytes and maintains only the section/style state needed to render one logical message; it is not a domain-diagnostic object, builder hierarchy, or virtual interface. All heading layout, numeric code width, note/help delimiters, blank-line rules, and ANSI transitions MUST be implemented by a single family of Format algorithms used by both paths.

```text
Miracle.Color [RGB/HSV/HEX values, no terminal API]
    |
    v
Miracle.Format [Severity, Presentation/ColorMode, Code identity view,
    |            Style, Fragment, Message, canonical layout, plain/ANSI writer]
    |\
    | \_____ Miracle.StaticString [unchanged local predicates/staticFormat]
    |_______ Miracle.Diagnostics [code annotations, spans, causes, trace]
                    |
                    +---- Error / Panic / higher-level validations
```

**INV-REC-DIAG-001 [REQUIRED].** The only `Severity` owner is the module `Miracle.Format`, with public spelling `Miracle::Severity`, four existing enumerators and the existing underlying width. The only ANSI encoder and code-padding algorithm are owned by Format. The existing `Miracle::Diagnostic` object and current note/cause/source APIs remain in Diagnostics. `RenderOptions` can remain in Diagnostics because its source/trace controls are domain-specific; its `Presentation`/`ColorMode` members refer to the **same Format-owned types**. Diagnostics must `export import Miracle.Format;` or provide an equivalent compiler-verified API reachability solution so direct consumers can use the public names they previously obtained.

**INV-REC-DIAG-002 [REQUIRED].** There must be exactly one canonical diagnostic presentation grammar. Both a constexpr message and a runtime Diagnostic configured for equivalent plain display MUST produce the same heading, optional code placement, zero padding, field separators, and note/help markers. Source-specific snippets, paths, locations, causes, and trace extension remain runtime Diagnostics responsibilities and need not be available in compile-time Format.

## D.3 Complete proposed low-level event-writer seam

The following chooses one implementable non-OOP **candidate** for the previously unresolved `DEC-FMT-002`; this remains **APPROVED ARCHITECTURAL DIRECTION 2026-10-30; exact C++ declarations NOT RUN**. It is a short-lived rendering protocol, not a second public diagnostic class.

```cpp
// PROPOSED public integration surface of Miracle.Format;
// declaration syntax and exported reachability NOT RUN on pinned C++26.
namespace Miracle::format {
    struct CodeView final {
        std::string_view prefix{};
        std::uint64_t numeric{};
        std::size_t minimumDigits{3};
    };

    struct TextRunView final {
        std::string_view text{};    // immediate consumption, never retained
        Style style{};
    };

    enum class EventKind : std::uint8_t {
        Text, Heading, Context, Field, Note, Help
    };

    struct Event final {
        EventKind kind{EventKind::Text};
        std::string_view text{};
        std::string_view key{};     // only for Field
        Style style{};
        Miracle::Severity severity{Miracle::Severity::Note};
        CodeView code{};           // only for Heading
    };

    enum class WriteResult : std::uint8_t {
        Written, InvalidOrder, SinkFailed
    };

    struct RenderPolicy final {
        Miracle::Presentation presentation{Miracle::Presentation::Plain};
        Miracle::ColorMode color{Miracle::ColorMode::Never};
    };

    struct RenderState final {
        RenderPolicy policy{};
        Style activeStyle{};
        Section section{Section::Plain};
        bool started{};
        bool contextSeen{};
    };

    template<class Sink>
    concept TextSink = requires(Sink& sink, std::string_view bytes) {
        { sink.write(bytes) } -> std::same_as<bool>;
    };

    template<TextSink Sink>
    [[nodiscard]] constexpr auto emit(
        Sink& sink, RenderState& state, const Event& event)
        -> WriteResult;

    template<TextSink Sink>
    [[nodiscard]] constexpr auto finish(Sink& sink, RenderState& state)
        -> WriteResult;
}
```

**REQ-REC-EVENT-001 [PROPOSED].** `TextSink::write(bytes)` consumes the view before returning; `true` means the complete byte sequence was accepted, `false` means emission failed. Format never retains `Sink`, `string_view`, or a pointer to Event. The owner is responsible for adapters for `std::ostream`, `std::string`, and the compile-time fixed storage backend. `std::ostream::write`/`std::string::append` might throw under standard-library or stream exception policies; no unconditional `noexcept` declaration is permitted for this generic writer. Miracle's policy bans **custom format-construction exceptions**, not the possibility of an external allocating sink reporting failure according to its own contract.

**REQ-REC-EVENT-002 [PROPOSED].** The first event of a diagnostic message must be `Heading`. It emits exactly one complete header line and sets the state to Heading. Subsequent sections must follow `Context`, `Field`, `Note`, `Help` order; fields/notes/help repeat; context is one-shot. `InvalidOrder` must not alter the sink or state. A separate ordinary text stream begins with `Text` and stays in Plain mode. A second Heading needs a fresh state after the previous logical message is completed. Keep the per-message state small and trivially copyable; do not materialize a dynamic `std::vector<Event>`.

**REQ-REC-EVENT-003 [PROPOSED].** A `Style` carries terminal color intent but never raw ANSI. Format writes *plain* bytes when `Presentation::Plain` OR `ColorMode::Never`; this takes precedence over `ColorMode::Always`. In Terminal mode with colors allowed, Format computes SGR sequences using a single deterministic encoding path. Preserve old `ColorMode::Automatic` behavior until a separate approved policy permits a new TTY detection scheme. `finish()` resets active styling if needed, returns `SinkFailed` if bytes cannot be emitted, and never appends a duplicate newline beyond the grammar's own final line.

**REQ-REC-EVENT-004 [PROPOSED].** `emit()` must reject structural misuse *before* writing bytes. It may calculate the next transition and validate the event first; then serialize one event. A sink failure partway through an event is irreversible for a stream; the renderer returns `SinkFailed`, and the caller must not misreport the output as complete. A sink supporting atomic writes may provide stronger guarantees, but Format must not assume transactional output from arbitrary sinks.

**REQ-REC-EVENT-005 [PROPOSED].** `CodeView` is a borrowed presentation projection of a single domain-owned code identity, not a registry. `format::Code<P,W>` can be converted locally into `CodeView` for rendering; Diagnostics uses the same view from its reflected `DiagnosticCodeDescriptor`. Formatting receives prefix, numeric value and minimum digit width only. This avoids importing Diagnostics into Format. Numeric code widths in Format are minimum widths, not fixed truncating widths.

### D.3.1 Encoder algorithm and failure boundaries

```text
emit(sink, state, e):
  if e.kind == Text:
      if state.started OR state.section != Plain: return InvalidOrder
      encodeStyledText(sink, e.text, e.style, state.policy, state.activeStyle)
      if sink failed: return SinkFailed
      return Written
  if e.kind == Heading:
      if state.started: return InvalidOrder
      validate header code/summary domain-neutral syntax
      encode severity + optional '[' + prefix + paddedUnsigned + ']' + ': '
      encode e.text + '\n' using Format's style engine
      on complete success: state.started=true; state.section=Heading
      on partial sink failure: return SinkFailed, leave state unusable
  else:
      if !state.started: return InvalidOrder
      target = stageOf(e.kind)
      if target < state.section: return InvalidOrder
      if e.kind == Context and state.contextSeen: return InvalidOrder
      if target > state.section: emit exactly one blank-line separator
      emit section-specific prefix and the text/key/value fragments, then '\n'
      if complete: update state.section=target, state.contextSeen |= Context
      else: return SinkFailed; caller must abandon the partial output
```

In production, the branch that handles sink failure needs an explicit terminal `Failed` state or an equivalent documented rule preventing retry after partial emission; `RenderState` above intentionally omits that field and is **incomplete until `DEC-FMT-002` is approved**. Prefer adding `bool failed{}` and rejecting future `emit()` calls with `SinkFailed`, rather than requiring callers to intuit a retry rule. `finish()` is idempotent when nothing remains to reset; partial failure ends the logical rendering session without promising recovery.

**Time complexity:** O(bytes emitted + events + styled run count), with bounded additional state and no event-list heap allocation. The exact number of bytes generated by ANSI formatting is proportional to the number of style transitions. **Space:** O(1) per active sink/render state excluding caller-owned output storage. Source-tree recursion and cause stack remain outside Format.

## D.4 Compile-time Message constructors, capacity inference, and deduction mechanics

The user-approved surface uses **constructor-based construction** and no separate diagnostic Builder. A C++ class template deduction guide can compute a *type-level upper bound* from sized literal and `Code<P,W>` template parameters. Exact active text length is then a `constexpr` value stored inside the returned Message. Public callers do not specify the capacity.

The following is a complete **shape test** for the required constructors and deduction guides. It omits implementation bodies, style run copying and source-specific domain semantics deliberately; each omitted body must follow the corresponding normative algorithm in Parts V/VII. It does not pretend to be a production interface.

```cpp
// PROPOSED, NOT reference-compiler-verified
namespace Miracle::format {
  template<std::size_t PrefixBytes, std::size_t MinDigits>
  struct Code {
      std::array<char, PrefixBytes> prefix{};
      std::uint64_t numeric{};
  };

  template<std::size_t Width = 3, std::size_t N>
  [[nodiscard]] constexpr auto code(const char (&name)[N],
                                    std::uint64_t number)
      -> Code<N - 1, Width>;

  template<std::size_t Capacity, std::size_t Runs,
           bool Diagnostic = false,
           Section Current = Section::Plain>
  class Message {
  public:
      template<std::size_t N>
      constexpr explicit Message(const char (&literal)[N])
          requires(!Diagnostic);

      template<std::size_t P, std::size_t W, std::size_t N>
      constexpr Message(Miracle::Severity level,
                        Code<P, W> code,
                        const char (&summary)[N])
          requires(Diagnostic && Current == Section::Heading);

      [[nodiscard]] constexpr auto size() const noexcept -> std::size_t;
      [[nodiscard]] constexpr auto data() const noexcept -> const char*;
      [[nodiscard]] constexpr auto view() const noexcept -> std::string_view;
      [[nodiscard]] constexpr auto styles() const noexcept
          -> std::span<const StyleRun>;
  };

  template<std::size_t N>
  Message(const char (&)[N])
      -> Message<N - 1, 0, false, Section::Plain>;

  template<std::size_t P, std::size_t W, std::size_t N>
  Message(Miracle::Severity, Code<P, W>, const char (&)[N])
      -> Message</* bounded header bytes derived from P,W,N */,
                 /* style run maximum */, true, Section::Heading>;
}
```

**Correction to a common misconception:** `consteval` on a function does *not* make its ordinary `std::string_view` parameter value a constant-expression template argument. `std::array<char, parameter.size()>` remains invalid. Inference comes from `N`, `P`, `W`, or an explicit NTTP carrying a value. For reflection display names, first make a local `constexpr` result (or materialize an NTTP in caller code) whose `.size()` is constant-expression usable, then copy its bytes into an owned `Fragment<N>`; Format does not import Meta to discover type names.

**INV-REC-CAP-001 [REQUIRED].** The capacity bound is a pure function of compile-time template parameters and documented worst cases; the active length is a value. No append writes past `Capacity`, no style run writes past `Runs`, and the null-terminator resides in storage of size `Capacity+1`. Template arithmetic must check for `std::size_t` overflow **before** materializing a potentially gigantic `std::array`; otherwise it can wrap around and corrupt storage or fail with an unrelated error. Reject overflow at template constraint/static assertion time. The module never silently drops a fragment, style run, summary, code or help.

**INV-REC-CAP-002 [REQUIRED].** `append(Fragment<N>)` returns a fresh message with capacity at least `Capacity+N` and run upper bound at least `Runs+1`, preserving previous bytes and style intervals. A zero-length or default-style fragment need not consume a run. Coalescing contiguous same-style runs is recommended only if it preserves observable `styles()` semantics that the owner approves. The authored plain bytes and order must remain unchanged. Repeated single-character append can cost O(total bytes squared); a bulk factory or multi-fragment composition may optimize future hot paths if measured, without exposing user-guessed capacities.

**INV-REC-CAP-003 [REQUIRED].** The `char[N]` constructor's `N-1` convention must not silently discard an ordinary **unterminated** `char` array. Before API lock, decide whether (a) literal-style overload is `consteval` and requires a final NUL, (b) non-NUL arrays are rejected with a documented precondition/invariant check, or (c) only an NTTP literal/fixed-string provider is admitted. For exact byte arrays, the separate `std::array<char,N>` fragment overload retains *all* N bytes. Whatever choice is approved requires negative tests. `N-1` is not automatically correct for every `char[N]` caller.

### D.4.1 Section-state return type and external reachability

One extensible Message template can track diagnostic state in the type, with `Current` updated by every section method. This enforces ordering before evaluation and prohibits an invalid chain at overload-resolution time. A representative operation has the form:

```cpp
// PROPOSED member shape; exact return bound to be derived mechanically.
template<std::size_t N>
[[nodiscard]] constexpr auto note(const char (&text)[N]) const
    -> Message</*capacity + separators + literal content*/,
               /*run upper bound*/,
               true, Section::Notes>
    requires(Diagnostic && Current <= Section::Notes);
```

`context` requires `Current == Heading`; `field` requires `Current <= Fields`; `note` requires `Current <= Notes`; `help` requires `Current <= Help`. A plain Message has only append/ordinary text operations, and diagnostic Message cannot bypass the ordered layout with a generic public `.append()`. For all these functions, the *previous* value remains unchanged, and the returned value includes the previous text/style metadata exactly once. A reference to an internal previous buffer must not escape into the newly returned message. The compiler should reject a field after a note via an unsatisfied constraint; it should not need to enter a throwing constructor.

### D.4.2 Syntax and type deduction: recovered host evidence

**P-HOST-003 (new, PASS, limited).** `format_ctad_section_host_probe.cpp` was compiled and executed using host Clang 17.0.0, `-std=c++23 -Wall -Wextra -Wpedantic -Werror`, system GCC 14 standard headers, x86_64 Linux. It constructs a diagnostic Message with `Severity`, a code template deduced from `"ITR"`, and a summary literal, uses a deduction guide to infer the bounded Message type, appends a note that changes the Section template parameter, and verifies bytes and a rejected backward-field expression in a concept. Process exit 0. This **does not** prove C++26 generated `static_assert` or Miracle module reachability; the sketch is intentionally local and lacks style-run implementation.

**P-HOST-001 (re-executed, PASS, limited).** The earlier `format_core_host_probe.cpp` compiled/linked/executed successfully on the same host setup. It verifies styled fragment ownership and contiguous typed growth only. The implementation remains a miniature feasibility probe rather than the contract's complete API.

**P-HOST-002 (re-executed, EXPECTED REJECTION, limited).** `consteval_size_host_negative.cpp` fails host compilation with `non-type template argument is not a constant expression` for `std::array<char,text.size()>`. This is the expected negative test, not a FAIL for a proposed valid program.

## D.5 Color API accuracy and finite-input algorithms

The approved Color direction is **one standalone `Miracle.Color` module and a small RGB8 representation, RGB/HEX/HSV conversion**, with Format consuming it. The precise spelling and optional-return choices in Part V were proposed after approval of that direction and remain provisional. These details can materially affect call-site ergonomics; the owner must choose them before calling the public API locked.

**DEC-COL-001 [RESOLVED 2026-10-30].** `Color::rgb(std::uint8_t, ...)` permits implicit integral conversion before the function call; a caller typing a runtime `int` value of 300 may get modulo-narrowed data. Either use checked full-range integral parameters and return `optional<Color>`, or document the typed-channel guarantee and provide a separate checked factory. Do not promise silent invalid-input rejection through an 8-bit parameter list.

**DEC-COL-002 [RESOLVED 2026-10-30].** `Color::hex(std::uint32_t)` may be a convenience constructor returning Color or a checked factory returning `optional<Color>`. The former requires clearly documented out-of-range behavior; the latter is safer for unknown runtime input but less ergonomic for known literals. `Color::parseHex` should remain checked. A call such as `Color::hex(0xFF8040)` is deliberately held as a **proposed spelling**, not compiler-confirmed contractual usage.

**DEC-COL-003 [RESOLVED 2026-10-30].** Both `Color::toHsv()` and short `#RGB` parsing are small enough for Phase M, but the exact inclusion decision wasn't explicitly approved. `Color::fromHsv()` and full six-digit hex conversion are part of the approved direction. Make the owner choice explicit rather than assuming the scope expansion is automatically accepted.

**Algorithmic finite-input rule (recommended).** For an `Hsv` record with `double h,s,v`, the conjunction `h>=0 && h<360 && s>=0 && s<=1 && v>=0 && v<=1` rejects NaN and infinities via ordinary IEEE comparison behavior (subject to environment/fast-math assumptions). If `-ffast-math` or non-IEEE arithmetic is allowed, test the compiler mode separately and do not promise behavior not guaranteed by actual flags. After range validation, sector `static_cast<int>(h/60)` is in `[0,5]`. For channel quantization, multiply clamped `[0,1]` by 255 and use `static_cast<int>(y*255+0.5)` to select nearest with half-up ties; document floating-point rounding sensitivity at boundaries.

**P-HOST-004 (new, PASS, limited).** `color_math_host_probe.cpp` compiled and ran with the same host Clang17 C++23 settings. Constant-evaluation tests cover packed RGB, high-bit rejection, `#RGB` and `#RRGGBB` parsing, malformed input, six HSV hue sectors, grayscale, and rejection of invalid hue, saturation, and NaN. No C++26 module or pinned toolchain was exercised, and the probe does not validate reverse `Color::toHsv()` or comprehensive 24-bit RGB roundtrip properties.

## D.6 Unicode, byte-view and control-character contract

The proposed Format content representation is a **byte sequence** with `std::string_view` observers and ANSI style spans expressed as byte offsets. For Phase M, this means:

- A code unit has width one for indexing; the renderer does not compute Unicode grapheme columns or display-cell width. Styles may cover a subset of UTF-8 code units if the caller supplies such a range; Format does not validate Unicode code-point boundaries unless an approved extension adds that guarantee.
- `data()`/`view()` are always plain bytes. `render(Plain)` emits those exact bytes in order. ANSI styling adds transport sequences only to the separately rendered stream; it does not rewrite or escape the plain byte payload.
- A `Fragment` created from `std::array<char,N>` may include embedded `\0`; `view().size()` remains N. A constructor documented as taking a null-terminated literal excludes the **final** terminator; it must not interpret an *interior* NUL as an end-of-string marker.
- User-controlled ESC, LF, CR, TAB, BEL and other controls in arbitrary text are **not implicitly sanitized by the general Message representation**. A source-specific diagnostic adapter MUST escape untrusted controls before using them in header summaries, field names, location labels or snippets when injection would undermine canonical layout or terminal safety. Whether Format provides a dedicated public escaping helper is an unresolved API choice, not permission to duplicate incompatible escape grammars.
- Label alignment is measured in code-unit bytes, not terminal columns; formatted compiler-authored messages with Unicode should avoid claiming character-grid alignment unless display width is known. Full Unicode shaping/terminal grapheme support remains deferred to Core Revisit.

**DEC-FMT-004 [RESOLVED 2026-10-30].** For canonical diagnostic content, choose a contractually stable escaping grammar, e.g. C-style ASCII escapes (`\\n`, `\\r`, `\\t`, `\\x1B`) for disallowed control bytes in single-line summaries/context/field keys. Render detail text intended to be multi-line as separate explicit events rather than inserting arbitrary raw LFs into a single-line field. The **choice of specific escape spellings** must be approved because it changes authored message snapshots. Until then, do not claim an all-input canonical message grammar.

## D.7 Invariants and property generators for final verification

| ID | Scope | Generator / fixture | Required property | Evidence gate |
|---|---|---|---|---|
| TEST-REC-COLOR-001 | HEX | 24-bit RGB values including 0,1,255 boundaries | unpack→pack is identity for valid values | reference constexpr suite |
| TEST-REC-COLOR-002 | HEX short | all 4,096 combinations `#RGB` | each nibble maps to 17×n, no loss of middle nibble | reference constexpr or runtime suite |
| TEST-REC-COLOR-003 | HSV | sectors, both sides of boundaries, NaN/Inf/negative | valid values produce bounded RGB; invalid return empty | pinned reference result |
| TEST-REC-COLOR-004 | RGB→HSV→RGB | representative/all RGB8 triples as time budget allows | rounding bound explicitly agreed before locking | measured reference results |
| TEST-REC-MSG-001 | Capacity | lengths including 0,1,large and mixed sized literals | `size<=capacity`, sentinel zero, bytes identical | reference constexpr suite |
| TEST-REC-MSG-002 | Style runs | random sequences of plain/styled/empty fragments | valid ordered non-overlapping half-open ranges | reference constexpr suite |
| TEST-REC-MSG-003 | Chain typing | append ten differently sized fragments | result type reflects sufficient capacity; original unchanged | reference consumer compile |
| TEST-REC-MSG-004 | Section | every illegal backward transition | rejected by constraint, not unrelated compile error | compile-fail harness |
| TEST-REC-MSG-005 | Large values | signed minimum, uint64 maximum, long code prefix | correct decimal and prefix padding; no narrowing | reference constexpr suite |
| TEST-REC-ANSI-001 | Styled segments | alternating terminal 16/indexed/RGB/default | correct SGR ordering and reset; no bleed | raw-byte golden fixtures |
| TEST-REC-ANSI-002 | Plain | same style inputs with Plain/Always and Terminal/Never | byte-identical to `view()` with no ESC | raw-byte equality |
| TEST-REC-EVENT-001 | Event writer | heading, field, note, help stream | matches owned Message plain bytes/layout | runtime and constexpr parity |
| TEST-REC-EVENT-002 | Writer misuse | no heading, repeated context, field after note, second heading | InvalidOrder, sink unmodified for that event | writer unit tests |
| TEST-REC-EVENT-003 | Partial sink | fail during a colored run, fail on reset | SinkFailed, no false successful completion | fault-injecting sink |
| TEST-REC-SEC-001 | Controls | ESC/newline in summary and field key | agreed escaping grammar blocks layout injection | normalized golden fixtures |
| TEST-REC-IMPORT-001 | Modules | direct import Format/Color/Diagnostics/StaticString | exactly one Severity, no upward import/cycle | pinned CMake consumer |
| TEST-REC-API-001 | API syntax | complete public declarations, deduction guides, overloads | compiler accepts intended use, rejects misuse | pinned compiler logs |

**Test traceability:** Each TEST-REC row applies to `REQ-REC-*`, `INV-REC-*`, or a corresponding Part V/VII requirement. These rows are *test specifications*, not executed results. The host probes are evidence of limited subsets only, as explicitly recorded in §D.4.2 and §D.5.

## D.8 Readiness verdict and next author actions

**Recovered and preserved:** the large v2.0 design, technical appendices, 109-page original PDF, and new narrow host probes. **Architecture approved:** named-module migration, strict no-upward-dependency rule, Color foundation, unified Format, one Severity, inferred append-only Message, fragment styling, ANSI/plain split, Diagnostics as adapter. **Not automatically approved:** exact Color signatures, CTAD-based Message spelling, runtime event-writer public API, and the control-character/grammar decisions. **NOT RUN:** all new API checks on the pinned clang-cxx26 reference toolchain and matching libc++/CMake 4.4. **Acceptance:** pending explicit author decisions and source-level implementation evidence.

Before implementation is declared unblocked, settle the pending `DEC-COL-*` and `DEC-FMT-*` decisions, accept the exact public API inventory, record an explicit contract amendment/ADR in the authoritative repository, and run the reference probes in the actual toolchain. The author may continue their independently committed migration milestones at their discretion, but the final review must never infer approval from a passing pre-Format baseline. The next independent reviewer should continue from this version and the exact recovery evidence paths, **not** reconstruct the design from conversation summaries.


---

**Historical end-marker from v2.1 retained for traceability; superseded by v2.2 author-approved reconciliation and outstanding reference-toolchain verification.**


---

## Post-reconciliation disposition (author-approved, 2026-10-30)

Parts I–XII and Appendices A–D retain the technical analysis and pre-approval decision provenance from draft v2.1. All decisions enumerated in the **Normative reconciliation notice** are now approved; earlier **PENDING** cells for those identifiers are historical and must not be interpreted as unresolved owner questions. DEC-FMT-005 (StyleRun visibility) and DEC-FMT-006 (CodeView bridge) are implementation-level consequences of the approved single-renderer design and are specified as mandatory for the target interface, subject to reference compiler verification. No new production API has been compiler-verified by this documentation-only revision. The required next gate is source implementation with pinned `clang-cxx26-2026.10.27` tests, not another architecture-approval round.
