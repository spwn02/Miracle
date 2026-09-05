import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Switch;

// NOLINTBEGIN(readability-identifier-naming)
namespace {

namespace featureTest {

inline constexpr auto base = feature::define<"base">(feature::semantic,
    feature::description<"Shared semantic base">,
    feature::version<1, 2, 3>);
inline constexpr auto logging = feature::define<"logging">(feature::semantic,
    feature::dependsOn<base>,
    feature::defaultEnabled,
    feature::inGroup<"defaults">);
inline constexpr auto tracing = feature::define<"tracing">(feature::semantic,
    feature::implies<logging>,
    feature::optionallyDependsOnNamed<"metrics">);
inline constexpr auto gpu = feature::define<"gpu">(feature::build, feature::dependsOn<base>);
inline constexpr auto metrics = feature::define<"metrics">(feature::semantic);
inline constexpr auto incompatible =
    feature::define<"incompatible">(feature::semantic, feature::conflictsWith<tracing>);
inline constexpr auto reflection = feature::define<"reflection">(feature::semantic,
    feature::requiresCapability<feature::Capability::ReflectionCore>);

inline constexpr auto defaults = feature::group<"defaults">;
inline constexpr auto complete = feature::group<"complete", tracing, gpu, metrics>;
inline constexpr auto features =
    feature::catalog<base, logging, tracing, gpu, metrics, incompatible, reflection>;

inline constexpr auto cycleA =
    feature::define<"cycle-a">(feature::semantic, feature::dependsOnNamed<"cycle-b">);
inline constexpr auto cycleB =
    feature::define<"cycle-b">(feature::semantic, feature::dependsOnNamed<"cycle-c">);
inline constexpr auto cycleC =
    feature::define<"cycle-c">(feature::semantic, feature::dependsOnNamed<"cycle-a">);
inline constexpr auto cycle = feature::catalog<cycleA, cycleB, cycleC>;

inline constexpr auto missing =
    feature::define<"missing">(feature::semantic, feature::dependsOnNamed<"not-present">);
inline constexpr auto missingCatalog = feature::catalog<missing>;

inline constexpr auto duplicateOne = feature::define<"duplicate">(feature::semantic);
inline constexpr auto duplicateTwo =
    feature::define<"duplicate">(feature::semantic, feature::description<"other">);
inline constexpr auto duplicateCatalog = feature::catalog<duplicateOne, duplicateTwo>;

struct ExternalFeature final {
  static constexpr auto name = BasicStaticString{"external"};
  static constexpr auto description = BasicStaticString{"Third-party descriptor"};
  static constexpr feature::Version version{.major = 4, .minor = 2, .patch = 0};
  static constexpr auto featureKind = feature::FeatureKind::Semantic;
  static constexpr bool enabledByDefault = false;

