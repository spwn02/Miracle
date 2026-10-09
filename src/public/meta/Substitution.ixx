export module Miracle.Meta:Substitution;

import std;
import :Types;

export namespace Miracle::meta {

/// Returns the originating template for a reflection that carries template arguments.
///
/// Non-template-instantiation subjects are rejected with `std::meta::exception`; callers asking only whether
/// a subject is templated should use an appropriate predicate instead of exception-driven probing.
struct TemplateOf final {
  /// Returns the originating template of an instantiated/template-argument-bearing reflection.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::template_of(info);
  }
};

/// Stateless callable instance of `TemplateOf`.
inline constexpr TemplateOf templateOf{};

/// Non-throwing probe for whether `arguments` can instantiate the reflected template `templ`.
///
/// The argument range must yield `Info` values. No argument storage is retained after constant evaluation.
struct CanSubstitute final {
  /// Returns whether `arguments` form a valid substitution for `templ` without throwing on substitution
  /// failure.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info templ, Range &&arguments) const -> bool {
    return std::meta::can_substitute(templ, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info templ, std::initializer_list<Info> arguments) const -> bool {
    return std::meta::can_substitute(templ, arguments);
  }
};

/// Stateless callable instance of `CanSubstitute`.
inline constexpr CanSubstitute canSubstitute{};

/// Instantiates a reflected template from a range of reflection arguments.
///
/// Invalid substitution deliberately propagates `std::meta::exception`, enabling both strict diagnostics and
/// the speculative consteval `try`/`catch` control-flow model defined for Meta.
struct Substitute final {
  /// Returns the instantiated reflection; invalid substitution propagates `std::meta::exception`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info templ, Range &&arguments) const -> Info {
    return std::meta::substitute(templ, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info templ, std::initializer_list<Info> arguments) const -> Info {
    return std::meta::substitute(templ, arguments);
  }
};

/// Stateless callable instance of `Substitute`.
inline constexpr Substitute substitute{};

} // namespace Miracle::meta
