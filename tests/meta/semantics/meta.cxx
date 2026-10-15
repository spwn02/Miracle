import std;
import Miracle.Meta;
import Switch;

namespace Tests::metaVocabulary {

struct Marker final {
  int value{};
  constexpr auto operator==(const Marker &) const -> bool = default;
};

struct OtherMarker final {
  int value{};
  constexpr auto operator==(const OtherMarker &) const -> bool = default;
};

struct[[= Marker{41}]][[= OtherMarker{73}]] CacheAnnotated final {};

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

public:
  static consteval auto fieldsFromMemberContext();
  static consteval auto reflectedFieldsFromMemberContext();
  static consteval auto cacheMatchesRawFromMemberContext() -> bool;
};

consteval auto Sample::fieldsFromMemberContext() {
  // The default access context must be captured at this member-function call site, where `hidden` is
  // accessible.
  return Miracle::meta::fields<Sample>();
}

consteval auto Sample::reflectedFieldsFromMemberContext() {
  // `Reflect<T>` must forward the caller's access value rather than recomputing `Access::current()` inside
  // the façade.
  return Miracle::reflect<Sample>().fields();
}

consteval auto Sample::cacheMatchesRawFromMemberContext() -> bool {
  // Validate the cache projection against the standard query in the *same* privileged context; using an
  // expected count alone could miss an access-context bug if both the test and implementation accidentally
  // used an unprivileged caller.
  const auto cached = Miracle::meta::fields<Sample>();
  const auto raw = std::meta::nonstatic_data_members_of(^^Sample, Miracle::Access::current());
  return std::ranges::equal(cached, raw);
}

enum class Color { red, green, blue };
enum class CompactColor : unsigned short { red, green, blue };

struct TransformProbe final {
  const int &reference;
  volatile long value{};
  CompactColor color{};
};

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

consteval auto cachedInvalidSubjectStillThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::fields(^^int, Miracle::Access::unchecked()));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto typedInvalidSubjectStillThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::fields<int>(Miracle::Access::unchecked()));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto invalidTypeTransformStillThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::removeCvref(^^Scope));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto invalidUnderlyingTypeStillThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::underlyingType(^^Sample));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto strictProjectionFailuresRemainCatchable() -> bool {
  try {
    static_cast<void>(Miracle::meta::parent(^^int));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::templateOf(^^int));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::constant(^^int));
    return false;
  } catch (const std::meta::exception &) {
  }
  constexpr std::array invalidArguments{^^int, ^^long};
  try {
    static_cast<void>(Miracle::meta::substitute(^^Box, invalidArguments));
    return false;
  } catch (const std::meta::exception &) {
  }
  return true;
}

// Source-query coverage deliberately exercises caller-sensitive access, namespace subjects, function types,
// annotations, and template arguments separately; these are the semantic categories the cache layer  must
// later cache without API changes.
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
static_assert(cachedInvalidSubjectStillThrows());
static_assert(typedInvalidSubjectStillThrows());
static_assert(invalidTypeTransformStillThrows());
static_assert(invalidUnderlyingTypeStillThrows());
static_assert(strictProjectionFailuresRemainCatchable());

// Cache invariants: unchecked type queries expose the canonical backing span directly, and the
// Reflect façade delegates to those same typed caches rather than maintaining a parallel reflection universe.
constexpr auto cachedMembers = Miracle::meta::members<Sample>(Miracle::Access::unchecked());
constexpr auto cachedFields = Miracle::meta::fields<Sample>(Miracle::Access::unchecked());
constexpr auto cachedStaticFields = Miracle::meta::staticFields<Sample>(Miracle::Access::unchecked());
constexpr auto cachedFunctions = Miracle::meta::functions<Sample>(Miracle::Access::unchecked());
constexpr auto cachedConstructors = Miracle::meta::constructors<Sample>(Miracle::Access::unchecked());
constexpr auto cachedBases = Miracle::meta::bases<Sample>(Miracle::Access::unchecked());
constexpr auto cachedEnumerators = Miracle::meta::enumerators<Color>();
constexpr auto cachedAnnotations = Miracle::meta::annotations<Sample>();
constexpr auto cachedTypedAnnotations = Miracle::meta::annotations<Marker, Sample>();

constexpr auto cachedFieldsAgain = Miracle::meta::fields<Sample>(Miracle::Access::unchecked());
constexpr auto reflected = Miracle::reflect<Sample>();

static_assert(cachedMembers.data() == reflected.members(Miracle::Access::unchecked()).data());
static_assert(cachedFields.data() == cachedFieldsAgain.data());
static_assert(cachedFields.data() == reflected.fields(Miracle::Access::unchecked()).data());
static_assert(cachedStaticFields.data() == reflected.staticFields(Miracle::Access::unchecked()).data());
static_assert(cachedFunctions.data() == reflected.functions(Miracle::Access::unchecked()).data());
static_assert(cachedConstructors.data() == reflected.constructors(Miracle::Access::unchecked()).data());
static_assert(cachedBases.data() == reflected.bases(Miracle::Access::unchecked()).data());
static_assert(cachedEnumerators.data() == Miracle::reflect<Color>().enumerators().data());
static_assert(cachedAnnotations.data() == reflected.annotations().data());
static_assert(cachedTypedAnnotations.data() == Miracle::meta::annotations<Marker, Sample>().data());

// Raw-Info subjects cross a private reflection/substitution bridge into the same NTTP specialization. Pointer
// identity proves that `meta::fields(^^T)`, `meta::fields<T>()`, and `Reflect<T>::fields()` do not own
// parallel universes.
constexpr auto rawUncheckedFields = Miracle::meta::fields(^^Sample, Miracle::Access::unchecked());
static_assert(rawUncheckedFields.data() == cachedFields.data());

// Access-filtered typed/raw calls also converge on one `(subject, category, Access)` cache specialization.
constexpr auto visibleTypedFields = Miracle::meta::fields<Sample>(Miracle::Access::unprivileged());
constexpr auto visibleRawFields = Miracle::meta::fields(^^Sample, Miracle::Access::unprivileged());
static_assert(visibleTypedFields.data() == visibleRawFields.data());
static_assert(visibleTypedFields.data() != cachedFields.data());
static_assert(std::ranges::equal(visibleTypedFields,
    std::meta::nonstatic_data_members_of(^^Sample, Miracle::Access::unprivileged())));
