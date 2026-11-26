# Miracle Development Protocol v1.0

**Project:** Miracle — C++26 Core Library
**Baseline:** v0.5.28, following completion of Phase 5
**Status:** APPROVED — ACTIVE
**Approved:** October 16, 2026
**Scope:** Phase 6 and subsequent development
**Authority:** Project owner

---

## 1. Purpose and philosophy

Miracle adopts a **Contract-Driven Development (CDD)** methodology.

The project is developed through close technical collaboration between its human author and an AI architectural partner, with a deliberate separation between **design, implementation, and verification**.

The objective is not merely to produce functioning software. It is to build a coherent, efficient, maintainable C++26 library while developing the author's independent architectural and implementation expertise.

The central principles are:

1. **Human-owned implementation.** The project author writes, integrates, and maintains production code, tests, and infrastructure.
2. **Collaborative architecture.** Significant design decisions are explored and challenged jointly before adoption.
3. **Specification before implementation.** Features are defined by explicit contracts describing their APIs, semantics, architecture, and acceptance requirements.
4. **Strict contractual integrity.** Approved public APIs and internal architectural decisions are binding until explicitly amended.
5. **Evidence-based verification.** Compilation, tests, benchmarks, and architectural reviews provide evidence of correctness; unsupported completion claims are prohibited.
6. **Incremental delivery.** Development proceeds through coherent, independently reviewable milestones rather than large implementation dumps.
7. **Continuous understanding.** Explanations prioritize why a solution works, not merely which code produces the desired result.

**Foundational rule:**

> The contract defines what must be built and which architectural constraints must be preserved. The author determines how to express the approved design in production code.

## 2. Responsibilities and authority

### 2.1 Project author

The author has final authority over:

- Product direction, scope, priorities, and release timing.
- Acceptance or rejection of architectural proposals.
- Public API and internal architecture approval.
- Implementation, tests, infrastructure, and integration.
- Repository changes, commits, tags, and releases.
- Contract amendments and exceptional deviations.

The author retains complete ownership of the codebase and may challenge, reject, or reopen any recommendation.

### 2.2 AI architectural partner

The architectural partner is responsible for:

- Researching and explaining relevant C++26 language and compiler capabilities.
- Exploring architectural alternatives and identifying trade-offs.
- Challenging questionable abstractions and unnecessary complexity.
- Drafting Design Contracts and implementation roadmaps.
- Defining appropriate algorithms, data structures, ownership models, and invariants.
- Explaining complex mechanisms through diagrams, pseudocode, and illustrative C++26.
- Reviewing implementation changes for correctness, performance, maintainability, and compliance.
- Assisting with compiler diagnostics, test failures, debugging, and optimization.
- Identifying gaps between the implementation and the approved specification.
- Preparing technical review reports and recommending acceptance or rejection.

The partner must distinguish **established facts, verified results, assumptions, and design recommendations**.

The partner must not silently modify contractual requirements, claim unperformed verification, or treat its own recommendations as approved decisions.

### 2.3 Code-generation policy

Full production implementations and large patches are **not the default collaboration mechanism**.

The partner may provide:

- Proposed public API declarations.
- Illustrative internal structure definitions.
- Algorithmic pseudocode.
- Focused C++26 examples.
- Compiler experiments.
- Minimal reproductions and debugging examples.

These serve as design or explanatory material unless explicitly designated otherwise.

Complete implementation assistance remains available when expressly requested by the author. Such exceptions do not transfer ownership of implementation decisions.

## 3. Repository authority and persistence

The **authoritative source of truth is the Miracle Git repository**, with the author's designated local repository state taking precedence over unverified remote copies.

Repository-tracked contracts and decisions govern implementation. Conversation history, generated responses, external documents, and temporary AI workspaces are supporting material only.

No independent authoritative copies of contracts shall be maintained in ChatGPT Library or other external storage.

Recommended documentation structure:

```text
docs/
├── DEVELOPMENT_PROTOCOL.md
├── contracts/
│   ├── core.md
│   ├── diagnostics.md
│   ├── iter.md
│   ├── meta.md
│   └── ...
├── phases/
│   └── 06/
│       ├── ROADMAP.md
│       ├── DESIGN.md
│       ├── PROGRESS.md
│       └── ACCEPTANCE.md
└── adr/
    ├── 0001-...md
    └── ...
```

This structure is a recommended starting arrangement, not a requirement to create empty files or directories.

Contracts may be organized by subsystem, phase, or feature when appropriate.

