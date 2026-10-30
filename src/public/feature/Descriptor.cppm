export module Miracle.Feature:Descriptor;

import std;

import Miracle.Numeric;
import Miracle.Capabilities;

import :Types;

// NOLINTBEGIN(readability-identifier-naming)
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

  if constexpr (Miracle::capabilities::importStd) {
    result = result.with(Capability::ImportStd);
  }

  if constexpr (Miracle::capabilities::reflectionCore) {
    result = result.with(Capability::ReflectionCore);
  }

  if constexpr (Miracle::capabilities::reflectionQueries) {
    result = result.with(Capability::ReflectionQueries);
  }

  if constexpr (Miracle::capabilities::reflectionAnnotations) {
    result = result.with(Capability::ReflectionAnnotations);
  }

  if constexpr (Miracle::capabilities::reflectionStaticStorage) {
    result = result.with(Capability::ReflectionStaticStorage);
  }

  if constexpr (Miracle::capabilities::expansionStatements) {
    result = result.with(Capability::ExpansionStatements);
  }

  if constexpr (Miracle::capabilities::stdVocabulary) {
    result = result.with(Capability::StdVocabulary);
  }

  if constexpr (Miracle::capabilities::stdHive) {
    result = result.with(Capability::StdHive);
  }

  return result;
}

/// Returns the stable human-facing name for a capability enumerator.
///
/// Capability diagnostics use names rather than implementation-dependent numberic ordinals so failures remain
/// actionable when the enum vocabulary grows.
[[nodiscard]] constexpr auto capabilityName(Capability capability) noexcept -> std::string_view {
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
    std::array<std::string_view, optionCount<OptionKind::Dependency, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Dependency, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached optional dependency names in declaration order.
  static constexpr auto optionalDependencyValues = [] -> auto {
    std::array<std::string_view, optionCount<OptionKind::OptionalDependency, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::OptionalDependency, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached conflict names in declaration order.
  static constexpr auto conflictValues = [] -> auto {
    std::array<std::string_view, optionCount<OptionKind::Conflict, Options...>> result{};
    usize index{};
    Miracle::feature::forEachOption<OptionKind::Conflict, Options...>(
        [&](auto option) -> void { result[index++] = option.target.view(); });
    return result;
  }();

  /// Cached implied feature names in declaration order.
  static constexpr auto implicationValues = [] -> auto {
    std::array<std::string_view, optionCount<OptionKind::Implication, Options...>> result{};
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
    std::array<std::string_view, optionCount<OptionKind::Group, Options...>> result{};
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
    auto &&callback = std::forward<Function>(function);
    (std::invoke(callback, Options{}), ...);
  }
};

/// Defines a structural feature descriptor with concise option spelling.
template <BasicStaticString Name, class... Options>
  requires(DescriptorOption<std::remove_cvref_t<Options>> and ...)
[[nodiscard]] consteval auto define([[maybe_unused]] Options... /*unused*/)
    -> Descriptor<Name, std::remove_cvref_t<Options>...> {
  return {};
}

/// Primitive bootstrap concept implemented without dependending on the later Structural Trait engine.
template <class T>
concept Feature = requires {
  { T::name.view() } -> std::same_as<std::string_view>;
  { T::description.view() } -> std::same_as<std::string_view>;
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
    return std::array<std::string_view, sizeof...(Members)>{Members.name.view()...};
  }
};

/// Concise structural feature-group value template.
template <BasicStaticString Name, auto... Members>
inline constexpr Group<Name, Members...> group{};

/// Primitive concept recognized by catalog resolution as a feature-group selector.
template <class T>
concept FeatureGroup = requires {
  { T::name.view() } -> std::same_as<std::string_view>;
  { T::members() };
} and not Feature<T>;

/// Returns whether the NTTP value is a structural feature-group selector.
template <auto Value>
concept FeatureGroupValue = FeatureGroup<std::remove_cvref_t<decltype(Value)>>;

} // namespace Miracle::feature
// NOLINTEND(readability-identifier-naming)
