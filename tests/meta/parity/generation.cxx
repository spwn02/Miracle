import std;
import Miracle.Meta;

namespace Tests::metaGenerationParity {

struct GeneratedAggregate;

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
}

static_assert(std::meta::is_complete_type(^^GeneratedAggregate));
static_assert(sizeof(GeneratedAggregate) >= sizeof(int) + sizeof(long));
static_assert(
    std::meta::nonstatic_data_members_of(^^GeneratedAggregate, std::meta::access_context::unchecked())
        .size() == 2);
static_assert(Miracle::meta::isDataMemberSpec(Miracle::meta::dataMemberSpec(^^int,
    {
        .name = "probe"})));

} // namespace Tests::metaGenerationParity

auto main() -> int {
}
