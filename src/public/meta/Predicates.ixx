export module Miracle.Meta:Predicates;

import std;
import :Types;

namespace Miracle::meta {

/// Structural marker used only to constrain the predicate-composition operators.
///
/// A marker avoids inheritance or a public predicate base class: any data-oriented callable carrying this
/// member can participate in `!`, `&&`, and `||` composition without runtime polymorphism.
template <class T>
inline constexpr bool metaPredicate = requires { std::remove_cvref_t<T>::miracleMetaPredicate_; } and
                                      std::remove_cvref_t<T>::miracleMetaPredicate_;

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Data-oriented wrapper for a consteval reflection predicate.
///
/// Predicate state (including constant-evaluable captures) is stored directly with empty-state optimization.
/// The marker enables structural composition without inheritance, virtual dispatch, or a public predicate
/// base class.
template <class Function>
struct Predicate final {
  /// Structural tag consumed by the private composition constraint; it carries no runtime state.
  static constexpr bool miracleMetaPredicate_{true};

  /// Concrete predicate callable/capture state; empty callables consume no storage.
  [[no_unique_address]] Function function;

  /// Evaluates the predicate for one reflection and normalizes the result to `bool`.
  [[nodiscard]] consteval auto operator()(Info info) const -> bool {
    return static_cast<bool>(function(info));
  }
};

/// Deduction guide preserving the concrete callable/capture type as predicate value state.
template <class Function>
Predicate(Function) -> Predicate<Function>;

/// Negates a Meta predicate while retaining its captured state by value.
template <class PredicateType>
  requires metaPredicate<PredicateType>
[[nodiscard]] consteval auto operator!(PredicateType predicate) {
  return Predicate{[predicate](Info info) consteval -> bool { return not predicate(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical AND.
template <class Left, class Right>
  requires(metaPredicate<Left> and metaPredicate<Right>)
[[nodiscard]] consteval auto operator&&(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) and right(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical OR.
template <class Left, class Right>
  requires(metaPredicate<Left> and metaPredicate<Right>)
[[nodiscard]] consteval auto operator||(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) or right(info); }};
}

/// Matches reflections carrying a declared identifier.
///
/// Use this (or the optional `meta::name` projection) for ordinary Boolean branching; `meta::requireName` is
/// the strict semantic operation for callers that require an identifier and want a diagnostic failure; exact
/// wrapper for `std::meta::has_identifier`.
inline constexpr Predicate hasIdentifier{std::meta::has_identifier};

/// Matches reflections with an associated semantic/lexical parent; exact wrapper for
/// `std::meta::has_parent`.
inline constexpr Predicate hasParent{std::meta::has_parent};

/// Matches reflections carrying a recoverable template-argument sequence; exact wrapper for
/// `std::meta::has_template_arguments`.
inline constexpr Predicate hasTemplateArguments{std::meta::has_template_arguments};

/// Matches function declaration reflections, excluding function templates; exact wrapper for
/// `std::meta::is_function`.
inline constexpr Predicate isFunction{std::meta::is_function};

/// Matches function-template reflections; exact wrapper for `std::meta::is_function_template`.
inline constexpr Predicate isFunctionTemplate{std::meta::is_function_template};

/// Matches non-static data members and static member variables.
inline constexpr Predicate isField{[](Info info) consteval -> bool {
  return std::meta::is_nonstatic_data_member(info) or
         (std::meta::is_variable(info) and std::meta::is_static_member(info));
}};

/// Matches static class members; exact wrapper for `std::meta::is_static_member`.
inline constexpr Predicate isStatic{std::meta::is_static_member};

/// Matches non-static data members only; exact wrapper for `std::meta::is_nonstatic_data_member`.
inline constexpr Predicate isInstanceData{std::meta::is_nonstatic_data_member};

/// Matches constructor declaration reflections; exact wrapper for `std::meta::is_constructor`.
inline constexpr Predicate isConstructor{std::meta::is_constructor};

/// Matches namespace reflections; exact wrapper for `std::meta::is_namespace`.
inline constexpr Predicate isNamespace{std::meta::is_namespace};

/// Matches base-specifier reflections returned by `bases`; exact wrapper for `std::meta::is_base`.
inline constexpr Predicate isBase{std::meta::is_base};

/// Matches enumerator reflections; exact wrapper for `std::meta::is_enumerator`.
inline constexpr Predicate isEnumerator{std::meta::is_enumerator};

/// Matches variable declaration reflections; exact wrapper for `std::meta::is_variable`.
inline constexpr Predicate isVariable{std::meta::is_variable};

/// Matches template reflections supported by the standard reflection predicate; exact wrapper for
/// `std::meta::is_template`.
inline constexpr Predicate isTemplate{std::meta::is_template};

/// Matches type-alias reflections without automatically dealiasing them; exact wrapper for
/// `std::meta::is_type_alias`.
inline constexpr Predicate isTypeAlias{std::meta::is_type_alias};

/// Matches namespace-alias declarations without automatically dealiasing them; exact wrapper for
/// `std::meta::is_namespace_alias`.
inline constexpr Predicate isNamespaceAlias{std::meta::is_namespace_alias};

/// Matches concept reflections; exact wrapper for `std::meta::is_concept`.
inline constexpr Predicate isConcept{std::meta::is_concept};

/// Matches reflections whose declared access is public; exact wrapper for `std::meta::is_public`.
inline constexpr Predicate isPublic{std::meta::is_public};

/// Matches reflections whose declared access is protected; exact wrapper for `std::meta::is_protected`.
inline constexpr Predicate isProtected{std::meta::is_protected};

/// Matches reflections whose declared access is private; exact wrapper for `std::meta::is_private`.
inline constexpr Predicate isPrivate{std::meta::is_private};

/// Matches entities declared virtual; exact wrapper for `std::meta::is_virtual`.
inline constexpr Predicate isVirtual{std::meta::is_virtual};

/// Matches pure-virtual function entities; exact wrapper for `std::meta::is_pure_virtual`.
inline constexpr Predicate isPureVirtual{std::meta::is_pure_virtual};

/// Matches entities declared with override semantics; exact wrapper for `std::meta::is_override`.
inline constexpr Predicate isOverride{std::meta::is_override};

/// Matches deleted function entities; exact wrapper for `std::meta::is_deleted`.
inline constexpr Predicate isDeleted{std::meta::is_deleted};

/// Matches defaulted function entities; exact wrapper for `std::meta::is_defaulted`.
inline constexpr Predicate isDefaulted{std::meta::is_defaulted};

/// Matches explicit declarations; exact wrapper for `std::meta::is_explicit`.
inline constexpr Predicate isExplicit{std::meta::is_explicit};

/// Matches noexcept function declarations/types supported by the standard predicate; exact wrapper for
/// `std::meta::is_noexcept`.
inline constexpr Predicate isNoexcept{std::meta::is_noexcept};

/// Matches bit-field data members; exact wrapper for `std::meta::is_bit_field`.
inline constexpr Predicate isBitField{std::meta::is_bit_field};

/// Matches const-qualified reflected declarations; exact wrapper for `std::meta::is_const`.
inline constexpr Predicate isConst{std::meta::is_const};

/// Matches volatile-qualified reflected declarations; exact wrapper for `std::meta::is_volatile`.
inline constexpr Predicate isVolatile{std::meta::is_volatile};

/// Matches mutable non-static data members; exact wrapper for `std::meta::is_mutable_member`.
inline constexpr Predicate isMutableMember{std::meta::is_mutable_member};

/// Matches lvalue-reference-qualified member functions; exact wrapper for
/// `std::meta::is_lvalue_reference_qualified`.
inline constexpr Predicate isLvalueReferenceQualified{std::meta::is_lvalue_reference_qualified};

/// Matches rvalue-reference-qualified member functions; exact wrapper for
/// `std::meta::is_rvalue_reference_qualified`.
inline constexpr Predicate isRvalueReferenceQualified{std::meta::is_rvalue_reference_qualified};

/// Matches entities with static storage duration; exact wrapper for `std::meta::has_static_storage_duration`.
inline constexpr Predicate hasStaticStorageDuration{std::meta::has_static_storage_duration};

/// Matches entities with thread storage duration; exact wrapper for `std::meta::has_thread_storage_duration`.
inline constexpr Predicate hasThreadStorageDuration{std::meta::has_thread_storage_duration};

/// Matches entities with automatic storage duration; exact wrapper for
/// `std::meta::has_automatic_storage_duration`.
inline constexpr Predicate hasAutomaticStorageDuration{std::meta::has_automatic_storage_duration};

/// Matches entities with internal linkage; exact wrapper for `std::meta::has_internal_linkage`.
inline constexpr Predicate hasInternalLinkage{std::meta::has_internal_linkage};

/// Matches entities with module linkage; exact wrapper for `std::meta::has_module_linkage`.
inline constexpr Predicate hasModuleLinkage{std::meta::has_module_linkage};

/// Matches entities with external linkage; exact wrapper for `std::meta::has_external_linkage`.
inline constexpr Predicate hasExternalLinkage{std::meta::has_external_linkage};

/// Matches entities having any language linkage; exact wrapper for `std::meta::has_linkage`.
inline constexpr Predicate hasLinkage{std::meta::has_linkage};

/// Matches entities with C language linkage; exact wrapper for `std::meta::has_c_language_linkage`.
inline constexpr Predicate hasCLanguageLinkage{std::meta::has_c_language_linkage};

/// Matches class-member entities; exact wrapper for `std::meta::is_class_member`.
inline constexpr Predicate isClassMember{std::meta::is_class_member};

/// Matches namespace-member entities; exact wrapper for `std::meta::is_namespace_member`.
inline constexpr Predicate isNamespaceMember{std::meta::is_namespace_member};

/// Matches non-static data-member entities; exact wrapper for `std::meta::is_nonstatic_data_member`.
inline constexpr Predicate isNonstaticDataMember{std::meta::is_nonstatic_data_member};

/// Matches static class-member entities; exact wrapper for `std::meta::is_static_member`.
inline constexpr Predicate isStaticMember{std::meta::is_static_member};

/// Matches variable-template reflections; exact wrapper for `std::meta::is_variable_template`.
inline constexpr Predicate isVariableTemplate{std::meta::is_variable_template};

/// Matches class-template reflections; exact wrapper for `std::meta::is_class_template`.
inline constexpr Predicate isClassTemplate{std::meta::is_class_template};

/// Matches alias-template reflections; exact wrapper for `std::meta::is_alias_template`.
inline constexpr Predicate isAliasTemplate{std::meta::is_alias_template};

/// Matches conversion-function-template reflections; exact wrapper for
/// `std::meta::is_conversion_function_template`.
inline constexpr Predicate isConversionFunctionTemplate{std::meta::is_conversion_function_template};

/// Matches operator-function-template reflections; exact wrapper for
/// `std::meta::is_operator_function_template`.
inline constexpr Predicate isOperatorFunctionTemplate{std::meta::is_operator_function_template};

/// Matches literal-operator-template reflections; exact wrapper for
/// `std::meta::is_literal_operator_template`.
inline constexpr Predicate isLiteralOperatorTemplate{std::meta::is_literal_operator_template};

/// Matches constructor-template reflections; exact wrapper for `std::meta::is_constructor_template`.
inline constexpr Predicate isConstructorTemplate{std::meta::is_constructor_template};

/// Matches structured-binding reflections; exact wrapper for `std::meta::is_structured_binding`.
inline constexpr Predicate isStructuredBinding{std::meta::is_structured_binding};

/// Matches data members with default member initializers; exact wrapper for
/// `std::meta::has_default_member_initializer`.
inline constexpr Predicate hasDefaultMemberInitializer{std::meta::has_default_member_initializer};

/// Matches conversion-function declarations; exact wrapper for `std::meta::is_conversion_function`.
inline constexpr Predicate isConversionFunction{std::meta::is_conversion_function};

/// Matches operator-function declarations; exact wrapper for `std::meta::is_operator_function`.
inline constexpr Predicate isOperatorFunction{std::meta::is_operator_function};

/// Matches literal-operator declarations; exact wrapper for `std::meta::is_literal_operator`.
inline constexpr Predicate isLiteralOperator{std::meta::is_literal_operator};

/// Matches default-constructor declarations; exact wrapper for `std::meta::is_default_constructor`.
inline constexpr Predicate isDefaultConstructor{std::meta::is_default_constructor};

/// Matches copy-constructor declarations; exact wrapper for `std::meta::is_copy_constructor`.
inline constexpr Predicate isCopyConstructor{std::meta::is_copy_constructor};

/// Matches move-constructor declarations; exact wrapper for `std::meta::is_move_constructor`.
inline constexpr Predicate isMoveConstructor{std::meta::is_move_constructor};

/// Matches assignment-operator declarations; exact wrapper for `std::meta::is_assignment`.
inline constexpr Predicate isAssignment{std::meta::is_assignment};

/// Matches copy-assignment declarations; exact wrapper for `std::meta::is_copy_assignment`.
inline constexpr Predicate isCopyAssignment{std::meta::is_copy_assignment};

/// Matches move-assignment declarations; exact wrapper for `std::meta::is_move_assignment`.
inline constexpr Predicate isMoveAssignment{std::meta::is_move_assignment};

/// Matches destructor declarations; exact wrapper for `std::meta::is_destructor`.
inline constexpr Predicate isDestructor{std::meta::is_destructor};

/// Matches special-member-function declarations; exact wrapper for `std::meta::is_special_member_function`.
inline constexpr Predicate isSpecialMemberFunction{std::meta::is_special_member_function};

/// Matches user-provided special members/functions; exact wrapper for `std::meta::is_user_provided`.
inline constexpr Predicate isUserProvided{std::meta::is_user_provided};

/// Matches user-declared special members/functions; exact wrapper for `std::meta::is_user_declared`.
inline constexpr Predicate isUserDeclared{std::meta::is_user_declared};

/// Matches reflections representing values; exact wrapper for `std::meta::is_value`.
inline constexpr Predicate isValue{std::meta::is_value};

/// Matches reflections representing objects; exact wrapper for `std::meta::is_object`.
inline constexpr Predicate isObject{std::meta::is_object};

/// Matches entities carrying the final property; exact wrapper for `std::meta::is_final`.
inline constexpr Predicate isFinal{std::meta::is_final};

/// Matches variadic C-style function reflections; exact wrapper for `std::meta::is_vararg_function`.
inline constexpr Predicate isVarargFunction{std::meta::is_vararg_function};

/// Matches function parameters with default arguments; exact wrapper for `std::meta::has_default_argument`.
inline constexpr Predicate hasDefaultArgument{std::meta::has_default_argument};

/// Matches explicit-object parameter reflections; exact wrapper for
/// `std::meta::is_explicit_object_parameter`.
inline constexpr Predicate isExplicitObjectParameter{std::meta::is_explicit_object_parameter};

/// Matches function-parameter reflections; exact wrapper for `std::meta::is_function_parameter`.
inline constexpr Predicate isFunctionParameter{std::meta::is_function_parameter};

/// Matches entities whose access is explicitly specified; exact wrapper for `std::meta::is_access_specified`.
inline constexpr Predicate isAccessSpecified{std::meta::is_access_specified};

/// Matches reflections denoting C++ types.
inline constexpr Predicate isType{[](Info info) consteval -> bool { return std::meta::is_type(info); }};

/// Matches complete reflected types. Non-type reflections are simply false.
inline constexpr Predicate isComplete{[](Info info) consteval -> bool {
  return std::meta::is_type(info) and std::meta::is_complete_type(info);
}};

/// Matches types whose member/enumerator set is available for enumeration.
inline constexpr Predicate isEnumerable{[](Info info) consteval -> bool {
  return std::meta::is_type(info) and std::meta::is_enumerable_type(info);
}};

/// Matches reflections denoting class types.
inline constexpr Predicate isClass{
    [](Info info) consteval -> bool { return std::meta::is_type(info) and std::meta::is_class_type(info); }};

/// Matches reflections denoting union types.
inline constexpr Predicate isUnion{
    [](Info info) consteval -> bool { return std::meta::is_type(info) and std::meta::is_union_type(info); }};

/// Matches reflections denoting enum types.
inline constexpr Predicate isEnum{
    [](Info info) consteval -> bool { return std::meta::is_type(info) and std::meta::is_enum_type(info); }};

/// Matches function type reflections; exact wrapper for `std::meta::is_function_type`.
inline constexpr Predicate isFunctionType{std::meta::is_function_type};

/// Matches complete type reflections; exact wrapper for `std::meta::is_complete_type`.
inline constexpr Predicate isCompleteType{std::meta::is_complete_type};

/// Matches enumerable type reflections; exact wrapper for `std::meta::is_enumerable_type`.
inline constexpr Predicate isEnumerableType{std::meta::is_enumerable_type};

/// Matches structural type reflections; exact wrapper for `std::meta::is_structural_type`.
inline constexpr Predicate isStructuralType{std::meta::is_structural_type};

/// Matches void type reflections; exact wrapper for `std::meta::is_void_type`.
inline constexpr Predicate isVoidType{std::meta::is_void_type};

/// Matches null-pointer type reflections; exact wrapper for `std::meta::is_null_pointer_type`.
inline constexpr Predicate isNullPointerType{std::meta::is_null_pointer_type};

/// Matches integral type reflections; exact wrapper for `std::meta::is_integral_type`.
inline constexpr Predicate isIntegralType{std::meta::is_integral_type};

/// Matches floating-point type reflections; exact wrapper for `std::meta::is_floating_point_type`.
inline constexpr Predicate isFloatingPointType{std::meta::is_floating_point_type};

/// Matches array type reflections; exact wrapper for `std::meta::is_array_type`.
inline constexpr Predicate isArrayType{std::meta::is_array_type};

/// Matches pointer type reflections; exact wrapper for `std::meta::is_pointer_type`.
inline constexpr Predicate isPointerType{std::meta::is_pointer_type};

/// Matches lvalue-reference type reflections; exact wrapper for `std::meta::is_lvalue_reference_type`.
inline constexpr Predicate isLvalueReferenceType{std::meta::is_lvalue_reference_type};

/// Matches rvalue-reference type reflections; exact wrapper for `std::meta::is_rvalue_reference_type`.
inline constexpr Predicate isRvalueReferenceType{std::meta::is_rvalue_reference_type};

/// Matches member-object-pointer type reflections; exact wrapper for
/// `std::meta::is_member_object_pointer_type`.
inline constexpr Predicate isMemberObjectPointerType{std::meta::is_member_object_pointer_type};

/// Matches member-function-pointer type reflections; exact wrapper for
/// `std::meta::is_member_function_pointer_type`.
inline constexpr Predicate isMemberFunctionPointerType{std::meta::is_member_function_pointer_type};

/// Matches enum type reflections; exact wrapper for `std::meta::is_enum_type`.
inline constexpr Predicate isEnumType{std::meta::is_enum_type};

/// Matches union type reflections; exact wrapper for `std::meta::is_union_type`.
inline constexpr Predicate isUnionType{std::meta::is_union_type};

/// Matches class type reflections; exact wrapper for `std::meta::is_class_type`.
inline constexpr Predicate isClassType{std::meta::is_class_type};

/// Matches reflection type reflections; exact wrapper for `std::meta::is_reflection_type`.
inline constexpr Predicate isReflectionType{std::meta::is_reflection_type};

/// Matches reference type reflections; exact wrapper for `std::meta::is_reference_type`.
inline constexpr Predicate isReferenceType{std::meta::is_reference_type};

/// Matches arithmetic type reflections; exact wrapper for `std::meta::is_arithmetic_type`.
inline constexpr Predicate isArithmeticType{std::meta::is_arithmetic_type};

/// Matches fundamental type reflections; exact wrapper for `std::meta::is_fundamental_type`.
inline constexpr Predicate isFundamentalType{std::meta::is_fundamental_type};

/// Matches object type reflections; exact wrapper for `std::meta::is_object_type`.
inline constexpr Predicate isObjectType{std::meta::is_object_type};

/// Matches scalar type reflections; exact wrapper for `std::meta::is_scalar_type`.
inline constexpr Predicate isScalarType{std::meta::is_scalar_type};

/// Matches compound type reflections; exact wrapper for `std::meta::is_compound_type`.
inline constexpr Predicate isCompoundType{std::meta::is_compound_type};

/// Matches member-pointer type reflections; exact wrapper for `std::meta::is_member_pointer_type`.
inline constexpr Predicate isMemberPointerType{std::meta::is_member_pointer_type};

/// Matches const-qualified type reflections; exact wrapper for `std::meta::is_const_type`.
inline constexpr Predicate isConstType{std::meta::is_const_type};

/// Matches volatile-qualified type reflections; exact wrapper for `std::meta::is_volatile_type`.
inline constexpr Predicate isVolatileType{std::meta::is_volatile_type};

/// Matches trivial type reflections using the reference-toolchain extension `std::meta::is_trivial_type`.
///
/// This is intentionally outside the canonical cppreference C++26 parity matrix but retained as a useful
/// extension while the reference implementation exposes it.
inline constexpr Predicate isTrivialType{std::meta::is_trivial_type};

/// Matches trivially-copyable type reflections; exact wrapper for `std::meta::is_trivially_copyable_type`.
inline constexpr Predicate isTriviallyCopyableType{std::meta::is_trivially_copyable_type};

/// Matches standard-layout type reflections; exact wrapper for `std::meta::is_standard_layout_type`.
inline constexpr Predicate isStandardLayoutType{std::meta::is_standard_layout_type};

/// Matches empty class type reflections; exact wrapper for `std::meta::is_empty_type`.
inline constexpr Predicate isEmptyType{std::meta::is_empty_type};

/// Matches polymorphic type reflections; exact wrapper for `std::meta::is_polymorphic_type`.
inline constexpr Predicate isPolymorphicType{std::meta::is_polymorphic_type};

/// Matches abstract type reflections; exact wrapper for `std::meta::is_abstract_type`.
inline constexpr Predicate isAbstractType{std::meta::is_abstract_type};

/// Matches final class type reflections; exact wrapper for `std::meta::is_final_type`.
inline constexpr Predicate isFinalType{std::meta::is_final_type};

/// Matches aggregate type reflections; exact wrapper for `std::meta::is_aggregate_type`.
inline constexpr Predicate isAggregateType{std::meta::is_aggregate_type};

/// Matches implicit-lifetime type reflections; exact wrapper for `std::meta::is_implicit_lifetime_type`.
inline constexpr Predicate isImplicitLifetimeType{std::meta::is_implicit_lifetime_type};

/// Matches signed arithmetic type reflections; exact wrapper for `std::meta::is_signed_type`.
inline constexpr Predicate isSignedType{std::meta::is_signed_type};

/// Matches unsigned arithmetic type reflections; exact wrapper for `std::meta::is_unsigned_type`.
inline constexpr Predicate isUnsignedType{std::meta::is_unsigned_type};

/// Matches bounded array type reflections; exact wrapper for `std::meta::is_bounded_array_type`.
inline constexpr Predicate isBoundedArrayType{std::meta::is_bounded_array_type};

/// Matches unbounded array type reflections; exact wrapper for `std::meta::is_unbounded_array_type`.
inline constexpr Predicate isUnboundedArrayType{std::meta::is_unbounded_array_type};

/// Matches scoped enum type reflections; exact wrapper for `std::meta::is_scoped_enum_type`.
inline constexpr Predicate isScopedEnumType{std::meta::is_scoped_enum_type};

/// Matches default-constructible type reflections; exact wrapper for
/// `std::meta::is_default_constructible_type`.
inline constexpr Predicate isDefaultConstructibleType{std::meta::is_default_constructible_type};

/// Matches copy-constructible type reflections; exact wrapper for `std::meta::is_copy_constructible_type`.
inline constexpr Predicate isCopyConstructibleType{std::meta::is_copy_constructible_type};

/// Matches move-constructible type reflections; exact wrapper for `std::meta::is_move_constructible_type`.
inline constexpr Predicate isMoveConstructibleType{std::meta::is_move_constructible_type};

/// Matches copy-assignable type reflections; exact wrapper for `std::meta::is_copy_assignable_type`.
inline constexpr Predicate isCopyAssignableType{std::meta::is_copy_assignable_type};

/// Matches move-assignable type reflections; exact wrapper for `std::meta::is_move_assignable_type`.
inline constexpr Predicate isMoveAssignableType{std::meta::is_move_assignable_type};

/// Matches swappable type reflections; exact wrapper for `std::meta::is_swappable_type`.
inline constexpr Predicate isSwappableType{std::meta::is_swappable_type};

/// Matches destructible type reflections; exact wrapper for `std::meta::is_destructible_type`.
inline constexpr Predicate isDestructibleType{std::meta::is_destructible_type};

/// Matches trivially default-constructible type reflections; exact wrapper for
/// `std::meta::is_trivially_default_constructible_type`.
inline constexpr Predicate isTriviallyDefaultConstructibleType{
    std::meta::is_trivially_default_constructible_type};

/// Matches trivially copy-constructible type reflections; exact wrapper for
/// `std::meta::is_trivially_copy_constructible_type`.
inline constexpr Predicate isTriviallyCopyConstructibleType{std::meta::is_trivially_copy_constructible_type};

/// Matches trivially move-constructible type reflections; exact wrapper for
/// `std::meta::is_trivially_move_constructible_type`.
inline constexpr Predicate isTriviallyMoveConstructibleType{std::meta::is_trivially_move_constructible_type};

/// Matches trivially copy-assignable type reflections; exact wrapper for
/// `std::meta::is_trivially_copy_assignable_type`.
inline constexpr Predicate isTriviallyCopyAssignableType{std::meta::is_trivially_copy_assignable_type};

/// Matches trivially move-assignable type reflections; exact wrapper for
/// `std::meta::is_trivially_move_assignable_type`.
inline constexpr Predicate isTriviallyMoveAssignableType{std::meta::is_trivially_move_assignable_type};

/// Matches trivially destructible type reflections; exact wrapper for
/// `std::meta::is_trivially_destructible_type`.
inline constexpr Predicate isTriviallyDestructibleType{std::meta::is_trivially_destructible_type};

/// Matches nothrow default-constructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_default_constructible_type`.
inline constexpr Predicate isNothrowDefaultConstructibleType{
    std::meta::is_nothrow_default_constructible_type};

/// Matches nothrow copy-constructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_copy_constructible_type`.
inline constexpr Predicate isNothrowCopyConstructibleType{std::meta::is_nothrow_copy_constructible_type};

/// Matches nothrow move-constructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_move_constructible_type`.
inline constexpr Predicate isNothrowMoveConstructibleType{std::meta::is_nothrow_move_constructible_type};

/// Matches nothrow copy-assignable type reflections; exact wrapper for
/// `std::meta::is_nothrow_copy_assignable_type`.
inline constexpr Predicate isNothrowCopyAssignableType{std::meta::is_nothrow_copy_assignable_type};

/// Matches nothrow move-assignable type reflections; exact wrapper for
/// `std::meta::is_nothrow_move_assignable_type`.
inline constexpr Predicate isNothrowMoveAssignableType{std::meta::is_nothrow_move_assignable_type};

/// Matches nothrow swappable type reflections; exact wrapper for `std::meta::is_nothrow_swappable_type`.
inline constexpr Predicate isNothrowSwappableType{std::meta::is_nothrow_swappable_type};

/// Matches nothrow destructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_destructible_type`.
inline constexpr Predicate isNothrowDestructibleType{std::meta::is_nothrow_destructible_type};

/// Matches types with virtual destructors; exact wrapper for `std::meta::has_virtual_destructor`.
inline constexpr Predicate hasVirtualDestructor{std::meta::has_virtual_destructor};

/// Matches types with unique object representations; exact wrapper for
/// `std::meta::has_unique_object_representations`.
inline constexpr Predicate hasUniqueObjectRepresentations{std::meta::has_unique_object_representations};

} // namespace Miracle::meta