  [[nodiscard]] static consteval auto dependencies() {
    return std::array<StringView, 0>{};
  }
  [[nodiscard]] static consteval auto optionalDependencies() {
    return std::array<StringView, 0>{};
  }
  [[nodiscard]] static consteval auto conflicts() {
    return std::array<StringView, 0>{};
  }
  [[nodiscard]] static consteval auto implications() {
    return std::array<StringView, 0>{};
  }
  [[nodiscard]] static consteval auto capabilities() {
    return std::array<feature::Capability, 0>{};
  }
  [[nodiscard]] static consteval auto groups() {
    return std::array<StringView, 0>{};
  }
};
inline constexpr ExternalFeature external{};
static_assert(feature::Feature<ExternalFeature>);
static_assert(feature::catalog<external>.analyze<external>(feature::buildSet<>).succeeded());

struct EcosystemMetadata final {
  static constexpr auto kind = feature::OptionKind::Custom;
  static constexpr auto label = BasicStaticString{"ecosystem"};
};

struct StatefulMetadata final {
  static constexpr auto kind = feature::OptionKind::Custom;
  int value;
};
static_assert(not feature::DescriptorOption<StatefulMetadata>);
inline constexpr auto extensible = feature::define<"extensible">(EcosystemMetadata{});

consteval auto preservedCustomMetadata() -> bool {
  bool found{};
  decltype(extensible)::forEachOption([&](auto option) -> void {
    if constexpr (std::same_as<decltype(option), EcosystemMetadata>) {
      found = option.label == BasicStaticString{"ecosystem"};
    }
  });
  return found;
}
static_assert(preservedCustomMetadata());

constexpr auto requirement = feature::require(base, logging);
template <auto Requirement>
struct RequirementHolder final {};
using RequirementNttpProof = RequirementHolder<requirement>;
static_assert(requirement.contains("base"));
static_assert(requirement.contains("logging"));
static_assert(not requirement.contains("gpu"));

[[= feature::require(base, logging)]] auto annotatedRequirement() -> void;
constexpr auto requirementAnnotations =
    std::define_static_array(std::meta::annotations_of(^^annotatedRequirement));
static_assert(requirementAnnotations.size() == 1);
static_assert(std::meta::extract<std::remove_cv_t<decltype(requirement)>>(requirementAnnotations.front())
        .contains("logging"));

constexpr auto semanticReport = features.analyze<tracing>(feature::buildSet<>);
static_assert(semanticReport.succeeded());
static_assert(semanticReport.features.contains(base));
static_assert(semanticReport.features.contains(logging));
static_assert(semanticReport.features.contains(tracing));
static_assert(not semanticReport.features.contains(metrics)); // optional does not auto-enable

constexpr auto buildRejected = features.analyze<gpu>(feature::buildSet<>);
static_assert(buildRejected.error == feature::ResolutionError::MissingBuildFeature);
static_assert(buildRejected.message.contains("gpu"));

constexpr auto buildAccepted = features.analyze<gpu>(feature::buildSet<"gpu">);
static_assert(buildAccepted.succeeded());
static_assert(buildAccepted.features.contains(gpu));
static_assert(buildAccepted.features.contains(base));

constexpr auto grouped =
    features.analyze<complete>(feature::buildSet<"gpu">, feature::detectedCapabilities());
static_assert(grouped.succeeded());
static_assert(grouped.features.size() == 5);

constexpr auto groupedByMetadata = features.analyze<defaults>(feature::buildSet<>);
static_assert(groupedByMetadata.succeeded());
static_assert(groupedByMetadata.features.contains(logging));
static_assert(groupedByMetadata.features.contains(base));
static_assert(groupedByMetadata.features.size() == 2);

constexpr auto conflictReport = features.analyze<tracing, incompatible>(feature::buildSet<>);
static_assert(conflictReport.error == feature::ResolutionError::Conflict);
static_assert(conflictReport.message.contains("tracing"));
static_assert(conflictReport.message.contains("incompatible"));

constexpr auto missingReport = missingCatalog.analyze<missing>(feature::buildSet<>);
static_assert(missingReport.error == feature::ResolutionError::MissingFeature);
static_assert(missingReport.message.contains("not-present"));

constexpr auto cycleReport = cycle.analyze<cycleA>(feature::buildSet<>);
static_assert(cycleReport.error == feature::ResolutionError::DependencyCycle);
static_assert(cycleReport.message.contains("cycle-a --> cycle-b --> cycle-c --> cycle-a"));

constexpr auto duplicateReport = duplicateCatalog.analyze<duplicateOne>(feature::buildSet<>);
static_assert(duplicateReport.error == feature::ResolutionError::DuplicateFeature);

static_assert(feature::capabilityName(feature::Capability::ReflectionCore) == "reflection_core");
constexpr auto noCapabilities = feature::CapabilitySet{};
constexpr auto capabilityReport = features.analyze<reflection>(feature::buildSet<>, noCapabilities);
static_assert(capabilityReport.error == feature::ResolutionError::MissingCapability);
static_assert(capabilityReport.message.contains("reflection"));

constexpr auto defaultReport = features.analyzeDefaults(feature::buildSet<>);
static_assert(defaultReport.succeeded());
static_assert(defaultReport.features.contains(logging));
static_assert(defaultReport.features.contains(base));
static_assert(!defaultReport.features.contains(tracing));

constexpr auto orderA = features.analyze<tracing, metrics>(feature::buildSet<>).features;
constexpr auto orderB = features.analyze<metrics, tracing>(feature::buildSet<>).features;
static_assert(orderA == orderB);
static_assert(orderA.fingerprint() == orderB.fingerprint());

constexpr auto hardResolved = features.resolve<feature::buildSet<"gpu">, complete>();
static_assert(hardResolved.contains(gpu));
static_assert(hardResolved.contains(tracing));

template <auto Set>
struct SetNttpProof final {};
using StructuralSet = SetNttpProof<hardResolved>;

static_assert(
    feature::buildSet<"alpha", "beta">.fingerprint() == feature::buildSet<"beta", "alpha">.fingerprint());

constexpr auto configuredSemantic = feature::current<tracing>(features);
static_assert(configuredSemantic.contains(base));
static_assert(configuredSemantic.contains(logging));
static_assert(configuredSemantic.contains(tracing));
constexpr auto configuredDefaults = feature::defaults(features);
static_assert(configuredDefaults.contains(base));
static_assert(configuredDefaults.contains(logging));
static_assert(feature::buildIdentity.size() == 64);
static_assert(!feature::buildEnabled<gpu>);

} // namespace featureTest

} // namespace
// NOLINTEND(readability-identifier-naming)

