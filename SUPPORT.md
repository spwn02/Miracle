# Support for Miracle

Miracle is a pre-1.0 C++26 foundation library maintained as an open-source project. Community support is best-effort; no response time, compatibility promise, or individual debugging service is guaranteed.

## Choose the right channel

- **Library bug or regression:** Open a [Bug Report](https://github.com/spwn02/Miracle/issues/new?template=bug_report.yml). Include the affected revision, observed and expected behavior, and reproduction or isolation status.
- **Compiler, standard library, modules or reflection:** Open a [Compiler / Toolchain Issue](https://github.com/spwn02/Miracle/issues/new?template=compiler_issue.yml). Include the compiler identity, Miracle revision, and available diagnostics. Consult [compiler support](docs/compiler-support.md) and [reference toolchain](docs/reference-toolchain.md) first.
- **New API capability:** Open a [Feature Proposal](https://github.com/spwn02/Miracle/issues/new?template=feature_proposal.yml).
- **General architecture design:** Open an [Architecture Proposal](https://github.com/spwn02/Miracle/issues/new?template=architecture_proposal.yml). This does not amend a locked contract.
- **Explicit versioned contract amendment or revision:** Open a [Contract Revision Proposal](https://github.com/spwn02/Miracle/issues/new?template=contract_revision.yml) and follow the [contract revision protocol](docs/contracts/REVISION-PROTOCOL.md). The author must explicitly accept any normative change.
- **Build, CI, docs infrastructure, repository operations:** Open an [Infrastructure Request](https://github.com/spwn02/Miracle/issues/new?template=infrastructure_issue.yml).
- **Contribution and development workflow:** Read [CONTRIBUTING.md](CONTRIBUTING.md) and the [Development Protocol](docs/DEVELOPMENT_PROTOCOL.md).
- **Security vulnerabilities:** Follow [SECURITY.md](SECURITY.md). Do not disclose a vulnerability in a public issue.
- **Community-conduct concerns:** Follow [CODE_OF_CONDUCT.md](CODE_OF_CONDUCT.md), not the library bug tracker.

Before filing an issue, search [existing issues](https://github.com/spwn02/Miracle/issues) and [Discussions](https://github.com/spwn02/Miracle/discussions). General support and exploratory conversations belong in Discussions; actionable defects and concrete implementation proposals belong in the issue forms.

## Discussions

- **[Q&A](https://github.com/spwn02/Miracle/discussions/categories/q-a):** Community questions and help with Miracle or the reference C++26 toolchain. This category has GitHub's **Answers enabled**; an answer-selection workflow has not yet been tested with a real question.
- **[Ideas](https://github.com/spwn02/Miracle/discussions/categories/ideas):** Early-stage feature brainstorming and design exploration; a discussion is **not** a locked Design Contract or an approved implementation issue.
- **[Announcements](https://github.com/spwn02/Miracle/discussions/categories/announcements):** Maintainer news and important repository updates.
- **[Show and tell](https://github.com/spwn02/Miracle/discussions/categories/show-and-tell):** Share experiments, C++26 examples, tools and projects using Miracle.

The optional **General** and **Polls** categories also exist. They are not required G6 entry points. Do **not** disclose vulnerabilities, credentials, private reports or conduct allegations in public Discussions. Use the security and conduct reporting routes above instead.

## What to include

Identify the tested Miracle revision or release, operating system, compiler/standard-library versions, and CMake preset or relevant commands if known. Include the smallest representative example available, exact observed behavior, expected result, and sanitized diagnostics. If an environment detail or reproducer is unavailable, say `Unknown` or explain what is blocking isolation rather than inventing data.

Miracle's `master` targets the full C++26 model using a separately maintained reference compiler. The `gcc` branch is a compatibility line and may legitimately lag `master`. A compiler limitation should not be presented as a guaranteed Miracle library defect.

## Support boundaries

- Issues are publicly visible; never attach secrets, credentials, non-public code, private paths, or personal data.
- The maintainer may request a smaller reproducer, redirect the issue, close duplicates, or decline out-of-scope feature requests.
- A new issue does not confer priority, implementation commitment, contract acceptance, or a release schedule.
- Releases and verified CI runs are evidence for specific versions and environments, not general support promises.

For project status, consult the [repository](https://github.com/spwn02/Miracle), [changelog](CHANGELOG.md), and published [releases](https://github.com/spwn02/Miracle/releases).
