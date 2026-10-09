export module Miracle.Meta:Values;

import std;
import :Types;

export namespace Miracle::meta {

/// Converts an entity/annotation reflection to the reflection of its constant value.
///
/// This is intentionally a callable projection so annotation queries can compose as `.map(meta::constant)` in
/// Query.
struct Constant final {
  /// Returns the reflection of the constant value represented by `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::constant_of(info);
  }
};

/// Stateless callable instance of `Constant`.
inline constexpr Constant constant{};

/// Typed extraction projection for a reflected constant.
///
/// `T` is the requested C++ value type. Standard extraction preconditions are preserved and diagnosed during
/// constant evaluation; the callable stores no state.
template <class T>
struct Extract final {
  /// Extracts the reflected constant as `T`.
  [[nodiscard]] consteval auto operator()(Info info) const -> T {
    return std::meta::extract<T>(info);
  }
};

/// Lowercase callable variable template used as `meta::extract<T>(info)` or a Query projection.
template <class T>
inline constexpr Extract<T> extract{};

/// Exact callable wrapper resolving the object represented by an object reflection.
inline constexpr MetaFunction objectOf{std::meta::object_of};

/// Exact callable wrapper for `std::meta::constant_of`.
inline constexpr MetaFunction constantOf{std::meta::constant_of};

/// Callable wrapper for `std::meta::reflect_constant` preserving the standard by-value argument semantics.
struct ReflectConstant final {
  /// Reflects one structural constant value; invalid/non-structural values follow the standard failure
  /// domain.
  template <class T>
  [[nodiscard]] consteval auto operator()(T value) const -> Info {
    return std::meta::reflect_constant(value);
  }
};

/// Stateless exact wrapper for `std::meta::reflect_constant`.
inline constexpr ReflectConstant reflectConstant{};

/// Callable wrapper for `std::meta::reflect_object`.
struct ReflectObject final {
  /// Reflects the identity of the supplied constant-evaluable object without copying it.
  template <class T>
  [[nodiscard]] consteval auto operator()(T &value) const -> Info {
    return std::meta::reflect_object(value);
  }
};

/// Stateless exact wrapper for `std::meta::reflect_object`.
inline constexpr ReflectObject reflectObject{};

/// Callable wrapper for `std::meta::reflect_function`.
struct ReflectFunction final {
  /// Reflects the identity of the supplied function/function-object entity according to the standard
  /// facility.
  template <class T>
  [[nodiscard]] consteval auto operator()(T &value) const -> Info {
    return std::meta::reflect_function(value);
  }
};

/// Stateless exact wrapper for `std::meta::reflect_function`.
inline constexpr ReflectFunction reflectFunction{};

/// Callable wrapper for `std::meta::reflect_invoke` with and without explicit template arguments.
struct ReflectInvoke final {
  /// Reflectively invokes `target` with one reflection range of ordinary arguments.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info target, Range &&arguments) const -> Info {
    return std::meta::reflect_invoke(target, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced ordinary-argument list.
  [[nodiscard]] consteval auto operator()(Info target, std::initializer_list<Info> arguments) const -> Info {
    return std::meta::reflect_invoke(target, arguments);
  }

  /// Reflectively invokes `target` with explicit template and ordinary argument reflection ranges.
  template <ReflectionRange TemplateArguments, ReflectionRange Arguments>
  [[nodiscard]] consteval auto operator()(Info target,
      TemplateArguments &&templateArguments,
      Arguments &&arguments) const -> Info {
    return std::meta::reflect_invoke(
        target, std::forward<TemplateArguments>(templateArguments), std::forward<Arguments>(arguments));
  }

  /// Preserves the standard call form when only the template-argument range is a braced list.
  template <ReflectionRange Arguments>
  [[nodiscard]] consteval auto operator()(Info target,
      std::initializer_list<Info> templateArguments,
      Arguments &&arguments) const -> Info {
    return std::meta::reflect_invoke(target, templateArguments, std::forward<Arguments>(arguments));
  }

  /// Preserves the standard call form when only the ordinary-argument range is a braced list.
  template <ReflectionRange TemplateArguments>
  [[nodiscard]] consteval auto operator()(Info target,
      TemplateArguments &&templateArguments,
      std::initializer_list<Info> arguments) const -> Info {
    return std::meta::reflect_invoke(target, std::forward<TemplateArguments>(templateArguments), arguments);
  }

  /// Preserves the fully braced standard call form for template and ordinary arguments.
  [[nodiscard]] consteval auto operator()(Info target,
      std::initializer_list<Info> templateArguments,
      std::initializer_list<Info> arguments) const -> Info {
    return std::meta::reflect_invoke(target, templateArguments, arguments);
  }
};

/// Exact overload-preserving wrapper for `std::meta::reflect_invoke`.
inline constexpr ReflectInvoke reflectInvoke{};

} // namespace Miracle::meta
