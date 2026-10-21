**Frozen Design and Implementation Contract**

| **Status**             | FROZEN - normative for Phase 5.4 implementation |
|------------------------|-------------------------------------------------|
| **Version**            | 1.0                                             |
| **Freeze date**        | 2026-10-02                                      |
| **Reference compiler** | cxx26-2026.09.30.2                              |
| **Reference revision** | 998b670e39292d6b6c1b8ffa94359e802020206a        |

| **Contract authority.** This document freezes the Phase 5.4 scope, architectural invariants, performance philosophy, completion gate, and compiler-defect handling. Public semantics from the reviewed Phase 5.3 baseline remain authoritative; Phase 5.3 implementation machinery does not. |
|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

**Design thesis.** Miracle.Meta Query is treated as a compile-time compiler: ergonomic Iter-like syntax lowers into immutable value IR, which is normalized, segmented by genuine type boundaries, fused, and executed consteval with no residual runtime machinery.

**Negative-cost target.** Generated code must match optimally handwritten code. Frontend cost should approach the irreducible handwritten floor, while realistic repeated Meta workloads should become cheaper than independently repeated raw \<meta\> by reusing reflection universes, interned IR, normalized plans, and cached reusable results.

# 0. Governing principles

## 0.1 Public semantics are locked; machinery is not

Nothing introduced during Phases 5.1-5.3 is implementation-sacred. Internal machinery may be replaced whenever semantics are preserved and the replacement improves compiler work, compiler memory, generated code, object representation, diagnostics, or conceptual simplicity.

The following are explicitly non-contractual implementation details: **QueryState, tuple-backed stage storage, recursive stage walkers, stage-as-type IR, central semantic dispatch, and duplicated static/stateful stage hierarchies.**

## 0.2 Performance is multidimensional correctness

runtime:  
parity with optimally handwritten code  
  
compiler frontend:  
approach optimally handwritten irreducible cost  
  
realistic repeated workloads:  
less compiler work than independently repeated raw \<meta\>

Strict negative cost against an omniscient handwritten implementation of Miracle's own final algorithm is not a meaningful target; that implementation can reproduce the same algorithm. Parity is the theoretical floor. Negative cost is expected at workload scale when Miracle can reuse information that independently handwritten code recomputes.

## 0.3 Language facilities before library machinery

- Prefer direct C++26 language facilities when they preserve semantics and measure better: reflection/splicing, reflect_constant, reflect_object, reflect_invoke, define_static\_\*, define_aggregate, pack indexing, expansion statements, explicit-object parameters, and consteval exceptions.

- Avoid hot-path template-library machinery such as tuple, tuple_cat, index_sequence, recursive traits, generic ranges adaptors, unnecessary std::invoke, and trivial forwarding wrappers where direct syntax expresses the same operation more cheaply.

- Do not introduce compatibility paths solely because the reference compiler is incomplete. Compiler defects are handled as compiler defects.

## 0.4 Compiler-defect rule

If valid Phase 5.4 implementation work exposes a compiler crash, ICE, assertion, miscompile, or incorrect rejection attributable to the reference compiler, Miracle implementation stops at that point. Reduce to a minimal reproducer where practical and file/report it against spwn02/clang-cxx26 before continuing.

No workaround, legacy path, feature downgrade, or compatibility implementation may be added merely to bypass that defect. A facility genuinely absent from C++26 is not a compiler defect and must be documented as a language boundary instead.

# 1. Frozen Phase 5.4 scope

## 1. Complete the applicable Meta/Query surface

- Close contracted Query vocabulary gaps.

- Make every supported reflection source compose correctly.

- Eliminate implemented-for-common-case-only behavior.

- Harden exact reference and value semantics.

## 2. Foundational transformations needed by Validation and Traits

- Build only the reflection/structural transformation primitives the next systems genuinely require.

- Do not implement Validation or Traits themselves.

## 3. Reflection edge-case completeness

- Cover namespaces, templates/template arguments, annotations and typed annotations.

- Cover access/reachability, caller-sensitive projections, function/function-template parameters, aliases, incomplete subjects, less-common categories, and semantic failure paths.

## 4. Compile-time exception pipeline hardening

- Harden strict semantic failures and speculative consteval interpretation/fallback.

- Keep exceptions as intentional semantic control flow rather than generic branching syntax.

- Compare exception fallback against predicate alternatives where compiler cost matters.

## 5. Cache hardening

- Keep reflection-category caches independently lazy.

- Prevent accidental eager instantiation of unrelated reflection universes.

- Preserve access-sensitive and caller-sensitive correctness.

