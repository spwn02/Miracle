export module Miracle.Meta:TypeTransforms;

import std;
import :Types;

export namespace Miracle::meta {

/// Exact callable wrapper for removing top-level const/volatile qualification from a reflected type.
struct RemoveCv final {
  /// Returns the exact standard `remove_cv` transformation result.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::remove_cv(info);
  }
};

/// Stateless callable instance of `RemoveCv`.
inline constexpr RemoveCv removeCv{};

/// Exact callable wrapper for removing one reference layer from a reflected type.
struct RemoveReference final {
  /// Returns the exact standard `remove_reference` transformation result.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::remove_reference(info);
  }
};

/// Stateless callable instance of `RemoveReference`.
inline constexpr RemoveReference removeReference{};

/// Exact callable wrapper removing both reference and top-level cv qualification from a reflected type.
struct RemoveCvref final {
  /// Returns the exact standard `remove_cvref` transformation result.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::remove_cvref(info);
  }
};

/// Stateless callable instance of `RemoveCvref`.
inline constexpr RemoveCvref removeCvref{};

/// Exact binary callable wrapper returning the reflected result of tuple application.
inline constexpr MetaFunction applyResult{std::meta::apply_result};

/// Exact callable wrapper for removing top-level const qualification from a reflected type.
inline constexpr MetaFunction removeConst{std::meta::remove_const};

/// Exact callable wrapper for removing top-level volatile qualification from a reflected type.
inline constexpr MetaFunction removeVolatile{std::meta::remove_volatile};

/// Exact callable wrapper for adding top-level const qualification to a reflected type.
inline constexpr MetaFunction addConst{std::meta::add_const};

/// Exact callable wrapper for adding top-level volatile qualification to a reflected type.
inline constexpr MetaFunction addVolatile{std::meta::add_volatile};

/// Exact callable wrapper for adding top-level const and volatile qualification to a reflected type.
inline constexpr MetaFunction addCv{std::meta::add_cv};

/// Exact callable wrapper for adding an lvalue-reference layer to a reflected type.
inline constexpr MetaFunction addLvalueReference{std::meta::add_lvalue_reference};

/// Exact callable wrapper for adding an rvalue-reference layer to a reflected type.
inline constexpr MetaFunction addRvalueReference{std::meta::add_rvalue_reference};

/// Exact callable wrapper for the signed counterpart of a reflected integral/enum type.
inline constexpr MetaFunction makeSigned{std::meta::make_signed};

/// Exact callable wrapper for the unsigned counterpart of a reflected integral/enum type.
inline constexpr MetaFunction makeUnsigned{std::meta::make_unsigned};

/// Exact callable wrapper removing one array extent from a reflected type.
inline constexpr MetaFunction removeExtent{std::meta::remove_extent};

/// Exact callable wrapper removing every array extent from a reflected type.
inline constexpr MetaFunction removeAllExtents{std::meta::remove_all_extents};

/// Exact callable wrapper removing one pointer layer from a reflected type.
inline constexpr MetaFunction removePointer{std::meta::remove_pointer};

/// Exact callable wrapper adding a pointer layer to a reflected type.
inline constexpr MetaFunction addPointer{std::meta::add_pointer};

/// Exact callable wrapper applying the reflected equivalent of `std::decay_t`.
inline constexpr MetaFunction decay{std::meta::decay};

/// Exact callable wrapper for `std::meta::unwrap_reference`.
inline constexpr MetaFunction unwrapReference{std::meta::unwrap_reference};

/// Exact callable wrapper for `std::meta::unwrap_ref_decay`.
inline constexpr MetaFunction unwrapRefDecay{std::meta::unwrap_ref_decay};

/// Callable wrapper for `std::meta::common_type`.
struct CommonType final {
  /// Returns the reflected common type of the supplied type-reflection range.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Range &&arguments) const -> Info {
    return std::meta::common_type(std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(std::initializer_list<Info> arguments) const -> Info {
    return std::meta::common_type(arguments);
  }
};

/// Exact wrapper for `std::meta::common_type`.
inline constexpr CommonType commonType{};

/// Callable wrapper for `std::meta::common_reference`.
struct CommonReference final {
  /// Returns the reflected common-reference type of the supplied type-reflection range.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Range &&arguments) const -> Info {
    return std::meta::common_reference(std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(std::initializer_list<Info> arguments) const -> Info {
    return std::meta::common_reference(arguments);
  }
};

/// Exact wrapper for `std::meta::common_reference`.
inline constexpr CommonReference commonReference{};

/// Exact callable wrapper returning the underlying integral type of a reflected enum type.
struct UnderlyingType final {
  /// Returns the exact standard `underlying_type` transformation result.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::underlying_type(info);
  }
};

/// Stateless callable instance of `UnderlyingType`.
inline constexpr UnderlyingType underlyingType{};

/// Callable wrapper for `std::meta::invoke_result`.
struct InvokeResult final {
  /// Returns the reflected invocation result for `type` and the supplied argument-type reflections.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info type, Range &&arguments) const -> Info {
    return std::meta::invoke_result(type, std::forward<Range>(arguments));
  }

  /// Preserves the standard default-range call form for a braced reflection list.
  [[nodiscard]] consteval auto operator()(Info type, std::initializer_list<Info> arguments) const -> Info {
    return std::meta::invoke_result(type, arguments);
  }
};

/// Exact wrapper for `std::meta::invoke_result`.
inline constexpr InvokeResult invokeResult{};

/// Callable wrapper for the indexed tuple-element transformation.
struct TupleElement final {
  /// Returns the reflected element type at `index` in the reflected tuple-like type.
  [[nodiscard]] consteval auto operator()(std::size_t index, Info type) const -> Info {
    return std::meta::tuple_element(index, type);
  }
};

/// Exact wrapper for `std::meta::tuple_element`.
inline constexpr TupleElement tupleElement{};

/// Callable wrapper for the indexed variant-alternative transformation.
struct VariantAlternative final {
  /// Returns the reflected alternative type at `index` in the reflected variant type.
  [[nodiscard]] consteval auto operator()(std::size_t index, Info type) const -> Info {
    return std::meta::variant_alternative(index, type);
  }
};

/// Exact wrapper for `std::meta::variant_alternative`.
inline constexpr VariantAlternative variantAlternative{};

} // namespace Miracle::meta
