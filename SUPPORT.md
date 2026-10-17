# Support for Miracle

Miracle is a pre-1.0 C++26 foundation library maintained as an open-source project. Community support is best-effort; no response time, compatibility promise, or individual debugging service is guaranteed.

## Choose the right channel

- **Library bug or regression:** Open a [Bug Report](https://github.com/spwn02/Miracle/issues/new?template=bug_report.yml). Include the affected revision, observed and expected behavior, and reproduction or isolation status.
- **Compiler, standard library, modules or reflection:** Open a [Compiler / Toolchain Issue](https://github.com/spwn02/Miracle/issues/new?template=compiler_issue.yml). Include the compiler identity, Miracle revision, and available diagnostics. Consult [compiler support](docs/compiler-support.md) and [reference toolchain](docs/reference-toolchain.md) first.
- **New API capability:** Open a [Feature Proposal](https://github.com/spwn02/Miracle/issues/new?template=feature_proposal.yml).
- **Design or contract change:** Open an [Architecture Proposal](https://github.com/spwn02/Miracle/issues/new?template=architecture_proposal.yml). Proposals do not automatically amend a locked contract.
- **Build, CI, docs infrastructure, repository operations:** Open an [Infrastructure Request](https://github.com/spwn02/Miracle/issues/new?template=infrastructure_issue.yml).
- **Contribution and development workflow:** Read [CONTRIBUTING.md](CONTRIBUTING.md) and the [Development Protocol](docs/DEVELOPMENT_PROTOCOL.md).
- **Security vulnerabilities:** Follow [SECURITY.md](SECURITY.md). Do not disclose a vulnerability in a public issue.
- **Community-conduct concerns:** Follow [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), not the library bug tracker.

Before filing an issue, search [existing issues](https://github.com/spwn02/Miracle/issues). Questions that do not yet fit a specific defect or proposal may be raised through an appropriate issue form when actionable. General Q&A is planned for GitHub Discussions but is **not yet enabled**.

## What to include

Identify the tested Miracle revision or release, operating system, compiler/standard-library versions, and CMake preset or relevant commands if known. Include the smallest representative example available, exact observed behavior, expected result, and sanitized diagnostics. If an environment detail or reproducer is unavailable, say `Unknown` or explain what is blocking isolation rather than inventing data.

Miracle's `master` targets the full C++26 model using a separately maintained reference compiler. The `gcc` branch is a compatibility line and may legitimately lag `master`. A compiler limitation should not be presented as a guaranteed Miracle library defect.

## Support boundaries

- Issues are publicly visible; never attach secrets, credentials, non-public code, private paths, or personal data.
- The maintainer may request a smaller reproducer, redirect the issue, close duplicates, or decline out-of-scope feature requests.
- A new issue does not confer priority, implementation commitment, contract acceptance, or a release schedule.
- Releases and verified CI runs are evidence for specific versions and environments, not general support promises.

For project status, consult the [repository](https://github.com/spwn02/Miracle), [changelog](CHANGELOG.md), and published [releases](https://github.com/spwn02/Miracle/releases).
