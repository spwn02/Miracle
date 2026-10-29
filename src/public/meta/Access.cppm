export module Miracle.Meta:Access;

import std;
import :Types;

export namespace Miracle::meta {

/// Exact callable wrapper for access testing undr an explicit reflection access context.
inline constexpr MetaFunction isAccessible{std::meta::is_accessible};

/// Caller-sensitive wrapper for `std::meta::current_function`.
///
/// The default argument is intentional: C++ evaluates it at the caller, preserving the standard facility's
/// lexical reflection context instead of reporting this wrapper's `operator()`.
struct CurrentFunction final {
  /// Returns the current function reflection captured at the call site.
  [[nodiscard]] consteval auto operator()(Info current = std::meta::current_function()) const -> Info {
    return current;
  }
};

/// Stateless caller-sensitive wrapper for `std:meta::current_function`.
inline constexpr CurrentFunction currentFunction{};

/// Caller-sensitive wrapper for `std::meta::current_class` using a call-site evaluated default argument.
struct CurrentClass final {
  /// Returns the current class reflection captured at the call site.
  [[nodiscard]] consteval auto operator()(Info current = std::meta::current_class()) const -> Info {
    return current;
  }
};

/// Stateless caller-sensitive wrapper for `std::meta::current_class`.
inline constexpr CurrentClass currentClass{};

/// Caller-sensitive wrapper for `std::meta::current_namespace` using a call-site-evaluated default argument.
struct CurrentNamespace final {
  /// Returns the current namespace reflection captured at the call site.
  [[nodiscard]] consteval auto operator()(Info current = std::meta::current_namespace()) const -> Info {
    return current;
  }
};

/// Stateless caller-sensitive wrapper for `std::meta::current_namespace`.
inline constexpr CurrentNamespace currentNamespace;

/// Exact callable wrapper testing whether a class has an inaccessible non-static data member.
inline constexpr MetaFunction hasInaccessibleNonstaticDataMembers{
    std::meta::has_inaccessible_nonstatic_data_members};

/// Exact callable wrapper testing whether a class has an inaccessible base.
inline constexpr MetaFunction hasInaccessibleBases{std::meta::has_inaccessible_bases};

/// Exact callable wrapper testing whether any reflected class subobject is inaccessible.
inline constexpr MetaFunction hasInaccessibleSubobjects{std::meta::has_inaccessible_subobjects};

} // namespace Miracle::meta
