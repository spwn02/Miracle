# Frozen contract archive

**Status:** historical approved/frozen design record, imported for discoverability; not a new approval or implementation-status claim.
**Tracking:** [contract migration Issue #7](https://github.com/spwn02/Miracle/issues/7) and [Miracle Development Project #3](https://github.com/users/spwn02/projects/3).

Repository-tracked files are the authoritative published *copies* of verified locked texts. The Project remains authoritative for **current planning and status**, not editable duplicates of these contracts. The underlying source artifacts and any exceptional provenance limitations are identified below.

## Verified source inventory

| Source | Status at issuance | Canonical readable copy | Verification and qualification |
| --- | --- | --- | --- |
| Miracle Foundation & Structural Programming Contract Pack v1.0 | Combined locked reference, six contracts and Q1–Q238 decision ledger | [Foundation and structural programming contract pack](FOUNDATION_V1.0.md) | **Byte-identical** to the historical Library Markdown artifact `miracle-foundation-contract-v1.0.md`, Git blob SHA `d6bf031026187a17553c3f5d9d73d00f14c48ed2`; original SHA-256 `dd51415ac699c3c7818ab598fcf2eaad54ad7878e3f378b64cf0237f5db36ec9`. |

The combined Foundation pack contains the foundation, core runtime vocabulary, Iter, Meta/Validation/Shapes/Traits, Structural Programming, engineering/delivery contracts and their 238-question decision ledger. Do not split or selectively paraphrase that source during historical import.

## Machine-checked preservation

The `docs/contracts/SOURCES.json` register records the retained Foundation Markdown source and its original hashes. Phase 5.4 and Phase 5.6 artifacts were intentionally removed; their historical versions remain in Git history. The offline verifier runs in both reference and documentation CI:

```sh
python3 scripts/docs/verify-frozen-contracts.py --self-test
python3 scripts/docs/verify-frozen-contracts.py
```

It fails if the retained Foundation source changes unexpectedly. These checks prove source identity only, not current implementation conformance or renewed owner approval.

## Historical project catalog

[PROJECT_CATALOG.json](PROJECT_CATALOG.json) preserves the historical Project import inventory. It includes retired phase sources and is not a validation inventory for the current documentation tree. Use the [Miracle Development Project #3](https://github.com/users/spwn02/projects/3) for current planning and status.

## Authority and history rules

- **No retroactive approval.** Original approval/freeze statements reflect their historical dates and scope, not a fresh sign-off.
- **No silent supersession.** Current source, accepted phase amendments and later contracts may differ from historical decisions. Such differences require explicit dated reconciliation, not rewriting older text or declaring it invalid by implication.
- **No current-implementation inference.** A frozen design contract may describe planned work that was not yet implemented when issued. Consult tests, current [reference compiler policy](../COMPILER_SUPPORT.md) and [development protocol](../DEVELOPMENT_PROTOCOL.md) for actual support and current implementation evidence.
- **Remaining discovery.** The precise standalone approvals/source revisions for other Phase 5.X contracts (if any) have not yet been verified. They remain [Issue #7](https://github.com/spwn02/Miracle/issues/7) work, not fabricated completed migration.

For current progress use the [Miracle Development Project](https://github.com/users/spwn02/projects/3) and the relevant Issues; preserve locked contract bodies in Git.
