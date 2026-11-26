# Phase 6 — Miracle.Validation: Blueprint Design Contract v1.0

| Field | Value |
|---|---|
| Phase | 6 — Validation |
| Class | Blueprint Contract |
| Status | DRAFT — proposed public signatures require author review and compiler feasibility gates |
| Date | 2026-11-26 |
| Foundation | docs/contracts/FOUNDATION_V1.0.md, Meta/Validation section 6 |
| Protocol | docs/DEVELOPMENT_PROTOCOL.md |
| Dependency | Phase M (module migration) |
| Out of scope | Shapes, Traits, Derive, ADTs (ADTs deferred until L4), Core Revisit |

## 1. Intent, authority, and scope

Miracle.Validation is the single general-purpose, extensible, C++26 reflection validation engine for types, members, enumerators, functions, parameters, bases, annotations, and other reflected entities. The locked foundation requires valid, validate, and require. A later design questionnaire added check, three-valued status, typed rules, predicate adapters, composition, recursion, trace opt-in, and stability classifications. November 26 decisions clarified read-only engine-managed contexts, context-first/Info-fallback dispatch, Option<Diagnostic>, and runtime-only Diagnostic materialization.

This document consolidates settled conceptual decisions and proposes concrete declarations. The novel spellings and exact declaration choices here are **PROPOSED** until reviewed by the author. All examples are API targets, not compiled implementation evidence. The locked foundation and protocol take precedence in any ambiguity.

**Required**: minimal extensible rules; meaningful compile-time diagnostics; short-circuit Boolean mode; bounded structured reporting. **Deferred**: general-purpose rule IR/bytecode, rule registry, runtime interpretation, auto-annotation discovery, aggressive memoization, specialized validators, comprehensive DSL, integration with Shapes/Traits/Derive, ADTs.

## 2. Vocabulary and execution modes

~~~cpp
namespace Miracle::meta {
enum class ValidationStatus : u8 { Satisfied, Unsatisfied, InvalidDomain };
enum class ValidationStability : u8 { Stable, SnapshotSensitive, Unknown };
enum class CycleHandling : u8 { Reject, FailRequirement, AssumeSatisfied };

struct ValidationConfig final {
    usize maxFindings = 16;
    usize maxDepth = 64;
    usize maxRuleVisits = 4096;
    usize maxTraceEvents = 128;
    CycleHandling cycles = CycleHandling::Reject;
    bool includeAdvisories = false;
    bool trace = false;
};

struct ValidationContext final {
    Info root;
    Info current;
    Access access;
};

template<class Evidence = std::monostate>
struct RuleOutcome {
    ValidationStatus status;
    [[no_unique_address]] Evidence evidence;
};

// The concrete CheckResult/evidence storage may be templated; preserve its public observers:
// constexpr status() -> ValidationStatus
// constexpr explicit operator bool() -> true iff Satisfied
// constexpr stability() -> ValidationStability
// runtime describe() -> Option<Diagnostic>

// Type subject, default configuration, caller-sensitive access:
template<class T, class Policy, ValidationConfig Config = ValidationConfig{}>
[[nodiscard]] consteval auto valid(Access access = Access::current()) -> bool;

template<class T, class Policy, ValidationConfig Config = ValidationConfig{}>
[[nodiscard]] consteval auto check(Access access = Access::current()); // CheckResult

template<class T, class Policy, ValidationConfig Config = ValidationConfig{}>
consteval auto require(Access access = Access::current()) -> void;

// Reflection subject:
template<Info Subject, class Policy, ValidationConfig Config = ValidationConfig{}>
[[nodiscard]] consteval auto valid(Access access = Access::current()) -> bool;

template<Info Subject, class Policy, ValidationConfig Config = ValidationConfig{}>
[[nodiscard]] consteval auto check(Access access = Access::current()); // CheckResult

template<Info Subject, class Policy, ValidationConfig Config = ValidationConfig{}>
consteval auto require(Access access = Access::current()) -> void;
}
~~~

NOTE: Two public patterns that each take two template arguments (type first versus Info NTTP first) are intended overloads; compiler resolution must be tested. The check return type remains deduced until snapshot feasibility is confirmed. The types above are declarations/proposals, not compilable standalone definitions.