- Do not allow semantic-error paths to escape through cache-variable initialization.

- Use std::meta::info NTTP specialization only when measured and justified.

## 6. Query execution hardening

- Fused pipelines and explicit materialization barriers.

- Temporary/rvalue lifetime correctness.

- Reference-valued pipelines and source composition.

- No unnecessary compile-time intermediate containers.

## 7. Compiler regression/performance work

- Track frontend structural allocation, specialization counts, generated assembly, wall time, peak RSS, and scaling behavior.

- Use cheap structural evidence during architecture iteration; reserve full wall-time/RSS runs for meaningful checkpoints and completion.

- Compare the optimized Query architecture against less-efficient formulations and optimized handwritten baselines.

## 8. Documentation and completion gate

- Public Meta documentation and examples.

- Negative and compile-fail cases.

- Stress subjects and production cleanup.

- No temporary TODO semantics survive completion.

# 2. Explicitly not in Phase 5.4

- Validation itself.

- Traits.

- Shapes.

- Derive.

- Runtime algorithms.

- Dependency on runtime Iter.

- groupBy.

- Query introspection/recovery machinery.

- Broad Meta type-wrapper hierarchies.

- General-purpose wrappers merely because \<meta\> exposes something.

- Broader type/vocabulary questions intentionally deferred to Core Revisit.

**Boundary rule:** Phase 5.4 means making Meta trustworthy infrastructure, not extending it with every conceivable reflection abstraction.

# 3. Public Query API

The canonical API remains deliberately Iter-like and ergonomic:

query(xs)  
.filter(\[threshold\](int x) consteval {  
return x \> threshold;  
})  
.map(\[\](int x) consteval {  
return x \* 2;  
})  
.take(8)  
.first();

Captured lambdas are a mandatory supported case. Users are not required to expose IR, binding policies, capture environments, or ordinary configuration values through template argument syntax.

# 4. Query architectural model

Iter-like Query API  
-\>  
immutable value IR  
-\>  
normalization / interning  
-\>  
typed segment planning  
-\>  
fused consteval execution  
-\>  
terminal / materialization

Query is therefore treated as a miniature compile-time compiler: the public API is the frontend; IR construction is lowering; normalization is optimization; typed segmentation performs static dataflow analysis; and terminal execution is the backend.

# 5. Stage topology becomes value IR

The primary representation of a logical operation is a compact homogeneous structural instruction value, not a bespoke class-template specialization.

enum class QueryOp : u8 {  
Map,  
Filter,  
FilterMap,  
Flatten,  
Inspect,  
Take,  
Skip,  
TakeWhile,  
SkipWhile,  
Slice,  
Enumerate,  
// ... contracted vocabulary  
};  
  
struct Instruction {  
QueryOp op;  
std::meta::info operand{};  
u32 a{};  
u32 b{};  
};

The exact private layout is measurement-driven. The locked invariant is that N logical operations must not automatically require N distinct stage class-template types.

# 6. Persistent immutable program IR

The preferred initial representation is a structural persistent node interned/reflected through std::meta::reflect_constant:

struct ProgramNode {  
std::meta::info previous;  
Instruction instruction;  
};  
  
consteval auto append(std::meta::info previous,  
Instruction instruction) {  
return std::meta::reflect_constant(  
ProgramNode{previous, instruction}  
);  
}

- Query should identify a static plan through a compact reflection/value handle instead of an ever-growing Stages... type pack.

- Equivalent structural IR should reuse compiler identity where the language/compiler naturally provides it; canonicalization must be verified, not assumed.

- An implementation may replace linked reflected nodes with another homogeneous value representation only if it preserves the architectural invariants and measures better.

# 7. Query type growth

Appending same-output-type static instructions should avoid changing the Query C++ type merely because another instruction exists. Type changes are justified only by real semantic information such as logical output type changes, capture-environment representation changes, source representation changes, or materialization barriers.

| **Primary frontend objective.** Stop encoding operation count and pipeline prefixes into ever-growing C++ types. The plan is primarily data; types appear only where static typing genuinely requires them. |
|-------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

# 8. Callable representation

## 8.1 Structural and captureless callables

Where legal and cheaper, captureless/structural callable values should live in IR via reflection rather than produce persistent stage objects or binding types. Direct splice/invocation is preferred when it is semantically exact and structurally cheaper than generic invocation machinery.

Do not introduce StaticMap, StaticFilter, StaticBinding, or equivalent types merely to carry callable identity.

## 8.2 Captured lambdas are first-class

query(xs).filter(\[threshold\](int x) consteval {  
return x \> threshold;  
});

