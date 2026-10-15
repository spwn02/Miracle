import std;
import Miracle.Meta;

consteval auto localReferenceBarrier() -> std::size_t {
  std::array values{1, 2, 3};
  return Miracle::meta::query(values).reverse().count();
}

static_assert(localReferenceBarrier() == 3);
