# Repository scripts

All project automation scripts live beneath this **single** top-level `scripts/` directory. The root [`Makefile`](../Makefile) provides a thin command-line facade over these scripts, CMake presets and Docker; it does not form a second scripts tree.

- `ci/` — GCC validation, GCC convergence reporting, release provenance, G5 container/toolchain verification and CI helper tests
- `github/` — repository/Project configuration, security audits and workflow guards
- `docs/` — mdBook site verification

GitHub Actions workflow YAML remains under `.github/workflows/`; GitHub configuration under `.github/` is not a second scripts directory.

```sh
python3 scripts/ci/check-makefile.py
python3 scripts/ci/test-ci-helpers.py
python3 scripts/github/check-workflow-security.py
```

Migration note: the original `.github/scripts/` helper **contents were kept byte-for-byte identical**, and their workflow consumers were updated in the same commit. Earlier audit documents may name the legacy folder as a historical state. No compiler/API contracts changed.
