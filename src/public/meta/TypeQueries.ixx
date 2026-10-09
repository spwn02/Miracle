export module Miracle.Meta:TypeQueries;

import std;
import :Types;

export namespace Miracle::meta {

/// Projects a reflection to its reflected type.
///
/// Type reflections are returned unchanged; other supported entities delegate to `std::meta::type_of`.
/// Standard semantic failures propagate as compile-time reflection failures rather than being converted into
/// sentinel values.
struct Type final {
  /// Returns `info` for type reflections, otherwise returns the reflected entity's type.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (std::meta::is_type(info)) {
      return info;
    }
    return std::meta::type_of(info);
  }
};

/// Stateless callable instance of `Type`.
inline constexpr Type type{};

/// Projects a function/function-type reflection to its return type.
///
/// Accepts reflected functions and reflected function types. Any other subject throws `std::meta::exception`
/// before the lower-level standard precondition is evaluated, preserving Miracle's stable failure boundary.
struct ReturnType final {
  /// Returns the reflected return type of a function or function type.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    const bool functionType = std::meta::is_type(info) and std::meta::is_function_type(info);
    if (not std::meta::is_function(info) and not functionType) {
      throw std::meta::exception{"returnType() requires a function reflection", info};
    }
    return std::meta::return_type_of(info);
  }
};

/// Stateless callable instance of `ReturnType`.
inline constexpr ReturnType returnType{};

/// Resolves a standard reflection alias/proxy to its canonical underlying entity.
struct Dealias final {
  /// Applies the standard canonical dealiasing operation to `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::dealias(info);
  }
};

/// Stateless callable instance of `Dealias`.
inline constexpr Dealias dealias{};

/// Exact callable wrapper for `std::meta::type_of`; unlike `type`, a type reflection is not accepted
/// specially.
inline constexpr MetaFunction typeOf{std::meta::type_of};

/// Exact callable wrapper for reflected array rank.
inline constexpr MetaFunction rank{std::meta::rank};

/// Exact binary callable wrapper for `std::meta::is_assignable_type`.
inline constexpr MetaFunction isAssignableType{std::meta::is_assignable_type};

/// Exact binary callable wrapper for `std::meta::is_swappable_with_type`.
inline constexpr MetaFunction isSwappableWithType{std::meta::is_swappable_with_type};

/// Exact binary callable wrapper for `std::meta::is_trivially_assignable_type`.
inline constexpr MetaFunction isTriviallyAssignableType{std::meta::is_trivially_assignable_type};

/// Exact binary callable wrapper for `std::meta::is_nothrow_assignable_type`.
inline constexpr MetaFunction isNothrowAssignableType{std::meta::is_nothrow_assignable_type};

/// Exact binary callable wrapper for `std::meta::is_nothrow_swappable_with_type`.
inline constexpr MetaFunction isNothrowSwappableWithType{std::meta::is_nothrow_swappable_with_type};

/// Exact binary callable wrapper for the reflected reference temporary-construction predicate.
inline constexpr MetaFunction referenceConstructsFromTemporary{
    std::meta::reference_constructs_from_temporary};

/// Exact binary callable wrapper for the reflected reference temporary-conversion predicate.
inline constexpr MetaFunction referenceConvertsFromTemporary{std::meta::reference_converts_from_temporary};

/// Exact binary callable wrapper for reflected type identity.
inline constexpr MetaFunction isSameType{std::meta::is_same_type};

/// Exact binary callable wrapper for the standard total ordering of reflected types.
inline constexpr MetaFunction typeOrder{std::meta::type_order};

/// Exact binary callable wrapper for reflected base/derived type relationships.
inline constexpr MetaFunction isBaseOfType{std::meta::is_base_of_type};

/// Exact binary callable wrapper for reflected virtual-base relationships.
inline constexpr MetaFunction isVirtualBaseOfType{std::meta::is_virtual_base_of_type};

/// Exact binary callable wrapper for reflected type convertibility.
inline constexpr MetaFunction isConvertibleType{std::meta::is_convertible_type};

/// Exact binary callable wrapper for non-throwing reflected type convertibility.
inline constexpr MetaFunction isNothrowConvertibleType{std::meta::is_nothrow_convertible_type};

/// Exact binary callable wrapper for reflected layout compatibility.
inline constexpr MetaFunction isLayoutCompatibleType{std::meta::is_layout_compatible_type};

/// Exact binary callable wrapper for pointer-interconvertible base relationships.
inline constexpr MetaFunction isPointerInterconvertibleBaseOfType{
    std::meta::is_pointer_interconvertible_base_of_type};

/// Exact binary callable wrapper for tuple-application validity.
inline constexpr MetaFunction isApplicableType{std::meta::is_applicable_type};

