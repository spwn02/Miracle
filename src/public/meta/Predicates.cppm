export module Miracle.Meta:Predicates;

import std;
import :Types;

namespace Miracle::meta {

/// Structural marker used only to constrain the predicate-composition operators.
///
/// A marker avoids inheritance or a public predicate base class: any data-oriented callable carrying this
/// member can participate in `!`, `&&`, and `||` composition without runtime polymorphism.
template <class T>
concept MetaPredicate = requires { std::remove_cvref_t<T>::miracleMetaPredicate_; } and
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
  requires MetaPredicate<PredicateType>
[[nodiscard]] consteval auto operator!(PredicateType predicate) {
  return Predicate{[predicate](Info info) consteval -> bool { return not predicate(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical AND.
template <class Left, class Right>
  requires(MetaPredicate<Left> and MetaPredicate<Right>)
[[nodiscard]] consteval auto operator&&(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) and right(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical OR.
template <class Left, class Right>
  requires(MetaPredicate<Left> and MetaPredicate<Right>)
[[nodiscard]] consteval auto operator||(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) or right(info); }};
}

/// Matches reflections carrying a declared identifier.
///
/// Use this (or the optional `meta::name` projection) for ordinary Boolean branching; `meta::requireName` is
/// the strict semantic operation for callers that require an identifier and want a diagnostic failure; exact
/// wrapper for `std::meta::has_identifier`.
inline constexpr Predicate hasIdentifier{[](Info info) consteval -> bool { return std::meta::has_identifier(info); }};

/// Matches reflections with an associated semantic/lexical parent; exact wrapper for
/// `std::meta::has_parent`.
inline constexpr Predicate hasParent{[](Info info) consteval -> bool { return std::meta::has_parent(info); }};

/// Matches reflections carrying a recoverable template-argument sequence; exact wrapper for
/// `std::meta::has_template_arguments`.
inline constexpr Predicate hasTemplateArguments{[](Info info) consteval -> bool { return std::meta::has_template_arguments(info); }};

/// Matches function declaration reflections, excluding function templates; exact wrapper for
/// `std::meta::is_function`.
inline constexpr Predicate isFunction{[](Info info) consteval -> bool { return std::meta::is_function(info); }};

/// Matches function-template reflections; exact wrapper for `std::meta::is_function_template`.
inline constexpr Predicate isFunctionTemplate{[](Info info) consteval -> bool { return std::meta::is_function_template(info); }};

/// Matches non-static data members and static member variables.
inline constexpr Predicate isField{[](Info info) consteval -> bool {
  return std::meta::is_nonstatic_data_member(info) or
         (std::meta::is_variable(info) and std::meta::is_static_member(info));
}};

/// Matches static class members; exact wrapper for `std::meta::is_static_member`.
inline constexpr Predicate isStatic{[](Info info) consteval -> bool { return std::meta::is_static_member(info); }};

/// Matches non-static data members only; exact wrapper for `std::meta::is_nonstatic_data_member`.
inline constexpr Predicate isInstanceData{[](Info info) consteval -> bool { return std::meta::is_nonstatic_data_member(info); }};

/// Matches constructor declaration reflections; exact wrapper for `std::meta::is_constructor`.
inline constexpr Predicate isConstructor{[](Info info) consteval -> bool { return std::meta::is_constructor(info); }};

/// Matches namespace reflections; exact wrapper for `std::meta::is_namespace`.
inline constexpr Predicate isNamespace{[](Info info) consteval -> bool { return std::meta::is_namespace(info); }};

/// Matches base-specifier reflections returned by `bases`; exact wrapper for `std::meta::is_base`.
inline constexpr Predicate isBase{[](Info info) consteval -> bool { return std::meta::is_base(info); }};

/// Matches enumerator reflections; exact wrapper for `std::meta::is_enumerator`.
inline constexpr Predicate isEnumerator{[](Info info) consteval -> bool { return std::meta::is_enumerator(info); }};

/// Matches variable declaration reflections; exact wrapper for `std::meta::is_variable`.
inline constexpr Predicate isVariable{[](Info info) consteval -> bool { return std::meta::is_variable(info); }};

/// Matches template reflections supported by the standard reflection predicate; exact wrapper for
/// `std::meta::is_template`.
inline constexpr Predicate isTemplate{[](Info info) consteval -> bool { return std::meta::is_template(info); }};

/// Matches type-alias reflections without automatically dealiasing them; exact wrapper for
/// `std::meta::is_type_alias`.
inline constexpr Predicate isTypeAlias{[](Info info) consteval -> bool { return std::meta::is_type_alias(info); }};

/// Matches namespace-alias declarations without automatically dealiasing them; exact wrapper for
/// `std::meta::is_namespace_alias`.
inline constexpr Predicate isNamespaceAlias{[](Info info) consteval -> bool { return std::meta::is_namespace_alias(info); }};

/// Matches concept reflections; exact wrapper for `std::meta::is_concept`.
inline constexpr Predicate isConcept{[](Info info) consteval -> bool { return std::meta::is_concept(info); }};

/// Matches reflections whose declared access is public; exact wrapper for `std::meta::is_public`.
inline constexpr Predicate isPublic{[](Info info) consteval -> bool { return std::meta::is_public(info); }};

/// Matches reflections whose declared access is protected; exact wrapper for `std::meta::is_protected`.
inline constexpr Predicate isProtected{[](Info info) consteval -> bool { return std::meta::is_protected(info); }};

/// Matches reflections whose declared access is private; exact wrapper for `std::meta::is_private`.
inline constexpr Predicate isPrivate{[](Info info) consteval -> bool { return std::meta::is_private(info); }};

/// Matches entities declared virtual; exact wrapper for `std::meta::is_virtual`.
inline constexpr Predicate isVirtual{[](Info info) consteval -> bool { return std::meta::is_virtual(info); }};

/// Matches pure-virtual function entities; exact wrapper for `std::meta::is_pure_virtual`.
inline constexpr Predicate isPureVirtual{[](Info info) consteval -> bool { return std::meta::is_pure_virtual(info); }};

/// Matches entities declared with override semantics; exact wrapper for `std::meta::is_override`.
inline constexpr Predicate isOverride{[](Info info) consteval -> bool { return std::meta::is_override(info); }};

/// Matches deleted function entities; exact wrapper for `std::meta::is_deleted`.
inline constexpr Predicate isDeleted{[](Info info) consteval -> bool { return std::meta::is_deleted(info); }};

/// Matches defaulted function entities; exact wrapper for `std::meta::is_defaulted`.
inline constexpr Predicate isDefaulted{[](Info info) consteval -> bool { return std::meta::is_defaulted(info); }};

/// Matches explicit declarations; exact wrapper for `std::meta::is_explicit`.
inline constexpr Predicate isExplicit{[](Info info) consteval -> bool { return std::meta::is_explicit(info); }};

/// Matches noexcept function declarations/types supported by the standard predicate; exact wrapper for
/// `std::meta::is_noexcept`.
inline constexpr Predicate isNoexcept{[](Info info) consteval -> bool { return std::meta::is_noexcept(info); }};

/// Matches bit-field data members; exact wrapper for `std::meta::is_bit_field`.
inline constexpr Predicate isBitField{[](Info info) consteval -> bool { return std::meta::is_bit_field(info); }};

/// Matches const-qualified reflected declarations; exact wrapper for `std::meta::is_const`.
inline constexpr Predicate isConst{[](Info info) consteval -> bool { return std::meta::is_const(info); }};

/// Matches volatile-qualified reflected declarations; exact wrapper for `std::meta::is_volatile`.
inline constexpr Predicate isVolatile{[](Info info) consteval -> bool { return std::meta::is_volatile(info); }};

/// Matches mutable non-static data members; exact wrapper for `std::meta::is_mutable_member`.
inline constexpr Predicate isMutableMember{[](Info info) consteval -> bool { return std::meta::is_mutable_member(info); }};

/// Matches lvalue-reference-qualified member functions; exact wrapper for
/// `std::meta::is_lvalue_reference_qualified`.
inline constexpr Predicate isLvalueReferenceQualified{[](Info info) consteval -> bool { return std::meta::is_lvalue_reference_qualified(info); }};

/// Matches rvalue-reference-qualified member functions; exact wrapper for
/// `std::meta::is_rvalue_reference_qualified`.
inline constexpr Predicate isRvalueReferenceQualified{[](Info info) consteval -> bool { return std::meta::is_rvalue_reference_qualified(info); }};

/// Matches entities with static storage duration; exact wrapper for `std::meta::has_static_storage_duration`.
inline constexpr Predicate hasStaticStorageDuration{[](Info info) consteval -> bool { return std::meta::has_static_storage_duration(info); }};

/// Matches entities with thread storage duration; exact wrapper for `std::meta::has_thread_storage_duration`.
inline constexpr Predicate hasThreadStorageDuration{[](Info info) consteval -> bool { return std::meta::has_thread_storage_duration(info); }};

/// Matches entities with automatic storage duration; exact wrapper for
/// `std::meta::has_automatic_storage_duration`.
inline constexpr Predicate hasAutomaticStorageDuration{[](Info info) consteval -> bool { return std::meta::has_automatic_storage_duration(info); }};

/// Matches entities with internal linkage; exact wrapper for `std::meta::has_internal_linkage`.
inline constexpr Predicate hasInternalLinkage{[](Info info) consteval -> bool { return std::meta::has_internal_linkage(info); }};

/// Matches entities with module linkage; exact wrapper for `std::meta::has_module_linkage`.
inline constexpr Predicate hasModuleLinkage{[](Info info) consteval -> bool { return std::meta::has_module_linkage(info); }};

/// Matches entities with external linkage; exact wrapper for `std::meta::has_external_linkage`.
inline constexpr Predicate hasExternalLinkage{[](Info info) consteval -> bool { return std::meta::has_external_linkage(info); }};

/// Matches entities having any language linkage; exact wrapper for `std::meta::has_linkage`.
inline constexpr Predicate hasLinkage{[](Info info) consteval -> bool { return std::meta::has_linkage(info); }};

/// Matches entities with C language linkage; exact wrapper for `std::meta::has_c_language_linkage`.
inline constexpr Predicate hasCLanguageLinkage{[](Info info) consteval -> bool { return std::meta::has_c_language_linkage(info); }};

/// Matches class-member entities; exact wrapper for `std::meta::is_class_member`.
inline constexpr Predicate isClassMember{[](Info info) consteval -> bool { return std::meta::is_class_member(info); }};

/// Matches namespace-member entities; exact wrapper for `std::meta::is_namespace_member`.
inline constexpr Predicate isNamespaceMember{[](Info info) consteval -> bool { return std::meta::is_namespace_member(info); }};

/// Matches non-static data-member entities; exact wrapper for `std::meta::is_nonstatic_data_member`.
inline constexpr Predicate isNonstaticDataMember{[](Info info) consteval -> bool { return std::meta::is_nonstatic_data_member(info); }};

/// Matches static class-member entities; exact wrapper for `std::meta::is_static_member`.
inline constexpr Predicate isStaticMember{[](Info info) consteval -> bool { return std::meta::is_static_member(info); }};

/// Matches variable-template reflections; exact wrapper for `std::meta::is_variable_template`.
inline constexpr Predicate isVariableTemplate{[](Info info) consteval -> bool { return std::meta::is_variable_template(info); }};

/// Matches class-template reflections; exact wrapper for `std::meta::is_class_template`.
inline constexpr Predicate isClassTemplate{[](Info info) consteval -> bool { return std::meta::is_class_template(info); }};

/// Matches alias-template reflections; exact wrapper for `std::meta::is_alias_template`.
inline constexpr Predicate isAliasTemplate{[](Info info) consteval -> bool { return std::meta::is_alias_template(info); }};

/// Matches conversion-function-template reflections; exact wrapper for
/// `std::meta::is_conversion_function_template`.
inline constexpr Predicate isConversionFunctionTemplate{[](Info info) consteval -> bool { return std::meta::is_conversion_function_template(info); }};

/// Matches operator-function-template reflections; exact wrapper for
/// `std::meta::is_operator_function_template`.
inline constexpr Predicate isOperatorFunctionTemplate{[](Info info) consteval -> bool { return std::meta::is_operator_function_template(info); }};

/// Matches literal-operator-template reflections; exact wrapper for
/// `std::meta::is_literal_operator_template`.
inline constexpr Predicate isLiteralOperatorTemplate{[](Info info) consteval -> bool { return std::meta::is_literal_operator_template(info); }};

/// Matches constructor-template reflections; exact wrapper for `std::meta::is_constructor_template`.
inline constexpr Predicate isConstructorTemplate{[](Info info) consteval -> bool { return std::meta::is_constructor_template(info); }};

/// Matches structured-binding reflections; exact wrapper for `std::meta::is_structured_binding`.
inline constexpr Predicate isStructuredBinding{[](Info info) consteval -> bool { return std::meta::is_structured_binding(info); }};

/// Matches data members with default member initializers; exact wrapper for
/// `std::meta::has_default_member_initializer`.
inline constexpr Predicate hasDefaultMemberInitializer{[](Info info) consteval -> bool { return std::meta::has_default_member_initializer(info); }};

/// Matches conversion-function declarations; exact wrapper for `std::meta::is_conversion_function`.
inline constexpr Predicate isConversionFunction{[](Info info) consteval -> bool { return std::meta::is_conversion_function(info); }};

/// Matches operator-function declarations; exact wrapper for `std::meta::is_operator_function`.
inline constexpr Predicate isOperatorFunction{[](Info info) consteval -> bool { return std::meta::is_operator_function(info); }};

/// Matches literal-operator declarations; exact wrapper for `std::meta::is_literal_operator`.
inline constexpr Predicate isLiteralOperator{[](Info info) consteval -> bool { return std::meta::is_literal_operator(info); }};

/// Matches default-constructor declarations; exact wrapper for `std::meta::is_default_constructor`.
inline constexpr Predicate isDefaultConstructor{[](Info info) consteval -> bool { return std::meta::is_default_constructor(info); }};

/// Matches copy-constructor declarations; exact wrapper for `std::meta::is_copy_constructor`.
inline constexpr Predicate isCopyConstructor{[](Info info) consteval -> bool { return std::meta::is_copy_constructor(info); }};

/// Matches move-constructor declarations; exact wrapper for `std::meta::is_move_constructor`.
inline constexpr Predicate isMoveConstructor{[](Info info) consteval -> bool { return std::meta::is_move_constructor(info); }};

/// Matches assignment-operator declarations; exact wrapper for `std::meta::is_assignment`.
inline constexpr Predicate isAssignment{[](Info info) consteval -> bool { return std::meta::is_assignment(info); }};

/// Matches copy-assignment declarations; exact wrapper for `std::meta::is_copy_assignment`.
inline constexpr Predicate isCopyAssignment{[](Info info) consteval -> bool { return std::meta::is_copy_assignment(info); }};

/// Matches move-assignment declarations; exact wrapper for `std::meta::is_move_assignment`.
inline constexpr Predicate isMoveAssignment{[](Info info) consteval -> bool { return std::meta::is_move_assignment(info); }};

/// Matches destructor declarations; exact wrapper for `std::meta::is_destructor`.
inline constexpr Predicate isDestructor{[](Info info) consteval -> bool { return std::meta::is_destructor(info); }};

/// Matches special-member-function declarations; exact wrapper for `std::meta::is_special_member_function`.
inline constexpr Predicate isSpecialMemberFunction{[](Info info) consteval -> bool { return std::meta::is_special_member_function(info); }};

/// Matches user-provided special members/functions; exact wrapper for `std::meta::is_user_provided`.
inline constexpr Predicate isUserProvided{[](Info info) consteval -> bool { return std::meta::is_user_provided(info); }};

/// Matches user-declared special members/functions; exact wrapper for `std::meta::is_user_declared`.
inline constexpr Predicate isUserDeclared{[](Info info) consteval -> bool { return std::meta::is_user_declared(info); }};

/// Matches reflections representing values; exact wrapper for `std::meta::is_value`.
inline constexpr Predicate isValue{[](Info info) consteval -> bool { return std::meta::is_value(info); }};

/// Matches reflections representing objects; exact wrapper for `std::meta::is_object`.
inline constexpr Predicate isObject{[](Info info) consteval -> bool { return std::meta::is_object(info); }};

/// Matches entities carrying the final property; exact wrapper for `std::meta::is_final`.
inline constexpr Predicate isFinal{[](Info info) consteval -> bool { return std::meta::is_final(info); }};

/// Matches variadic C-style function reflections; exact wrapper for `std::meta::is_vararg_function`.
inline constexpr Predicate isVarargFunction{[](Info info) consteval -> bool { return std::meta::is_vararg_function(info); }};

/// Matches function parameters with default arguments; exact wrapper for `std::meta::has_default_argument`.
inline constexpr Predicate hasDefaultArgument{[](Info info) consteval -> bool { return std::meta::has_default_argument(info); }};

/// Matches explicit-object parameter reflections; exact wrapper for
/// `std::meta::is_explicit_object_parameter`.
inline constexpr Predicate isExplicitObjectParameter{[](Info info) consteval -> bool { return std::meta::is_explicit_object_parameter(info); }};

/// Matches function-parameter reflections; exact wrapper for `std::meta::is_function_parameter`.
inline constexpr Predicate isFunctionParameter{[](Info info) consteval -> bool { return std::meta::is_function_parameter(info); }};

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
inline constexpr Predicate isFunctionType{[](Info info) consteval -> bool { return std::meta::is_function_type(info); }};

/// Matches complete type reflections; exact wrapper for `std::meta::is_complete_type`.
inline constexpr Predicate isCompleteType{[](Info info) consteval -> bool { return std::meta::is_complete_type(info); }};

/// Matches enumerable type reflections; exact wrapper for `std::meta::is_enumerable_type`.
inline constexpr Predicate isEnumerableType{[](Info info) consteval -> bool { return std::meta::is_enumerable_type(info); }};

/// Matches structural type reflections; exact wrapper for `std::meta::is_structural_type`.
inline constexpr Predicate isStructuralType{[](Info info) consteval -> bool { return std::meta::is_structural_type(info); }};

/// Matches void type reflections; exact wrapper for `std::meta::is_void_type`.
inline constexpr Predicate isVoidType{[](Info info) consteval -> bool { return std::meta::is_void_type(info); }};

/// Matches null-pointer type reflections; exact wrapper for `std::meta::is_null_pointer_type`.
inline constexpr Predicate isNullPointerType{[](Info info) consteval -> bool { return std::meta::is_null_pointer_type(info); }};

/// Matches integral type reflections; exact wrapper for `std::meta::is_integral_type`.
inline constexpr Predicate isIntegralType{[](Info info) consteval -> bool { return std::meta::is_integral_type(info); }};

/// Matches floating-point type reflections; exact wrapper for `std::meta::is_floating_point_type`.
inline constexpr Predicate isFloatingPointType{[](Info info) consteval -> bool { return std::meta::is_floating_point_type(info); }};

/// Matches array type reflections; exact wrapper for `std::meta::is_array_type`.
inline constexpr Predicate isArrayType{[](Info info) consteval -> bool { return std::meta::is_array_type(info); }};

/// Matches pointer type reflections; exact wrapper for `std::meta::is_pointer_type`.
inline constexpr Predicate isPointerType{[](Info info) consteval -> bool { return std::meta::is_pointer_type(info); }};

/// Matches lvalue-reference type reflections; exact wrapper for `std::meta::is_lvalue_reference_type`.
inline constexpr Predicate isLvalueReferenceType{[](Info info) consteval -> bool { return std::meta::is_lvalue_reference_type(info); }};

/// Matches rvalue-reference type reflections; exact wrapper for `std::meta::is_rvalue_reference_type`.
inline constexpr Predicate isRvalueReferenceType{[](Info info) consteval -> bool { return std::meta::is_rvalue_reference_type(info); }};

/// Matches member-object-pointer type reflections; exact wrapper for
/// `std::meta::is_member_object_pointer_type`.
inline constexpr Predicate isMemberObjectPointerType{[](Info info) consteval -> bool { return std::meta::is_member_object_pointer_type(info); }};

/// Matches member-function-pointer type reflections; exact wrapper for
/// `std::meta::is_member_function_pointer_type`.
inline constexpr Predicate isMemberFunctionPointerType{[](Info info) consteval -> bool { return std::meta::is_member_function_pointer_type(info); }};

/// Matches enum type reflections; exact wrapper for `std::meta::is_enum_type`.
inline constexpr Predicate isEnumType{[](Info info) consteval -> bool { return std::meta::is_enum_type(info); }};

/// Matches union type reflections; exact wrapper for `std::meta::is_union_type`.
inline constexpr Predicate isUnionType{[](Info info) consteval -> bool { return std::meta::is_union_type(info); }};

/// Matches class type reflections; exact wrapper for `std::meta::is_class_type`.
inline constexpr Predicate isClassType{[](Info info) consteval -> bool { return std::meta::is_class_type(info); }};

/// Matches reflection type reflections; exact wrapper for `std::meta::is_reflection_type`.
inline constexpr Predicate isReflectionType{[](Info info) consteval -> bool { return std::meta::is_reflection_type(info); }};

/// Matches reference type reflections; exact wrapper for `std::meta::is_reference_type`.
inline constexpr Predicate isReferenceType{[](Info info) consteval -> bool { return std::meta::is_reference_type(info); }};

/// Matches arithmetic type reflections; exact wrapper for `std::meta::is_arithmetic_type`.
inline constexpr Predicate isArithmeticType{[](Info info) consteval -> bool { return std::meta::is_arithmetic_type(info); }};

/// Matches fundamental type reflections; exact wrapper for `std::meta::is_fundamental_type`.
inline constexpr Predicate isFundamentalType{[](Info info) consteval -> bool { return std::meta::is_fundamental_type(info); }};

/// Matches object type reflections; exact wrapper for `std::meta::is_object_type`.
inline constexpr Predicate isObjectType{[](Info info) consteval -> bool { return std::meta::is_object_type(info); }};

/// Matches scalar type reflections; exact wrapper for `std::meta::is_scalar_type`.
inline constexpr Predicate isScalarType{[](Info info) consteval -> bool { return std::meta::is_scalar_type(info); }};

/// Matches compound type reflections; exact wrapper for `std::meta::is_compound_type`.
inline constexpr Predicate isCompoundType{[](Info info) consteval -> bool { return std::meta::is_compound_type(info); }};

/// Matches member-pointer type reflections; exact wrapper for `std::meta::is_member_pointer_type`.
inline constexpr Predicate isMemberPointerType{[](Info info) consteval -> bool { return std::meta::is_member_pointer_type(info); }};

/// Matches const-qualified type reflections; exact wrapper for `std::meta::is_const_type`.
inline constexpr Predicate isConstType{[](Info info) consteval -> bool { return std::meta::is_const_type(info); }};

/// Matches volatile-qualified type reflections; exact wrapper for `std::meta::is_volatile_type`.
inline constexpr Predicate isVolatileType{[](Info info) consteval -> bool { return std::meta::is_volatile_type(info); }};

/// Matches trivially-copyable type reflections; exact wrapper for `std::meta::is_trivially_copyable_type`.
inline constexpr Predicate isTriviallyCopyableType{[](Info info) consteval -> bool { return std::meta::is_trivially_copyable_type(info); }};

/// Matches standard-layout type reflections; exact wrapper for `std::meta::is_standard_layout_type`.
inline constexpr Predicate isStandardLayoutType{[](Info info) consteval -> bool { return std::meta::is_standard_layout_type(info); }};

/// Matches empty class type reflections; exact wrapper for `std::meta::is_empty_type`.
inline constexpr Predicate isEmptyType{[](Info info) consteval -> bool { return std::meta::is_empty_type(info); }};

/// Matches polymorphic type reflections; exact wrapper for `std::meta::is_polymorphic_type`.
inline constexpr Predicate isPolymorphicType{[](Info info) consteval -> bool { return std::meta::is_polymorphic_type(info); }};

/// Matches abstract type reflections; exact wrapper for `std::meta::is_abstract_type`.
inline constexpr Predicate isAbstractType{[](Info info) consteval -> bool { return std::meta::is_abstract_type(info); }};

/// Matches final class type reflections; exact wrapper for `std::meta::is_final_type`.
inline constexpr Predicate isFinalType{[](Info info) consteval -> bool { return std::meta::is_final_type(info); }};

/// Matches aggregate type reflections; exact wrapper for `std::meta::is_aggregate_type`.
inline constexpr Predicate isAggregateType{[](Info info) consteval -> bool { return std::meta::is_aggregate_type(info); }};

/// Matches implicit-lifetime type reflections; exact wrapper for `std::meta::is_implicit_lifetime_type`.
inline constexpr Predicate isImplicitLifetimeType{[](Info info) consteval -> bool { return std::meta::is_implicit_lifetime_type(info); }};

/// Matches signed arithmetic type reflections; exact wrapper for `std::meta::is_signed_type`.
inline constexpr Predicate isSignedType{[](Info info) consteval -> bool { return std::meta::is_signed_type(info); }};

/// Matches unsigned arithmetic type reflections; exact wrapper for `std::meta::is_unsigned_type`.
inline constexpr Predicate isUnsignedType{[](Info info) consteval -> bool { return std::meta::is_unsigned_type(info); }};

/// Matches bounded array type reflections; exact wrapper for `std::meta::is_bounded_array_type`.
inline constexpr Predicate isBoundedArrayType{[](Info info) consteval -> bool { return std::meta::is_bounded_array_type(info); }};

/// Matches unbounded array type reflections; exact wrapper for `std::meta::is_unbounded_array_type`.
inline constexpr Predicate isUnboundedArrayType{[](Info info) consteval -> bool { return std::meta::is_unbounded_array_type(info); }};

/// Matches scoped enum type reflections; exact wrapper for `std::meta::is_scoped_enum_type`.
inline constexpr Predicate isScopedEnumType{[](Info info) consteval -> bool { return std::meta::is_scoped_enum_type(info); }};

/// Matches default-constructible type reflections; exact wrapper for
/// `std::meta::is_default_constructible_type`.
inline constexpr Predicate isDefaultConstructibleType{[](Info info) consteval -> bool { return std::meta::is_default_constructible_type(info); }};

/// Matches copy-constructible type reflections; exact wrapper for `std::meta::is_copy_constructible_type`.
inline constexpr Predicate isCopyConstructibleType{[](Info info) consteval -> bool { return std::meta::is_copy_constructible_type(info); }};

/// Matches move-constructible type reflections; exact wrapper for `std::meta::is_move_constructible_type`.
inline constexpr Predicate isMoveConstructibleType{[](Info info) consteval -> bool { return std::meta::is_move_constructible_type(info); }};

/// Matches copy-assignable type reflections; exact wrapper for `std::meta::is_copy_assignable_type`.
inline constexpr Predicate isCopyAssignableType{[](Info info) consteval -> bool { return std::meta::is_copy_assignable_type(info); }};

/// Matches move-assignable type reflections; exact wrapper for `std::meta::is_move_assignable_type`.
inline constexpr Predicate isMoveAssignableType{[](Info info) consteval -> bool { return std::meta::is_move_assignable_type(info); }};

/// Matches swappable type reflections; exact wrapper for `std::meta::is_swappable_type`.
inline constexpr Predicate isSwappableType{[](Info info) consteval -> bool { return std::meta::is_swappable_type(info); }};

/// Matches destructible type reflections; exact wrapper for `std::meta::is_destructible_type`.
inline constexpr Predicate isDestructibleType{[](Info info) consteval -> bool { return std::meta::is_destructible_type(info); }};

/// Matches trivially default-constructible type reflections; exact wrapper for
/// `std::meta::is_trivially_default_constructible_type`.
inline constexpr Predicate isTriviallyDefaultConstructibleType{[](Info info) consteval -> bool { return std::meta::is_trivially_default_constructible_type(info); }};

/// Matches trivially copy-constructible type reflections; exact wrapper for
/// `std::meta::is_trivially_copy_constructible_type`.
inline constexpr Predicate isTriviallyCopyConstructibleType{[](Info info) consteval -> bool { return std::meta::is_trivially_copy_constructible_type(info); }};

/// Matches trivially move-constructible type reflections; exact wrapper for
/// `std::meta::is_trivially_move_constructible_type`.
inline constexpr Predicate isTriviallyMoveConstructibleType{[](Info info) consteval -> bool { return std::meta::is_trivially_move_constructible_type(info); }};

/// Matches trivially copy-assignable type reflections; exact wrapper for
/// `std::meta::is_trivially_copy_assignable_type`.
inline constexpr Predicate isTriviallyCopyAssignableType{[](Info info) consteval -> bool { return std::meta::is_trivially_copy_assignable_type(info); }};

/// Matches trivially move-assignable type reflections; exact wrapper for
/// `std::meta::is_trivially_move_assignable_type`.
inline constexpr Predicate isTriviallyMoveAssignableType{[](Info info) consteval -> bool { return std::meta::is_trivially_move_assignable_type(info); }};

/// Matches trivially destructible type reflections; exact wrapper for
/// `std::meta::is_trivially_destructible_type`.
inline constexpr Predicate isTriviallyDestructibleType{[](Info info) consteval -> bool { return std::meta::is_trivially_destructible_type(info); }};

/// Matches nothrow default-constructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_default_constructible_type`.
inline constexpr Predicate isNothrowDefaultConstructibleType{[](Info info) consteval -> bool { return std::meta::is_nothrow_default_constructible_type(info); }};

/// Matches nothrow copy-constructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_copy_constructible_type`.
inline constexpr Predicate isNothrowCopyConstructibleType{[](Info info) consteval -> bool { return std::meta::is_nothrow_copy_constructible_type(info); }};

/// Matches nothrow move-constructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_move_constructible_type`.
inline constexpr Predicate isNothrowMoveConstructibleType{[](Info info) consteval -> bool { return std::meta::is_nothrow_move_constructible_type(info); }};

/// Matches nothrow copy-assignable type reflections; exact wrapper for
/// `std::meta::is_nothrow_copy_assignable_type`.
inline constexpr Predicate isNothrowCopyAssignableType{[](Info info) consteval -> bool { return std::meta::is_nothrow_copy_assignable_type(info); }};

/// Matches nothrow move-assignable type reflections; exact wrapper for
/// `std::meta::is_nothrow_move_assignable_type`.
inline constexpr Predicate isNothrowMoveAssignableType{[](Info info) consteval -> bool { return std::meta::is_nothrow_move_assignable_type(info); }};

/// Matches nothrow swappable type reflections; exact wrapper for `std::meta::is_nothrow_swappable_type`.
inline constexpr Predicate isNothrowSwappableType{[](Info info) consteval -> bool { return std::meta::is_nothrow_swappable_type(info); }};

/// Matches nothrow destructible type reflections; exact wrapper for
/// `std::meta::is_nothrow_destructible_type`.
inline constexpr Predicate isNothrowDestructibleType{[](Info info) consteval -> bool { return std::meta::is_nothrow_destructible_type(info); }};

/// Matches types with virtual destructors; exact wrapper for `std::meta::has_virtual_destructor`.
inline constexpr Predicate hasVirtualDestructor{[](Info info) consteval -> bool { return std::meta::has_virtual_destructor(info); }};

/// Matches types with unique object representations; exact wrapper for
/// `std::meta::has_unique_object_representations`.
inline constexpr Predicate hasUniqueObjectRepresentations{[](Info info) consteval -> bool { return std::meta::has_unique_object_representations(info); }};

} // namespace Miracle::meta
