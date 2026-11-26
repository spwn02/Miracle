export module Miracle.Meta:Storage;

import std;
import :Types;

export namespace Miracle::meta {

/// Callable wrapper for `std::meta::reflect_constant_array`.
struct ReflectConstantArray final {
  /// Reflects one statically materializable range as an array object with the standard element/shape rules.
  template <std::ranges::input_range Range>
  [[nodiscard]] consteval auto operator()(Range &&elements) const -> Info {
    return std::meta::reflect_constant_array(std::forward<Range>(elements));
  }
};

/// Stateless exact wrapper for `std::meta::reflect_constant_array`.
inline constexpr ReflectConstantArray reflectConstantArray{};

/// Callable wrapper for `std::meta::reflect_constant_string`.
struct ReflectConstantString final {
  /// Reflects one supported character range as a null-terminated constant string object.
  template <std::ranges::input_range Range>
  [[nodiscard]] consteval auto operator()(Range &&characters) const -> Info {
    return std::meta::reflect_constant_string(std::forward<Range>(characters));
  }
};

/// Stateless exact wrapper for `std::meta::reflect_constant_string`.
inline constexpr ReflectConstantString reflectConstantString{};

/// Callable wrapper for `std::define_static_array` used by Meta materialization APIs.
struct DefineStaticArray final {
  /// Materializes `elements` into program-lifetime static storage and returns the standard const span.
  template <std::ranges::input_range Range>
  [[nodiscard]] consteval auto operator()(Range &&elements) const -> decltype(auto) {
    return std::define_static_array(std::forward<Range>(elements));
  }
};

/// Stateless wrapper for `std::define_static_array`.
inline constexpr DefineStaticArray defineStaticArray{};

/// Callable wrapper for `std::define_static_string`.
struct DefineStaticString final {
  /// Materializes a supported character range into program-lifetime null-terminated storage.
  template <std::ranges::input_range Range>
  [[nodiscard]] consteval auto operator()(Range &&characters) const -> decltype(auto) {
    return std::define_static_string(std::forward<Range>(characters));
  }
};

/// Stateless wrapper for `std::define_static_string`.
inline constexpr DefineStaticString defineStaticString{};

/// Callable wrapper for `std::define_static_object`.
struct DefineStaticObject final {
  /// Materializes one structural object into program-lifetime storage and returns its const address.
  template <class T>
  [[nodiscard]] consteval auto operator()(T &&value) const -> decltype(auto) {
    return std::define_static_object(std::forward<T>(value));
  }
};

/// Stateless wrapper for `std::define_static_object`.
inline constexpr DefineStaticObject defineStaticObject{};

/// Callable wrapper for the standard string-literal provenance predicate used by static-string reflection.
struct IsStringLiteral final {
  /// Returns whether `characters` points at a language string literal recognized by the implementation.
  [[nodiscard]] consteval auto operator()(const char *characters) const -> bool {
    return std::is_string_literal(characters);
  }

  [[nodiscard]] consteval auto operator()(const wchar_t *characters) const -> bool {
    return std::is_string_literal(characters);
  }

  [[nodiscard]] consteval auto operator()(const char8_t *characters) const -> bool {
    return std::is_string_literal(characters);
  }

  [[nodiscard]] consteval auto operator()(const char16_t *characters) const -> bool {
    return std::is_string_literal(characters);
  }

  [[nodiscard]] consteval auto operator()(const char32_t *characters) const -> bool {
    return std::is_string_literal(characters);
  }
};

/// Stateless overload-preserving wrapper for `std::is_string_literal`.
inline constexpr IsStringLiteral isStringLiteral{};

} // namespace Miracle::meta
