export module Miracle.Feature:Requirement;

import std;

import Miracle.Numeric;

import :Descriptor;

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
  [[nodiscard]] constexpr auto contains([[maybe_unused]] std::string_view) const noexcept -> bool { // NOLINT
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
  [[nodiscard]] constexpr auto contains(std::string_view name) const noexcept -> bool {
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
  static constexpr usize featureCount = sizeof...(Required); // NOLINT(readability-identifier-naming)

  /// Structural descriptor values retained for reflection extraction.
  RequirementValues<Required...> features{};

  /// Returns whether a feature name belongs to this requirement.
  [[nodiscard]] constexpr auto contains(std::string_view name) const noexcept -> bool {
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
