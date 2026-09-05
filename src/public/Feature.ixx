export module Miracle:Feature;

import std;
import :Types;
import :StaticString;
import :Capabilities;

export namespace Miracle::feature {

/// Option categories used by the generic descriptor factory.
///
/// The category is public so third-party metadata options can participate in `Descriptor` without
/// specialization hooks. Ordinary callers normally use the semantic descriptor queries instead of inspecting
/// option categories.
enum class OptionKind : u8 {
  /// Human-facing descriptor description.
  Description,
  /// Semantic version metadata.
  Version,
  /// Build-selectable feature-kind marker.
  Build,
  /// Local semantic feature-kind marker.
  Semantic,
  /// Default-selection marker.
  DefaultEnabled,
  /// Required transitive dependency.
  Dependency,
  /// Non-enabling optional dependency metadata.
  OptionalDependency,
  /// Mutually incompatible feature relation.
  Conflict,
  /// Transitively implied feature relation.
  Implication,
  /// Toolchain/build capability requirement.
  Capability,
  /// Descriptive group membership.
  Group,
  /// Third-party metadata preserved but not interpreted by Miracle.
  Custom,
};

} // namespace Miracle::feature

// NOLINTBEGIN(readability-identifier-naming)
namespace Miracle::feature {

/// Appends text to a structural static string whose capacity is already known to be sufficient by the
/// caller's compile-time bound.
template <usize Capacity>
constexpr auto append(StaticString<Capacity> &target, StringView text) noexcept -> void {
  for (const char value : text) {
    target.storage[target.length++] = value;
  }
  target.storage[target.length] = '\0';
}

/// Appends one character to a structural static string.
template <usize Capacity>
constexpr auto append(StaticString<Capacity> &target, char value) noexcept -> void {
  target.storage[target.length++] = value;
  target.storage[target.length] = '\0';
}

/// Appends an unsigned integer without allocation.
template <usize Capacity>
constexpr auto append(StaticString<Capacity> &target, usize value) noexcept -> void {
  std::array<char, std::numeric_limits<usize>::digits10 + 2> digits{};
  usize count{};

  // NOLINTBEGIN
  do {
    digits[count++] = static_cast<char>('0' + value % 10);
    value /= 10;
  } while (value != 0);
  // NOLINTEND

  while (count != 0) {
    append(target, digits[--count]); // NOLINT
  }
}

/// Returns whether two compile-time names contain the same active code units.
template <class Left, class Right>
constexpr auto sameName(const Left &left, const Right &right) noexcept -> bool {
  return left.view() == right.view();
}

/// Returns the number of options in `Options...` whose category is `Kind`.
template <OptionKind Kind, class... Options>
inline constexpr usize optionCount = (usize{} + ... + (Options::kind == Kind ? usize{1} : usize{0}));

/// Selects the first description option without imposing a fixed capacity.
template <class... Options>
struct DescriptionSelector;

template <>
struct DescriptionSelector<> final {
  static constexpr BasicStaticString value{""};
};

template <class First, class... Rest>
struct DescriptionSelector<First, Rest...> final {
  static constexpr BasicStaticString value = [] -> auto {
    if constexpr (First::kind == OptionKind::Description) {
      return First::value;
    } else {
      return DescriptionSelector<Rest...>::value;
    }
  }();
};

/// Invokes `function` for every option in `Options...` matching `Kind`.
template <OptionKind Kind, class... Options, class Function>
constexpr auto forEachOption(Function &&function) -> void {
  (
      [&] -> void {
        if constexpr (Options::kind == Kind) {
          std::invoke(std::forward<Function>(function), Options{});
        }
      }(),
      ...);
}

/// Computes a deterministic FNV-1a hash for build/set identity diagnostics,
constexpr auto hashName(u64 seed, StringView name) noexcept -> u64 {
  u64 value = seed;
  for (const char codeUnit : name) {
    value ^= static_cast<u64>(static_cast<unsigned char>(codeUnit));
    value *= 1099511628211ULL; // NOLINT
  }
  return value;
}

} // namespace Miracle::feature

export namespace Miracle::feature {

/// Distinguishes compile/build selectable features from local semantic options.
enum class FeatureKind : u8 {
  /// The feature may only be selected when its implementation exists in the configured build universe.
  Build,

  /// The feature changes compile-time behavior without requiring additional compiled implementation to exist
  /// in the Miracle build.
  Semantic,
};

/// Semantic version attached to feature metadata.
///
/// A feature's `name` remains its identity. The version describes the feature contract and is intentionally
/// separate from versioned families such as a future `uuid.v7` feature name.
struct Version final {
  /// Major contract version.
  u16 major{};
  /// Minor contract version.
  u16 minor{};
  /// Patch contract version.
  u16 patch{};

  /// Lexicographically compares semantic versions by major/minor/patch.
  constexpr auto operator<=>(const Version &) const noexcept -> std::strong_ordering = default;
};

/// Toolchain/build capabilities that a feature may require.
enum class Capability : u8 {
  /// C++26 `import std;` module consumption.
  ImportStd,
  /// Core P2996 reflection/splicing support.
  ReflectionCore,
  /// Reflection member/function query support.
  ReflectionQueries,
  /// Reflection annotation support.
  ReflectionAnnotations,
  /// Reflection-backed static storage synthesis.
  ReflectionStaticStorage,
  /// C++26 expansion statements.
  ExpansionStatements,
  /// Standard vocabulary required by current Miracle modules.
  StdVocabulary,
  /// C++26 `std::hive`.
  StdHive,
  /// Sentinel equal to the number of real capability enumerators.
  Count,
};

/// Compact structural capability set used by feature resolution.
struct CapabilitySet final {
  /// One bit per `Capability` enumerator.
  u64 bits{};

