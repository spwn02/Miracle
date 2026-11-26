<!--
Miracle contribution review template.
Use "Not applicable" with a reason where a section does not apply.
Do not claim a check passed unless it was actually run.
For vulnerabilities, stop: see SECURITY.md and use private reporting instead.
-->

## Summary

<!-- What changed, why is it needed, and what is deliberately out of scope? -->

## Traceability

- Related issue(s): <!-- #123, or "None — explain why" -->
- Governing contract(s): <!-- docs/contracts/... or docs/infrastructure/...; state "None" when not applicable -->
- ADR(s): <!-- docs/adr/... or "Not applicable" -->
- Phase / milestone: <!-- Identify planned milestone, or "Unscheduled" -->

> Design proposals and merged code do not automatically amend a locked contract.
> See [Miracle Development Protocol v1.0](https://github.com/spwn02/Miracle/blob/master/docs/DEVELOPMENT_PROTOCOL.md).

## Technical changes

<!-- Summarize the important design and implementation decisions. -->

- Public API / observable behavior: <!-- Changed, unchanged, or not applicable -->
- Internal architecture / ownership / lifetime: <!-- What changed or "Unchanged" -->
- Modules, targets and dependencies: <!-- What changed or "Unchanged" -->
- Compiler / standard library assumptions: <!-- Snapshot/branch, new feature requirement, or "Unchanged" -->
- Backwards compatibility / migration: <!-- Breaking, compatible, uncertain, or not applicable -->

## Verification evidence

**Record actual results, not intentions.** Use **Passed**, **Failed**, **Not run**, or **Not applicable**, and explain omissions when a check would otherwise be expected.

| Check | Result | Toolchain / command / evidence |
| --- | --- | --- |
| Reference C++26 build | Not run | |
| Relevant Switch tests / examples | Not run | |
| Release build | Not run | |
| CMake consumer modes, if affected | Not applicable | |
| Compile-time / negative tests, if affected | Not applicable | |
| Benchmark / allocation checks, if performance-relevant | Not applicable | |
| GCC compatibility, if affected | Not applicable | |

<!-- Link CI runs, failure logs, compiler snapshot and commit SHA where available. CI pending is NOT "Passed". -->

## Documentation and scope

- [ ] I reviewed applicable contracts and documented any approved amendments.
- [ ] I updated affected documentation / examples, or explained why no update is needed.
- [ ] I updated `CHANGELOG.md` for user-visible changes, or explained why no entry is needed.
- [ ] I verified that no secrets, credentials, private reports, or sensitive diagnostic data are included.
- [ ] I inspected the diff and identified any outstanding review or verification gaps.

**Documentation / changelog notes:** <!-- Links or rationale for non-applicability -->

## Risks and open questions

<!-- Correctness, compiler support, security, API, performance, rollout, TODOs, unresolved findings.
     State "None identified" if appropriate. Do not silently mark risks as resolved. -->

## Reviewer focus

<!-- Which invariants, edge cases, or decisions deserve special scrutiny? -->