**valid**: compile time, cheapest quiet Boolean, may short-circuit earlier than status/report collection as long as truth value agrees. **check**: compile time, inspectable three-valued status and decisive evidence, Boolean-convertible, runtime describe() returning Option<Diagnostic> without predicate reevaluation. **validate**: compile-time rules + runtime Diagnostic materialization, bounded findings, returns Option<Diagnostic>. **require**: compile-time enforcement, uses Miracle.Format/C++26 static_assert messages, not runtime Diagnostic. Advisory-only results can have a Diagnostic despite Satisfied status.

The precise validate() function signature is a **mandatory feasibility gate**; see section 7. A suggested two-stage API follows:

~~~cpp
// Conceptual signatures, PROVISIONAL, pending caller-sensitive compiler probe.
template<class T, class Policy, ValidationConfig Config = ValidationConfig{}>
struct PreparedReport; // opaque runtime-safe token, no Info inside

template<class T, class Policy, ValidationConfig Config = ValidationConfig{}>
[[nodiscard]] consteval auto prepareReport(Access access = Access::current())
    -> PreparedReport<T, Policy, Config>;

template<class T, class Policy, ValidationConfig Config = ValidationConfig{}>
[[nodiscard]] auto validate(
    PreparedReport<T, Policy, Config> report =
        prepareReport<T, Policy, Config>(Access::current()))
    -> Option<Diagnostic>;
~~~

The supplied explicit access form is validate<T, Policy>(prepareReport<T, Policy>(Access::unchecked())). A concise direct Access overload is optional, not guaranteed. A reflection-NTTP subject variant should mirror this token model. Do not lock these signatures before an installed-consumer compiler probe.

## 3. Rule and selector concepts

A Rule is an immutable value exposing check(context) or check(context.current), or an adapted predicate. A Selector is an immutable value exposing select(context), where its result distinguishes an invalid domain from a valid empty collection.

~~~cpp
namespace Miracle::meta {
struct Selection final {
    ValidationStatus status;     // Satisfied or InvalidDomain; not Unsatisfied
    Span<const Info> entities;   // borrowed static-backed reflection sequence
};

// Public concept contracts, described schematically (implementation defines requires-expressions):
template<class T> concept Rule = /* supported consteval check(ctx) or check(Info) */;
template<class T> concept Selector = /* consteval select(ctx) -> Selection */;
template<class T> concept Predicate = /* consteval callable(ctx) or callable(Info), returning bool */;
}
~~~

Rule method shapes:

~~~cpp
struct SimpleRule {
    [[nodiscard]] consteval auto check(Miracle::meta::Info subject) const -> bool;
};

struct ContextRule {
    [[nodiscard]] consteval auto check(
        const Miracle::meta::ValidationContext& context) const
        -> Miracle::meta::ValidationStatus;
};

struct RichRule {
    [[nodiscard]] consteval auto check(
        const Miracle::meta::ValidationContext& context) const
        -> Miracle::meta::RuleOutcome<MyEvidence>;
};
~~~

Dispatch: prefer callable check(context); otherwise check(context.current). A callable context overload with invalid return type is a malformed rule, not grounds for raw-info fallback. Do not retry on Unsatisfied/InvalidDomain. Accepted results: bool (true => Satisfied; false => Unsatisfied), ValidationStatus, RuleOutcome<Payload>. Malformed rule definitions are deliberate compile errors, not InvalidDomain.

Rule-owned declarative diagnostic metadata: optional canonical code, summary, help; optional consteval explain(context, outcome/evidence) with Info fallback. Explanations use prior evidence without evaluating predicates again; returned descriptions are transient compile-time text/metadata normalized into runtime-safe static storage. No runtime Diagnostic inside rule evaluation.

## 4. Exact factory signatures (PROPOSED)

Use Miracle::meta::rules (plural). The earlier rule:: spelling was illustrative. In C++, not is a reserved alternative operator token, so the valid spelling is negate.

All factories take rule/selector **objects by value**, own their immutable, concrete types directly, and return a concrete structural composite. They never accept an evaluation subject as a factory argument.