constexpr auto viaSample = Miracle::Access::unprivileged().via(^^Sample);
static_assert(std::ranges::equal(Miracle::meta::fields<Sample>(viaSample),
    std::meta::nonstatic_data_members_of(^^Sample, viaSample)));
static_assert(std::ranges::equal(Miracle::meta::members(^^Scope, Miracle::Access::unprivileged()),
    std::meta::members_of(^^Scope, Miracle::Access::unprivileged())));

// Non-access-sensitive sources use the same canonical identity rule, including two-key typed annotations.
static_assert(Miracle::meta::parameters(^^freeFunction).data() == parameters.data());
static_assert(Miracle::meta::templateArguments(^^Box<int>).data() == templateArguments.data());
static_assert(Miracle::meta::annotations<Marker>(^^Sample).data() == cachedTypedAnnotations.data());

// Typed annotation universes are keyed independently by annotation object type. Repeating one key must
// reuse its backing without forcing the other typed universe to become part of the same cache entry.
constexpr auto cacheMarkerAnnotations = Miracle::meta::annotations<Marker>(^^CacheAnnotated);
constexpr auto cacheOtherAnnotations = Miracle::meta::annotations<OtherMarker>(^^CacheAnnotated);
constexpr auto cacheMarkerAnnotationsAgain = Miracle::meta::annotations<Marker>(^^CacheAnnotated);
static_assert(cacheMarkerAnnotations.size() == 1);
static_assert(cacheOtherAnnotations.size() == 1);
static_assert(cacheMarkerAnnotations.data() == cacheMarkerAnnotationsAgain.data());
static_assert(
    Miracle::meta::extract<Marker>(Miracle::meta::constant(cacheMarkerAnnotations.front())).value == 41);
static_assert(
    Miracle::meta::extract<OtherMarker>(Miracle::meta::constant(cacheOtherAnnotations.front())).value == 73);

// Every supported reflection-source carrier is the same composable Query source shape. Chaining these spans
// must preserve reflection identity rather than copying Info values into a parallel representation.
constexpr auto namespaceMembers = Miracle::meta::members(^^Scope, Miracle::Access::unchecked());
constexpr auto namespaceFunctions = Miracle::meta::functions(^^Scope, Miracle::Access::unchecked());
constexpr auto composedReflectionSources = Miracle::meta::query(cachedMembers)
                                               .chain(Miracle::meta::query(cachedFields))
                                               .chain(Miracle::meta::query(cachedStaticFields))
                                               .chain(Miracle::meta::query(cachedFunctions))
                                               .chain(Miracle::meta::query(cachedConstructors))
                                               .chain(Miracle::meta::query(cachedBases))
                                               .chain(Miracle::meta::query(cachedEnumerators))
                                               .chain(Miracle::meta::query(parameters))
                                               .chain(Miracle::meta::query(typeParameters))
                                               .chain(Miracle::meta::query(cachedAnnotations))
                                               .chain(Miracle::meta::query(cachedTypedAnnotations))
                                               .chain(Miracle::meta::query(templateArguments))
                                               .chain(Miracle::meta::query(namespaceMembers))
                                               .chain(Miracle::meta::query(namespaceFunctions));
static_assert(std::same_as<typename decltype(composedReflectionSources)::Value, const Miracle::meta::Info &>);
static_assert(composedReflectionSources.count() ==
              cachedMembers.size() + cachedFields.size() + cachedStaticFields.size() +
                  cachedFunctions.size() + cachedConstructors.size() + cachedBases.size() +
                  cachedEnumerators.size() + parameters.size() + typeParameters.size() +
                  cachedAnnotations.size() + cachedTypedAnnotations.size() + templateArguments.size() +
                  namespaceMembers.size() + namespaceFunctions.size());
constexpr auto pairedReflectionSources =
    Miracle::meta::query(cachedFields).zip(Miracle::meta::query(cachedFields));
static_assert(std::same_as<typename decltype(pairedReflectionSources)::Value,
    std::pair<const Miracle::meta::Info &, const Miracle::meta::Info &>>);
static_assert(std::addressof(pairedReflectionSources.first()->first) == std::addressof(cachedFields.front()));
static_assert(
    std::addressof(pairedReflectionSources.first()->second) == std::addressof(cachedFields.front()));

// Caller-sensitive filtering happens over the cached unchecked universe: an ordinary namespace-scope call
// sees only the public field while a call originating in Sample's member context sees both public and private
// data members.
static_assert(publicFields.size() == 1);
static_assert(visibleTypedFields.size() == 1);
static_assert(Sample::fieldsFromMemberContext().size() == 2);
static_assert(Sample::reflectedFieldsFromMemberContext().size() == 2);
static_assert(Sample::cacheMatchesRawFromMemberContext());

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
static_assert(Miracle::meta::removeCv(^^const volatile int) == ^^int);
static_assert(Miracle::meta::removeReference(^^const int &) == ^^const int);
static_assert(Miracle::meta::removeCvref(^^const int &) == ^^int);
static_assert(Miracle::meta::underlyingType(^^CompactColor) == ^^unsigned short);
static_assert(Miracle::meta::templateOf(^^Box<int>) == ^^Box);
static_assert(Miracle::meta::canSubstitute(^^Box, boxArguments));
static_assert(Miracle::meta::substitute(^^Box, boxArguments) == ^^Box<int>);
static_assert(Miracle::meta::extract<Marker>(Miracle::meta::constant(annotation)).value == 7);
static_assert(Miracle::meta::sourceLocation(^^Sample).line() > 0);

constexpr auto normalizedTransformTypes =
    Miracle::meta::query(Miracle::meta::fields<TransformProbe>(Miracle::Access::unchecked()))
        .map(Miracle::meta::type)
        .map(Miracle::meta::removeCvref)
        .materialize();
static_assert(normalizedTransformTypes.size() == 3);
static_assert(normalizedTransformTypes[0] == ^^int);
static_assert(normalizedTransformTypes[1] == ^^long);
static_assert(normalizedTransformTypes[2] == ^^CompactColor);

constexpr auto enumUnderlyingFromField =
    Miracle::meta::query(Miracle::meta::fields<TransformProbe>(Miracle::Access::unchecked()))
        .map(Miracle::meta::type)
        .filter(Miracle::meta::isEnum)
        .map(Miracle::meta::underlyingType)
        .first();
static_assert(enumUnderlyingFromField.has_value());
static_assert(*enumUnderlyingFromField == ^^unsigned short);

