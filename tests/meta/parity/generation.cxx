import std;
import Miracle.Meta;

namespace Tests::metaGenerationParity {

struct GeneratedAggregate;
inline constexpr std::uint8_t firstValue{1};
inline constexpr std::uint8_t secondValue{2};
inline constexpr std::uint8_t plainValue{3};
inline constexpr std::uint8_t probeValue{4};
enum class GeneratedScoped : std::uint8_t;
enum GeneratedUnscoped : std::uint8_t; // NOLINT(cppcoreguidelines-use-enum-class)

consteval {
  (void)Miracle::meta::defineAggregate(^^GeneratedAggregate,
      {
          Miracle::meta::dataMemberSpec(^^int,
              {
                  .name = "value"}),
          Miracle::meta::dataMemberSpec(^^long,
              {
                  .name = "other"}),
      });

  (void)Miracle::meta::defineEnum(^^GeneratedScoped,
      {
          Miracle::meta::enumeratorSpec({.name = "first", .value = std::meta::reflect_constant(firstValue)}),
          Miracle::meta::enumeratorSpec(
              {.name = "second", .value = std::meta::reflect_constant(secondValue)}),
      });

  (void)Miracle::meta::defineUnscopedEnum(^^GeneratedUnscoped,
      {
          Miracle::meta::enumeratorSpec({.name = "plain", .value = std::meta::reflect_constant(plainValue)}),
      });
}

static_assert(std::meta::is_complete_type(^^GeneratedAggregate));
static_assert(sizeof(GeneratedAggregate) >= sizeof(int) + sizeof(long));
static_assert(
    std::meta::nonstatic_data_members_of(^^GeneratedAggregate, std::meta::access_context::unchecked())
        .size() == 2);
static_assert(GeneratedScoped::first == static_cast<GeneratedScoped>(firstValue));
static_assert(GeneratedScoped::second == static_cast<GeneratedScoped>(secondValue));
static_assert(plain == static_cast<GeneratedUnscoped>(plainValue));
static_assert(Miracle::meta::isEnumeratorSpec(
    Miracle::meta::enumeratorSpec({.name = "probe", .value = std::meta::reflect_constant(probeValue)})));
static_assert(Miracle::meta::isDataMemberSpec(Miracle::meta::dataMemberSpec(^^int,
    {
        .name = "probe"})));

} // namespace Tests::metaGenerationParity

auto main() -> int {
}
