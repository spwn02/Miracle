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
static_assert(cachedInvalidSubjectStillThrows());
static_assert(typedInvalidSubjectStillThrows());

// Phase 5.2 cache invariants: unchecked type queries expose the canonical backing span directly, and the
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