// Predicate checks include primitive and composed forms so the future Query DSL can rely on value-level
// boolean algebra.
static_assert(Miracle::meta::hasIdentifier(^^Sample));
static_assert(not Miracle::meta::hasIdentifier(^^const int));
static_assert(Miracle::meta::hasParent(visibleField));
static_assert(not Miracle::meta::hasParent(^^int));
static_assert(Miracle::meta::hasTemplateArguments(^^Box<int>));
static_assert(not Miracle::meta::hasTemplateArguments(^^int));
using PredicateFunctionType = int(double);
static_assert(Miracle::meta::isFunctionType(^^PredicateFunctionType));
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

namespace Tests::meta_edge_cases {

struct EdgeMarker final {
  int value{};
  constexpr auto operator==(const EdgeMarker &) const -> bool = default;
};

struct Complete final {
  int value{};
};
struct Incomplete;
enum class ForwardEnum : int;
enum class EdgeColor : unsigned char { red = 4, blue = 9 };
struct EdgeBase {};
struct EdgeDerived : EdgeBase {};

template <class T>
struct EdgeBox {};

template <class T, int N, template <class> class Container>
struct MixedTemplate {};

template <class T>
auto templated(T first[[= EdgeMarker{11}]], int second) -> long {
  return static_cast<long>(second);
}

using EdgeFunctionType = long(double, int);

struct[[ = EdgeMarker{1}, = EdgeMarker{2} ]] RepeatedAnnotated final {};

struct AccessBase {
public:
  int publicValue{};

protected:
  int protectedValue{};

private:
  int privateValue{};
};

struct AccessDerived : AccessBase {};

union EdgeUnion {
  int integer;
  double real;
};

struct ExplicitObject final {
  auto method(this const ExplicitObject &, int value) -> int {
    return value;
  }
};

namespace Scope {
using Alias = Complete;
namespace AliasTarget {}
namespace NamespaceAlias = AliasTarget;
template <class T>
inline int variableTemplate{};
template <class T>
auto functionTemplate(T) -> T;
template <class T>
concept EdgeConcept = true;
inline int value{};
} // namespace Scope

consteval auto findAlias() -> Miracle::meta::Info {
  for (const auto info : Miracle::meta::members(^^Scope, Miracle::Access::unchecked())) {
    if (Miracle::meta::isTypeAlias(info)) {
      return info;
    }
  }
  return {};
}

consteval auto incompleteRawSourcesThrow() -> bool {
  try {
    static_cast<void>(Miracle::meta::members(^^Incomplete, Miracle::Access::unchecked()));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::fields(^^Incomplete, Miracle::Access::unchecked()));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::staticFields(^^Incomplete, Miracle::Access::unchecked()));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::functions(^^Incomplete, Miracle::Access::unchecked()));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::constructors(^^Incomplete, Miracle::Access::unchecked()));
    return false;
  } catch (const std::meta::exception &) {
  }
  try {
    static_cast<void>(Miracle::meta::bases(^^Incomplete, Miracle::Access::unchecked()));
    return false;
  } catch (const std::meta::exception &) {
  }
  return true;
}

