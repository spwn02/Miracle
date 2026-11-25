import Miracle.Format;

static_assert(Miracle::format::severityStyle(Miracle::Severity::Error) != Miracle::format::Style{});

auto main() -> int {
  return 0;
}