  /// Returns whether `capability` belongs to this set.
  [[nodiscard]] constexpr auto contains(Capability capability) const noexcept -> bool {
    const auto index = static_cast<u8>(capability);
    return index < static_cast<u8>(Capability::Count) and (bits & (u64{1} << index)) != 0;
  }

  /// Returns a copy with `capability` enabled.
  [[nodiscard]] constexpr auto with(Capability capability) const noexcept -> CapabilitySet {
    CapabilitySet result = *this;
    const auto index = static_cast<u8>(capability);
    if (index < static_cast<u8>(Capability::Count)) {
      result.bits |= u64{1} << index;
    }
    return result;
  }

  /// Returns a copy with `capability` disabled.
  [[nodiscard]] constexpr auto without(Capability capability) const noexcept -> CapabilitySet {
    CapabilitySet result = *this;
    const auto index = static_cast<u8>(capability);
    if (index < static_cast<u8>(Capability::Count)) {
      result.bits &= ~(u64{1} << index);
    }
    return result;
  }

  /// Compares capability membership exactly.
  constexpr auto operator==(const CapabilitySet &) const noexcept -> bool = default;
};

/// Returns the capability set detected by Miracle's executable build probes.
[[nodiscard]] consteval auto detectedCapabilities() noexcept -> CapabilitySet {
  CapabilitySet result{};

  if constexpr (Miracle::capability::importStd) {
    result = result.with(Capability::ImportStd);
  }

  if constexpr (Miracle::capability::reflectionCore) {
    result = result.with(Capability::ReflectionCore);
  }

  if constexpr (Miracle::capability::reflectionQueries) {
    result = result.with(Capability::ReflectionQueries);
  }

  if constexpr (Miracle::capability::reflectionAnnotations) {
    result = result.with(Capability::ReflectionAnnotations);
  }

  if constexpr (Miracle::capability::reflectionStaticStorage) {
    result = result.with(Capability::ReflectionStaticStorage);
  }

  if constexpr (Miracle::capability::expansionStatements) {
    result = result.with(Capability::ExpansionStatements);
  }

  if constexpr (Miracle::capability::stdVocabulary) {
    result = result.with(Capability::StdVocabulary);
  }

  if constexpr (Miracle::capability::stdHive) {
    result = result.with(Capability::StdHive);
  }

  return result;
}

/// Returns the stable human-facing name for a capability enumerator.
///
/// Capability diagnostics use names rather than implementation-dependent numberic ordinals so failures remain
/// actionable when the enum vocabulary grows.
[[nodiscard]] constexpr auto capabilityName(Capability capability) noexcept -> StringView {
  switch (capability) {
    case Capability::ImportStd: return "import_std";
    case Capability::ReflectionCore: return "reflection_core";
    case Capability::ReflectionQueries: return "reflection_queries";
    case Capability::ReflectionAnnotations: return "reflection_annotations";
    case Capability::ReflectionStaticStorage: return "reflection_static_storage";
    case Capability::ExpansionStatements: return "expansion_statements";
    case Capability::StdVocabulary: return "std_vocabulary";
    case Capability::StdHive: return "std_hive";
    case Capability::Count: break;
  }
  return "unknown";
}

/// Describes why compile-time feature resolution failed.
enum class ResolutionError : u8 {
  /// Resolution completed successfully.
  None,
  /// Two catalog descriptors expose the same stable name.
  DuplicateFeature,
  /// A request/dependency/implication references a descriptor absent from the catalog.
  MissingFeature,
  /// A build feature is selected locally but absent from the compiled build universe.
  MissingBuildFeature,
  /// A selected feature requires a toolchain/build capability that is unavailable.
  MissingCapability,
  /// Required dependencies/implications form a cycle.
  DependencyCycle,
  /// Two enabled features declare an incompatibility.
  Conflict,
};

// ----- Descriptor option vocabulary -----

/// Declares human-facing feature documentation.
template <BasicStaticString Text>
struct Description final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Description;
  /// Human-facing description text.
  static constexpr auto value = Text;
};

/// Concise description option value.
template <BasicStaticString Text>
inline constexpr Description<Text> description{};

/// Declares feature semantic version metadata.
template <u16 Major, u16 Minor = 0, u16 Patch = 0>
struct VersionOption final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Version;
  /// Semantic version contributed by this option.
  static constexpr Version value{.major = Major, .minor = Minor, .patch = Patch};
};

/// Concise semantic-version option value.
template <u16 Major, u16 Minor = 0, u16 Patch = 0>
inline constexpr VersionOption<Major, Minor, Patch> version{};

/// Marks a descriptor as build-selectable.
struct BuildOption final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Build;
};
/// Concise build-feature kind marker.
inline constexpr BuildOption build{};

/// Marks a descriptor as semantic/local-only. This is the default kind.
struct SemanticOption final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Semantic;
};
/// Concise semantic-feature kind marker.
inline constexpr SemanticOption semantic{};

/// Marks a feature as enabled by catalog default resolution.
struct DefaultEnabledOption final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::DefaultEnabled;
};
/// Concise default-enabled marker.
inline constexpr DefaultEnabledOption defaultEnabled{};

/// Declares a required feature dependency by name.
template <BasicStaticString Name>
struct Dependency final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Dependency;
  /// Stable name of the required target feature.
  static constexpr auto target = Name;
};

/// Declares an optional feature relationship by name.
///
/// Optional dependencies are metadata and do not auto-enable their target.
template <BasicStaticString Name>
struct OptionalDependency final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::OptionalDependency;
  /// Stable name of the optional dependency.
  static constexpr auto target = Name;
};

/// Declares a feature conflict by name.
template <BasicStaticString Name>
struct Conflict final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Conflict;
  /// Stable name of the incompatible target feature.
  static constexpr auto target = Name;
};