/// Exact binary callable wrapper for non-throwing tuple-application validity.
inline constexpr MetaFunction isNothrowApplicableType{std::meta::is_nothrow_applicable_type};

/// Exact callable wrapper returning the tuple size represented by a reflected tuple-like type.
inline constexpr MetaFunction tupleSize{std::meta::tuple_size};

/// Exact callable wrapper returning the variant size represented by a reflected variant type.
inline constexpr MetaFunction variantSize{std::meta::variant_size};

/// Exact callable wrapper for `std::meta::return_type_of`; unlike `returnType`, no Miracle preflight is
/// added.
inline constexpr MetaFunction returnTypeOf{std::meta::return_type_of};

/// Callable wrapper for the constructibility predicate taking an arbitrary reflection range of argument
/// types.
struct IsConstructibleType final {
  /// Returns whether `type` is constructible from the reflected argument-type sequence.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info type, Range &&arguments) const -> bool {
    return std::meta::is_constructible_type(type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info type, std::initializer_list<Info> arguments) const -> bool {
    return std::meta::is_constructible_type(type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_constructible_type`.
inline constexpr IsConstructibleType isConstructibleType{};

/// Callable wrapper for the trivially-constructible predicate with reflected argument types.
struct IsTriviallyConstructibleType final {
  /// Returns whether construction from `arguments` is trivial for `type`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info type, Range &&arguments) const -> bool {
    return std::meta::is_trivially_constructible_type(type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info type, std::initializer_list<Info> arguments) const -> bool {
    return std::meta::is_trivially_constructible_type(type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_trivially_constructible_type`.
inline constexpr IsTriviallyConstructibleType isTriviallyConstructibleType{};

/// Callable wrapper for the nothrow-constructible predicate with reflected argument types.
struct IsNothrowConstructibleType final {
  /// Returns whether construction from `arguments` is non-throwing for `type`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info type, Range &&arguments) const -> bool {
    return std::meta::is_nothrow_constructible_type(type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info type, std::initializer_list<Info> arguments) const -> bool {
    return std::meta::is_nothrow_constructible_type(type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_nothrow_constructible_type`.
inline constexpr IsNothrowConstructibleType isNothrowConstructibleType{};

/// Callable wrapper for `std::meta::is_invocable_type`.
struct IsInvocableType final {
  /// Returns whether the reflected callable type can be invoked with `arguments`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info type, Range &&arguments) const -> bool {
    return std::meta::is_invocable_type(type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info type, std::initializer_list<Info> arguments) const -> bool {
    return std::meta::is_invocable_type(type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_invocable_type`.
inline constexpr IsInvocableType isInvocableType{};

/// Callable wrapper for `std::meta::is_invocable_r_type`.
struct IsInvocableRType final {
  /// Returns whether the reflected callable type is invocable with a result convertible to `result`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info result, Info type, Range &&arguments) const -> bool {
    return std::meta::is_invocable_r_type(result, type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info result, Info type, std::initializer_list<Info> arguments) const
      -> bool {
    return std::meta::is_invocable_r_type(result, type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_invocable_r_type`.
inline constexpr IsInvocableRType isInvocableRType{};

/// Callable wrapper for `std::meta::is_nothrow_invocable_type`.
struct IsNothrowInvocableType final {
  /// Returns whether the reflected callable type is non-throwingly invocable with `arguments`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info type, Range &&arguments) const -> bool {
    return std::meta::is_nothrow_invocable_type(type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info type, std::initializer_list<Info> arguments) const -> bool {
    return std::meta::is_nothrow_invocable_type(type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_nothrow_invocable_type`.
inline constexpr IsNothrowInvocableType isNothrowInvocableType{};

/// Callable wrapper for `std::meta::is_nothrow_invocable_r_type`.
struct IsNothrowInvocableRType final {
  /// Returns whether invocation is non-throwing and its result is convertible to `result`.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info result, Info type, Range &&arguments) const -> bool {
    return std::meta::is_nothrow_invocable_r_type(result, type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info result, Info type, std::initializer_list<Info> arguments) const
      -> bool {
    return std::meta::is_nothrow_invocable_r_type(result, type, arguments);
  }
};

/// Exact wrapper for `std::meta::is_nothrow_invocable_r_type`.
inline constexpr IsNothrowInvocableRType isNothrowInvocableRType{};

/// Callable wrapper for array extent with the standard default dimension of zero.
struct Extent final {
  /// Returns the extent of the selected reflected array dimension.
  [[nodiscard]] consteval auto operator()(Info type, unsigned dimension = 0) const -> std::size_t {
    return std::meta::extent(type, dimension);
  }
};

/// Exact wrapper for `std::meta::extent`.
inline constexpr Extent extent{};

} // namespace Miracle::meta