namespace FeatureTests {

[[ = test, = group("foundation"), = tag("feature") ]] auto descriptorMetadata() -> void {
  using namespace featureTest;

  check(decltype(base)::name == BasicStaticString{"base"});
  check(decltype(base)::description == BasicStaticString{"Shared semantic base"});
  check(decltype(base)::version.major == 1_exp);
  check(decltype(base)::version.minor == 2_exp);
  check(decltype(base)::version.patch == 3_exp);
  check(decltype(base)::featureKind == feature::FeatureKind::Semantic);
  check(decltype(gpu)::featureKind == feature::FeatureKind::Build);
}

[[ = test, = group("foundation"), = tag("feature") ]] auto canonicalResolution() -> void {
  using namespace featureTest;

  constexpr auto first = features.analyze<tracing, metrics>(feature::buildSet<>).features;
  constexpr auto second = features.analyze<metrics, tracing>(feature::buildSet<>).features;

  check(first == second);
  check(first.size() == 4_exp);
  check(first.contains(base));
  check(first.contains(logging));
  check(first.contains(tracing));
  check(first.contains(metrics));

  constexpr auto metadataGroup = features.analyze<defaults>(feature::buildSet<>).features;
  check(metadataGroup.size() == 2_exp);
  check(metadataGroup.contains(base));
  check(metadataGroup.contains(logging));
}

[[ = test, = group("foundation"), = tag("feature") ]] auto negativeSpace() -> void {
  using namespace featureTest;

  check(buildRejected.error == feature::ResolutionError::MissingBuildFeature);
  check(conflictReport.error == feature::ResolutionError::Conflict);
  check(missingReport.error == feature::ResolutionError::MissingFeature);
  check(cycleReport.error == feature::ResolutionError::DependencyCycle);
  check(duplicateReport.error == feature::ResolutionError::DuplicateFeature);
  check(capabilityReport.error == feature::ResolutionError::MissingCapability);
}

[[ = test, = group("foundation"), = tag("feature") ]] auto detectedCapabilityBridge() -> void {
  constexpr auto detected = feature::detectedCapabilities();

  check(detected.contains(feature::Capability::ImportStd) == capability::importStd);
  check(detected.contains(feature::Capability::ReflectionCore) == capability::reflectionCore);
  check(detected.contains(feature::Capability::ReflectionQueries) == capability::reflectionQueries);
  check(detected.contains(feature::Capability::ReflectionAnnotations) == capability::reflectionAnnotations);
  check(
      detected.contains(feature::Capability::ReflectionStaticStorage) == capability::reflectionStaticStorage);
  check(detected.contains(feature::Capability::ExpansionStatements) == capability::expansionStatements);
  check(detected.contains(feature::Capability::StdVocabulary) == capability::stdVocabulary);
  check(detected.contains(feature::Capability::StdHive) == capability::stdHive);
}

} // namespace FeatureTests

consteval {
  discover<^^FeatureTests>();
}