/// Declares a feature implied by this feature.
template <BasicStaticString Name>
struct Implication final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Implication;
  /// Stable name of the feature enabled transitively with this descriptor.
  static constexpr auto target = Name;
};

/// Declares a build/toolchain capability required by this feature.
template <Capability Required>
struct CapabilityRequirement final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Capability;
  /// Required toolchain/build capability.
  static constexpr auto capability = Required;
};

/// Concise capability-requirement option value.
template <Capability Required>
inline constexpr CapabilityRequirement<Required> requiresCapability{};

/// Records descriptive membership in a named feature group.
template <BasicStaticString Name>
struct GroupMembership final {
  /// Descriptor-option category.
  static constexpr auto kind = OptionKind::Group;
  /// Stable group name used by ecosystem tooling.
  static constexpr auto target = Name;
};

/// Concise descriptive group-membership option value.
template <BasicStaticString Name>
inline constexpr GroupMembership<Name> inGroup{};

/// Marker available to third-party descriptor options that should be preserved by Miracle without gaining
/// built-in resolution semantics.
struct CustomOptionTag {
  /// Descriptor-option category identifying uninterpreted ecosystem metadata.
  static constexpr auto kind = OptionKind::Custom;
};

/// Describes a type-level option accepted by `Descriptor`.
///
/// Options deliberately encode their metadata in their type/static members.
/// `define(...)` is a function call, and C++ does not permit arbitrary function parameters to become NTTP
/// values in its return type. Requiring empty option objects prevents a stateful third-party option from
/// being silently discarded.
template <class T>
concept DescriptorOption = requires {
  { T::kind } -> std::convertible_to<OptionKind>;
} and std::is_empty_v<T>;

/// Feature descriptor supplied by Miracle's concise `define` factory.
///
/// Metadata lives in template arguments rather than dynamic storage. Descriptor objects are therefore empty
/// structural values suitable for NTTP use.
template <BasicStaticString Name, DescriptorOption... Options>
struct Descriptor final {
  static_assert(optionCount<OptionKind::Description, Options...> <= 1,
      "A Feature may contain at most one description option");
  static_assert(optionCount<OptionKind::Version, Options...> <= 1,
      "A Feature may contain at most one version option");
  static_assert(
      optionCount<OptionKind::Build, Options...> + optionCount<OptionKind::Semantic, Options...> <= 1,
      "A Feature cannot be both build and semantic");
  static_assert(optionCount<OptionKind::DefaultEnabled, Options...> <= 1,
      "A Feature may contain defaultEnabled at most once");

  /// Stable feature identity used by dependency/conflict resolution.
  static constexpr auto name = Name;

  /// Human-facing description. Empty when omitted.
  static constexpr auto description = DescriptionSelector<Options...>::value;

  /// Feature contract version.
  static constexpr Version version = [] -> Version {
    Version result{};
    (
        [&] -> void {
          if constexpr (Options::kind == OptionKind::Version) {
            result = Options::value;
          }
        }(),
        ...);
    return result;
  }();

  /// Build features require presence in a `BuildFeatureSet`; semantic features can be selected locally
  /// without adding compiled implementation.
  static constexpr FeatureKind featureKind = [] -> FeatureKind {
    if constexpr (optionCount<OptionKind::Build, Options...> != 0) {
      return FeatureKind::Build;
    }
    return FeatureKind::Semantic;
  }();

