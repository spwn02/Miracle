# G4 — CodeQL C++26 Source-Coverage Inventory

**Program:** GitHub Infrastructure G4 — Security & CI
**Generated from:** [Miracle `master` commit `f2dc424c51eb5df6c9b3c004cd6fe6920bcbaacf`](https://github.com/spwn02/Miracle/commit/f2dc424c51eb5df6c9b3c004cd6fe6920bcbaacf) (GitHub Git Tree recursive listing, read-only)
**Inventory date:** 2026-10-21
**Tracking:** [G4 CodeQL issue #24](https://github.com/spwn02/Miracle/issues/24)
**Authority:** [G4 security audit](G4-SECURITY-AUDIT.md) and [G4 progress ledger](G4-PROGRESS.md)

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

## Required CodeQL Tool Status evidence — not yet provided

1. Navigate to [Miracle Code Scanning](https://github.com/spwn02/Miracle/security/code-scanning) → **Tool status → CodeQL**. Record the actual **C/C++ coverage percentage**, configuration, warning/error summaries and build mode if shown.
2. Download the **analyzed-files CSV** (NOT the list-of-rules CSV). GitHub documents four semantic fields: configuration, file path, programming language and extraction success. Preserve original headers because their literal names are not asserted here.
3. Check all **33 module-interface paths** and **8 implementation-unit paths** listed above. Normalize repository-relative path separators; verify the CSV's commit/configuration matches this source snapshot or document the mismatch.
4. Report each group separately as **present+success**, **present+failure**, **not listed**, and **unclear/other**; do not treat a row absent from the file CSV as conclusive proof of an unsupported language feature without checking CodeQL configuration/eligibility.
5. Inspect at least `src/public/Miracle.ixx`, `src/public/meta/Reflect.ixx`, and `src/public/meta/Annotations.ixx` for actual extraction and warnings. The reflection expression `^^T` in `Reflect.ixx` is a good canary for language support, but its appearance in a line-based CSV is **not a semantic correctness test**.
6. Record the output and evidence in [G4-PROGRESS.md](G4-PROGRESS.md), [G4-SECURITY-AUDIT.md](G4-SECURITY-AUDIT.md) and [#24](https://github.com/spwn02/Miracle/issues/24). Keep the unsupported module/reflection claim bounded to the observed CodeQL behavior and provider documentation.

**Pending:** The maintainer's actual CodeQL analyzed-files CSV. No file-level extraction claim should be made until it is inspected.