A capturing closure is non-structural in the general C++26 model and cannot generically be promoted with reflect_constant as though its state were a normal structural value. The closure value therefore remains the authoritative callable when its state cannot be represented statically.

## 8.3 Structural capture mirroring

A structural mirror of capture state may be explored as a research primitive, including reflection-generated aggregate storage where standard facilities permit it. It is not the canonical callable representation.

- Captured data is not equivalent to callable semantics.

- define_aggregate can synthesize storage members, but it does not transfer the original closure's operator() body to the new type.

- Do not depend on non-standard closure layout, bit-casting, reinterpretation, or compiler-private closure details.

- Use capture mirroring only if it provides an independent optimizer/cache/storage benefit and remains standards-correct.

## 8.4 Minimal capture environment

Only genuinely non-structural configuration belongs in the capture environment. If N is total operations and M is operations requiring real non-structural captured state, environment machinery should scale primarily with M, not N.

A flat reflection-generated aggregate is a candidate representation. It is measurement-gated; no recursively nested Env\<Previous, Capture\> chain is permitted.

# 9. No static/stateful semantic duplication

The semantics of each operation exist exactly once. Representation of a callable/configuration is an operand-storage concern, not a reason to duplicate Map, Filter, or any other operation into static/captured/stateful families.

# 10. IR normalization and fusion

## 10.1 Mandatory safe normalization candidates

Take(a) + Take(b) -\> Take(min(a, b))  
Skip(a) + Skip(b) -\> Skip(a + b) // exact overflow semantics  
Skip(0) -\> remove  
Take(0) -\> known-empty suffix where observationally valid  
Skip(a) + Take(b) -\> Slice-like canonical form where exact  
adjacent slice limits -\> compact canonical form

Normalization must preserve evaluation order, consteval-observable effects, exception behavior, short-circuiting, reference categories, and lifetime semantics.

## 10.2 Fusion must not substitute one template tree for another

Do not call an optimization "fusion" when Map\<A\> + Map\<B\> merely becomes Compose\<A,B\> and moves type nesting elsewhere. Prefer value-level IR fusion and segment compilation.

Multiple same-type filters may remain separate instruction values while compiling into one execution segment. This removes dispatch boundaries without creating nested composition types.

# 11. Typed segment compiler

Consecutive operations that preserve the logical item type should compile into a single same-type execution segment. Only genuine static type/cardinality boundaries require typed continuation machinery.

Info  
filter  
filter  
inspect  
skip  
take  
\|  
\| map: Info -\> string_view  
v  
string_view  
filter  
inspect  
\|  
\| map: string_view -\> AssetId  
v  
AssetId

This should compile conceptually as Segment\<Info\> -\> typed Map boundary -\> Segment\<string_view\> -\> typed Map boundary -\> Segment\<AssetId\>.

| **Complexity target.** Let N be the number of logical Query operations and B the number of genuine typed/cardinality boundaries. The evaluator should trend toward O(B) template complexity rather than O(N) wherever semantics permit. |
|-----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|

# 12. Expansion statements

C++26 expansion statements are the preferred candidate for lowering homogeneous same-type IR instructions into direct statement sequences. They are appropriate for same-type instruction expansion, IR validation, state initialization, diagnostics, and other heterogeneous meta iteration.

They are not used to force a heterogeneous type-changing pipeline into one fake mutable loop. If a valid required expansion-statement use exposes a compiler defect, implementation stops and the defect is reported; recursive template machinery is not introduced as a workaround.

# 13. Typed boundaries

- Only operations that genuinely change static dataflow require typed continuation machinery.

- Examples include Map, output-changing FilterMap, Flatten, FlatMap, materialization, and source composition when element type changes.

- The boundary implementation must be the smallest direct language construct that preserves exact semantics. There is no general recursive stage walker.

# 14. Evaluation-local state

Traversal mutation does not live persistently inside Query. Most state is primitive and should be represented as compact indexed data rather than one StageState class template per operation.

template\<size_t CounterCount, size_t FlagCount\>  
struct Evaluation {  
size_t counters\[CounterCount\]{};  
bool flags\[FlagCount\]{};  
};

Skip -\> counter 0  
Take -\> counter 1  
Enumerate -\> counter 2  
TakeWhile -\> flag 0

The architecture distinguishes exactly three categories: Program = immutable semantic IR; Environment = unavoidable non-structural captured configuration; Evaluation = ephemeral mutable traversal state.

# 15. Materialization barriers

Operations that fundamentally require persistence or reordering are explicit barriers, including applicable materialize, sort, reverse, partition-like persistence, and set-like operations.

