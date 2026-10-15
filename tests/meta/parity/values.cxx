import std;
import Miracle.Meta;

namespace Tests::metaValuesParity {

struct CopyOnly final {
  int value;

  constexpr explicit CopyOnly(int input)
      : value(input) {
  }
  constexpr CopyOnly(const CopyOnly &) = default;
  constexpr CopyOnly(CopyOnly &&) = delete;
  constexpr auto operator=(const CopyOnly &) -> CopyOnly & = default;
  constexpr auto operator=(CopyOnly &&) -> CopyOnly & = delete;
  constexpr ~CopyOnly() = default;
  constexpr auto operator<=>(const CopyOnly &) const = default;
};

inline constexpr int persistentObject{42};
inline constexpr int copyOnlyValue{7};
constexpr auto add(int left, int right) -> int {
  return left + right;
}

constexpr auto standardCopyOnly = std::meta::reflect_constant(CopyOnly{copyOnlyValue});
static_assert(Miracle::meta::extract<CopyOnly>(standardCopyOnly).value == copyOnlyValue);

static_assert(Miracle::meta::reflectObject(persistentObject) == std::meta::reflect_object(persistentObject));
static_assert(Miracle::meta::reflectFunction(add) == std::meta::reflect_function(add));
static_assert(Miracle::meta::objectOf(^^persistentObject) == std::meta::object_of(^^persistentObject));
static_assert(Miracle::meta::constantOf(^^persistentObject) == std::meta::constant_of(^^persistentObject));

} // namespace Tests::metaValuesParity

auto main() -> int {
}
