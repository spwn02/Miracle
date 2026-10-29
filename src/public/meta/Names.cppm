export module Miracle.Meta:Names;

import std;
import :Types;

export namespace Miracle::meta {

/// Optional declared-identifier projection.
///
/// Returns `nullopt` for reflections without an identifier instead of triggering standard precondition on
/// `identifier_of`. The returned `string_view` refers to compiler-managed static reflection text.
struct Name final {
  /// Returns the declared identifier when one exists; otherwise returns an empty optional.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::optional<std::string_view> {
    if (not std::meta::has_identifier(info)) {
      return std::nullopt;
    }
    return std::meta::identifier_of(info);
  }
};

/// Stateless callable instance of `Name`.
inline constexpr Name name{};

/// Strict declared-identifier projection.
///
/// Unlike `name`, absence is a semantic error: constant evaluation throws `std::meta::exception` carrying the
/// offending reflection. This makes the operation appropriate for pipelines whose contract requires a source
/// identifier.
struct RequireName final {
  /// Returns the declared identifier or throws `std::meta::exception` when the reflection has none.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::string_view {
    if (not std::meta::has_identifier(info)) {
      throw std::meta::exception{"requireName() requires a reflection with an identifier", info};
    }
    return std::meta::identifier_of(info);
  }
};

/// Stateless callable instance of `RequireName`.
inline constexpr RequireName requireName{};

/// Descriptive reflection spelling intended for diagnostics rather than source-identifier logic.
///
/// Delegates to `std::meta::display_string_of`, so entities without identifers still receive a printable
/// description.
struct DisplayName final {
  /// Returns the standard descriptive spelling of `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::string_view {
    return std::meta::display_string_of(info);
  }
};

/// Stateless callable isntance of `DisplayName`.
inline constexpr DisplayName displayName{};

/// Returns the lexical/semantic parent reflection of an entity.
///
/// Subjects without a parent are rejected with `std::meta::exception` carrying the original reflection.
struct Parent final {
  /// Returns the reflected parent or throws when the subject has no parent.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::has_parent(info)) {
      throw std::meta::exception{"parent() requires a reflection with a parent", info};
    }
    return std::meta::parent_of(info);
  }
};

/// Stateless callable instance of `Parent`.
inline constexpr Parent parent{};

/// Returns the source location associated with a reflected declaration/entity.
///
/// The result is the standard `std::source_location` and can be fed directly into Miracle diagnostics.
struct SourceLocation final {
  /// Returns the standard source location attached to `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::source_location {
    return std::meta::source_location_of(info);
  }
};

/// Stateless callable instance of `SourceLocation`.
inline constexpr SourceLocation sourceLocation{};

/// Exact callable wrapper for `std::meta::identifier_of`; unlike `name`, absence remains a standard semantic
/// error.
inline constexpr MetaFunction identifierOf{std::meta::identifier_of};

/// Exact callable wrapper for `std::meta::display_string_of`.
inline constexpr MetaFunction displayStringOf{std::meta::display_string_of};

/// Exact UTF-8 callable wrapper for `std::meta::u8identifier_of`.
inline constexpr MetaFunction u8IdentifierOf{std::meta::u8identifier_of};

/// Exact UTF-8 callable wrapper for `std::meta::u8display_string_of`.
inline constexpr MetaFunction u8DisplayStringOf{std::meta::u8display_string_of};

/// Exact callable wrapper for `std::meta::source_location_of`.
inline constexpr MetaFunction sourceLocationOf{std::meta::source_location_of};

/// Exact callable wrapper for `std::meta::parent_of`; unlike `parent`, Miracle performs no preflight
/// diagnostic.
inline constexpr MetaFunction parentOf{std::meta::parent_of};

} // namespace Miracle::meta