Every implementation review must identify the relevant source revision or change set. If the reviewer cannot access the current repository state, the author supplies the required files, diff, or build evidence.

**Conversation approval becomes repository authority only when reflected in an accepted repository-tracked contract or decision record.**

## 4. Development hierarchy

Miracle development is organized into three scopes.

### 4.1 Phase

A phase represents a strategic development objective spanning one or more related capabilities.

Each phase defines:

- Goals and scope.
- Dependencies and prerequisites.
- Feature inventory and proposed order.
- Architectural risks.
- Completion criteria.
- Release considerations.

A phase is not necessarily equivalent to a single software release.

### 4.2 Feature

A feature is a coherent unit of functionality governed by a Design Contract.

Each feature must have a defined purpose, API and semantic expectations, architectural requirements, implementation plan, and acceptance criteria.

Complex features may receive standalone contracts. Small related features may share a contract.

### 4.3 Milestone

A milestone is the smallest formally reviewed implementation unit.

It should ordinarily be:

- Narrow enough to implement without prolonged architectural uncertainty.
- Independently reviewable.
- Compilable within its intended development scope.
- Accompanied by relevant verification evidence.
- Traceable to its parent feature contract.

Milestones may include infrastructure-only work, provided the expected observable or inspectable outcome is defined.

**Milestones are sized by architectural coherence and verification cost, not an arbitrary line-count limit.**

## 5. Design Contract standard

A Design Contract is the binding specification for a feature.

Contracts use two levels of detail.

### 5.1 Lightweight Contract

Appropriate for isolated utilities, small extensions, and low-risk changes.

Minimum contents:

1. Purpose and requirements.
2. Public API shape.
3. Behavioral semantics and invariants.
4. Relevant constraints and non-goals.
5. Implementation milestones, if needed.
6. Acceptance criteria.

A lightweight contract should be concise and must not introduce unnecessary documentation overhead.

### 5.2 Blueprint Contract

Required for complex, high-risk, or foundational subsystems.

A Blueprint Contract covers:

**A. Motivation and design rationale**
- Problem definition.
- Functional and nonfunctional requirements.
- Alternatives considered.
- Chosen design and trade-offs.
- Explicit non-goals.

**B. Public API**
- Module and namespace boundaries.
- Types, functions, templates, concepts, and annotations.
- Representative usage examples.
- Type constraints and compile-time behavior.
- Error and result semantics.
- Compatibility expectations.

**C. Internal architecture**
- Required subsystems and responsibilities.
- Internal data model and ownership.
- Dependency direction.
- Lifetime and invalidation rules.
- Concurrency and synchronization requirements, where applicable.
- Mandatory architectural invariants.

**D. Algorithms and data structures**
- Selected algorithmic approaches.
- Relevant pseudocode or explanatory snippets.
- Complexity targets where meaningful.
- Memory-layout and allocation considerations.
- Important edge cases and failure modes.
- Optimization opportunities and their prerequisites.

**E. Implementation roadmap**
- Ordered milestones.
- Dependencies between milestones.
- Expected intermediate states.
- Integration boundaries.
- Anticipated technical risks.

**F. Verification and acceptance**
- Positive and negative behavioral cases.
- Compile-time constraints and rejection cases.
- Runtime and integration tests.
- Performance or allocation requirements, where applicable.
- Concrete acceptance evidence.

Contracts shall not require line-by-line implementation prescriptions unless those details are themselves essential architectural requirements.

### 5.3 Contractual precision

Every significant specification must distinguish:

- **Required:** Binding behavior, API, invariant, or architectural decision.
- **Recommended:** Preferred technique that may be replaced without violating the contract.
- **Illustrative:** An example communicating intent, not a required implementation.
- **Deferred:** Explicitly excluded from the current scope.

Requirements must be testable or otherwise reviewable wherever practical.

Public examples may express intended API behavior before an implementation exists. They must not be presented as successfully compiled until verified.

## 6. Architectural locking and change control

Miracle adopts **strict API and internal architecture locking**.

Once a feature contract is approved, the following are binding to the extent specified:

- Public API signatures and observable semantics.
- Required constraints and invariants.
- Error behavior and compatibility guarantees.
- Approved subsystem boundaries and dependency direction.
- Required ownership and lifetime model.
- Mandatory data structures and algorithmic properties.
- Concurrency, determinism, and performance guarantees.

Unspecified implementation details remain under the author's control.

### 6.1 Contract amendments

If implementation reveals a defect or a superior architectural alternative, the existing contract must not be silently bypassed.

The amendment process is:

1. Identify the conflicting requirement.
2. Explain the cause and the proposed alternative.
3. Assess API, architectural, compatibility, and verification impact.
4. Discuss alternatives and reach an explicit author decision.
5. Update the authoritative contract.
6. Add or revise an ADR when the decision is architecturally significant.
7. Adjust the roadmap and acceptance tests as necessary.
8. Continue implementation against the revised contract.

Experimental deviations are allowed in isolated development work but cannot be accepted as contract-compliant without approval.

### 6.2 Minor implementation changes

No amendment is necessary for changes that do not affect approved API semantics, required architectural choices, or contractual invariants.

Examples include local refactoring, renaming private variables, implementation simplification, and optimization within established constraints.

**Strict architectural locking does not imply freezing every line of implementation.**

### 6.3 Contract precedence

Where documents conflict, the following precedence applies:

1. Explicitly approved newer amendments to the affected requirement.
2. The applicable locked feature or subsystem contract.
3. The applicable locked phase contract.
4. The development protocol and general project conventions.

A later document does not implicitly override unrelated requirements. Ambiguous conflicts must be resolved explicitly before acceptance.

## 7. Implementation guidance standard

The partner's guidance must enable independent implementation.

For complex subsystems, explanations should ordinarily cover:

- The underlying problem and conceptual model.
- Why the chosen algorithm or data structure is appropriate.
- The invariants that make it correct.
- Time and space complexity.
- Ownership and memory behavior.
- Known failure cases.
- How to test correctness independently.
- Possible future extensions without premature implementation.

Pseudocode and compact code examples are preferred when prose alone would be ambiguous.

Guidance should teach the mechanism rather than disguise a complete production patch as a contract.

The author may request deeper explanations, compiler experiments, reference implementations, or debugging assistance at any point.

## 8. Milestone review and verification

**Every milestone requires review before acceptance.**

Reviews compare the implementation against the locked contract and the milestone's acceptance criteria.

### 8.1 Risk-based verification

Verification depth depends on the affected subsystem and failure impact.

**Low risk:** Isolated helpers, documentation-related implementation changes, and narrowly scoped behavior.

Typical evidence:
- Relevant compilation.
- Focused behavioral tests.
- API and contract inspection.

**Medium risk:** Generic algorithms, template constraints, container operations, and interactions between components.

Additional evidence may include:
- Edge-case tests.
- Negative compilation tests.
- Lifetime and move/copy behavior.
- Integration coverage.
- Relevant compiler configurations.

**High risk:** Reflection machinery, serialization infrastructure, error propagation foundations, ABI-sensitive code, concurrency, memory ownership, or broad API changes.

Additional evidence may include:
- Compile-time and runtime invariant tests.
- Failure-path and adversarial tests.
- Cross-component integration.
- Multiple compiler configurations where supported.
- Sanitizer or other dynamic analysis where appropriate.
- Performance and allocation measurements when relevant.

Each milestone identifies the verification requirements before implementation is accepted.

### 8.2 Compiler policy

Experimental C++26 features must be evaluated against an explicitly identified compiler version or reference snapshot.

Compiler limitations must be distinguished from library defects.

A workaround caused by a compiler limitation must be documented if it affects the contract or architectural assumptions.

Unverified portability must never be represented as established portability.

### 8.3 Review findings

Review reports classify findings as:

- **Blocker:** Incorrect behavior, violated contract, safety issue, or required verification failure preventing acceptance.
- **Major:** Significant architectural, correctness, or maintainability concern requiring resolution or explicit exception.
- **Minor:** Nonblocking improvement with limited impact.
- **Observation:** Optional design alternative, future consideration, or educational note.

A review should identify the violated invariant, provide supporting evidence, explain its consequences, and describe the properties of an acceptable correction.

The reviewer should not automatically replace the author's implementation with a complete rewritten patch.

### 8.4 Review outcomes

A milestone review results in:

- **Accepted:** All required gates pass; no unresolved blocking findings.
- **Changes requested:** Known issues prevent acceptance.
- **Verification pending:** Implementation appears complete, but required evidence is missing.
- **Contract revision required:** A substantive design conflict must be resolved before acceptance.

Missing evidence must not be treated as evidence of success or failure.

## 9. Progress states and acceptance

Implementation progress and verification progress are tracked separately.

The following terminology is adopted:

- **Proposed:** Design is under discussion.
- **Locked:** Contract has been approved and recorded.
- **In progress:** Implementation is underway.
- **Implementation-complete:** Required code and infrastructure appear implemented.
- **Verification-complete:** Required checks have passed with recorded evidence.
- **Accepted:** Author has approved the milestone or feature following review.
- **Released:** Accepted functionality has been included in an identified release.