  /// Whether default catalog resolution selects this feature initially.
  static constexpr bool enabledByDefault = optionCount<OptionKind::DefaultEnabled, Options...> != 0;

private:
  /// Cached required dependency names in declaration order.
  static constexpr auto dependencyValues = [] -> auto {
    std::array<StringView, optionCount<OptionKind::Dependency, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Dependency, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached optional dependency names in declaration order.
  static constexpr auto optionalDependencyValues = [] -> auto {
    std::array<StringView, optionCount<OptionKind::OptionalDependency, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::OptionalDependency, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached conflict names in declaration order.
  static constexpr auto conflictValues = [] -> auto {
    std::array<StringView, optionCount<OptionKind::Conflict, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Conflict, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached implied feature names in declaration order.
  static constexpr auto implicationValues = [] -> auto {
    std::array<StringView, optionCount<OptionKind::Implication, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Implication, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached build/toolchain capability requirements in declaration order.
  static constexpr auto capabilityValues = [] -> auto {
    std::array<Capability, optionCount<OptionKind::Capability, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Capability, Options...>(
        [&](auto option) -> void { result[index++] = option.capability; });
    return result;
  }();

  /// Cached descriptive group memberships in declaration order.
  static constexpr auto groupValues = [] -> auto {
    std::array<StringView, optionCount<OptionKind::Group, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Group, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

public:
  /// Returns the cached required dependency names.
  [[nodiscard]] static constexpr auto dependencies() noexcept -> const auto & {
    return dependencyValues;
  }

  /// Returns the cached optional dependency names.
  [[nodiscard]] static constexpr auto optionalDependencies() noexcept -> const auto & {
    return optionalDependencyValues;
  }

  /// Returns the cached conflict names.
  [[nodiscard]] static constexpr auto conflicts() noexcept -> const auto & {
    return conflictValues;
  }

  /// Returns the cached implied feature names.
  [[nodiscard]] static constexpr auto implications() noexcept -> const auto & {
    return implicationValues;
  }

  /// Returns the cached build/toolchain capability requirements.
  [[nodiscard]] static constexpr auto capabilities() noexcept -> const auto & {
    return capabilityValues;
  }

  /// Returns the cached group memberships.
  [[nodiscard]] static constexpr auto groups() noexcept -> const auto & {
    return groupValues;
  }

  /// Exposes every descriptor option to generic ecosystem tooling, including third-party custom metadata that
  /// Miracle itself does not interpret.
  template <class Function>
  static constexpr auto forEachOption(Function &&function) -> void {
    (std::invoke(std::forward<Function>(function), Options{}), ...);
  }
};

/// Defines a structural feature descriptor with concise option spelling.
template <BasicStaticString Name, class... Options>
  requires(DescriptorOption<std::remove_cvref_t<Options>> and ...)
[[nodiscard]] consteval auto define(Options... /*unused*/)
    -> Descriptor<Name, std::remove_cvref_t<Options>...> {
  return {};
}

/// Primitive bootstrap concept implemented without dependending on the later Structural Trait engine.
template <class T>
concept Feature = requires {
  { T::name.view() } -> std::same_as<StringView>;
  { T::description.view() } -> std::same_as<StringView>;
  { T::version } -> std::convertible_to<Version>;
  { T::featureKind } -> std::convertible_to<FeatureKind>;
  { T::enabledByDefault } -> std::convertible_to<bool>;
  { T::dependencies() };
  { T::optionalDependencies() };
  { T::conflicts() };
  { T::implications() };
  { T::capabilities() };
  { T::groups() };
};

/// Returns whether an NTTP value satisfies the bootstrap Feature protocol.
template <auto Value>
concept FeatureValue = Feature<std::remove_cvref_t<decltype(Value)>>;

/// Required dependency referring to an already-declared descriptor value.
template <auto Target>
  requires FeatureValue<Target>
inline constexpr Dependency<Target.name> dependsOn{};

/// Required dependency referring to a feature declared later or by another catalog participant. Named
/// references make cycle diagnostics possible even when descriptors cannot recursively contain each other's
/// C++ values.
template <BasicStaticString Name>
inline constexpr Dependency<Name> dependsOnNamed{};

/// Optional dependency referring to an already-declared feature value.
template <auto Target>
  requires FeatureValue<Target>
inline constexpr OptionalDependency<Target.name> optionallyDependsOn{};

/// Optional dependency referring to a feature declared later or externally.
template <BasicStaticString Name>
inline constexpr OptionalDependency<Name> optionallyDependsOnNamed{};

/// Conflict referring to an already-declared feature value.
template <auto Target>
  requires FeatureValue<Target>
inline constexpr Conflict<Target.name> conflictsWith{};

/// Conflict referring to a feature declared later or externally.
template <BasicStaticString Name>
inline constexpr Conflict<Name> conflictsWithNamed{};

/// Implication referring to an already-declared feature value.
template <auto Target>
  requires FeatureValue<Target>
inline constexpr Implication<Target.name> implies{};

/// Implication referring to a feature declared later or externally.
template <BasicStaticString Name>
inline constexpr Implication<Name> impliesNamed{};

/// Structural feature group. Groups are selectors, not features themselves.
template <BasicStaticString Name, auto... Members>
  requires(FeatureValue<Members> and ...)
struct Group final {
  /// Stable group identity.
  static constexpr auto name = Name;

  /// Returns member feature names in declaration order.
  [[nodiscard]] static consteval auto members() {
    return std::array<StringView, sizeof...(Members)>{Members.name.view()...};
  }
};

/// Concise structural feature-group value template.
template <BasicStaticString Name, auto... Members>
inline constexpr Group<Name, Members...> group{};

/// Primitive concept recognized by catalog resolution as a feature-group selector.
template <class T>
concept FeatureGroup = requires {
  { T::name.view() } -> std::same_as<StringView>;
  { T::members() };
} and not Feature<T>;

/// Returns whether the NTTP value is a structural feature-group selector.
template <auto Value>
concept FeatureGroupValue = FeatureGroup<std::remove_cvref_t<decltype(Value)>>;

/// Structural set describing which build-feature implementation are compiled into a particular binary/module
/// configuration.
template <BasicStaticString... Names>
struct BuildFeatureSet final {
private:
  /// Canonically sorted stable names, materialized once per build-universe type.
  static constexpr auto sortedNames = [] -> auto {
    std::array<StringView, sizeof...(Names)> result{Names.view()...};
    std::ranges::sort(result);
    return result;
  }();

  /// Validates that the build universe is a set rather than a multiset.
  [[nodiscard]] static consteval auto uniqueNames() noexcept -> bool {
    return std::ranges::adjacent_find(sortedNames) == sortedNames.end();
  }

  /// Cached deterministic identity for this immutable build universe.
  static constexpr u64 fingerprintValue = [] -> u64 {
    u64 value = 14695981039346656037ULL; // NOLINT
    for (const StringView name : sortedNames) {
      value = hashName(value, name);
      value = hashName(value, "\n");
    }
    return value;
  }();

public:
  static_assert(uniqueNames(), "BuildFeatureSet cannot contain duplicate feature names");

  /// Returns the number of configured build features.
  [[nodiscard]] static consteval auto size() noexcept -> usize {
    return sizeof...(Names);
  }

  /// Returns whether `name` is present in this build universe.
  [[nodiscard]] static constexpr auto contains(StringView name) noexcept -> bool {
    if constexpr (sizeof...(Names) == 0) {
      static_cast<void>(name);
      return false;
    } else {
      return (false or ... or (Names.view() == name));
    }
  }

  /// Returns whether `feature` is present in this build universe.
  template <class DescriptorType>
    requires Feature<std::remove_cvref_t<DescriptorType>>
  [[nodiscard]] static constexpr auto contains(const DescriptorType & /*unused*/) noexcept -> bool {
    return contains(std::remove_cvref_t<DescriptorType>::name.view());
  }

  /// deterministic set fingerprint independent of compiler/vendor identities.
  [[nodiscard]] static consteval auto fingerprint() noexcept -> u64 {
    return fingerprintValue;
  }
};

/// Concise build-universe value template.
template <BasicStaticString... Names>
inline constexpr BuildFeatureSet<Names...> buildSet{};

/// Failure/success report returned by non-terminating compile-time analysis.
///
/// `MessageCapacity` is derived from the complete catalog names so dependency cycle diagnostics can include
/// the entire cycle without arbitrary truncation.
template <class Set, usize MessageCapacity>
struct ResolutionReport final {
  /// Canonical resolved set produced so far.
  Set features{};

  /// Failure category, or `None` on success.
  ResolutionError error{ResolutionError::None};

  /// Human-facing compile-time diagnostic.
  StaticString<MessageCapacity> message{};

  /// Returns whether resolution succeeded.
  [[nodiscard]] constexpr auto succeeded() const noexcept -> bool {
    return error == ResolutionError::None;
  }

  /// Allows direct boolean testing in constexpr code.
  [[nodiscard]] constexpr explicit operator bool() const noexcept {
    return succeeded();
  }
};

template <auto... Features>
  requires(FeatureValue<Features> and ...)
struct Catalog;

/// Structural resolved local set. The bit layout always follows catalog order, which makes resolution
/// deterministic and canonical regardless of request order.
template <class CatalogType>
struct FeatureSet final {
  /// One canonical enable bit per catalog descriptor.
  std::array<bool, CatalogType::featureCount> enabled{};

  /// Returns the number of enabled features.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return static_cast<usize>(std::ranges::count(enabled, true));
  }

  /// Returns whether no feature is enabled.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return size() == 0;
  }

  /// Returns whether a catalog feature name is enabled.
  [[nodiscard]] constexpr auto contains(StringView name) const noexcept -> bool {
    const usize index = CatalogType::findIndex(name);
    return index != CatalogType::npos and enabled[index];
  }

  /// Returns whether the given descriptor is enabled.
  template <class DescriptorType>
    requires Feature<std::remove_cvref_t<DescriptorType>>
  [[nodiscard]] constexpr auto contains(const DescriptorType & /*unused*/) const noexcept -> bool {
    return contains(std::remove_cvref_t<DescriptorType>::name.view());
  }

  /// Compile-time convenience form for NTTP feature values.
  template <auto Value>
    requires FeatureValue<Value>
  [[nodiscard]] constexpr auto contains() const noexcept -> bool {
    return contains(Value.name.view());
  }

  /// Returns whether every enabled feature also appears in `other`.
  [[nodiscard]] constexpr auto subsetOf(const FeatureSet &other) const noexcept -> bool {
    for (usize index{}; index < enabled.size(); ++index) {
      if (enabled[index] and not other.enabled[index]) {
        return false;
      }
    }
    return true;
  }

  /// Returns a deterministic fingerprint over enabled feature names.
  [[nodiscard]] constexpr auto fingerprint() const noexcept -> u64 {
    u64 value = 14695981039346656037ULL; // NOLINT
    for (usize index{}; index < enabled.size(); ++index) {
      if (enabled[index]) {
        value = hashName(value, CatalogType::nameAt(index));
        value = hashName(value, "\n");
      }
    }
    return value;
  }

  /// Compares canonical enable bits exactly.
  constexpr auto operator==(const FeatureSet &) const noexcept -> bool = default;
};

/// Catalog containing the complete descriptor universe used for one resolution.
///
/// Catalog order defines canonical resolved-set order; no sort or type recreation occurs during normal
/// feature selection. Required dependency closure, default resolution, groups, capabilities and conflicts all
/// share one seed-driven resolver so the compiler never instantiates parallel resolution engines.
template <auto... Features>
  requires(FeatureValue<Features> and ...)
struct Catalog final {
  /// Sentinel returned by failed catalog name lookup.
  static constexpr usize npos = std::numeric_limits<usize>::max();
  /// Number of feature descriptors in canonical catalog order.
  static constexpr usize featureCount = sizeof...(Features);
  /// Diagnostic capacity sufficient for every catalog name in a full cycle.
  static constexpr usize messageCapacity =
      usize{384} + (usize{} + ... + (decltype(Features.name)::capacityValue + usize{8}));

  /// Canonical resolved-set type associated with this catalog.
  using Set = FeatureSet<Catalog>;
  /// non-terminating analysis report type associated with this catalog.
  using Report = ResolutionReport<Set, messageCapacity>;

  /// Returns descriptor count.
  [[nodiscard]] static consteval auto size() noexcept -> usize {
    return featureCount;
  }

  /// Invokes `function` for every descriptor in canonical catalog order.
  template <class Function>
  static constexpr auto forEach(Function &&function) -> void {
    (std::invoke(std::forward<Function>(function), Features), ...);
  }

  /// Returns the catalog index for `name`, or `npos` when absent.
  [[nodiscard]] static constexpr auto findIndex(StringView name) noexcept -> usize {
    for (usize index{}; index < featureCount; ++index) {
      if (catalogNames[index] == name)
        return index;
    }
    return npos;
  }

  /// Returns the descriptor name at `index`; invalid indices yield an empty view.
  [[nodiscard]] static constexpr auto nameAt(usize target) noexcept -> StringView {
    return target < featureCount ? catalogNames[target] : StringView{};
  }

private:
  /// Stable catalog names cached once for lookup, diagnostics and fingerprints.
  static constexpr std::array<StringView, featureCount> catalogNames{Features.name.view()...};

  /// Build/semantic kind cached once for each catalog slot.
  static constexpr std::array<FeatureKind, featureCount> featureKinds{
      std::remove_cvref_t<decltype(Features)>::featureKind...};

  /// Default seed state cached once for each catalog slot.
  static constexpr std::array<bool, featureCount> defaultStates{
      std::remove_cvref_t<decltype(Features)>::enabledByDefault...};

  /// Converts one descriptor's capability list into the compact set used by resolution. Third-party Feature
  /// descriptors benefit from the same cache.
  template <class DescriptorType>
  [[nodiscard]] static consteval auto capabilitySetFor() noexcept -> CapabilitySet {
    CapabilitySet result{};
    for (const Capability capability : DescriptorType::capabilities()) {
      result = result.with(capability);
    }
    return result;
  }

  /// Required capability masks cached once for the complete catalog.
  static constexpr std::array<CapabilitySet, featureCount> capabilityRequirements{
      capabilitySetFor<std::remove_cvref_t<decltype(Features)>>()...};

  /// Number of required dependency/implication edges contributed by one feature.
  template <class DescriptorType>
  [[nodiscard]] static consteval auto requiredEdgeCountFor() noexcept -> usize {
    return DescriptorType::dependencies().size() + DescriptorType::implications().size();
  }

  /// Number of conflict edges contributed by one feature.
  template <class DescriptorType>
  [[nodiscard]] static consteval auto conflictEdgeCountFor() noexcept -> usize {
    return DescriptorType::conflicts().size();
  }

  /// Total required dependency/implication edges in the catalog graph.
  static constexpr usize requiredEdgeCount =
      (usize{} + ... + requiredEdgeCountFor<std::remove_cvref_t<decltype(Features)>>());

  /// Total declared conflict edges in the catalog graph.
  static constexpr usize conflictEdgeCount =
      (usize{} + ... + conflictEdgeCountFor<std::remove_cvref_t<decltype(Features)>>());

  /// Compact canonical adjacency storage. `offsets[i]..offsets[i + 1]` contains every relation declared by
  /// feature `i`; unresolved names retain `npos` so analysis can produce a source-aware diagnostic without
  /// repeating lookup.
  template <usize EdgeCount>
  struct RelationGraph final {
    std::array<usize, featureCount + 1> offsets{};
    std::array<usize, EdgeCount> targets{};
    std::array<StringView, EdgeCount> names{};
  };

  /// Builds the required dependency/implication graph exactly once per catalog.
  [[nodiscard]] static consteval auto makeRequiredGraph() {
    RelationGraph<requiredEdgeCount> graph{};
    usize source{};
    usize edge{};

    (
        [&] -> void {
          using DescriptorType = std::remove_cvref_t<decltype(Features)>;
          graph.offsets[source] = edge;
          for (const StringView dependency : DescriptorType::dependencies()) {
            graph.names[edge] = dependency;
            graph.targets[edge] = findIndex(dependency);
            ++edge;
          }
          for (const StringView implication : DescriptorType::implications()) {
            graph.names[edge] = implication;
            graph.targets[edge] = findIndex(implication);
            ++edge;
          }
          ++source;
        }(),
        ...);

    graph.offsets[featureCount] = edge;
    return graph;
  }

  /// Builds the conflict graph exactly once per catalog.
  [[nodiscard]] static consteval auto makeConflictGraph() {
    RelationGraph<conflictEdgeCount> graph{};
    usize source{};
    usize edge{};

    (
        [&] -> void {
          using DescriptorType = std::remove_cvref_t<decltype(Features)>;
          graph.offsets[source] = edge;
          for (const StringView conflict : DescriptorType::conflicts()) {
            graph.names[edge] = conflict;
            graph.targets[edge] = findIndex(conflict);
            ++edge;
          }
          ++source;
        }(),
        ...);

    graph.offsets[featureCount] = edge;
    return graph;
  }

  /// Canonical cached relation graphs. These turn repeated string relationships into integer edge traversal
  /// before any individual resolution request runs.
  static constexpr RelationGraph requiredGraph = makeRequiredGraph();
  static constexpr RelationGraph conflictGraph = makeConflictGraph();

  /// Finds the first duplicate stable feature name during catalog materialization.
  [[nodiscard]] static consteval auto findDuplicateName() noexcept -> StringView {
    for (usize left{}; left < catalogNames.size(); ++left) {
      for (usize right = left + 1; right < catalogNames.size(); ++right) {
        if (catalogNames[left] == catalogNames[right]) {
          return catalogNames[left];
        }
      }
    }
    return {};
  }

  /// Cached duplicate identity; empty when the catalog names are unique.
  static constexpr StringView duplicateFeatureName = findDuplicateName();

  /// Returns whether a descriptor declares membership in `groupName`.
  template <class DescriptorType>
  [[nodiscard]] static consteval auto belongsToGroup(StringView groupName) noexcept -> bool {
    return std::ranges::any_of(DescriptorType::groups(),
        [&](const StringView groupNameCandidate) -> bool { return groupNameCandidate == groupName; });
  }

  /// Resolves a canonical boolean seed set through the complete feature graph.
  ///
  /// Traversal is intentionally iterative. Dependency depth therefore consumes explicit structural storage
  /// rather than compiler constexpr call depth, and every relationship target has already been canonicalized
  /// to a catalog index.
  template <class BuildSet>
  // NOLINTNEXTLINE(readability-function-size, readability-function-cognitive-complexity)
  [[nodiscard]] static consteval auto analyzeSeeds(std::array<bool, featureCount> seeds,
      BuildSet buildFeatures,
      CapabilitySet capabilities) -> Report {
    Report report{};

    if constexpr (featureCount > 1) {
      if constexpr (!duplicateFeatureName.empty()) {
        report.error = ResolutionError::DuplicateFeature;
        append(report.message, "feature catalog contains duplicate feature name `");
        append(report.message, duplicateFeatureName);
        append(report.message, "`");
        return report;
      }
    }

    std::array<u8, featureCount> state{}; // 0 = unvisited, 1 = visiting, 2 = complete
    std::array<usize, featureCount> stack{};
    std::array<usize, featureCount> nextEdge{};
    usize depth{};

    auto failCapability = [&](usize index) -> void {
      const u64 missing = capabilityRequirements[index].bits & ~capabilities.bits;
      if (missing == 0) {
        return;
      }

      report.error = ResolutionError::MissingCapability;
      append(report.message, "feature `");
      append(report.message, nameAt(index));
      append(report.message, "` requires unavailable capability `");
      for (u8 raw{}; raw < static_cast<u8>(Capability::Count); ++raw) {
        if ((missing & (u64{1} << raw)) != 0) {
          append(report.message, capabilityName(static_cast<Capability>(raw)));
          break;
        }
      }
      append(report.message, "`");
    };

    auto push = [&](usize index) -> void {
      state[index] = 1;
      report.features.enabled[index] = true;

      if (featureKinds[index] == FeatureKind::Build and not buildFeatures.contains(nameAt(index))) {
        report.error = ResolutionError::MissingBuildFeature;
        append(report.message, "build feature `");
        append(report.message, nameAt(index));
        append(report.message, "` is not available in this Miracle configuration");
        return;
      }

      failCapability(index);
      if (not report.succeeded()) {
        return;
      }

      stack[depth] = index;
      nextEdge[depth] = requiredGraph.offsets[index];
      ++depth;
    };

    for (usize seed{}; seed < featureCount and report.succeeded(); ++seed) {
      if (not seeds[seed] or state[seed] != 0) {
        continue;
      }

      push(seed);
      while (depth != 0 and report.succeeded()) {
        const usize frame = depth - 1;
        const usize source = stack[frame];
        const usize edgeEnd = requiredGraph.offsets[source + 1];

        if (nextEdge[frame] == edgeEnd) {
          state[source] = 2;
          --depth;
          continue;
        }

        const usize edge = nextEdge[frame]++;
        const usize target = requiredGraph.targets[edge];
        if (target == npos) {
          report.error = ResolutionError::MissingFeature;
          append(report.message, "feature `");
          append(report.message, nameAt(source));
          append(report.message, "` references missing required feature `");
          append(report.message, requiredGraph.names[edge]);
          append(report.message, "`");
          break;
        }

        if (state[target] == 0) {
          push(target);
          continue;
        }
        if (state[target] == 2) {
          continue;
        }

        report.error = ResolutionError::DependencyCycle;
        append(report.message, "feature dependency cycle: ");
        usize cycleStart{};
        while (cycleStart < depth and stack[cycleStart] != target) {
          ++cycleStart;
        }
        for (usize position = cycleStart; position < depth; ++position) {
          if (position != cycleStart) {
            append(report.message, " --> ");
          }
          append(report.message, nameAt(stack[position]));
        }
        append(report.message, " --> ");
        append(report.message, nameAt(target));
      }
    }

    if (not report.succeeded()) {
      return report;
    }

    // Conflicts are checked over cached index edges after complete closure, so request order cannot alter the
    // result and names are never re-resolved.
    for (usize source{}; source < featureCount; ++source) {
      if (not report.features.enabled[source]) {
        continue;
      }
      for (usize edge = conflictGraph.offsets[source]; edge < conflictGraph.offsets[source + 1]; ++edge) {
        const usize target = conflictGraph.targets[edge];
        if (target != npos and report.features.enabled[target]) {
          report.error = ResolutionError::Conflict;
          append(report.message, "feature conflict: `");
          append(report.message, nameAt(source));
          append(report.message, "` conflicts with `");
          append(report.message, conflictGraph.names[edge]);
          append(report.message, "`");
          return report;
        }
      }
    }

    return report;
  }

public:
  /// Returns whether descriptor names in this catalog are unique.
  [[nodiscard]] static consteval auto uniqueNames() noexcept -> bool {
    return duplicateFeatureName.empty();
  }

  /// Performs non-terminating resolution for explicitly selected features/groups.
  ///
  /// A group expands both its explicitly declared members and every catalog descriptor carrying matching
  /// `inGroup<...>` metadata. This mirrors the build-system resolver and allows group/feature declarations in
  /// either order.
  template <auto... Requested, class BuildSet>
    requires((FeatureValue<Requested> or FeatureGroupValue<Requested>) and ...)
  // NOLINTNEXTLINE(readability-function-cognitive-complexity)
  [[nodiscard]] static consteval auto analyze(BuildSet buildFeatures,
      CapabilitySet capabilities = detectedCapabilities()) -> Report {
    std::array<bool, featureCount> seeds{};

    if constexpr (sizeof...(Requested) != 0) {
      Report selectionFailure{};

      auto selectName = [&](StringView name, StringView owner) -> void {
        if (not selectionFailure.succeeded()) {
          return;
        }
        const usize index = findIndex(name);
        if (index == npos) {
          selectionFailure.error = ResolutionError::MissingFeature;
          if (not owner.empty()) {
            append(selectionFailure.message, "feature group `");
            append(selectionFailure.message, owner);
            append(selectionFailure.message, "` references missing feautre `");
          } else {
            append(selectionFailure.message, "requested feature is not in catalog: `");
          }
          append(selectionFailure.message, name);
          append(selectionFailure.message, "`");
          return;
        }
        seeds[index] = true;
      };

      auto select = [&](auto selector) -> void {
        using Selector = std::remove_cvref_t<decltype(selector)>;
        if constexpr (Feature<Selector>) {
          selectName(Selector::name.view(), {});
        } else {
          for (const StringView member : Selector::members()) {
            selectName(member, Selector::name.view());
          }

          usize index{};
          (
              [&] -> void {
                using DescriptorType = std::remove_cvref_t<decltype(Features)>;
                if constexpr (belongsToGroup<DescriptorType>(Selector::name.view())) {
                  seeds[index] = true;
                }
                ++index;
              }(),
              ...);
        }
      };

      (select(Requested), ...);
      if (not selectionFailure.succeeded()) {
        return selectionFailure;
      }
    }

    return analyzeSeeds(seeds, buildFeatures, capabilities);
  }

  /// Performs non-terminating resolution of every default-enabled descriptor.
  template <class BuildSet>
  [[nodiscard]] static consteval auto analyzeDefaults(BuildSet buildFeatures,
      CapabilitySet capabilities = detectedCapabilities()) -> Report {
    return analyzeSeeds(defaultStates, buildFeatures, capabilities);
  }

  /// Resolves explicit selectors and emits a C++26 compile-time diagnostic on invalid configuration. The
  /// build universe is an NTTP so the complete report can itself participate in a user-generated
  /// `static_assert` message.
  template <auto BuildFeatures, auto... Requested>
    requires((FeatureValue<Requested> or FeatureGroupValue<Requested>) and ...)
  [[nodiscard]] static consteval auto resolve() -> Set {
    constexpr auto report = analyze<Requested...>(BuildFeatures, detectedCapabilities());
    static_assert(report.succeeded(), report.message);
    return report.features;
  }

  /// Hard-resolution variant accepting an explicit capability universe.
  template <auto BuildFeatures, auto Capabilities, auto... Requested>
    requires((FeatureValue<Requested> or FeatureGroupValue<Requested>) and ...)
  [[nodiscard]] static consteval auto resolveWithCapabilities() -> Set {
    constexpr auto report = analyze<Requested...>(BuildFeatures, Capabilities);
    static_assert(report.succeeded(), report.message);
    return report.features;
  }

  /// Resolves default-enabled descriptors with the same hard-diagnostic policy as `resolve`.
  template <auto BuildFeatures>
  [[nodiscard]] static consteval auto resolveDefaults() -> Set {
    constexpr auto report = analyzeDefaults(BuildFeatures, detectedCapabilities());
    static_assert(report.succeeded(), report.message);
    return report.features;
  }

  /// Default-resolution variant accepting an explicit capability universe.
  template <auto BuildFeatures, auto Capabilities>
  [[nodiscard]] static consteval auto resolveDefaultsWithCapabilities() -> Set {
    constexpr auto report = analyzeDefaults(BuildFeatures, Capabilities);
    static_assert(report.succeeded(), report.message);
    return report.features;
  }
};

/// Concise catalog value template.
template <auto... Features>
  requires(FeatureValue<Features> and ...)
inline constexpr Catalog<Features...> catalog{};

} // namespace Miracle::feature

namespace Miracle::feature {

/// Module-local structural heterogeneous storage used by exported `Requirement`. The backend type is
/// reachable through the public object representation but is intentionally not a named user-facing
/// customization surface.
template <class... Values>
struct RequirementValues;

/// Empty requirement-value terminator.
template <>
struct RequirementValues<> final {
  /// Constructs the empty requirement-value terminator.
  constexpr RequirementValues() noexcept = default;

  /// Empty storage never contains a requested feature name.
  [[nodiscard]] constexpr auto contains(StringView /*unused*/) const noexcept -> bool { // NOLINT
    return false;
  }
};

/// One structural requirement value followed by the remaining values.
template <class Head, class... Tail>
struct RequirementValues<Head, Tail...> final {
  /// First structural descriptor value.
  Head head{};
  /// Remaining structural descriptor values.
  RequirementValues<Tail...> tail{};

  /// Constructs default-initialized structural requirement values.
  constexpr RequirementValues() noexcept = default;

  /// Stores one descriptor followed by the remaining structural values.
  constexpr explicit RequirementValues(Head value, Tail... remaining) noexcept
      : head(value)
      , tail(remaining...) {
  }

  /// Returns whether this recursive structural storage contains `name`.
  [[nodiscard]] constexpr auto contains(StringView name) const noexcept -> bool {
    return Head::name.view() == name or tail.contains(name);
  }
};

} // namespace Miracle::feature

export namespace Miracle::feature {

/// Annotation payload used to associate feature requirements with reflected declarations. `require(...)` is
/// the standard-accurate spelling: `requires` is a C++ keyword and therefore cannot name a function/object.
template <class... Required>
struct Requirement final {
  /// Number of feature descriptors stored by this requirement.
  static constexpr usize featureCount = sizeof...(Required);

  /// Structural descriptor values retained for reflection extraction.
  RequirementValues<Required...> features{};

  /// Returns whether a feature name belongs to this requirement.
  [[nodiscard]] constexpr auto contains(StringView name) const noexcept -> bool {
    return features.contains(name);
  }
};

/// Creates an annotation-friendly structural requirement payload.
template <class... Required>
  requires(Feature<std::remove_cvref_t<Required>> and ...)
[[nodiscard]] consteval auto require(Required... required) -> Requirement<std::remove_cvref_t<Required>...> {
  using Values = RequirementValues<std::remove_cvref_t<Required>...>;
  using Result = Requirement<std::remove_cvref_t<Required>...>;
  return Result{Values{required...}};
}

/// Emits a feature-specific compile-time diagnostic for lightweight façade APIs whose implementation is
/// absent from the current set.
template <auto Required, auto Set>
  requires FeatureValue<Required>
consteval auto requireEnabled() -> void {
  constexpr bool enabled = Set.template contains<Required>();
  constexpr auto message =
      staticFormat<"feature `{}` is not enabled\n\nhelp:\n  enable `{}` in the active Miracle FeatureSet">(
          Required.name, Required.name);
  static_assert(enabled, message);
}

} // namespace Miracle::feature
// NOLINTEND(readability-identifier-naming)
