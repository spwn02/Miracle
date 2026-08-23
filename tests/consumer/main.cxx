import Miracle;

auto main() -> int {
  Miracle::Vec<int> values{1, 2, 3};
  return values.size() == 3 ? 0 : 1;
}
