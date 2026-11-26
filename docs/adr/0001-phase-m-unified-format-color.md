# ADR-0001: Independent Color and unified Format presentation

**Status:** Accepted architectural decision — implementation and compiler verification pending
**Date:** 2026-10-30
**Applies to:** Phase M (`docs/phases/M/DESIGN.md` v2.0)
**Decision authority:** Miracle project author.

## Context

The original umbrella module mixed low-level compilation utilities with higher-level Diagnostics. Foundational `static_assert` errors require consistent rich rendering without creating `StaticString → Diagnostics → StaticString` or Meta/Validation cycles. A separate DiagnosticFormat with its own severity, code catalog or renderer would duplicate `Miracle::Diagnostic` semantics. The project also needs selectively colored fragments, plain compiler messages and type-inferred compile-time storage.

## Alternatives considered

1. Keep Diagnostics as sole renderer and duplicate literal diagnostics in low-level modules. Rejected: inconsistent presentation, no reusable bootstrap layer.
2. Introduce a narrow `DiagnosticFormat` builder with its own Level/Severity. Rejected: parallel semantic models and future API friction.
3. Provide a style-only format protocol with independent Diagnostics formatting. Rejected: two layout implementations can drift.
4. **Selected:** `Miracle.Color` defines tiny standard-library-only RGB/HSV/HEX primitives. `Miracle.Format` imports Color and owns one shared presentation grammar, one Severity, styled fragments, inferred compile-time Message and event/sink rendering. Diagnostics remains the semantic model and adapts source spans/causes to Format.

## Decision and consequences

- `Miracle.Color` has no Miracle imports; `Miracle.Format` may import `Miracle.Color` and **no other** Miracle named module. This explicitly supersedes the original absolute Format no-import rule.
- Move the existing `Miracle::Severity` owner to Format while retaining namespace/name/values. Preserve the existing Diagnostics public ergonomics and diagnostic domain authorities.
- One append-only owned Message, no public Builder duplication, no nested diagnostic namespace, no caller-guessed capacity; plain text and style runs are separate. `data()`/`view()` always plain.
- `format::Event` with a structural TextSink is a synchronous adaptation protocol, not a second Diagnostic object, a virtual hierarchy, or an owning event list. Explicit sink failure and deterministic output grammar are mandatory.
- Formatted diagnostics end with a single LF, escape controls in single-line parts, and share layout across constexpr and runtime forms. ANSI remains terminal-only; styles can color selected fragments.
- `Color::rgb(uint8_t...)` remains convenient; `checkedRgb(...)` validates wider integral inputs. HEX is checked and optional. Short/full HEX and RGB↔HSV are included.
- `StaticString` continues to own its API and string operations; rich formatting extension belongs to Core Revisit.
- Moving module ownership is ABI/BMI-incompatible; source-level compatibility and clean rebuild tests are required.

## Verification and migration impact

`DESIGN.md` and `IMPLEMENTATION.md` define exact API targets and test matrices. The new public APIs have not been compiled with pinned `clang-cxx26-2026.10.27`; verification is a Phase M acceptance gate, not an architectural approval prerequisite. New Format/Color changes must be committed by the project author.

## Historical provenance

Earlier Phase M contract v1.0: Git commit `469083ed1b30e3770667e1fe35473e9f96ed1e7a`, formerly `docs/phases/M/DESIGN.md`.
Current consulted snapshot: `e2da473e6dd5637f1f70625738f8378e211a7478`.

## Amendments recorded during implementation (2026-11-26)

Owner decisions (see `docs/phases/M/PROGRESS.md`, findings 1-8):

- `Miracle.Color` and `Miracle.Format` may import `Miracle.Numeric` (alias-only leaf) in addition to the standard library; Format still imports Color and no other higher-level module. `DESIGN.md` is amended accordingly.
- Control bytes (C0 and DEL) in headings, context, field keys/values, notes, help, paths and snippet source are escaped as `\n`, `\r`, `\t`, `\0`, `\xNN` (DEC-FMT-004 spelling). Raw `Text` events and `Message::append` are not escaped.
- Blank-line rule: the `--> context` line follows the heading directly (§5.3 example); every later section is preceded by one blank line (ALG-FMT-002).
- `Error::display` renders through `format::AnsiWriter`; chain labels use indexed colour 3 (`ESC[38;5;3m`), not `ESC[33m`.
- `Miracle.StaticString` imports `Miracle.Format` to author its hard errors; Format never imports StaticString.
- `Debug` and `Bitflags` moved to `src/public/debug/` and `src/public/bitflags/`.

Implementation choices awaiting owner confirmation:

- INV-REC-CAP-003 (owner decision, 2026-11-26): literal-style overloads accept any `char[N]` and truncate at the first NUL; the last array byte is always the terminator position. This is none of the spec's options (a)-(c) and has no negative test; the unterminated-array compile-fail fixture was removed. Exact byte arrays use `fragment(std::array<char, N>)`.
- Feature (`FEA`) and Iter (`ITR`) hard errors were moved to Format-authored messages with their original wording as the summary; Meta Query hard errors and `DiagnosticCode` concept failures keep their original text.
- Exported helpers beyond the spec inventory (owner chose to hide; kept exported only where `Miracle.Diagnostics` consumes them): `AnsiWriter`, `writeCode`, `writeEscaped`, `escapedSize`, `severityName`, `severityStyle`. Removed: `Message::empty/capacity`, `Code::size/data/view/capacity`.
- Implementation helpers are unexported declarations in the module namespace; no `detail` namespaces are used.
