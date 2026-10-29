import std;
import Miracle.Meta;

namespace Tests::metaParityStorage {

/// Structural values used for compile-time/static-storage parity checks.
inline constexpr std::array values{3, 1, 4, 1};

/// Character range used for reflected and concrete static-string parity checks.
inline constexpr std::string_view text{"Miracle"};

/// Compares reflected-array promotion by reflected type and extracted element sequence.
consteval auto reflectedArrayParity() -> bool {
  const auto miracle = Miracle::meta::reflectConstantArray(values);
  const auto standard = std::meta::reflect_constant_array(values);
  if (std::meta::type_of(miracle) != std::meta::type_of(standard)) {
    return false;
  }
  const auto miracleValues = std::span{std::meta::extract<const int *>(miracle), values.size()};
  const auto standardValues = std::span{std::meta::extract<const int *>(standard), values.size()};
  return std::ranges::equal(miracleValues, standardValues);
}

/// Compares reflected-string promotion by reflected type and extracted null-terminated text.
consteval auto reflectedStringParity() -> bool {
  const auto miracle = Miracle::meta::reflectConstantString(text);
  const auto standard = std::meta::reflect_constant_string(text);
  if (std::meta::type_of(miracle) != std::meta::type_of(standard)) {
    return false;
  }
  return std::string_view{std::meta::extract<const char *>(miracle)} ==
         std::string_view{std::meta::extract<const char *>(standard)};
}

/// Compares concrete static-array promotion by cardinality and element value.
consteval auto staticArrayParity() -> bool {
  const auto miracle = Miracle::meta::defineStaticArray(values);
  const auto standard = std::define_static_array(values);
  return miracle.size() == standard.size() && std::ranges::equal(miracle, standard);
}

/// Compares concrete static-string promotion by the resulting null-terminated text.
consteval auto staticStringParity() -> bool {
  return std::string_view{Miracle::meta::defineStaticString(text)} ==
         std::string_view{std::define_static_string(text)};
}

/// Compares concrete static-object promotion by pointee value.
consteval auto staticObjectParity() -> bool {
  constexpr int value = 42;
  return *Miracle::meta::defineStaticObject(value) == *std::define_static_object(value);
}

static_assert(reflectedArrayParity());
static_assert(reflectedStringParity());
static_assert(staticArrayParity());
static_assert(staticStringParity());
static_assert(staticObjectParity());
static_assert(Miracle::meta::isStringLiteral("Miracle") == std::is_string_literal("Miracle"));

} // namespace Tests::metaParityStorage

auto main() -> int {
}