compile/evaluate current program  
-\> create required backing  
-\> produce new source  
-\> begin a fresh program

Ordinary lazy operations must not introduce hidden compile-time intermediate containers.

# 16. Public Query API facade

The public adaptor vocabulary should preferably be factored through a reusable explicit-object facade when measurement confirms this reduces repeated class-template machinery:

struct QueryApi {  
template\<class Self, class Fn\>  
consteval auto map(this Self&& self, Fn&& fn);  
  
template\<class Self, class Fn\>  
consteval auto filter(this Self&& self, Fn&& fn);  
  
// ...  
};

This is a measurement-gated implementation detail. The locked invariant is that Query storage remains as data-oriented and type-stable as practical.

# 17. Exact logical type and lifetime semantics

- Preserve T, T&, const T&, and valid rvalue categories through every applicable operation.

- No accidental decay, copy, cv stripping, reference stripping, or temporary materialization.

- Temporary/rvalue flattening and source composition must not leave references into expired storage.

- Correctness of reference/lifetime semantics outranks an optimization.

# 18. Reflection-source completeness

Every supported Query source must compose intentionally with the rest of the system. This includes the relevant output of members, bases, fields, static fields, enumerators, parameters, template arguments, annotations, namespace entities, and caller/access-sensitive projections.

No source is considered supported merely because it happened to work in the common test case.

# 19. Cache architecture and IR interning

## 19.1 Reflection caches

- Canonical reflection universes remain independently lazy and category-local.

- Access- and caller-sensitive results are cached only with sufficient context to preserve correctness.

- Semantic validation occurs before entering cache specializations when needed to preserve catchable std::meta::exception behavior.

- Do not globally cache arbitrary captured-lambda-heavy intermediate pipelines merely because a cache mechanism exists.

## 19.2 Profitable cache candidates

- Canonical reflection universes.

- Reusable access-filtered projections when context-safe.

- Normalized static programs.

- Compiled segment plans when reuse justifies them.

- Reusable materialized results when measurement proves benefit.

## 19.3 Program interning

Equivalent structural static prefixes should share native compiler identity where C++26 reflection naturally provides it. Explicitly verify the equivalence behavior rather than building custom global memoization first.

# 20. Compile-time exception pipeline

- Strict invalid Meta operations expose the intended std::meta::exception path.

- Speculative consteval exception fallback is used only when it naturally represents interpretation/fallback semantics.

- Ordinary booleans use predicates/branches.

- Semantic failures must not accidentally escape through variable-template initialization, NTTP cache specialization, substitution failure, or unrelated eager cache initialization.

- Exception-driven and predicate-driven equivalent hot paths are compared when compiler cost is material.

# 21. Compiler-performance contract

Compiler work is a first-class quality attribute. Architecture iteration should use lightweight deterministic evidence first; expensive full-suite timing is reserved for significant checkpoints.

| **Metric**                              | **During iteration** | **Completion gate** |
|-----------------------------------------|----------------------|---------------------|
| AST type/decl allocation                | Yes                  | Yes                 |
| Template/function specialization counts | Yes                  | Yes                 |
| AST shape                               | Yes                  | As needed           |
| Optimized assembly                      | Yes                  | Yes                 |
| Object/layout probes                    | Yes                  | Yes                 |
| Frontend wall time                      | Checkpoint only      | Yes                 |
| Peak RSS                                | Checkpoint only      | Yes                 |
| Scaling/stress                          | Focused probes       | Yes                 |

## 21.1 Required comparison set

- Optimally handwritten formulation.

- Raw \<meta\>.

- Phase 5.3 QueryState/tuple architecture.

- Direct-adjacency benchmark baseline.

- Final reflected value-IR + typed-segment architecture.

## 21.2 Scaling points

subject sizes: 32 / 128 / 512 / 1024  
pipeline depths: 1 / 4 / 8 / 16 / 32 / 64

# 22. Performance gates

## 22.1 Generated code

For runtime-observable terminals, optimized assembly must be identical or demonstrably equivalent to optimized handwritten code. No residual Query object, stage dispatch, function-pointer dispatch, virtual dispatch, intermediate allocation, or unnecessary branch may survive.

## 22.2 Runtime storage

Compile-time Query machinery leaves no runtime storage. Values intentionally materialized into the runtime program must be no worse than their handwritten equivalent.

## 22.3 Frontend

The final reflected-IR architecture must beat the direct-adjacency baseline on representative structural frontend metrics. If it does not, Phase 5.4 does not simply declare the architecture final; the cause must be analyzed and the design pushed further.