A feature may be implementation-complete while verification remains pending.

A phase may be technically complete without being accepted for release.

**Completion claims must state which level has actually been reached.**

Phase acceptance requires its required features and milestones to be accepted, its integration criteria to pass, and any outstanding exceptions to be resolved or explicitly approved.

## 10. Architectural Decision Records

Significant decisions are documented through Architecture Decision Records (ADRs).

An ADR should include:

- Identifier and title.
- Status.
- Context and problem.
- Alternatives considered.
- Chosen decision.
- Rationale and trade-offs.
- Consequences.
- References to affected contracts.

ADRs are appropriate for decisions with lasting architectural consequences, including changes to ownership models, dependency rules, core representations, algorithmic approaches, and major API direction.

Minor local implementation decisions do not require ADRs.

ADRs explain **why** a decision was adopted. Design Contracts specify **what** the resulting implementation must satisfy.

Superseded decisions remain available in repository history and should be clearly marked rather than silently rewritten.

## 11. Progress ledger and cross-chat continuity

Each active phase maintains a compact `PROGRESS.md`.

Its purpose is to enable reliable continuation without reconstructing previous conversations.

The ledger records:

- Current phase and active feature.
- Relevant repository revision.
- Accepted milestones.
- Current milestone and its status.
- Outstanding review findings.
- Known compiler or environment constraints.
- Active ADR and contract references.
- Verification performed and remaining.
- Immediate next implementation action.

The ledger should remain concise and operational. It is not a diary of every implementation step.

At an important handoff, the author or partner updates the ledger to reflect the accepted state.

A future conversation resumes from the repository contracts, progress ledger, and actual source state—not from assumed continuity of AI memory.

When information is missing, the partner must identify the gap rather than reconstructing unverified implementation progress.

## 12. Examples as executable specifications

Substantial public features should include representative usage examples where appropriate.

Examples serve three purposes:

1. Demonstrate intended API ergonomics.
2. Validate the contract against practical consumer usage.
3. Provide durable documentation and regression coverage.

Examples should be designed early enough to influence the API, not merely produced after implementation is complete.

Where practical, examples are included in automated compilation or execution checks.

Miracle's examples must remain approachable and focused on the consumer-facing API rather than exposing unnecessary internal complexity.

The contract remains authoritative; examples must be updated when an approved contract amendment changes intended behavior.

## 13. Versioning and release discipline

Milestone acceptance, phase completion, and software release are distinct events.

A successful milestone does not automatically require a new release tag.

Release decisions consider:

- Accepted functionality.
- API and behavioral compatibility.
- Verification evidence.
- Outstanding known limitations.
- Documentation and example readiness.

Semantic versioning is used as the intended release vocabulary, subject to the project's explicit pre-1.0 compatibility policy.

The author determines release boundaries and approves tags.

Patch releases may incorporate compatible fixes, hardening work, and additions between larger milestones where appropriate.

Release notes must not claim capabilities or verification that have not been established.

## 14. Collaboration standards

The following rules apply throughout development.

**Design discussion**
- Alternatives must be considered on their technical merits.
- Architectural disagreement is encouraged when justified.
- No decision is considered approved merely because it was suggested.
- The author makes final decisions.

**Implementation**
- The author owns production changes.
- The partner provides guidance proportional to complexity.
- Unnecessary abstractions and speculative extensibility should be challenged.
- Performance decisions should be justified by concrete requirements or evidence.

**Review**
- Criticism must be technically specific.
- Reviewers must distinguish defects from preferences.
- Unsupported claims of correctness or completeness are prohibited.
- Corrections should improve understanding, not merely replace code.

**Communication**
- Unknowns and uncertainty must be explicit.
- Established contractual decisions should not be repeatedly reopened without cause.
- Scope changes are documented.
- Work should progress in meaningful milestones without unnecessary administrative overhead.

## 15. Protocol adoption and evolution

This protocol takes effect only after explicit author approval and repository integration.

It governs new development beginning with Phase 6. Previously accepted work is not retroactively invalidated solely because it followed a different process.

The protocol may be amended by the author following architectural discussion.

Changes that materially affect responsibilities, contract authority, review gates, or acceptance semantics should be recorded and versioned.

**Final principle:**

> Miracle is authored by its developer, designed through rigorous collaboration, and accepted through evidence. The purpose of this protocol is to produce better software while making its author a better engineer.
