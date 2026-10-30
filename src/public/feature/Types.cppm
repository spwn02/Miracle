export module Miracle.Feature:Types;

import std;

import Miracle.Numeric;
import Miracle.Types;
import Miracle.StaticString;

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

// NOLINTBEGIN(readability-identifier-naming)
template <>
struct DescriptionSelector<> final {
  static constexpr BasicStaticString value{""};
};

template <class First, class... Rest>
struct DescriptionSelector<First, Rest...> final {
  static constexpr BasicStaticString value = [] -> decltype(auto) {
    if constexpr (First::kind == OptionKind::Description) {
      return First::value;
    } else {
      return DescriptionSelector<Rest...>::value;
    }
  }();
};
// NOLINTEND(readability-identifier-naming)

/// Invokes `function` for every option in `Options...` matching `Kind`.
template <OptionKind Kind, class... Options, class Function>
constexpr auto forEachOption(Function &&function) -> void {
  auto &&callback = std::forward<Function>(function);

  (
      [&] -> void {
        if constexpr (Options::kind == Kind) {
          std::invoke(callback, Options{});
        }
      }(),
      ...);
}

/// Computes a deterministic FNV-1a hash for build/set identity diagnostics,
constexpr auto hashName(u64 seed, StringView name) noexcept -> u64 {
  u64 value = seed;
  for (const char codeUnit : name) {
    value ^= static_cast<u64>(static_cast<unsigned char>(codeUnit));
    value *= 1099511628211ULL; // NOLINT(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
  }
  return value;
}

} // namespace Miracle::feature