## 22.4 Workload-level negative cost

Miracle compiler work  
\<  
equivalent independently repeated raw \<meta\> work

The reduction must come from genuine shared reflection, interning, normalization, fusion, caching, and reuse - not from changing the measured workload.

# 23. Current architectural benchmark baseline

The following lightweight AST-allocation measurements are retained as a historical baseline for 64-stage synthetic pipelines. They are not the final acceptance numbers for the reflected-IR architecture.

| **Architecture**       | **Total AST type+decl allocation** | **Increment over ~1.926 MB empty baseline** |
|------------------------|------------------------------------|---------------------------------------------|
| Optimized handwritten  | ~2.019 MB                          | ~0.093 MB                                   |
| Direct-adjacency Query | ~3.311 MB                          | ~1.385 MB                                   |

The direct-adjacency prototype already demonstrated assembly parity with handwritten code. It remains a benchmark reference only. The value-IR/typed-segment architecture exists specifically to close the remaining frontend gap.

# 24. Forbidden final machinery

- QueryState as a redundant carrier.

- tuple\<Stages...\> pipeline representation.

- tuple_cat for adaptor append.

- Recursive expression-template trees.

- One class-template specialization per logical stage as the default IR.

- Generic recursive emit\<I\> walking all operations.

- Central IsMap/IsFilter/... semantic dispatch.

- StaticMap/StatefulMap or equivalent semantic duplication.

- Persistent traversal-state mutation inside Query.

- std::function or runtime type erasure.

- Virtual dispatch.

- Unnecessary compile-time intermediate containers between lazy stages.

- Compiler-specific closure layout tricks or UB-based capture reconstruction.

# 25. Completion gate

- \[ \] Applicable Meta/Query vocabulary is complete.

- \[ \] All supported reflection sources compose correctly.

- \[ \] Exact reference/value semantics are hardened.

- \[ \] Reflection edge cases and semantic failures are intentional and covered.

- \[ \] Foundational Validation/Traits prerequisites exist without implementing those systems.

- \[ \] Caches are independently lazy and access/caller-correct.

- \[ \] Semantic exceptions cannot escape accidentally through cache machinery.

- \[ \] Exception-vs-predicate hot paths have been evaluated where material.

- \[ \] QueryState and tuple-backed stage representation are gone.

- \[ \] Query is represented primarily as normalized value IR.

- \[ \] Non-structural capture environment contains only unavoidable state.

- \[ \] No duplicated static/stateful operation semantics remain.

- \[ \] Typed segmentation is implemented.

- \[ \] Evaluation mutation is local and data-oriented.

- \[ \] Materialization barriers are explicit.

- \[ \] Temporary/rvalue lifetimes and exact references are tested.

- \[ \] Source composition is tested.

- \[ \] Optimized assembly matches handwritten baselines.

- \[ \] Final IR beats direct adjacency in frontend structural metrics.

- \[ \] Realistic repeated workloads demonstrate genuine cache/reuse benefits.

- \[ \] Negative and compile-fail coverage is complete.

- \[ \] Stress coverage is complete.

- \[ \] Final wall-time and peak-RSS gates pass.

- \[ \] Public docs/examples are production-ready.

- \[ \] No temporary TODO semantics remain.

| **Final architectural gate.** If Validation or Traits would immediately need to work around a known Meta deficiency, Phase 5.4 is not complete. |
|-------------------------------------------------------------------------------------------------------------------------------------------------|

# 26. Frozen implementation posture

1.  Reconcile the reviewed Phase 5.3 semantic baseline against the retyped GitHub tree before implementation changes.

2.  Establish and keep the cxx26-2026.09.30.2 reference toolchain as the validation/compiler-probing baseline for this phase.

3.  Preserve all public Phase 5.3 Query semantics and tests while replacing internal machinery.

4.  Prototype reflected persistent value IR with a minimal same-type pipeline before scaling the full vocabulary.

5.  Prove static/captureless callables can live in IR without stage-type proliferation.

6.  Measure structural frontend cost against direct adjacency and handwritten before expanding the implementation.

7.  Implement IR normalization, minimal non-structural capture environment, typed segmentation, and evaluation-local primitive state.

8.  Harden materialization, source composition, reflection edges, caches, and exception paths.

9.  Run lightweight structural performance gates throughout; run wall-time/RSS only at significant checkpoints and final completion.

10. Update public documentation/changelog, remove temporary machinery, and verify every completion criterion before declaring Phase 5.4 complete.

**END OF FROZEN PHASE 5.4 CONTRACT**
