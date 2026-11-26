# Contract revision workflow — GitHub-native governance

**Authority:** [Approved Development Protocol v1.0](../DEVELOPMENT_PROTOCOL.md), and the [frozen contract archive](INDEX.md).
**Planning:** [Miracle Development Project #3](https://github.com/users/spwn02/projects/3) — the only interactive project workspace.
**Implementation tracking:** [Issue #45](https://github.com/spwn02/Miracle/issues/45), within [Workspace #29](https://github.com/spwn02/Miracle/issues/29).

This procedure applies to **future substantive revisions**, not to the historical contract migration or to an automatic rewrite of previous approved texts. A GitHub Issue is an editable proposal, **never** a second authoritative contract copy. A Project field is not an acceptance or freeze signature.

## Record ownership and boundaries

| Record | Purpose | Authority |
| --- | --- | --- |
| [Contract Revision Proposal](https://github.com/spwn02/Miracle/issues/new?template=contract_revision.yml) | Editable motivation, candidate changes, alternatives, review discussion, pending owner decision | Proposal only |
| Project #3 Issue item, `Kind=Contract Revision` | Discoverability, disposition, execution status, priority, relationships, actual roadmap scheduling where approved | Planning and status only |
| Reviewable Git pull request | Source diff, tests, compatibility review, draft version/addendum text | Proposed repository change |
| New Git-versioned contract or explicit amendment | Approved specification, with exact revision/provenance and owner decision linked | Authoritative *after* explicit owner acceptance |
| `docs/contracts/SOURCES.json` and frozen originals | Historical byte identity and provenance for locked source artifacts | **Immutable historical evidence**; not silently amended |

The author's designated **local Git repository takes precedence** over an unverified remote copy. Before a substantive decision or canonical source edit, reconcile with the author's local revision where it may differ. Repository contracts are authoritative after their explicit owner approval and integration; Issues and Projects cannot override them.

## Lifecycle: proposed → reviewed → accepted/declined

1. **Intake.** Search open Issues, prior decisions and the archive for an existing proposal. File one native **Contract Revision Proposal** for genuinely new work. Record the baseline path, version/commit or SHA, exact clause/decision IDs, proposed semantic change, rationale, alternatives, compatibility, migration and test evidence. If unknown, mark unknown; never guess history.
2. **Project classification.** Confirm the Issue is uniquely present in Project #3 (auto-add may be asynchronous). Set `Kind=Contract Revision`, `Disposition=Candidate` and the relevant `Area` through the guarded Project bridge. Do not infer `Planned`, an implementation milestone, a deadline, or approval from automatic intake.
3. **Design review.** Work inside the Issue discussion: scope, alternatives, affected contracts and unresolved decisions. Link genuine blockers via GitHub's native dependency graph only when explicitly established. Parent/sub-issue structure can reflect approved work decomposition, not fictional historical phase structure.
4. **Owner decision to proceed.** Record the owner's **explicit permission to draft an amendment** and its exact scope in the Issue. This can move `Disposition` from `Candidate` to `Planned` if an agent's guarded operation finds the expected current value. Authorization to *draft* is not approval to *accept*.
5. **Reviewable Git proposal.** Create a pull request linked to the Issue and the baseline commit. Write a **new version or amendment** (or a clearly non-normative corrected reader) as a separate tracked artifact. Do **not** overwrite `SOURCES.json`-registered originals, change their recorded hashes, silently mutate the Phase 5.4 normative DOCX, or claim that the already-issued text was wrong at its historical approval date. If provenance legitimately requires a change to the historical register, stop and request an explicit separate owner decision.
6. **Verification.** Run the frozen-contract verifier (`python3 scripts/docs/verify-frozen-contracts.py --require-originals` when available), documentation CI, relevant reference-C++26/consumer tests, and specific contract/compatibility checks. Record both failures and unrun checks. A green workflow verifies what it executed, not contract owner acceptance.
7. **Accept / reject / defer.** Only the owner may accept an amendment, freeze a new contract version, authorize a normative supersession, or reject/withdraw it. Record the decision in the Issue and on the PR. If approved, merge the reviewed new canonical artifact, link its exact commit/path back from the Issue, and **then** update the Project's review/status fields under the agreed acceptance protocol. Do not let a merged PR or closed Issue auto-mark `Accepted`.
8. **Audit the round trip.** Verify that the Project Issue links the actual merged version/addendum, the old normative file and its manifest still pass the preservation checker, and the author acceptance evidence is unambiguous. If the change is deferred or declined, preserve rationale without modifying frozen sources. A real round-trip demonstration must involve a **genuine owner-proposed revision**; never invent one as a test fixture.

## Machine-readable phase gates

The existing Project `Status` tracks execution only: `Backlog → Design → Ready → Implementing → Review → Verification`. These are **not automatically equal to** the separate contract lifecycle of Candidate, Under review, Owner approved, Frozen/Superseded. Until a separately reviewed contract-lifecycle field exists, use the Issue's recorded decision text and exact Git commit as the authority—**not** overloading `Status` or fabricating a `Frozen` option.

The GitHub Actions bridge already supports guarded `Kind`, `Area`, `Disposition`, and limited forward status transitions. No autonomous `Accepted` state change, normative contract overwrite, release tag or automatic owner-signature operation is exposed.

## Exceptional cases

- **Reader-format correction:** A change only to a non-normative Markdown reading edition or navigation entry must identify the underlying frozen source and avoid presenting its transformed text as byte-identical. Existing registered reader revisions may themselves be preservation-checked: treat any change as a provenance review, not a casual typo edit.
- **Standards correction:** Even if a newer C++26 specification contradicts an old contract, provide evidence and an explicit amendment. Historical approval is not retroactively erased.
- **Contract conflict:** Distinguish a documented current disagreement from implementation bugs. Reference the exact source/commit and involve the author before declaring one approved rule superseded.
- **Security or credentials:** Do not disclose secrets in public Issues or PRs; use private vulnerability reporting when applicable.
- **No real proposal yet:** Keep the workflow acceptance gate **open**. The Issue template, docs and CI can be ready without manufacturing a substantive change or approval.
