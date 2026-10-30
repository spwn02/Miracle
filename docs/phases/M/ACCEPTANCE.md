# Miracle — Phase M acceptance and review matrix

**Status:** VERIFICATION PENDING — not a signed acceptance report.
**Contract:** [`DESIGN.md`](DESIGN.md) v2.0; [`IMPLEMENTATION.md`](IMPLEMENTATION.md) v2.2.

A milestone commit is an implementation checkpoint, not automatic reviewer acceptance. For the agreed batched review, the author supplies the M.1–M.8 commit range, locked source revision, pinned reference compiler identity, build flags, logs, benchmark hardware/environment and failures/exceptions. Do not substitute host C++23 probes for reference C++26 validation.

| Gate | Required evidence | Status at reconciliation |
|---|---|---|
| A-M-001 | Target module inventory, acyclic imports, sole owning module for each public entity | NOT RUN for final tree |
| A-M-002 | Tracked source suffix inventory including generated templates/tests/examples/benchmarks/probes/editor scripts | NOT RUN for final tree |
| A-COL-001 | Color RGB checked ranges, HEX parse and conversion; HSV sectors/round trips/boundaries | NOT RUN |
| A-FMT-001 | Message/Fragment CTAD, sized growth, style runs, `static_assert` constant-expression operand | NOT RUN |
| A-FMT-002 | Source-invariant constructors, monotonic context/field/note/help order with negative builds | NOT RUN |
| A-FMT-003 | Plain and ANSI fragments, indexed + RGB colors, no ESC in `data()`/`view()` | NOT RUN |
| A-FMT-004 | Event/TextSink parity with owned Message, invalid order, partial stream failure and reset | NOT RUN |
| A-FMT-005 | Control escaping, exactly one trailing LF, deterministic section layout | NOT RUN |
| A-DIAG-001 | Existing Diagnostic APIs/enum annotation reflection/source spans/causes/stacktraces preserved | NOT RUN |
| A-META-001 | Meta Query caching, declaration-point observation, laziness and negative tests unchanged | NOT RUN |
| A-CONS-001 | Direct import each named module; `import Miracle;`; installed `find_package`, FetchContent, add_subdirectory | NOT RUN |
| A-CI-001 | Pinned Clang+CMake Debug/Release and test/compile-fail suite; GCC compatibility separately | NOT RUN |
| A-PERF-001 | Repeated cold/incremental build and BMI/peak memory results before/after | NOT RUN |
| A-SEC-001 | CodeQL inventory counts `.cppm` and `.cpp` accurately; no extraction claims without evidence | NOT RUN |
| A-OWN-001 | Explicit author sign-off after reviewer finding classification and exceptions | PENDING |

## Review template

For each milestone: `commit SHA`, `contract clause`, `test fixture`, `observed outcome`, `finding severity (Blocker/Major/Minor/Observation)`, `corrective property`, and `review outcome (Accepted/Changes requested/Verification pending/Contract revision required)`; include at least one representative negative test for high-risk constraints and report any limitations explicitly.

**Release state:** No Phase M completion, acceptance or version tag is claimed by this document.