consteval auto incompleteTypedSourcesThrow() -> bool {
  try {
    static_cast<void>(Miracle::meta::members<Incomplete>(Miracle::Access::unchecked()));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto forwardEnumRawThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::enumerators(^^ForwardEnum));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto forwardEnumTypedThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::enumerators<ForwardEnum>());
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto functionTemplateParametersThrow() -> bool {
  try {
    static_cast<void>(Miracle::meta::parameters(^^templated));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto functionTemplateReturnThrows() -> bool {
  try {
    static_cast<void>(Miracle::meta::returnType(^^templated));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto invalidTemplateArgumentsThrow() -> bool {
  try {
    static_cast<void>(Miracle::meta::templateArguments(^^Complete));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto reflectedIncompleteThrows() -> bool {
  try {
    static_cast<void>(Miracle::reflect<Incomplete>().fields(Miracle::Access::unchecked()));
  } catch (const std::meta::exception &) {
    return true;
  }
  return false;
}

consteval auto explicitObjectMethod() -> Miracle::meta::Info {
  for (const auto info : Miracle::meta::functions<ExplicitObject>(Miracle::Access::unchecked())) {
    if (Miracle::meta::name(info) == std::optional<std::string_view>{"method"}) {
      return info;
    }
  }
  return {};
}

constexpr auto alias = findAlias();
static_assert(alias != Miracle::meta::Info{});
static_assert(Miracle::meta::name(alias) == std::optional<std::string_view>{"Alias"});
// A type-alias reflection is itself a type reflection; `type()` intentionally preserves alias identity while
// `dealias()` performs the separate canonicalization step.
static_assert(Miracle::meta::type(alias) == alias);
static_assert(Miracle::meta::dealias(alias) == ^^Complete);
static_assert(Miracle::meta::dealias(Miracle::meta::type(alias)) == ^^Complete);
constexpr auto namespaceAlias =
    Miracle::meta::query(Miracle::meta::members(^^Scope, Miracle::Access::unchecked()))
        .find(Miracle::meta::isNamespaceAlias);
static_assert(namespaceAlias.has_value());
static_assert(Miracle::meta::name(*namespaceAlias) == std::optional<std::string_view>{"NamespaceAlias"});
static_assert(Miracle::meta::dealias(*namespaceAlias) == ^^Scope::AliasTarget);

static_assert(Miracle::meta::isComplete(^^Complete));
static_assert(not Miracle::meta::isComplete(^^Incomplete));
static_assert(not Miracle::meta::isComplete(^^Scope));
static_assert(incompleteRawSourcesThrow());
static_assert(incompleteTypedSourcesThrow());
static_assert(reflectedIncompleteThrows());

static_assert(not Miracle::meta::isEnumerable(^^ForwardEnum));
static_assert(forwardEnumRawThrows());
static_assert(forwardEnumTypedThrows());

constexpr auto specializationParameters = Miracle::meta::parameters(^^templated<int>);
static_assert(specializationParameters.size() == 2);
static_assert(Miracle::meta::returnType(^^templated<int>) == ^^long);
static_assert(Miracle::meta::annotations<EdgeMarker>(specializationParameters.front()).size() == 1);
static_assert(Miracle::meta::extract<EdgeMarker>(
                  Miracle::meta::constant(
                      Miracle::meta::annotations<EdgeMarker>(specializationParameters.front()).front()))
                  .value == 11);
static_assert(functionTemplateParametersThrow());
static_assert(functionTemplateReturnThrows());
static_assert(invalidTemplateArgumentsThrow());
static_assert(Miracle::meta::parameters(^^EdgeFunctionType).size() == 2);
static_assert(Miracle::meta::parent(specializationParameters.front()) == ^^templated<int>);

constexpr auto mixedArguments = Miracle::meta::templateArguments(^^MixedTemplate<int, 7, EdgeBox>);
static_assert(mixedArguments.size() == 3);
static_assert(mixedArguments[0] == ^^int);
static_assert(Miracle::meta::extract<int>(mixedArguments[1]) == 7);
static_assert(std::meta::is_template(mixedArguments[2]));
static_assert(Miracle::meta::templateOf(^^MixedTemplate<int, 7, EdgeBox>) == ^^MixedTemplate);
static_assert(Miracle::meta::canSubstitute(^^MixedTemplate, mixedArguments));
static_assert(Miracle::meta::substitute(^^MixedTemplate, mixedArguments) == ^^MixedTemplate<int, 7, EdgeBox>);

constexpr auto repeatedAnnotations = Miracle::meta::annotations<EdgeMarker>(^^RepeatedAnnotated);
static_assert(repeatedAnnotations.size() == 2);
static_assert(Miracle::meta::extract<EdgeMarker>(Miracle::meta::constant(repeatedAnnotations[0])).value == 1);
static_assert(Miracle::meta::extract<EdgeMarker>(Miracle::meta::constant(repeatedAnnotations[1])).value == 2);

constexpr auto unprivilegedBaseFields = Miracle::meta::fields<AccessBase>(Miracle::Access::unprivileged());
constexpr auto viaDerived = Miracle::Access::unprivileged().via(^^AccessDerived);
constexpr auto derivedVisibleBaseFields = Miracle::meta::fields(^^AccessBase, viaDerived);
static_assert(std::ranges::equal(derivedVisibleBaseFields,
    std::meta::nonstatic_data_members_of(^^AccessBase, viaDerived)));
static_assert(unprivilegedBaseFields.size() <= derivedVisibleBaseFields.size());
constexpr auto allAccessFields = Miracle::meta::fields<AccessBase>(Miracle::Access::unchecked());
static_assert(Miracle::meta::query(allAccessFields).count(Miracle::meta::isProtected) == 1);
static_assert(Miracle::meta::query(allAccessFields).count(Miracle::meta::isPrivate) == 1);

static_assert(Miracle::meta::isUnion(^^EdgeUnion));
static_assert(Miracle::meta::fields<EdgeUnion>(Miracle::Access::unchecked()).size() == 2);

constexpr auto explicitMethod = explicitObjectMethod();
static_assert(explicitMethod != Miracle::meta::Info{});
constexpr auto explicitParameters = Miracle::meta::parameters(explicitMethod);
static_assert(explicitParameters.size() == 2);
static_assert(std::meta::is_explicit_object_parameter(explicitParameters[0]));
static_assert(Miracle::meta::type(explicitParameters[0]) == ^^const ExplicitObject &);
static_assert(Miracle::meta::type(explicitParameters[1]) == ^^int);

constexpr auto scopeMembers = Miracle::meta::members(^^Scope, Miracle::Access::unchecked());
static_assert(Miracle::meta::query(scopeMembers).any(Miracle::meta::isTypeAlias));
static_assert(Miracle::meta::query(scopeMembers).any(Miracle::meta::isNamespaceAlias));
static_assert(Miracle::meta::query(scopeMembers).any(Miracle::meta::isFunctionTemplate));
static_assert(Miracle::meta::query(scopeMembers).any(Miracle::meta::isConcept));
static_assert(Miracle::meta::query(scopeMembers).any(std::meta::is_concept));
static_assert(Miracle::meta::query(scopeMembers).count(std::meta::is_concept) ==
              Miracle::meta::query(scopeMembers).count(Miracle::meta::isConcept));
static_assert(Miracle::meta::query(scopeMembers).any(Miracle::meta::isTemplate));

constexpr auto edgeBase = Miracle::meta::bases<EdgeDerived>(Miracle::Access::unchecked()).front();
constexpr auto edgeEnumerator = Miracle::meta::enumerators<EdgeColor>().front();
static_assert(Miracle::meta::type(edgeBase) == ^^EdgeBase);
static_assert(Miracle::meta::type(edgeEnumerator) == ^^EdgeColor);
static_assert(Miracle::meta::type(specializationParameters.front()) == ^^int);
static_assert(Miracle::meta::parent(edgeBase) == ^^EdgeDerived);
static_assert(Miracle::meta::parent(edgeEnumerator) == ^^EdgeColor);
static_assert(Miracle::meta::extract<EdgeColor>(Miracle::meta::constant(edgeEnumerator)) == EdgeColor::red);

// The reference compiler intentionally reports no declared identifier for cv-qualified types.
// Miracle's optional name projection follows that standard boundary while diagnostics remain printable.
static_assert(not Miracle::meta::name(^^const int).has_value());
static_assert(not Miracle::meta::displayName(^^const int).empty());

} // namespace Tests::meta_edge_cases

namespace Tests::meta_query {

constexpr auto fields = Miracle::meta::fields<Tests::metaVocabulary::Sample>(Miracle::Access::unchecked());
constexpr auto baseQuery = Miracle::meta::query(fields);

static_assert(std::same_as<typename decltype(baseQuery)::Value, const Miracle::meta::Info &>);
static_assert(baseQuery.size() == fields.size());
static_assert(baseQuery.count() == fields.size());
static_assert(not baseQuery.isEmpty());
static_assert(baseQuery.first().has_value());
static_assert(baseQuery.first().value() == fields.front());
static_assert(baseQuery.last().has_value());
static_assert(baseQuery.last().value() == fields.back());
static_assert(baseQuery.nth(1).has_value());
static_assert(baseQuery.position(Miracle::meta::isField) == 0);
static_assert(baseQuery.rposition(Miracle::meta::isField) == fields.size() - 1);
static_assert(baseQuery.all(Miracle::meta::isField));
static_assert(baseQuery.any(Miracle::meta::isField));
static_assert(not baseQuery.none(Miracle::meta::isField));
static_assert(baseQuery.contains(fields.front()));
static_assert(baseQuery.find(Miracle::meta::isField).has_value());

constexpr auto names = baseQuery.map(Miracle::meta::name);
static_assert(std::same_as<typename decltype(names)::Value, std::optional<std::string_view>>);
constexpr auto fieldTypes = baseQuery.map(std::meta::type_of);
static_assert(fieldTypes.count() == fields.size());
static_assert(fieldTypes.first().value() == std::meta::type_of(fields.front()));
static_assert(names.count() == fields.size());
static_assert(names.first().has_value());
static_assert(names.first().value().has_value());
static_assert(names.findMap([](std::optional<std::string_view> name) consteval { return name; }).has_value());

constexpr auto namedFields = baseQuery.filterMap(Miracle::meta::name);
static_assert(namedFields.count() == fields.size());
static_assert(namedFields.first().has_value());
static_assert(namedFields.first().value() == "visible");

constexpr auto filtered = baseQuery.filter(
    [](Miracle::meta::Info info) consteval { return Miracle::meta::name(info).has_value(); });
static_assert(filtered.count() == fields.size());
static_assert(filtered.take(1).count() == 1);
static_assert(filtered.skip(1).count() == fields.size() - 1);
static_assert(
    filtered.takeWhile([](Miracle::meta::Info) consteval { return true; }).count() == fields.size());
static_assert(
    filtered.skipWhile([](Miracle::meta::Info) consteval { return false; }).count() == fields.size());
static_assert(filtered.slice(0, 1).count() == 1);

constexpr auto enumerated = filtered.enumerate();
static_assert(enumerated.first().has_value());
static_assert(enumerated.first()->first == 0);
static_assert(enumerated.first()->second == fields.front());

constexpr auto boolValues = baseQuery.map(Miracle::meta::isField);
constexpr auto intSource = std::array{3, 1, 2, 2};
constexpr auto intValues = Miracle::meta::query(intSource);
static_assert(std::same_as<typename decltype(intValues)::Value, const int &>);

struct EndpointRange final {
  int start{};
  int stop{};

  constexpr auto begin() const {
    return std::views::iota(start, stop).begin();
  }
  constexpr auto end() const {
    return std::views::iota(start, stop).end();
  }
};

// Endpoint-bearing interval types (including Miracle::Range) describe the slice by their actual endpoints,
// rather than by the first two values generated when the interval itself is iterated.
static_assert(intValues.slice(EndpointRange{1, 3}).count() == 2);
static_assert(intValues.slice(std::array{1, 3}).count() == 2);

consteval auto mutableArraySourcePreservesExactReferences() -> bool {
  std::array values{10, 20, 30};
  auto mutableValues = Miracle::meta::query(values);
  static_assert(std::same_as<typename decltype(mutableValues)::Value, int &>);

  auto first = mutableValues.first();
  if (not first.has_value() or std::addressof(first.value()) != std::addressof(values[0])) {
    return false;
  }

  mutableValues.forEach([](int &value) consteval { value += 1; });
  return values == std::array{11, 21, 31};
}
static_assert(mutableArraySourcePreservesExactReferences());

consteval auto rvalueReferenceBarrierPreservesCategoryAndIdentity() -> bool {
  auto rvalues = Miracle::meta::query(std::span<const int>{intSource})
                     .map([](const int &value) consteval -> const int && { return std::move(value); })
                     .reverse();
  static_assert(std::same_as<typename decltype(rvalues)::Value, const int &&>);

  std::array<const int *, 4> addresses{};
  std::size_t index{};
  rvalues.forEach([&](const int &&value) consteval { addresses[index++] = std::addressof(value); });
  return addresses[0] == std::addressof(intSource[3]) and addresses[1] == std::addressof(intSource[2]) and
         addresses[2] == std::addressof(intSource[1]) and addresses[3] == std::addressof(intSource[0]);
}
static_assert(rvalueReferenceBarrierPreservesCategoryAndIdentity());

template <class QueryType>
concept HasFirstTerminal = requires(QueryType query) { query.first(); };

template <class QueryType>
concept HasMinTerminal = requires(QueryType query) { query.min(); };

constexpr auto rvalueReferenceValues =
    Miracle::meta::query(std::span<const int>{intSource}).map([](const int &value) consteval -> const int && {
      return std::move(value);
    });
static_assert(std::same_as<typename decltype(rvalueReferenceValues)::Value, const int &&>);
static_assert(not HasFirstTerminal<decltype(rvalueReferenceValues)>);
static_assert(not HasMinTerminal<decltype(rvalueReferenceValues)>);
static_assert(rvalueReferenceValues.count() == intSource.size());
static_assert(not rvalueReferenceValues.isEmpty());
static_assert(
    rvalueReferenceValues.filter([](const int &&value) consteval { return value > 1; }).count() == 3);
static_assert(rvalueReferenceValues.any([](const int &&value) consteval { return value == 1; }));

constexpr auto typeStableIntValues = intValues.filter([](int) consteval { return true; })
                                         .map([](const int &value) consteval -> const int & { return value; })
                                         .skip(0);
static_assert(std::same_as<decltype(typeStableIntValues), decltype(intValues)>);
static_assert(typeStableIntValues.count() == intValues.count());

// Structural same-type operations are plan data, not C++ type topology. A deep static pipeline therefore has
// the exact same public Query type and object size as its empty source query.
consteval auto deepStaticSameTypeQuery() {
  auto query = intValues;
  for (std::size_t index{}; index < 64; ++index) {
    query = std::move(query)
                .filter([](const int &) consteval { return true; })
                .map([](const int &value) consteval -> const int & { return value; });
  }
  return query;
}
constexpr auto deepStaticQuery = deepStaticSameTypeQuery();
static_assert(std::same_as<decltype(deepStaticQuery), decltype(intValues)>);
static_assert(sizeof(deepStaticQuery) == sizeof(intValues));
static_assert(deepStaticQuery.count() == intValues.count());

static_assert(intValues.slice(1, 3).count() == 2);
static_assert(intValues.slice(3, 1).count() == 0);

// Adjacent Slice restrictions normalize only when doing so preserves not just output values but also the
// exact number of source values observed by an earlier side effect. The exact-boundary cases deliberately
// stay as separate instructions because the established Slice semantics would otherwise request one
// additional source item.
consteval auto normalizedSliceFusionPreservesObservations() -> bool {
  constexpr std::array source{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

  auto run = [&]<class Build>(Build build, std::size_t expectedCount, std::size_t expectedSeen) consteval {
    std::size_t seen{};
    auto observed = Miracle::meta::query(source).inspect([&](const int &) consteval { ++seen; });
    auto query = build(std::move(observed));
    return query.count() == expectedCount and seen == expectedSeen;
  };

  return run([](auto query) consteval { return std::move(query).skip(2).slice(1, 3); }, 2, 5) and
         run([](auto query) consteval { return std::move(query).slice(1, 5).take(2); }, 2, 3) and
         run([](auto query) consteval { return std::move(query).slice(1, 5).skip(2); }, 2, 5) and
         run([](auto query) consteval { return std::move(query).slice(1, 6).slice(2, 4); }, 2, 5) and
         run([](auto query) consteval { return std::move(query).slice(1, 3).skip(2); }, 0, 3) and
         run([](auto query) consteval { return std::move(query).slice(1, 3).slice(2, 4); }, 0, 3) and
         run([](auto query) consteval { return std::move(query).take(0); }, 0, 1);
}
static_assert(normalizedSliceFusionPreservesObservations());

struct MappedValue final {
  int value{};
  constexpr auto operator==(const MappedValue &) const -> bool = default;
};

constexpr auto mappedValues = intValues.map([](int value) consteval { return MappedValue{value * 2}; });
static_assert(std::same_as<typename decltype(mappedValues)::Value, MappedValue>);
static_assert(mappedValues.nth(2)->value == 4);

// Owning Query values are xvalues during fused execution. Predicates/inspection and predicate terminals must
// receive that exact logical category rather than an implementation lvalue created by the segment cursor.
constexpr auto owningInts = intValues.map([](const int &value) consteval { return value; });
static_assert(std::same_as<typename decltype(owningInts)::Value, int>);
static_assert(owningInts.filter([](int &&value) consteval { return value % 2 == 0; }).count() == 2);
static_assert(owningInts.inspect([](int &&) consteval {}).take(2).count() == 2);
static_assert(owningInts.takeWhile([](int &&value) consteval { return value > 0; }).count() == 4);
static_assert(owningInts.skipWhile([](int &&value) consteval { return value > 1; }).count() == 3);
static_assert(owningInts.position([](int &&value) consteval { return value == 2; }).value() == 2);
static_assert(owningInts.rposition([](int &&value) consteval { return value == 2; }).value() == 3);
static_assert(owningInts.all([](int &&value) consteval { return value > 0; }));
static_assert(owningInts.any([](int &&value) consteval { return value == 1; }));

struct RvalueComparable final {
  int value{};
  consteval auto operator==(int other) && -> bool {
    return value == other;
  }
};
static_assert(intValues.map([](const int &value) consteval { return RvalueComparable{value}; }).contains(2));

constexpr auto tupleSource = std::array{std::pair{1, 2}, std::pair{3, 4}};
constexpr auto tupleSums =
    Miracle::meta::query(tupleSource).map([](int left, int right) consteval { return left + right; });
static_assert(tupleSums.first().value() == 3);

struct AggregateValue final {
  int left{};
  int right{};
};
constexpr auto aggregateSource = std::array{AggregateValue{2, 3}, AggregateValue{5, 7}};
constexpr auto aggregateProducts =
    Miracle::meta::query(aggregateSource).map([](int left, int right) consteval { return left * right; });
static_assert(aggregateProducts.first().value() == 6);
static_assert(aggregateProducts.last().value() == 35);

// Exceptions are reserved for genuine semantic interpretation/fallback. `meta::type` has no general
// applicability predicate: types are already their own type, typed entities project through `type_of`, while
// a namespace has no reflected type. This is therefore a natural speculative interpretation rather than
// Boolean branching disguised as exceptions.
constexpr auto mixedSubjects =
    std::array{^^int, ^^Tests::metaVocabulary::Sample, ^^Tests::metaVocabulary::Scope};
constexpr auto interpretedTypes =
    Miracle::meta::query(mixedSubjects).map([](Miracle::meta::Info info) consteval {
      try {
        return Miracle::meta::type(info);
      } catch (const std::meta::exception &) {
        return info;
      }
    });
static_assert(interpretedTypes.nth(0).value() == ^^int);
static_assert(interpretedTypes.nth(1).value() == ^^Tests::metaVocabulary::Sample);
static_assert(interpretedTypes.nth(2).value() == ^^Tests::metaVocabulary::Scope);

constexpr auto refFilterMap = Miracle::meta::query(std::span<const int>{intSource})
                                  .filterMap([](const int &value) consteval -> std::optional<const int &> {
                                    return value > 1 ? std::optional<const int &>{value} : std::nullopt;
                                  });
static_assert(std::same_as<typename decltype(refFilterMap)::Value, const int &>);
static_assert(refFilterMap.first().value() == 3);

struct IntProxy final {
  int value{};
  consteval operator int() && {
    return value;
  }
};

struct ProxyOptional final {
  using value_type = int;
  bool engaged{};
  int value{};

  [[nodiscard]] consteval auto has_value() const -> bool {
    return engaged;
  }
  [[nodiscard]] consteval auto operator*() & -> IntProxy {
    return {value};
  }
};

// Optional-like is a protocol, not a spelling of std::optional. Owning proxy dereferences materialize exactly
// one declared value_type instead of leaking the proxy into the next typed segment.
constexpr auto proxyFilterMap =
    intValues.filterMap([](int value) consteval -> ProxyOptional { return {value != 1, value * 10}; });
static_assert(std::same_as<typename decltype(proxyFilterMap)::Value, int>);
static_assert(proxyFilterMap.count() == 3);
static_assert(proxyFilterMap.first().value() == 30);
constexpr auto zipped = filtered.map(Miracle::meta::isField).zip(boolValues);
static_assert(zipped.count() == fields.size());
static_assert(zipped.first().has_value());
static_assert(zipped.first()->first);
static_assert(zipped.first()->second);

// Zip intentionally preserves the established composition order: both children finish evaluation before the
// first pair reaches the downstream sink. Its two evaluation-local pairing buffers are therefore semantically
// required, not hidden lazy-stage materialization.
consteval auto zipPreservesEstablishedChildEvaluationOrder() -> bool {
  constexpr std::array left{1, 2};
  constexpr std::array right{3, 4};
  int trace{};
  auto lhs = Miracle::meta::query(left).inspect([&](const int &) consteval { trace = trace * 10 + 1; });
  auto rhs = Miracle::meta::query(right).inspect([&](const int &) consteval { trace = trace * 10 + 2; });
  auto first = std::move(lhs).zip(std::move(rhs)).first();
  return first.has_value() and trace == 1122;
}
static_assert(zipPreservesEstablishedChildEvaluationOrder());

constexpr auto chained = filtered.chain(filtered);
static_assert(chained.count() == fields.size() * 2);

constexpr auto nested = std::array{std::array{1, 2}, std::array{3, 4}};
constexpr auto flattened = Miracle::meta::query(nested).flatten();
static_assert(flattened.count() == 4);
static_assert(flattened.nth(2).value() == 3);

constexpr auto flatSource = std::array{1, 2};
constexpr auto flatMapped = Miracle::meta::query(flatSource).flatMap([](int value) consteval {
  return std::array{value, value + 10};
});
static_assert(flatMapped.count() == 4);
static_assert(flatMapped.nth(3).value() == 12);

// Flattening a proxy range materializes the declared owning range value instead of exposing proxy internals.
constexpr auto proxyFlattened = Miracle::meta::query(flatSource).flatMap([](int value) consteval {
  std::vector<bool> bits;
  bits.push_back(value == 1);
  bits.push_back(value == 2);
  return bits;
});
static_assert(std::same_as<typename decltype(proxyFlattened)::Value, bool>);
static_assert(proxyFlattened.count() == 4);
static_assert(proxyFlattened.nth(0).value());
static_assert(not proxyFlattened.nth(1).value());
static_assert(not proxyFlattened.nth(2).value());
static_assert(proxyFlattened.nth(3).value());

constexpr auto inspected = names.inspect([](std::optional<std::string_view>) consteval {});
static_assert(inspected.count() == names.count());

constexpr auto reverseNames = intValues.reverse();
static_assert(reverseNames.count() == intValues.count());

constexpr auto sortedNames = intValues.sort();
constexpr auto sortedByNames = intValues.sortBy(std::less<>{});
constexpr auto sortedByKey = intValues.sortByKey(std::identity{});
static_assert(sortedNames.count() == intValues.count());
static_assert(sortedByNames.count() == intValues.count());
static_assert(sortedByKey.count() == intValues.count());

constexpr auto minName = names.min();
constexpr auto maxName = names.max();
static_assert(minName.has_value());
static_assert(maxName.has_value());
static_assert(intValues.minBy(std::less<>{}).has_value());
static_assert(intValues.maxBy(std::less<>{}).has_value());

constexpr auto minByName = baseQuery.minByKey(Miracle::meta::name);
constexpr auto maxByName = baseQuery.maxByKey(Miracle::meta::name);
static_assert(minByName.has_value());
static_assert(maxByName.has_value());

constexpr auto uniqueNames = intValues.unique();
constexpr auto uniqueByNames = intValues.uniqueBy(std::identity{});
static_assert(uniqueNames.count() == 3);
static_assert(uniqueByNames.count() == 3);

constexpr auto partitioned = baseQuery.partition(Miracle::meta::isField);
static_assert(partitioned.first.count() == fields.size());
static_assert(partitioned.second.count() == 0);

consteval auto forEachCount() -> std::size_t {
  std::size_t count{};
  intValues.forEach([&count](int) consteval { ++count; });
  return count;
}
static_assert(forEachCount() == intValues.count());

constexpr auto materializableInts = intValues.map([](int value) consteval { return value; });
constexpr auto materialized = materializableInts.materialize();
static_assert(materialized.size() == intValues.count());
static_assert(materialized.front() == 3);

// Persistence barriers must restart from static backing without changing an owning logical T into const T&.
constexpr auto reversedOwnedInts = materializableInts.reverse();
static_assert(std::same_as<typename decltype(reversedOwnedInts)::Value, int>);
static_assert(reversedOwnedInts.nth(0).value() == 2);
static_assert(reversedOwnedInts.nth(3).value() == 3);
constexpr auto sortedOwnedInts = materializableInts.sort();
static_assert(sortedOwnedInts.nth(0).value() == 1);
static_assert(sortedOwnedInts.nth(1).value() == 2);
static_assert(sortedOwnedInts.nth(2).value() == 2);
static_assert(sortedOwnedInts.nth(3).value() == 3);

// Reference-valued reorder barriers persist addresses rather than copies of their referents.
constexpr auto sortedReferenceInts = Miracle::meta::query(std::span<const int>{intSource}).sort();
static_assert(std::same_as<typename decltype(sortedReferenceInts)::Value, const int &>);
static_assert(std::addressof(sortedReferenceInts.nth(0).value()) == std::addressof(intSource[1]));
static_assert(std::addressof(sortedReferenceInts.nth(3).value()) == std::addressof(intSource[0]));

constexpr auto reversedReferenceInts = Miracle::meta::query(std::span<const int>{intSource}).reverse();
static_assert(std::addressof(reversedReferenceInts.nth(0).value()) == std::addressof(intSource[3]));
static_assert(std::addressof(reversedReferenceInts.nth(3).value()) == std::addressof(intSource[0]));

constexpr auto uniqueReferenceInts = Miracle::meta::query(std::span<const int>{intSource}).unique();
static_assert(uniqueReferenceInts.count() == 3);
static_assert(std::addressof(uniqueReferenceInts.nth(0).value()) == std::addressof(intSource[0]));
static_assert(std::addressof(uniqueReferenceInts.nth(1).value()) == std::addressof(intSource[1]));
static_assert(std::addressof(uniqueReferenceInts.nth(2).value()) == std::addressof(intSource[2]));

constexpr auto partitionedReferenceInts =
    Miracle::meta::query(std::span<const int>{intSource}).partition([](const int &value) consteval {
      return value % 2 == 0;
    });
static_assert(std::addressof(partitionedReferenceInts.first.nth(0).value()) == std::addressof(intSource[2]));
static_assert(std::addressof(partitionedReferenceInts.first.nth(1).value()) == std::addressof(intSource[3]));
static_assert(std::addressof(partitionedReferenceInts.second.nth(0).value()) == std::addressof(intSource[0]));
static_assert(std::addressof(partitionedReferenceInts.second.nth(1).value()) == std::addressof(intSource[1]));

// A child query ending through its own Take/TakeWhile must not be confused with terminal cancellation in
// chain.
constexpr auto chainedAfterTake = materializableInts.take(2).chain(materializableInts.sort().take(1));
static_assert(chainedAfterTake.count() == 3);
static_assert(chainedAfterTake.nth(2).value() == 1);

constexpr auto referenceFirst = baseQuery.first();
static_assert(
    std::same_as<std::remove_cvref_t<decltype(referenceFirst)>, std::optional<const Miracle::meta::Info &>>);

} // namespace Tests::meta_query

namespace Tests::meta_query_nested {

constexpr auto sourceA = std::array{1, 2};
constexpr auto sourceB = std::array{3, 4};
constexpr auto nestedA = Miracle::meta::query(sourceA);
constexpr auto nestedB = Miracle::meta::query(sourceB);
constexpr std::array nestedQueries{nestedA, nestedB};
constexpr auto flattenedQueries = Miracle::meta::query(nestedQueries).flatten();

static_assert(flattenedQueries.count() == 4);
static_assert(flattenedQueries.nth(0).value() == 1);
static_assert(flattenedQueries.nth(3).value() == 4);

constexpr auto captured = [] consteval {
  const int threshold = 3;
  return Miracle::meta::query(sourceB).filter([threshold](int value) consteval { return value > threshold; });
}();
static_assert(captured.count() == 1);
static_assert(captured.first().value() == 4);

struct MoveOnlyThreshold final {
  int value{};

  consteval explicit MoveOnlyThreshold(int threshold)
      : value(threshold) {
  }

  MoveOnlyThreshold(const MoveOnlyThreshold &) = delete;

  consteval MoveOnlyThreshold(MoveOnlyThreshold &&other)
      : value(other.value) {
  }
};

// Rvalue Query pipelines move their flat capture environment forward. A genuinely move-only captured lambda
// therefore remains usable through later adaptors and terminals instead of forcing an accidental
// persistent-state copy.
consteval auto moveOnlyCapturedPipeline() -> bool {
  auto filtered = Miracle::meta::query(sourceB).filter(
      [threshold = MoveOnlyThreshold{3}](int value) consteval { return value > threshold.value; });
  return std::move(filtered).map([](int value) consteval { return value * 2; }).count() == 1;
}
static_assert(moveOnlyCapturedPipeline());

// A nested owning Query may itself carry a move-only capture environment. Flatten consumes that nested Query
// instead of accidentally copying its state at the source-composition boundary.
consteval auto nestedMoveOnlyCapturedPipeline() -> bool {
  constexpr std::array thresholds{1, 2};
  constexpr std::array values{1, 2, 3, 4};
  auto flattened = Miracle::meta::query(thresholds).flatMap([values](int threshold) consteval {
    return Miracle::meta::query(values).filter(
        [limit = MoveOnlyThreshold{threshold}](int value) consteval { return value > limit.value; });
  });
  return std::move(flattened).count() == 5;
}
static_assert(nestedMoveOnlyCapturedPipeline());

// Source composition must move consumed child states as well; neither chain nor zip may reintroduce a copy of
// a move-only capture environment.
consteval auto moveOnlySourceComposition() -> bool {
  auto chainLeft = Miracle::meta::query(sourceB).filter(
      [limit = MoveOnlyThreshold{2}](int value) consteval { return value > limit.value; });
  auto chainRight = Miracle::meta::query(sourceB).filter(
      [limit = MoveOnlyThreshold{3}](int value) consteval { return value > limit.value; });
  if (std::move(chainLeft).chain(std::move(chainRight)).count() != 3) {
    return false;
  }

  auto zipLeft = Miracle::meta::query(sourceB).filter(
      [limit = MoveOnlyThreshold{2}](int value) consteval { return value > limit.value; });
  auto zipRight = Miracle::meta::query(sourceB).filter(
      [limit = MoveOnlyThreshold{3}](int value) consteval { return value > limit.value; });
  return std::move(zipLeft).zip(std::move(zipRight)).count() == 1;
}
static_assert(moveOnlySourceComposition());

struct MoveOnlyValue final {
  int value{};

  consteval explicit MoveOnlyValue(int input)
      : value(input) {
  }

  MoveOnlyValue(const MoveOnlyValue &) = delete;

  consteval MoveOnlyValue(MoveOnlyValue &&other)
      : value(other.value) {
    other.value = -1;
  }

  consteval auto operator=(MoveOnlyValue &&other) -> MoveOnlyValue & {
    value = other.value;
    other.value = -1;
    return *this;
  }
};

// Lazy execution and evaluation-local source composition must not require an owning logical value to be
// copyable. Persistence barriers have a stricter reusable-static-backing contract and are tested separately.
consteval auto moveOnlyLogicalValuesStayLazy() -> bool {
  constexpr std::array source{1, 2, 3};

  auto filtered =
      Miracle::meta::query(source).map([](const int &value) consteval { return MoveOnlyValue{value}; });
  if (std::move(filtered).filter([](MoveOnlyValue &&value) consteval { return value.value > 1; }).count() !=
      2) {
    return false;
  }

  auto firstQuery =
      Miracle::meta::query(source).map([](const int &value) consteval { return MoveOnlyValue{value}; });
  auto first = std::move(firstQuery).first();
  if (not first.has_value() or first->value != 1) {
    return false;
  }

  auto zipLeft =
      Miracle::meta::query(source).map([](const int &value) consteval { return MoveOnlyValue{value}; });
  auto zipRight =
      Miracle::meta::query(source).map([](const int &value) consteval { return MoveOnlyValue{value * 10}; });
  if (std::move(zipLeft).zip(std::move(zipRight)).count() != 3) {
    return false;
  }

  auto flattened = Miracle::meta::query(source).flatMap(
      [](const int &value) consteval { return std::array{MoveOnlyValue{value}, MoveOnlyValue{value + 10}}; });
  return std::move(flattened).count() == 6;
}
static_assert(moveOnlyLogicalValuesStayLazy());

// Copyable lvalue Queries remain reusable: lvalue adaptor/terminal calls copy their state rather than
// consuming it.
consteval auto reusableCapturedLvalue() -> bool {
  const int threshold = 2;
  auto reusable =
      Miracle::meta::query(sourceB).filter([threshold](int value) consteval { return value > threshold; });
  const auto count = reusable.count();
  const auto first = reusable.first();
  const auto doubled = reusable.map([](int value) consteval { return value * 2; });
  return count == 2 and first.has_value() and first.value() == 3 and doubled.count() == 2;
}
static_assert(reusableCapturedLvalue());

} // namespace Tests::meta_query_nested
