import std;
import Miracle.Meta;

using namespace Miracle::meta;

namespace {

template <class...>
struct Pack {};

struct Members final {
  int z{};
  long a{};
  int m{};
};

struct Marker final {
  int value{};
  constexpr auto operator<=>(const Marker &) const = default;
};

inline constexpr Marker marker{42};

struct[[= marker]] Annotated {};
struct Plain {};

struct Base {};
struct Derived : Base {};

constexpr Info kInt = ^^int;
constexpr Info kLong = ^^long;
constexpr Info kChar = ^^char;
constexpr Info kIntRef = ^^int &;
constexpr Info kBase = ^^Base;
constexpr Info kDerived = ^^Derived;

constexpr std::array kTypes{^^const int, ^^volatile long, ^^int &, ^^double *};
constexpr std::array kTerminalTypes{^^int, ^^long, ^^int &, ^^double};
constexpr std::array kTemplates{^^Pack<int, long>, ^^Pack<char>};
constexpr std::array kAnnotated{^^Annotated, ^^Plain};
constexpr std::array kZipLeft{^^int, ^^long, ^^const int};
constexpr std::array kZipRight{^^int, ^^int, ^^const int};
constexpr std::array kHierarchy{^^Derived, ^^Base};
constexpr std::array kValues{1, 2, 3};
constexpr std::array kReflectedValues{std::meta::reflect_constant(1), std::meta::reflect_constant(2)};

constexpr auto kRawMembersQuery =
    query(std::meta::nonstatic_data_members_of(^^Members, std::meta::access_context::unchecked()));
static_assert(std::same_as<decltype(kRawMembersQuery)::Value, Info>);
static_assert(kRawMembersQuery.size() == 3);
static_assert(kRawMembersQuery.count() == 3);

consteval auto rawReflectionRangeCanStartQuery() -> bool {
  return kRawMembersQuery.size() == 3 && kRawMembersQuery.count() == 3;
}

static_assert(rawReflectionRangeCanStartQuery());

consteval auto rawAndMiracleCallablesComposeEquivalently() -> bool {
  const auto raw = query(kTypes).filter(std::meta::is_type).map(std::meta::remove_cv).materialize();
  const auto miracle = query(kTypes).filter(isType).map(removeCv).materialize();
  if (raw.size() != miracle.size()) {
    return false;
  }
  for (std::size_t index{}; index < raw.size(); ++index) {
    if (raw.at(index) != miracle.at(index)) {
      return false;
    }
  }
  return true;
}

static_assert(rawAndMiracleCallablesComposeEquivalently());

consteval auto rawPredicatesReachAdaptorsAndTerminals() -> bool {
  const auto reference = query(kTerminalTypes).find(std::meta::is_reference_type);
  const auto firstReference = query(kTerminalTypes).position(std::meta::is_reference_type);
  const auto lastReference = query(kTerminalTypes).rposition(std::meta::is_reference_type);
  auto [references, nonReferences] = query(kTerminalTypes).partition(std::meta::is_reference_type);

  return query(kTerminalTypes).takeWhile(std::meta::is_integral_type).count() == 2 &&
         query(kTerminalTypes).skipWhile(std::meta::is_integral_type).count() == 2 &&
         query(kTerminalTypes).count(std::meta::is_reference_type) == 1 &&
         query(kTerminalTypes).all(std::meta::is_type) &&
         query(kTerminalTypes).any(std::meta::is_floating_point_type) &&
         query(kTerminalTypes).none(std::meta::is_array_type) && reference.has_value() &&
         *reference == kIntRef && firstReference.has_value() && *firstReference == 2 &&
         lastReference.has_value() && *lastReference == 2 && references.count() == 1 &&
         nonReferences.count() == 3;
}

static_assert(rawPredicatesReachAdaptorsAndTerminals());

consteval auto rawRangeProducerCanFlatMap() -> bool {
  const auto arguments = query(kTemplates).flatMap(std::meta::template_arguments_of).materialize();
  return arguments.size() == 3 && arguments.at(0) == kInt && arguments.at(1) == kLong &&
         arguments.at(2) == kChar;
}

static_assert(rawRangeProducerCanFlatMap());

consteval auto rawFunctionTemplateSpecializationsCompose() -> bool {
  const auto reflected =
      query(kValues).map(std::meta::reflect_constant<int>).map(std::meta::extract<int>).materialize();
  const auto extracted = query(kReflectedValues).map(std::meta::extract<int>).materialize();
  return reflected.size() == 3 && reflected.at(0) == 1 && reflected.at(2) == 3 && extracted.size() == 2 &&
         extracted.at(0) == 1 && extracted.at(1) == 2;
}

static_assert(rawFunctionTemplateSpecializationsCompose());

consteval auto rawBinaryCallableUsesTupleAdaptation() -> bool {
  auto equalPairs = query(kZipLeft).zip(query(kZipRight)).filter(std::meta::is_same_type);
  return equalPairs.count() == 2;
}

static_assert(rawBinaryCallableUsesTupleAdaptation());

consteval auto rawBinaryPredicatesReachOrderingTerminals() -> bool {
  const auto sorted = query(kHierarchy).sortBy(std::meta::is_base_of_type);
  const auto first = sorted.first();
  const auto minimum = query(kHierarchy).minBy(std::meta::is_base_of_type);
  const auto maximum = query(kHierarchy).maxBy(std::meta::is_base_of_type);
  return first.has_value() && *first == kBase && minimum.has_value() && *minimum == kBase &&
         maximum.has_value() && *maximum == kDerived;
}

static_assert(rawBinaryPredicatesReachOrderingTerminals());

consteval auto rawOptionalCallableUsesFilterMap() -> bool {
  const auto annotations = query(kAnnotated).filterMap(std::meta::annotation_of_type<Marker>).materialize();
  return annotations.size() == 1 && annotations.at(0).value == marker.value;
}

static_assert(rawOptionalCallableUsesFilterMap());

consteval auto rawOptionalCallableUsesFindMap() -> bool {
  const auto annotation = query(kAnnotated).findMap(std::meta::annotation_of_type<Marker>);
  return annotation.has_value() && annotation->value == marker.value;
}

static_assert(rawOptionalCallableUsesFindMap());

consteval auto rawKeyProjectionsReachOrderingTerminals() -> bool {
  const auto minimum = query(kZipLeft).minByKey(std::meta::display_string_of);
  const auto maximum = query(kZipLeft).maxByKey(std::meta::display_string_of);
  return minimum.has_value() && maximum.has_value() &&
         std::meta::display_string_of(*minimum) <= std::meta::display_string_of(*maximum);
}

static_assert(rawKeyProjectionsReachOrderingTerminals());

consteval auto rawCallableCanDriveForEach() -> bool {
  query(kTypes).forEach(std::meta::display_string_of);
  return true;
}

static_assert(rawCallableCanDriveForEach());

consteval auto rawProjectionsSurvivePersistenceBarriers() -> bool {
  const auto members =
      query(std::meta::nonstatic_data_members_of(^^Members, std::meta::access_context::unchecked()))
          .sortByKey(std::meta::identifier_of)
          .uniqueBy(std::meta::type_of)
          .materialize();
  return members.size() == 2 && std::meta::identifier_of(members.at(0)) == "a" &&
         std::meta::identifier_of(members.at(1)) == "m" && std::meta::type_of(members.at(0)) == kLong &&
         std::meta::type_of(members.at(1)) == kInt;
}

static_assert(rawProjectionsSurvivePersistenceBarriers());

consteval auto rawSourcePipelineMaterializesInfo() -> bool {
  const auto arguments = query(std::meta::template_arguments_of(^^Pack<const int, long>))
                             .filter(std::meta::is_type)
                             .map(std::meta::remove_cv)
                             .sortByKey(std::meta::display_string_of)
                             .materialize();
  return arguments.size() == 2 && arguments.at(0) == kInt && arguments.at(1) == kLong;
}

static_assert(rawSourcePipelineMaterializesInfo());

} // namespace
