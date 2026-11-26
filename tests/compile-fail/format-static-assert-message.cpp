// TEST-FMT-STATIC-001: a failing static_assert shows the authored Message body.
import Miracle;

using namespace Miracle;

// A compile-time Message is a valid C++26 static_assert message: the failure shows the rendered diagnostic.
static_assert(false,
    format::Message{Severity::Error, format::code("FMT", 7), "bad Iter pipeline"}.context("Iter::map").help(
        "pass a callable"));

auto main() -> int {
  return 0;
}