~~~cpp
namespace Miracle::meta::rules {
template<class... R> struct All;
template<class... R> struct Any;
template<class R> struct Negate;
template<class C, class R> struct When;
template<class P, class R> struct Guard;
template<class S, class R> struct Each;
template<class S, class R> struct Exists;
template<class F> struct PredicateRule;

template<class... R> requires (Rule<R> && ...)
[[nodiscard]] consteval auto all(R... children) -> All<R...>;

template<class... R> requires (Rule<R> && ...)
[[nodiscard]] consteval auto any(R... children) -> Any<R...>;

template<Rule R>
[[nodiscard]] consteval auto negate(R child) -> Negate<R>;

template<Rule C, Rule R>
[[nodiscard]] consteval auto when(C condition, R requirement) -> When<C, R>;

template<Rule P, Rule R>
[[nodiscard]] consteval auto guard(P prerequisite, R requirement) -> Guard<P, R>;

template<Selector S, Rule R>
[[nodiscard]] consteval auto each(S selector, R requirement) -> Each<S, R>;

template<Selector S, Rule R>
[[nodiscard]] consteval auto exists(S selector, R requirement) -> Exists<S, R>;

template<class F> requires Predicate<F>
[[nodiscard]] consteval auto predicate(F callable) -> PredicateRule<F>;
}
~~~

**Examples of accepted argument categories**:
- all(IsClass{}, HasId{}, HasVersion{}) — any number of Rule values, including zero.
- any(IsEnum{}, IsUnion{}) — any number of Rule values, including zero.
- negate(IsPrivate{}) — exactly one Rule value.
- when(HasSerializeAnnotation{}, MustBeSerializable{}) — condition Rule + requirement Rule.
- guard(IsClass{}, rules::each(rules::fields(), IsSerializableField{})) — prerequisite Rule + requirement Rule.
- each(rules::fields(), IsSerializableField{}) — Selector + Rule, applying rule to every selected child.
- exists(rules::fields(), rules::nameIs("id")) — Selector + Rule, requiring a matching child.
- predicate([](Info i) consteval { return std::meta::is_enum_type(i); }) — a callable predicate.
- all<IsClass, HasId>() and all(true, false) are NOT valid forms.

Selectors are deferred expressions; they are not immediate meta::fields(...) queries. Builtin selector shapes:

~~~cpp
namespace Miracle::meta::rules {
struct FieldsSelector {
    [[nodiscard]] consteval auto select(const ValidationContext&) const -> Selection;
};
struct MembersSelector {
    [[nodiscard]] consteval auto select(const ValidationContext&) const -> Selection;
};
struct FunctionsSelector {
    [[nodiscard]] consteval auto select(const ValidationContext&) const -> Selection;
};
struct BasesSelector {
    [[nodiscard]] consteval auto select(const ValidationContext&) const -> Selection;
};
struct EnumeratorsSelector {
    [[nodiscard]] consteval auto select(const ValidationContext&) const -> Selection;
};
struct ParametersSelector {
    [[nodiscard]] consteval auto select(const ValidationContext&) const -> Selection;
};

[[nodiscard]] consteval auto fields() -> FieldsSelector;
[[nodiscard]] consteval auto members() -> MembersSelector;
[[nodiscard]] consteval auto functions() -> FunctionsSelector;
[[nodiscard]] consteval auto bases() -> BasesSelector;
[[nodiscard]] consteval auto enumerators() -> EnumeratorsSelector;
[[nodiscard]] consteval auto parameters() -> ParametersSelector;

// Illustrative parameterized rule, exact-size structural name storage:
template<usize N> struct NameIs;
template<usize N>
[[nodiscard]] consteval auto nameIs(const char (&name)[N]) -> NameIs<N - 1>;
}
~~~

A selection on an unsupported reflection category yields Selection{InvalidDomain, empty}, not Selection{Satisfied, empty}. FieldsSelector uses Miracle.Meta field queries under the engine's access context. The engine, not the rule, updates context.current while preserving context.root and context.access. Parameterized rule values should use existing Miracle.StaticString<N> and no registry/interning.

Named policy contract:

~~~cpp
struct Identifiable {
    static consteval auto rules() {
        return Miracle::meta::rules::all(
            IsClass{},
            Miracle::meta::rules::exists(
                Miracle::meta::rules::fields(),
                Miracle::meta::rules::nameIs("id"))
        );
    }
};
~~~

