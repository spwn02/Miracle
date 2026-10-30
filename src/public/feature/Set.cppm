export module Miracle.Feature:Set;

import std;

import Miracle.Numeric;
import Miracle.Types;

import :Descriptor;

export namespace Miracle::feature {

/// Structural set describing which build-feature implementation are compiled into a particular binary/module
/// configuration.
template <BasicStaticString... Names>
struct BuildFeatureSet final {
private:
  /// Canonically sorted stable names, materialized once per build-universe type.
  static constexpr auto sortedNames = [] -> decltype(auto) { // NOLINT(readability-identifier-naming)
    std::array<StringView, sizeof...(Names)> result{Names.view()...};
    std::ranges::sort(result);
    return result;
  }();

  /// Validates that the build universe is a set rather than a multiset.
  [[nodiscard]] static consteval auto uniqueNames() noexcept -> bool {
    return std::ranges::adjacent_find(sortedNames) == sortedNames.end();
  }

  /// Cached deterministic identity for this immutable build universe.
  static constexpr u64 fingerprintValue = [] -> u64 { // NOLINT(readability-identifier-naming)
    u64 value =
        14695981039346656037ULL; // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
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

} // namespace Miracle::feature
