import Miracle.Diagnostics;

enum class [[= Miracle::diagnostics]] ProbeCode : unsigned char {
  Value = 1,
};

static_assert(sizeof(Miracle::Diagnostic) > 0 and sizeof(Miracle::RenderOptions) > 0);

// TEST-DIAG-API-003, TEST-FMT-IMPORT-002, TEST-REC-IMPORT-001: Severity, ColorMode and Presentation are visible from this one import, and
// the render options and diagnostics use those same types.
static_assert(__is_same(decltype(Miracle::RenderOptions{}.color), Miracle::ColorMode));
static_assert(__is_same(decltype(Miracle::RenderOptions{}.presentation), Miracle::Presentation));
static_assert(__is_same(decltype(Miracle::Diagnostic::error(ProbeCode::Value).severity()),
    Miracle::Severity));

auto main() -> int {
  return 0;
}
