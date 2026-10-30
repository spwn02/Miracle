export module Miracle.Feature:Catalog;

import std;

import Miracle.Numeric;
import Miracle.Types;

import :Set;

// NOLINTBEGIN(readability-identifier-naming)
export namespace Miracle::feature {

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
    auto &&callback = std::forward<Function>(function);
    (std::invoke(callback, Features), ...);
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
// NOLINTEND(readability-identifier-naming)
