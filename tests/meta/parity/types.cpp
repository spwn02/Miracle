import std;
import Miracle.Meta;

namespace Tests::metaParityTypes {

/// Reflection sequence used to prove Miracle's range concept exactly follows `std::meta::reflection_range`.
inline constexpr std::array reflections{^^int, ^^double};

/// Non-reflection sequence used to prove ordinary ranges are intentionally rejected.
inline constexpr std::array integers{1, 2};

static_assert(std::same_as<Miracle::meta::Info, std::meta::info>);
static_assert(std::same_as<Miracle::meta::exception, std::meta::exception>);
static_assert(std::same_as<Miracle::meta::Exception, std::meta::exception>);
static_assert(std::same_as<Miracle::Access, std::meta::access_context>);
static_assert(std::same_as<Miracle::meta::AccessContext, std::meta::access_context>);
static_assert(std::same_as<Miracle::meta::Operators, std::meta::operators>);
static_assert(std::same_as<Miracle::meta::DataMemberOptions, std::meta::data_member_options>);
static_assert(std::same_as<Miracle::meta::MemberOffset, std::meta::member_offset>);

static_assert(Miracle::meta::ReflectionRange<decltype(reflections)> ==
              std::meta::reflection_range<decltype(reflections)>);
static_assert(Miracle::meta::ReflectionRange<decltype(reflections)>);
static_assert(not Miracle::meta::ReflectionRange<decltype(integers)>);

} // namespace Tests::metaParityTypes

auto main() -> int {
}