The fully qualified Miracle::meta::rules:: spelling avoids any ambiguity with the enclosing static rules() member name. A named policy type exposes static consteval rules() -> a Rule object. Policies may compose named policies via an explicit adapter or by calling Policy::rules(); the exact nested-policy adapter is deferred unless required by milestone 6.2. Do not require inheritance or registration.

## 5. Three-valued algebra

S = Satisfied; U = Unsatisfied; I = InvalidDomain.

| A | B | all(A,B) | any(A,B) |
|---|---|---|---|
| S | S | S | S |
| S | U | U | S |
| S | I | I | S |
| U | U | U | U |
| U | I | U | I |
| I | I | I | I |

negate(S)=U; negate(U)=S; negate(I)=I. Empty all()=S; empty any()=U.

when(condition, requirement):
- S => evaluate requirement, return its status;
- U => S, do not evaluate requirement (conditional does not apply);
- I => I, do not evaluate requirement.

guard(prerequisite, requirement):
- S => evaluate requirement;
- U or I => I, do not evaluate requirement.

each(selector, rule) folds all across selected elements, in selector order. exists(selector, rule) folds any. Valid empty selector: each=S, exists=U. Invalid selector: I, not an empty set.

For status mode, all may stop at U, any at S; an I cannot always short-circuit because a subsequent decisive status may supersede it. Boolean-only valid() may stop at I for all because S can no longer result; for any, I must not stop before a possible S. A not/negate failure may need a synthesized parent explanation even when its child succeeded. For report collection, keep deterministic source and traversal order, bounded by configured budgets.

## 6. Context, recursion, and access

ValidationContext is engine-managed/read-only to custom rules. Root remains the original validation subject; current is updated on child traversal; access is the caller's effective reflection access context. Neither rules nor selectors can construct new engine traversal contexts.

Cycle detection uses an active recursion path (not a global seen set). Key by reflected subject, relevant rule/policy identity+parameters, access, and evaluation point when required. Repeated acyclic children are legal. Default Reject => InvalidDomain plus cycle diagnostic. FailRequirement => Unsatisfied. AssumeSatisfied => Satisfied, explicit opt-in only; do not silently infer coinductive semantics. Independent max depth and visit budgets; budget exhaustion => InvalidDomain with explicit reason. Conservative caching: no reuse across access/snapshot contexts where validity may change.

Call-site Access::current() semantics are mandatory. Never silently use Access::unchecked(). Expected unsupported domains return InvalidDomain. A malformed rule/API/invalid constant evaluation is a hard compile error; do not assert all compiler errors can be intercepted as InvalidDomain.

## 7. Evidence, diagnostics, and implementation feasibility

Runtime Diagnostic is move-only and cannot be materialized during immediate evaluation. The design uses:
1. Transient compile-time evaluation (Info, access, typed rules, contexts).
2. Compile-time normalization to a runtime-safe immutable snapshot (status, code, subject spelling/path, help, findings, stability), containing **no Info**.
3. Runtime materialization into Miracle::Option<Miracle::Diagnostic>; nested failures use Diagnostic::cause() and shared code/severity/message infrastructure.
4. require() uses Miracle.Format-generated compile-time messages, not Diagnostic.

check().describe() must explain the original outcome and never re-run check(). validate() runs a collecting evaluation, not a short-circuited valid() report conversion. Presence of an advisory diagnostic does not mean validation failed. A successful evaluation without reportable advisories returns None; a failed/invalid evaluation should yield a diagnostic, subject to resource errors being reported.

The precise PreparedReport default-argument strategy requires a C++26 compiler probe before freezing public validate() spelling. Verify:
- Reflection access is captured at the caller, including explicit unchecked access.
- Compile-time reflection evidence can be normalized and placed into static backing without runtime Info leakage.
- Runtime report construction uses only normalized evidence and standard objects.
- Installed-module consumers can import and invoke the APIs.
- The check result exposes status, bool, stability, describe without duplicating rule evaluation.

Failure of the default-argument approach is a **design amendment gate**: retain the explicit prepareReport + runtime materialize route; do not silently degrade access semantics or require runtime reflection evaluation.

