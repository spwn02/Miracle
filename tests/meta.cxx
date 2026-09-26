import std;
import Miracle.Meta;
import Switch;

namespace Tests::metaVocabulary {

struct Marker final {
  int value{};
  constexpr auto operator==(const Marker &) const -> bool = default;
};

struct Base {};

struct[[= Marker{7}]] Sample final : Base {
  int visible{};
  static inline int shared{};

  Sample() = default;
  auto method(long value) const -> long {
    return value;
  }

private:
  int hidden{};
};

enum class Color { red, green, blue };

template <class T>
struct Box {};

auto freeFunction(int, double) -> long;
using FreeFunctionType = long(int, double);

namespace Scope {
inline int value{};
auto function() -> void;
struct Type {};
} // namespace Scope

template <class T>
concept HasEnumerators = requires { Miracle::Reflect<T>{}.enumerators(); };

// Source-query coverage deliberately exercises caller-sensitive access, namespace subjects, function types,
// annotations, and template arguments separately; these are the semantic categories Phase 5.2 must later
// cache without API changes.
constexpr auto publicFields = Miracle::meta::fields<Sample>();
constexpr auto allFields = Miracle::meta::fields<Sample>(Miracle::Access::unchecked());
constexpr auto staticFields = Miracle::meta::staticFields<Sample>(Miracle::Access::unchecked());
constexpr auto enumValues = Miracle::meta::enumerators<Color>();
constexpr auto parameters = Miracle::meta::parameters(^^freeFunction);
constexpr auto typeParameters = Miracle::meta::parameters(^^FreeFunctionType);
constexpr auto annotationValues = Miracle::meta::annotations<Marker>(^^Sample);
constexpr auto templateArguments = Miracle::meta::templateArguments(^^Box<int>);

static_assert(publicFields.size() == 1);
static_assert(allFields.size() == 2);
static_assert(staticFields.size() == 1);
static_assert(Miracle::meta::bases<Sample>().size() == 1);
static_assert(Miracle::meta::functions<Sample>().size() >= 1);
static_assert(Miracle::meta::constructors<Sample>(Miracle::Access::unchecked()).size() >= 1);
static_assert(enumValues.size() == 3);
static_assert(parameters.size() == 2);
static_assert(typeParameters.size() == 2);
static_assert(annotationValues.size() == 1);
static_assert(templateArguments.size() == 1);
static_assert(Miracle::meta::members(^^Scope).size() >= 3);
static_assert(Miracle::meta::functions(^^Scope).size() >= 1);

// Functional-vocabulary coverage verifies both strict and optional projections before Query starts composing
// these values.
constexpr Miracle::meta::Info visibleField = publicFields.front();
constexpr Miracle::meta::Info annotation = annotationValues.front();
constexpr std::array<Miracle::meta::Info, 1> boxArguments{^^int};

static_assert(Miracle::meta::name(visibleField) == std::optional<std::string_view>{"visible"});
static_assert(not Miracle::meta::name(parameters.front()).has_value());
static_assert(Miracle::meta::requireName(visibleField) == "visible");
static_assert(not Miracle::meta::displayName(visibleField).empty());
static_assert(Miracle::meta::type(visibleField) == ^^int);
static_assert(Miracle::meta::type(^^Sample) == ^^Sample);
static_assert(Miracle::meta::returnType(^^freeFunction) == ^^long);
static_assert(Miracle::meta::returnType(^^FreeFunctionType) == ^^long);
static_assert(Miracle::meta::parent(visibleField) == ^^Sample);
static_assert(Miracle::meta::templateOf(^^Box<int>) == ^^Box);
static_assert(Miracle::meta::canSubstitute(^^Box, boxArguments));
static_assert(Miracle::meta::substitute(^^Box, boxArguments) == ^^Box<int>);
static_assert(Miracle::meta::extract<Marker>(Miracle::meta::constant(annotation)).value == 7);
static_assert(Miracle::meta::sourceLocation(^^Sample).line() > 0);

// Predicate checks include primitive and composed forms so the future Query DSL can rely on value-level
// boolean algebra.
static_assert(Miracle::meta::isType(^^Sample));
static_assert(Miracle::meta::isClass(^^Sample));
static_assert(Miracle::meta::isEnum(^^Color));
static_assert(Miracle::meta::isNamespace(^^Scope));
static_assert(Miracle::meta::isField(visibleField));
static_assert(Miracle::meta::isInstanceData(visibleField));
static_assert(Miracle::meta::annotated<Marker>(^^Sample));
static_assert((Miracle::meta::isType && !Miracle::meta::isNamespace)(^^Sample));
static_assert((Miracle::meta::isNamespace || Miracle::meta::isType)(^^Scope));
static_assert(not(Miracle::meta::isNamespace && Miracle::meta::isType)(^^Sample));

// `Reflect<T>` must remain only a façade over the same free vocabulary, including its enum-only constrained
// member.
static_assert(Miracle::reflect<Sample>().raw() == ^^Sample);
static_assert(Miracle::meta::raw(Miracle::reflect<Sample>()) == ^^Sample);
static_assert(Miracle::meta::raw(^^Sample) == ^^Sample);
static_assert(Miracle::reflect<Sample>().members().size() >= publicFields.size());
static_assert(Miracle::reflect<Sample>().fields().size() == publicFields.size());
static_assert(
    Miracle::reflect<Sample>().staticFields(Miracle::Access::unchecked()).size() == staticFields.size());
static_assert(Miracle::reflect<Sample>().functions().size() >= 1);
static_assert(Miracle::reflect<Sample>().constructors(Miracle::Access::unchecked()).size() >= 1);
static_assert(Miracle::reflect<Sample>().bases().size() == 1);
static_assert(Miracle::reflect<Sample>().annotations().size() == 1);
static_assert(Miracle::reflect<Color>().enumerators().size() == 3);
static_assert(HasEnumerators<Color>);
static_assert(not HasEnumerators<Sample>);
static_assert(not std::is_constructible_v<Miracle::Reflect<Sample>, Miracle::meta::Info>);
static_assert(
    std::same_as<std::remove_cvref_t<decltype(publicFields)>, std::span<const Miracle::meta::Info>>);

constexpr auto narrowName = Miracle::meta::requireName(visibleField);
constexpr auto narrowEnumCount = Miracle::meta::enumerators<Color>().size();
constexpr bool narrowReflectMatches = Miracle::reflect<Sample>().raw() == ^^Sample;

[[ = Switch::test, = Switch::group("foundation"), = Switch::tag("meta") ]] auto narrowModuleSurface()
    -> void {
  Switch::check(narrowName == "visible");
  Switch::check(narrowEnumCount == 3);
  Switch::check(narrowReflectMatches);
}

} // namespace Tests::metaVocabulary

consteval {
  Switch::discover<^^Tests::metaVocabulary>();
}
