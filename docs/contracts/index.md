# Frozen contract archive

**Status:** historical approved/frozen design record, imported for discoverability; not a new approval or implementation-status claim.
**Tracking:** [contract migration Issue #7](https://github.com/spwn02/Miracle/issues/7) and [Miracle Development Project #3](https://github.com/users/spwn02/projects/3).

Repository-tracked files are the authoritative published *copies* of verified locked texts. The Project remains authoritative for **current planning and status**, not editable duplicates of these contracts. The underlying source artifacts and any exceptional provenance limitations are identified below.

## Verified source inventory

| Source | Status at issuance | Canonical readable copy | Verification and qualification |
| --- | --- | --- | --- |
| Miracle Foundation & Structural Programming Contract Pack v1.0 | Combined locked reference, six contracts and Q1–Q238 decision ledger | [Foundation and structural programming contract pack](miracle-foundation-v1.0.md) | **Byte-identical** to the historical Library Markdown artifact `miracle-foundation-contract-v1.0.md`, Git blob SHA `d6bf031026187a17553c3f5d9d73d00f14c48ed2`; original SHA-256 `dd51415ac699c3c7818ab598fcf2eaad54ad7878e3f378b64cf0237f5db36ec9`. |
| Phase 5.4 — Meta Completeness & Hardening | Frozen design and implementation contract, 2026-10-02 | [Phase 5.4 Markdown reading edition](../phases/phase-5.4-frozen-contract.md) | A **Pandoc/GFM rendering**, **not a byte-identical source**. Original normative `Miracle-Phase-5.4-Frozen-Contract.docx` remains in the user's Library; its SHA-256 is `b2888f39eddb7bd0a6e669527066af6d890e36551b74a0e9b76bd7b0b36fa63f`. Reader Markdown Git blob SHA `abfc9b9567b174ac356f6dcd24f8f8185540f46b`. Exact original binary has **not yet been imported** to Git. |
| Phase 5.6 — Standards Reconciliation, Comprehensive Audit & Final Hardening | Locked execution contract v1.0, dated 2026-10-13 | [Phase 5.6 execution contract](../phases/phase-5.6-execution-contract-v1.0.md) | **Byte-identical** to Library Markdown `Miracle-Phase-5.6-Execution-Contract-v1.0.md`, Git blob SHA `54ad47e4b7dd877ccec8fa96f42f61a371936273`; original SHA-256 `b8721836b34fd616f8364cbac27219b56e23de24e1335db49eda4a6253ca2743`. |

The combined Foundation pack contains the foundation, core runtime vocabulary, Iter, Meta/Validation/Shapes/Traits, Structural Programming, engineering/delivery contracts and their 238-question decision ledger. Do not split or selectively paraphrase that source during historical import.

## Authority and history rules

- **No retroactive approval.** Original approval/freeze statements reflect their historical dates and scope, not a fresh sign-off.
- **No silent supersession.** Current source, accepted phase amendments and later contracts may differ from historical decisions. Such differences require explicit dated reconciliation, not rewriting older text or declaring it invalid by implication.
- **No current-implementation inference.** A frozen design contract may describe planned work that was not yet implemented when issued. Consult tests, current [reference compiler policy](../compiler-support.md) and [development protocol](../DEVELOPMENT_PROTOCOL.md) for actual support and current implementation evidence.
- **Reader edition caveat.** Phase 5.4's original DOCX remains the normative historical artifact until its **exact bytes** are included and verified in Git. The rendered Markdown is a convenience copy, not a substitute for the source in a byte-level audit.
- **Remaining discovery.** The precise standalone approvals/source revisions for other Phase 5.X contracts (if any), and original Phase 5.4 DOCX import, have not yet been verified. They remain [Issue #7](https://github.com/spwn02/Miracle/issues/7) work, not fabricated completed migration.

For current progress use the [Miracle Development Project](https://github.com/users/spwn02/projects/3) and the relevant Issues; preserve locked contract bodies in Git.

