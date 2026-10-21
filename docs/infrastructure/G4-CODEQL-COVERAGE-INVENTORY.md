# G4 — CodeQL C++26 Source-Coverage Inventory

**Program:** GitHub Infrastructure G4 — Security & CI
**Generated from:** [Miracle `master` commit `f2dc424c51eb5df6c9b3c004cd6fe6920bcbaacf`](https://github.com/spwn02/Miracle/commit/f2dc424c51eb5df6c9b3c004cd6fe6920bcbaacf) (GitHub Git Tree recursive listing, read-only)
**Inventory date:** 2026-10-21
**Tracking:** [G4 CodeQL issue #24](https://github.com/spwn02/Miracle/issues/24)
**Authority:** [G4 security audit](G4-SECURITY-AUDIT.md) and [G4 progress ledger](G4-PROGRESS.md)

## Reproducible source/CSV comparison (proposed G4 hardening)

The read-only `scripts/ci/audit-codeql-coverage.py` checks the maintainer-exported **Tools → CodeQL → Scanned files → Download CSV** against **currently tracked Git source paths**. It recognizes `.cxx`, `.ixx` and the future `.cppm` spelling, distinguishes **extracted**, **explicitly unextracted** and **not listed**, and rejects duplicate or malformed C/C++ extraction records. It is self-tested in reference CI; **CI does not require or download a private CodeQL export**.

```bash
python3 scripts/ci/audit-codeql-coverage.py --self-test

# Run in a local up-to-date Miracle checkout with your real exported CSV:
python3 scripts/ci/audit-codeql-coverage.py \
  --repo . \
  --csv /path/to/code-scanning-files-extracted.csv \
  --output /tmp/miracle-codeql-file-coverage.json
```

The optional `--require-modules` exits nonzero until the export shows complete file extraction, useful **only for a future experimental scanner**. **It is deliberately not enabled in CI today**, because this would turn a known upstream CodeQL limitation into a meaningless permanent failure.

**Crucial boundary:** Even a future `moduleInterfaces.extracted == moduleInterfaces.tracked` is only a **file recognition/extraction metric**. The tool deliberately always emits `semanticCxx26ModulesVerified: false`. It cannot prove correct CodeQL analysis of C++26 reflection, module imports, compiler semantics or security properties. A separate real CodeQL language-support review and representative semantic tests would be necessary before claiming full coverage. The local exported CSV may omit commit/configuration identity; compare it to the intended source revision. Keep sensitive repository paths, tokens and unreviewed findings out of public commits.

**Decision:** Retain the existing supported CodeQL default setup and pinned reference CI until a genuine advanced/manual-build experiment demonstrates an improvement in *relevant* analysis without degrading coverage or fragmenting active alerts. GitHub's [supported language matrix](https://codeql.github.com/docs/codeql-overview/supported-languages-and-frameworks/) currently excludes C++20 modules. GitHub's [compiled-language setup guide](https://docs.github.com/en/code-security/concepts/code-scanning/codeql/codeql-for-compiled-languages) documents `none` versus `manual` build modes; switching to `manual` alone does not grant experimental C++26 language support.

## CodeQL extracted-files CSV — definitive file-level result (2026-10-21)

The maintainer provided the **real CodeQL Tool Status → Scanned files → Download analyzed-files CSV** (`code-scanning-files-extracted.csv`). The file was analyzed locally without publishing raw security alert records; SHA-256 of the supplied CSV bytes:

```text
769fca48b7f156f3779df029e9afa55b38529db0dc32dfed521b3c9d05d51c05
```

The CSV has **159 rows** with headers `Configuration`, `Language`, `File Path`, `Successfully Extracted`, `Failure Reason`. Several workflow YAML files appear in multiple language configurations; **159 is not a unique file count**. Relevant **119** distinct `/language:c-cpp` rows:

| C++ source group | Listed | Successfully extracted | Failed extraction | Empty failure reason |
| --- | ---: | ---: | ---: | ---: |
| `src/public/**/*.ixx` (module interfaces) | **33** | **0** | **33** | **33** |
| `src/private/**/*.cxx` (core implementation) | **8** | **8** | **0** | 0 |
| `tests/**/*.cxx` | 72 | 72 | 0 | 0 |
| `examples/**/*.cxx` | 4 | 4 | 0 | 0 |
| `benchmarks/**/*.cxx` | 2 | 2 | 0 | 0 |
| **Total `/language:c-cpp`** | **119** | **86** | **33** | **33** |

The **set of all 33 failed CSV `.ixx` paths is exactly identical** to the 33 `.ixx` entries in the immutable source inventory below; no missing or extra paths. The **eight source `.cxx` paths** below all occur as `Successfully Extracted=true`. Every remaining repository `.cxx` file also reports `true`. These are file-level observations and resolve the earlier numerical-only correlation.

Other configurations in the CSV: `/language:actions` has 6/6 success; `/language:python` has 26/26 success (20 Python files plus 6 workflow YAML files); `/language:javascript-typescript` has 8/8 success (2 JavaScript files plus 6 workflow YAML files). The UI's 6/6, 20/20 and 2/2 coverage represents their actual language-specific source groups, not the duplicated workflow YAML rows.

**Result:** The current CodeQL configuration **does not successfully extract any of Miracle's module interfaces**. The CSV lists the interfaces with `false` and empty failure reasons; it does **not** reveal the exact cause, compiler invocation, effective build mode or whether future CodeQL versions might support modules. GitHub's [published support matrix](https://codeql.github.com/docs/codeql-overview/supported-languages-and-frameworks/) says C++20 modules are unsupported and does not list C++26 syntax or `.ixx` as supported. Thus **no security assurance over the C++26 module/reflection bodies follows from this scan**. Successful `.cxx` extraction also does not guarantee complete semantic analysis of imported module definitions.

**Traceability limit:** The export did not contain an analysis ID, commit SHA or timestamp column. Its file-path set matches this pinned inventory, and the maintainer submitted it immediately after the live 86/119 screenshot; the scan revision cannot be independently reconstructed from the CSV alone. Treat the result as an observed Tool Status snapshot, not a reproducible CodeQL compiler log. No CodeQL configuration changes were made.

## Purpose and evidentiary boundary

Use this **source-tree inventory** to compare GitHub's CodeQL **analyzed-files CSV** against Miracle's version-controlled C++ implementation. This document records **source presence**, NOT actual CodeQL file extraction or semantic analysis. A successful CodeQL C/C++ analysis with zero alerts is insufficient to prove these sources were processed.

GitHub's [Tool Status documentation](https://docs.github.com/en/code-security/concepts/code-scanning/tool-status-page) says a file is counted as scanned when **some lines were processed**; the detailed downloadable CSV reports configuration, file path, programming language and successful extraction. For compiled languages, files generated during the build do not appear in the Tool Status file inventory.

[CodeQL's supported languages](https://codeql.github.com/docs/codeql-overview/supported-languages-and-frameworks/) explicitly document that C++20 modules are unsupported; C++23 is beta. The support documentation does not list `.ixx` among the normal supported C/C++ source extensions. Even if an `.ixx` path is present in the CSV, **processed lines are not a guarantee of correct C++26 reflection/modules semantic modeling**.

## Baseline counts

| Category | Files | Meaning |
| --- | ---: | --- |
| `src/public/**/*.ixx` | **33** | Primary module-interface files at risk of exclusion or incomplete extraction |
| `src/**/*.cxx` | **8** | Source implementation units; also verify each is scanned |
| Repository-wide `*.cxx` | **86** | Includes tests, examples and benchmarks; not all files are necessarily compiled or intended to be scanned in every setup |
| **Core source `.ixx` + `.cxx`** | **41** | Inventory denominator only; **not** GitHub's reported C/C++ coverage denominator |

Do not use these counts as a denominator for GitHub's CodeQL language coverage percentage. CodeQL's file eligibility, excludes, configuration and source-generation rules may differ.

## Module-interface files (33)

- `src/public/Concepts.ixx`
- `src/public/Diagnostic.ixx`
- `src/public/Error.ixx`
- `src/public/Feature.ixx`
- `src/public/Fs.ixx`
- `src/public/Iter.ixx`
- `src/public/Memory.ixx`
- `src/public/Miracle.ixx`
- `src/public/Panic.ixx`
- `src/public/Profiling.ixx`
- `src/public/Range.ixx`
- `src/public/StaticString.ixx`
- `src/public/Types.ixx`
- `src/public/meta/Access.ixx`
- `src/public/meta/Annotations.ixx`
- `src/public/meta/Bitflags.ixx`
- `src/public/meta/Cache.ixx`
- `src/public/meta/Debug.ixx`
- `src/public/meta/Generation.ixx`
- `src/public/meta/Layout.ixx`
- `src/public/meta/Members.ixx`
- `src/public/meta/Meta.ixx`
- `src/public/meta/Names.ixx`
- `src/public/meta/Operators.ixx`
- `src/public/meta/Predicates.ixx`
- `src/public/meta/Query.ixx`
- `src/public/meta/Reflect.ixx`
- `src/public/meta/Storage.ixx`
- `src/public/meta/Substitution.ixx`
- `src/public/meta/TypeQueries.ixx`
- `src/public/meta/TypeTransforms.ixx`
- `src/public/meta/Types.ixx`
- `src/public/meta/Values.ixx`

## Source implementation units (8)

- `src/private/Diagnostic.cxx`
- `src/private/Error.cxx`
- `src/private/Fs.cxx`
- `src/private/Panic.cxx`
- `src/private/Profiling.cxx`
- `src/private/linux/Memory.cxx`
- `src/private/unsupported/Memory.cxx`
- `src/private/windows/Memory.cxx`

## Reproducible file-list command

At the exact inventory revision, compare tracked source paths:

```sh
git ls-tree -r --name-only f2dc424c51eb5df6c9b3c004cd6fe6920bcbaacf \
  | grep -E '^src/.*\.(ixx|cxx)$'
```

This is a **read-only** Git command. For later file additions/removals, regenerate the source inventory from the target scan commit. Do not compare a CSV from another revision without recording that mismatch.

## CodeQL Tool Status comparison procedure — completed 2026-10-21

1. Navigate to [Miracle Code Scanning](https://github.com/spwn02/Miracle/security/code-scanning) → **Tool status → CodeQL**. Record the actual **C/C++ coverage percentage**, configuration, warning/error summaries and build mode if shown.
2. Download the **analyzed-files CSV** (NOT the list-of-rules CSV). GitHub documents four semantic fields: configuration, file path, programming language and extraction success. Preserve original headers because their literal names are not asserted here.
3. Check all **33 module-interface paths** and **8 implementation-unit paths** listed above. Normalize repository-relative path separators; verify the CSV's commit/configuration matches this source snapshot or document the mismatch.
4. Report each group separately as **present+success**, **present+failure**, **not listed**, and **unclear/other**; do not treat a row absent from the file CSV as conclusive proof of an unsupported language feature without checking CodeQL configuration/eligibility.
5. Inspect at least `src/public/Miracle.ixx`, `src/public/meta/Reflect.ixx`, and `src/public/meta/Annotations.ixx` for actual extraction and warnings. The reflection expression `^^T` in `Reflect.ixx` is a good canary for language support, but its appearance in a line-based CSV is **not a semantic correctness test**.
6. Record the output and evidence in [G4-PROGRESS.md](G4-PROGRESS.md), [G4-SECURITY-AUDIT.md](G4-SECURITY-AUDIT.md) and [#24](https://github.com/spwn02/Miracle/issues/24). Keep the unsupported module/reflection claim bounded to the observed CodeQL behavior and provider documentation.

**Resolved (2026-10-21):** The actual analyzed-files CSV was provided and checked against all 33 module interfaces and 8 core implementation units. File-level outcome: **33/33 `.ixx` failed extraction; 8/8 core `.cxx` successful**. Effective build mode and detailed extractor diagnostics remain unavailable; do not claim the module/reflection coverage gap has been fixed.
