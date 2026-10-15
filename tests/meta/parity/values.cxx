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
inline constexpr int sumValue{5};
inline constexpr long identityValue{9L};

constexpr auto add(int left, int right) -> int {
  return left + right;
}

template <class T>
constexpr auto identity(T value) -> T {
  return value;
}

constexpr auto miracleCopyOnly = Miracle::meta::reflectConstant(CopyOnly{copyOnlyValue});
constexpr auto standardCopyOnly = std::meta::reflect_constant(CopyOnly{copyOnlyValue});
static_assert(Miracle::meta::extract<CopyOnly>(miracleCopyOnly).value == copyOnlyValue);
static_assert(Miracle::meta::extract<CopyOnly>(standardCopyOnly).value == copyOnlyValue);
static_assert(std::meta::type_of(miracleCopyOnly) == std::meta::type_of(standardCopyOnly));

static_assert(Miracle::meta::reflectObject(persistentObject) == std::meta::reflect_object(persistentObject));
static_assert(Miracle::meta::reflectFunction(add) == std::meta::reflect_function(add));
static_assert(Miracle::meta::objectOf(^^persistentObject) == std::meta::object_of(^^persistentObject));
static_assert(Miracle::meta::constantOf(^^persistentObject) == std::meta::constant_of(^^persistentObject));

constexpr auto miracleAdd = Miracle::meta::reflectInvoke(^^add,
    {
        std::meta::reflect_constant(2),
        std::meta::reflect_constant(3)});
constexpr auto standardAdd = std::meta::reflect_invoke(^^add,
    {
        std::meta::reflect_constant(2),
        std::meta::reflect_constant(3)});
static_assert(Miracle::meta::extract<int>(miracleAdd) == sumValue);
static_assert(Miracle::meta::extract<int>(standardAdd) == sumValue);

constexpr std::array templateArguments{^^long};
constexpr std::array ordinaryArguments{std::meta::reflect_constant(identityValue)};
static_assert(Miracle::meta::reflectInvoke(^^identity,
                  {
                      ^^long},
                  ordinaryArguments) == std::meta::reflect_invoke(^^identity,
                                            {
                                                ^^long},
                                            ordinaryArguments));
static_assert(Miracle::meta::reflectInvoke(^^identity,
                  templateArguments,
                  {
                      std::meta::reflect_constant(identityValue)}) ==
              std::meta::reflect_invoke(^^identity,
                  templateArguments,
                  {
                      std::meta::reflect_constant(identityValue)}));
static_assert(
    Miracle::meta::reflectInvoke(^^identity,
        {
            ^^long},
        {std::meta::reflect_constant(identityValue)}) == std::meta::reflect_invoke(^^identity,
                                                             {
                                                                 ^^long},
                                                             {std::meta::reflect_constant(identityValue)}));

} // namespace Tests::metaValuesParity

auto main() -> int {
}
