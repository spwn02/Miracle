import Miracle;

auto main() -> int {
  Miracle::Vec<int> values{1, 2, 3};

  Miracle::Hive<int> hive;
  hive.insert(69);

  return values.size() == 3 and hive.size() == 1 and *hive.begin() == 69 ? 0 : 1;
}
