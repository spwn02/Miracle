import Miracle.Concepts;

static_assert(Miracle::Numeric<int> and not Miracle::Numeric<bool>);

auto main() -> int {
  return 0;
}
