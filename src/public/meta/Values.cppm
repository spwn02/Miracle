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
  /// Reflects the identity of the supplied function entity (the standard requires a function type) according
  /// to the standard facility.
  template <class T>
  [[nodiscard]] consteval auto operator()(T &value) const -> Info {
    return std::meta::reflect_function(value);
  }
};

/// Stateless exact wrapper for `std::meta::reflect_function`.
inline constexpr ReflectFunction reflectFunction{};

} // namespace Miracle::meta
