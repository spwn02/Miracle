import std;
import Miracle.Meta;

consteval auto referenceMaterialize() -> std::size_t {
  std::array values{1, 2};
  return Miracle::meta::query(values).materialize().size();
}

static_assert(referenceMaterialize() == 2);