Recommended initial code vocabulary (provisional): VAL001 policy failure; VAL002 rule violation; VAL003 invalid domain; VAL004 cycle; VAL005 limit; VAL006 truncated report. Custom rule codes may use Miracle.Diagnostics annotated enums. Proposed defaults: 16 leaf findings, depth 64, 4096 visits, 128 trace events; these require measured review and are not prior benchmark results. Deterministic order: policy declaration order, reflected collection order, depth-first; no implicit deduplication. Retained findings are clearly marked truncated when reporting is bounded.

## 8. Tracing and stability

Trace opt-in, disabled by default; no trace storage on fast Boolean mode. Proposed event categories: EnterRule, ExitRule, ShortCircuit, SkipConditional, InvalidDomain, CycleDetected, LimitReached. Trace data are runtime-safe, bounded, and preserve actual evaluation order; skipped branches are not presented as evaluated.

Public stability classification:
- Stable: strong evidence result not dependent on relevant reflection snapshot changes.
- SnapshotSensitive: depends on declaration-point-sensitive reflection state.
- Unknown: cannot establish either stronger classification.

Conservative aggregation: Unknown dominates, otherwise SnapshotSensitive dominates, otherwise Stable. Classify sufficient actually used evidence for short-circuit outcomes. Avoid cache reuse across mismatched declaration points/access contexts.

## 9. Module ownership and phased build

A new named module Miracle.Validation exports the public API into Miracle::meta and Miracle::meta::rules. Depend on Miracle.Meta, Miracle.Format, and Miracle.Diagnostics, without requiring foundational modules to import Validation. Add export import Miracle.Validation to the Miracle umbrella when milestone integration is verified. Implementation partitions are chosen for cohesion, not an arbitrary number.

Milestones:
- **6.1** Status algebra, read-only context, rule dispatch and result normalization. Verify every truth-table row and overload-precedence/fallback/malformed-rule cases.
- **6.2** Typed value composition, predicate adapter, policy rules(), selectors and quantifiers; verify nested and parameterized policies.
- **6.3** valid/check and short-circuit/evidence distinction; verify Boolean/status consistency, no diagnostic work in quiet path.
- **6.4** Compile-time snapshot probe, describe and validate runtime materialization. Explicit approval gate if proposed signature is infeasible.
- **6.5** require, Miracle.Format diagnostics, stable code, bounded reporting, negative compile tests.
- **6.6** Recursion, budgets, opt-in traces and stability; verify active cycles vs acyclic shared subgraphs.
- **6.7** Granular/umbrella import, installed consumers, supported compilers, representative subject categories, performance review and acceptance ledger.

Each milestone is reviewed under the Development Protocol before acceptance; no C++26 compilation or performance results are claimed by this design contract.

## 10. Verification matrix

Mandatory acceptance cases:
- Three-valued complete truth table, negate, when, guard, empty and invalid selectors.
- Context overload preferred; Info fallback; malformed return is compile error; bool/status/RuleOutcome normalization.
- Parameters and exact-size storage; no mandatory allocations, virtual dispatch, registry, or global singletons.
- Direct reflected type/member/function/parameter/base/enumerator/annotation subject coverage.
- Caller-sensitive access, private access/unchecked opt-in, unsupported domains.
- No reevaluation after check; compare check description with original status.
- Nullopt, advisory-only Diagnostic, multiple causes, stable codes, deterministic ordering, truncation.
- Direct and indirect recursion, shared DAG nodes, budget exhaustion.
- Trace-off path and trace events; three stability classifications.
- require compile-fail cases with actionable bounded Format messages.
- Pinned C++26 reference compiler and C++29 fork where supported, module/import/installed consumer and compiler-work comparisons.

## 11. Non-goals and approval gates

Do not start Shapes, Traits, Derive, or ADTs in Phase 6. Rich generic policy DSL, high-cost automatic diagnostics and global rule IR are postponed until downstream evidence demands them.

**Approval gates before declaring this LOCKED**:
1. Author approves canonical namespace, factory signatures, selectors, and parameterized rule spelling.
2. Feasibility probe validates prepareReport/validate caller-access/snapshot model and any required CheckResult shape.
3. Any differences from earlier questionnaire decisions are explicitly reconciled, rather than silently treated as superseded.

The protocol's human-owned production implementation and milestone review requirements remain unchanged.
