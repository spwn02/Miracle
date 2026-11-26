import std;
import Miracle.Meta;

namespace Tests::metaStdParity {

struct Empty final {};
struct Aggregate final {
  int value;
};
union Union {
  int integer;
  double floating;
};
enum class Color : std::uint8_t { red, green };

struct Base {
  Base() = default;
  Base(const Base &) = default;
  Base(Base &&) = default;
  auto operator=(const Base &) -> Base & = default;
  auto operator=(Base &&) -> Base & = default;
  virtual auto pure() -> void = 0;
  virtual ~Base() = default;
};

struct Sample final : Base {
  int field{};
  int withDefault{};
  int bits : 3 = 0;
  mutable int mutableField{};
  static inline int staticField{};

  Sample() = default;
  explicit Sample(int value) {
    static_cast<void>(value);
  }
  Sample(const Sample &) = default;
  Sample(Sample &&) = default;
  auto operator=(const Sample &) -> Sample & = default;
  auto operator=(Sample &&) -> Sample & = default;
  ~Sample() override = default;

  auto pure() -> void override {
  }
  auto noThrow() noexcept -> void {
  }
  auto lvalueOnly() & -> void {
  }
  auto rvalueOnly() && -> void {
  }
  explicit operator int() const {
    return 0;
  }
  auto operator+(int value) const -> int {
    return value;
  }
};

template <class T>
struct Box {};

template <class T>
using Alias = T;

template <class T>
inline constexpr int variableTemplate{};

template <class T>
concept AnyType = true;

// Mutable globals are intentional reflection subjects for storage/linkage predicate parity.
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline int variable{};
inline const int constVariable{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline volatile int volatileVariable{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
[[maybe_unused]] static int internalVariable{};
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
inline thread_local int threadVariable{};
extern "C" auto cFunction() -> void;
auto function(int value = 1) -> long;
auto varargFunction(int value = 1, ...) -> void;

// Raw arrays are intentional type-reflection subjects; std::array would test a different category.
using Array = int[3];         // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
using UnboundedArray = int[]; // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
using MemberObjectPointer = int Sample::*;
using MemberFunctionPointer = void (Sample::*)();

/// Returns the first member of `Sample` accepted by `predicate`, or an empty reflection when none matches.
template <class Predicate>
consteval auto firstSampleMember(Predicate predicate) -> std::meta::info {
  for (const auto member : std::meta::members_of(^^Sample, Miracle::Access::unchecked())) {
    if (predicate(member)) {
      return member;
    }
  }
  return {};
}

/// Returns the first parameter reflection of `function` for parameter-predicate parity checks.
consteval auto firstParameter() -> std::meta::info {
  const auto values = std::meta::parameters_of(^^function);
  return values.front();
}

/// Compares one Miracle predicate with the corresponding standard unary predicate on the same reflection.
template <auto Standard, class Wrapper>
consteval auto unaryPredicateParity(const Wrapper &wrapper, std::meta::info info) -> bool {
  return wrapper(info) == Standard(info);
}

/// Compares two finite reflection sequences by cardinality, source order, and reflection identity.
template <std::ranges::input_range Left, std::ranges::input_range Right>
consteval auto sequenceParity(const Left &left, const Right &right) -> bool {
  return std::ranges::equal(left, right);
}

inline constexpr auto fieldInfo = ^^Sample::field;
inline constexpr auto defaultFieldInfo = ^^Sample::withDefault;
inline constexpr auto bitFieldInfo = ^^Sample::bits;
inline constexpr auto mutableFieldInfo = ^^Sample::mutableField;
inline constexpr auto staticFieldInfo = ^^Sample::staticField;
inline constexpr auto methodInfo = ^^Sample::pure;
inline constexpr auto basePureInfo = ^^Base::pure;
inline constexpr auto noexceptMethodInfo = ^^Sample::noThrow;
inline constexpr auto lvalueMethodInfo = ^^Sample::lvalueOnly;
inline constexpr auto rvalueMethodInfo = ^^Sample::rvalueOnly;
inline constexpr auto conversionInfo = ^^Sample::operator int;
inline constexpr auto operatorInfo = ^^Sample::operator+;
inline constexpr auto constructorInfo = firstSampleMember(std::meta::is_constructor);
inline constexpr auto assignmentInfo = firstSampleMember(std::meta::is_assignment);
inline constexpr auto destructorInfo = firstSampleMember(std::meta::is_destructor);
inline constexpr auto parameterInfo = firstParameter();
inline constexpr auto dataMemberSpecInfo = std::meta::data_member_spec(^^int,
    {
        .name = "generated"});
inline constexpr auto intType = ^^int;
inline constexpr auto voidType = ^^void;
inline constexpr auto doubleType = ^^double;
inline constexpr auto nullPtrType = ^^decltype(nullptr);
inline constexpr auto unsignedType = ^^unsigned;
inline constexpr auto constType = ^^const int;
inline constexpr auto volatileType = ^^volatile int;
inline constexpr auto pointerType = ^^int *;
inline constexpr auto lvalueRefType = ^^int &;
inline constexpr auto rvalueRefType = ^^int &&;
inline constexpr auto arrayType = ^^Array;
inline constexpr auto unboundedArrayType = ^^UnboundedArray;
inline constexpr auto memberObjectPointerType = ^^MemberObjectPointer;
inline constexpr auto memberFunctionPointerType = ^^MemberFunctionPointer;
inline constexpr auto reflectionType = ^^decltype(^^int);
inline constexpr auto emptyType = ^^Empty;
inline constexpr auto aggregateType = ^^Aggregate;
inline constexpr auto baseType = ^^Base;
inline constexpr auto sampleType = ^^Sample;
inline constexpr auto unionType = ^^Union;
inline constexpr auto enumType = ^^Color;
inline constexpr auto boxTemplate = ^^Box;
inline constexpr auto variableInfo = ^^variable;
inline constexpr auto constVariableInfo = ^^constVariable;
inline constexpr auto volatileVariableInfo = ^^volatileVariable;
inline constexpr auto internalVariableInfo = ^^internalVariable;
inline constexpr auto threadVariableInfo = ^^threadVariable;
inline constexpr auto cFunctionInfo = ^^cFunction;
inline constexpr auto functionInfo = ^^function;
inline constexpr auto varargFunctionInfo = ^^varargFunction;

static_assert(unaryPredicateParity<std::meta::is_virtual>(Miracle::meta::isVirtual, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_pure_virtual>(Miracle::meta::isPureVirtual, basePureInfo));
static_assert(unaryPredicateParity<std::meta::is_override>(Miracle::meta::isOverride, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_deleted>(Miracle::meta::isDeleted, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_defaulted>(Miracle::meta::isDefaulted, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_explicit>(Miracle::meta::isExplicit, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_noexcept>(Miracle::meta::isNoexcept, noexceptMethodInfo));
static_assert(unaryPredicateParity<std::meta::is_bit_field>(Miracle::meta::isBitField, bitFieldInfo));
static_assert(unaryPredicateParity<std::meta::is_const>(Miracle::meta::isConst, constVariableInfo));
static_assert(unaryPredicateParity<std::meta::is_volatile>(Miracle::meta::isVolatile, volatileVariableInfo));
static_assert(
    unaryPredicateParity<std::meta::is_mutable_member>(Miracle::meta::isMutableMember, mutableFieldInfo));
static_assert(
    unaryPredicateParity<std::meta::is_lvalue_reference_qualified>(Miracle::meta::isLvalueReferenceQualified,
        lvalueMethodInfo));
static_assert(
    unaryPredicateParity<std::meta::is_rvalue_reference_qualified>(Miracle::meta::isRvalueReferenceQualified,
        rvalueMethodInfo));
static_assert(
    unaryPredicateParity<std::meta::has_static_storage_duration>(Miracle::meta::hasStaticStorageDuration,
        variableInfo));
static_assert(
    unaryPredicateParity<std::meta::has_thread_storage_duration>(Miracle::meta::hasThreadStorageDuration,
        threadVariableInfo));
static_assert(unaryPredicateParity<std::meta::has_automatic_storage_duration>(
    Miracle::meta::hasAutomaticStorageDuration,
    variableInfo));
static_assert(unaryPredicateParity<std::meta::has_internal_linkage>(Miracle::meta::hasInternalLinkage,
    internalVariableInfo));
static_assert(
    unaryPredicateParity<std::meta::has_module_linkage>(Miracle::meta::hasModuleLinkage, variableInfo));
static_assert(
    unaryPredicateParity<std::meta::has_external_linkage>(Miracle::meta::hasExternalLinkage, variableInfo));
static_assert(unaryPredicateParity<std::meta::has_linkage>(Miracle::meta::hasLinkage, variableInfo));
static_assert(unaryPredicateParity<std::meta::has_c_language_linkage>(Miracle::meta::hasCLanguageLinkage,
    cFunctionInfo));
static_assert(unaryPredicateParity<std::meta::is_class_member>(Miracle::meta::isClassMember, fieldInfo));
static_assert(
    unaryPredicateParity<std::meta::is_namespace_member>(Miracle::meta::isNamespaceMember, variableInfo));
static_assert(unaryPredicateParity<std::meta::is_nonstatic_data_member>(Miracle::meta::isNonstaticDataMember,
    fieldInfo));
static_assert(
    unaryPredicateParity<std::meta::is_static_member>(Miracle::meta::isStaticMember, staticFieldInfo));
static_assert(unaryPredicateParity<std::meta::is_data_member_spec>(Miracle::meta::isDataMemberSpec,
    dataMemberSpecInfo));
static_assert(unaryPredicateParity<std::meta::is_complete_type>(Miracle::meta::isCompleteType, intType));
static_assert(
    unaryPredicateParity<std::meta::is_enumerable_type>(Miracle::meta::isEnumerableType, sampleType));
static_assert(
    unaryPredicateParity<std::meta::is_variable_template>(Miracle::meta::isVariableTemplate, boxTemplate));
static_assert(
    unaryPredicateParity<std::meta::is_class_template>(Miracle::meta::isClassTemplate, boxTemplate));
static_assert(
    unaryPredicateParity<std::meta::is_alias_template>(Miracle::meta::isAliasTemplate, boxTemplate));
static_assert(unaryPredicateParity<std::meta::is_conversion_function_template>(
    Miracle::meta::isConversionFunctionTemplate,
    boxTemplate));
static_assert(
    unaryPredicateParity<std::meta::is_operator_function_template>(Miracle::meta::isOperatorFunctionTemplate,
        boxTemplate));
static_assert(
    unaryPredicateParity<std::meta::is_literal_operator_template>(Miracle::meta::isLiteralOperatorTemplate,
        boxTemplate));
static_assert(unaryPredicateParity<std::meta::is_constructor_template>(Miracle::meta::isConstructorTemplate,
    boxTemplate));
static_assert(
    unaryPredicateParity<std::meta::is_structured_binding>(Miracle::meta::isStructuredBinding, variableInfo));
static_assert(unaryPredicateParity<std::meta::has_default_member_initializer>(
    Miracle::meta::hasDefaultMemberInitializer,
    defaultFieldInfo));
static_assert(unaryPredicateParity<std::meta::is_conversion_function>(Miracle::meta::isConversionFunction,
    conversionInfo));
static_assert(
    unaryPredicateParity<std::meta::is_operator_function>(Miracle::meta::isOperatorFunction, operatorInfo));
static_assert(
    unaryPredicateParity<std::meta::is_literal_operator>(Miracle::meta::isLiteralOperator, functionInfo));
static_assert(unaryPredicateParity<std::meta::is_default_constructor>(Miracle::meta::isDefaultConstructor,
    constructorInfo));
static_assert(
    unaryPredicateParity<std::meta::is_copy_constructor>(Miracle::meta::isCopyConstructor, constructorInfo));
static_assert(
    unaryPredicateParity<std::meta::is_move_constructor>(Miracle::meta::isMoveConstructor, constructorInfo));
static_assert(unaryPredicateParity<std::meta::is_assignment>(Miracle::meta::isAssignment, assignmentInfo));
static_assert(
    unaryPredicateParity<std::meta::is_copy_assignment>(Miracle::meta::isCopyAssignment, assignmentInfo));
static_assert(
    unaryPredicateParity<std::meta::is_move_assignment>(Miracle::meta::isMoveAssignment, assignmentInfo));
static_assert(unaryPredicateParity<std::meta::is_destructor>(Miracle::meta::isDestructor, destructorInfo));
static_assert(
    unaryPredicateParity<std::meta::is_special_member_function>(Miracle::meta::isSpecialMemberFunction,
        destructorInfo));
static_assert(unaryPredicateParity<std::meta::is_user_provided>(Miracle::meta::isUserProvided, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_user_declared>(Miracle::meta::isUserDeclared, methodInfo));
static_assert(unaryPredicateParity<std::meta::is_value>(Miracle::meta::isValue, variableInfo));
static_assert(unaryPredicateParity<std::meta::is_object>(Miracle::meta::isObject, variableInfo));
static_assert(unaryPredicateParity<std::meta::is_structural_type>(Miracle::meta::isStructuralType, intType));
static_assert(unaryPredicateParity<std::meta::is_final>(Miracle::meta::isFinal, sampleType));
static_assert(unaryPredicateParity<std::meta::is_void_type>(Miracle::meta::isVoidType, voidType));
static_assert(
    unaryPredicateParity<std::meta::is_null_pointer_type>(Miracle::meta::isNullPointerType, nullPtrType));
static_assert(unaryPredicateParity<std::meta::is_integral_type>(Miracle::meta::isIntegralType, intType));
static_assert(
    unaryPredicateParity<std::meta::is_floating_point_type>(Miracle::meta::isFloatingPointType, doubleType));
static_assert(unaryPredicateParity<std::meta::is_array_type>(Miracle::meta::isArrayType, arrayType));
static_assert(unaryPredicateParity<std::meta::is_pointer_type>(Miracle::meta::isPointerType, pointerType));
static_assert(unaryPredicateParity<std::meta::is_lvalue_reference_type>(Miracle::meta::isLvalueReferenceType,
    lvalueRefType));
static_assert(unaryPredicateParity<std::meta::is_rvalue_reference_type>(Miracle::meta::isRvalueReferenceType,
    rvalueRefType));
static_assert(
    unaryPredicateParity<std::meta::is_member_object_pointer_type>(Miracle::meta::isMemberObjectPointerType,
        memberObjectPointerType));
static_assert(unaryPredicateParity<std::meta::is_member_function_pointer_type>(
    Miracle::meta::isMemberFunctionPointerType,
    memberFunctionPointerType));
static_assert(unaryPredicateParity<std::meta::is_enum_type>(Miracle::meta::isEnumType, enumType));
static_assert(unaryPredicateParity<std::meta::is_union_type>(Miracle::meta::isUnionType, unionType));
static_assert(unaryPredicateParity<std::meta::is_class_type>(Miracle::meta::isClassType, sampleType));
static_assert(
    unaryPredicateParity<std::meta::is_reflection_type>(Miracle::meta::isReflectionType, reflectionType));
static_assert(
    unaryPredicateParity<std::meta::is_reference_type>(Miracle::meta::isReferenceType, lvalueRefType));
static_assert(unaryPredicateParity<std::meta::is_arithmetic_type>(Miracle::meta::isArithmeticType, intType));
static_assert(
    unaryPredicateParity<std::meta::is_fundamental_type>(Miracle::meta::isFundamentalType, intType));
static_assert(unaryPredicateParity<std::meta::is_object_type>(Miracle::meta::isObjectType, sampleType));
static_assert(unaryPredicateParity<std::meta::is_scalar_type>(Miracle::meta::isScalarType, intType));
static_assert(unaryPredicateParity<std::meta::is_compound_type>(Miracle::meta::isCompoundType, sampleType));
static_assert(unaryPredicateParity<std::meta::is_member_pointer_type>(Miracle::meta::isMemberPointerType,
    memberObjectPointerType));
static_assert(unaryPredicateParity<std::meta::is_const_type>(Miracle::meta::isConstType, constType));
static_assert(unaryPredicateParity<std::meta::is_volatile_type>(Miracle::meta::isVolatileType, volatileType));
static_assert(
    unaryPredicateParity<std::meta::is_trivially_copyable_type>(Miracle::meta::isTriviallyCopyableType,
        intType));
static_assert(
    unaryPredicateParity<std::meta::is_standard_layout_type>(Miracle::meta::isStandardLayoutType, intType));
static_assert(unaryPredicateParity<std::meta::is_empty_type>(Miracle::meta::isEmptyType, emptyType));
static_assert(
    unaryPredicateParity<std::meta::is_polymorphic_type>(Miracle::meta::isPolymorphicType, baseType));
static_assert(unaryPredicateParity<std::meta::is_abstract_type>(Miracle::meta::isAbstractType, baseType));
static_assert(unaryPredicateParity<std::meta::is_final_type>(Miracle::meta::isFinalType, sampleType));
static_assert(
    unaryPredicateParity<std::meta::is_aggregate_type>(Miracle::meta::isAggregateType, aggregateType));
static_assert(
    unaryPredicateParity<std::meta::is_implicit_lifetime_type>(Miracle::meta::isImplicitLifetimeType,
        intType));
static_assert(unaryPredicateParity<std::meta::is_signed_type>(Miracle::meta::isSignedType, intType));
static_assert(unaryPredicateParity<std::meta::is_unsigned_type>(Miracle::meta::isUnsignedType, unsignedType));
static_assert(
    unaryPredicateParity<std::meta::is_bounded_array_type>(Miracle::meta::isBoundedArrayType, arrayType));
static_assert(unaryPredicateParity<std::meta::is_unbounded_array_type>(Miracle::meta::isUnboundedArrayType,
    unboundedArrayType));
static_assert(
    unaryPredicateParity<std::meta::is_scoped_enum_type>(Miracle::meta::isScopedEnumType, enumType));
static_assert(
    unaryPredicateParity<std::meta::is_default_constructible_type>(Miracle::meta::isDefaultConstructibleType,
        intType));
static_assert(
    unaryPredicateParity<std::meta::is_copy_constructible_type>(Miracle::meta::isCopyConstructibleType,
        intType));
static_assert(
    unaryPredicateParity<std::meta::is_move_constructible_type>(Miracle::meta::isMoveConstructibleType,
        intType));
static_assert(
    unaryPredicateParity<std::meta::is_copy_assignable_type>(Miracle::meta::isCopyAssignableType, intType));
static_assert(
    unaryPredicateParity<std::meta::is_move_assignable_type>(Miracle::meta::isMoveAssignableType, intType));
static_assert(unaryPredicateParity<std::meta::is_swappable_type>(Miracle::meta::isSwappableType, intType));
static_assert(
    unaryPredicateParity<std::meta::is_destructible_type>(Miracle::meta::isDestructibleType, intType));
static_assert(unaryPredicateParity<std::meta::is_trivially_default_constructible_type>(
    Miracle::meta::isTriviallyDefaultConstructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_trivially_copy_constructible_type>(
    Miracle::meta::isTriviallyCopyConstructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_trivially_move_constructible_type>(
    Miracle::meta::isTriviallyMoveConstructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_trivially_copy_assignable_type>(
    Miracle::meta::isTriviallyCopyAssignableType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_trivially_move_assignable_type>(
    Miracle::meta::isTriviallyMoveAssignableType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_trivially_destructible_type>(
    Miracle::meta::isTriviallyDestructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_nothrow_default_constructible_type>(
    Miracle::meta::isNothrowDefaultConstructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_nothrow_copy_constructible_type>(
    Miracle::meta::isNothrowCopyConstructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_nothrow_move_constructible_type>(
    Miracle::meta::isNothrowMoveConstructibleType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_nothrow_copy_assignable_type>(
    Miracle::meta::isNothrowCopyAssignableType,
    intType));
static_assert(unaryPredicateParity<std::meta::is_nothrow_move_assignable_type>(
    Miracle::meta::isNothrowMoveAssignableType,
    intType));
static_assert(
    unaryPredicateParity<std::meta::is_nothrow_swappable_type>(Miracle::meta::isNothrowSwappableType,
        intType));
static_assert(
    unaryPredicateParity<std::meta::is_nothrow_destructible_type>(Miracle::meta::isNothrowDestructibleType,
        intType));
static_assert(
    unaryPredicateParity<std::meta::has_virtual_destructor>(Miracle::meta::hasVirtualDestructor, baseType));
static_assert(unaryPredicateParity<std::meta::has_unique_object_representations>(
    Miracle::meta::hasUniqueObjectRepresentations,
    intType));
static_assert(
    unaryPredicateParity<std::meta::is_vararg_function>(Miracle::meta::isVarargFunction, varargFunctionInfo));
static_assert(
    unaryPredicateParity<std::meta::has_default_argument>(Miracle::meta::hasDefaultArgument, parameterInfo));
static_assert(
    unaryPredicateParity<std::meta::is_explicit_object_parameter>(Miracle::meta::isExplicitObjectParameter,
        parameterInfo));
static_assert(unaryPredicateParity<std::meta::is_function_parameter>(Miracle::meta::isFunctionParameter,
    parameterInfo));

// Standard transformation/function parity: the wrappers preserve standard results on their valid domains.
static_assert(Miracle::meta::identifierOf(fieldInfo) == std::meta::identifier_of(fieldInfo));
static_assert(Miracle::meta::displayStringOf(fieldInfo) == std::meta::display_string_of(fieldInfo));
static_assert(Miracle::meta::u8IdentifierOf(fieldInfo) == std::meta::u8identifier_of(fieldInfo));
static_assert(Miracle::meta::u8DisplayStringOf(fieldInfo) == std::meta::u8display_string_of(fieldInfo));
static_assert(Miracle::meta::operatorOf(operatorInfo) == std::meta::operator_of(operatorInfo));
static_assert(Miracle::meta::symbolOf(std::meta::operators::op_plus) ==
              std::meta::symbol_of(std::meta::operators::op_plus));
static_assert(Miracle::meta::u8SymbolOf(std::meta::operators::op_plus) ==
              std::meta::u8symbol_of(std::meta::operators::op_plus));
static_assert(
    Miracle::meta::sourceLocationOf(fieldInfo).line() == std::meta::source_location_of(fieldInfo).line());
static_assert(Miracle::meta::typeOf(fieldInfo) == std::meta::type_of(fieldInfo));
static_assert(Miracle::meta::parentOf(fieldInfo) == std::meta::parent_of(fieldInfo));
static_assert(Miracle::meta::constantOf(^^Color::red) == std::meta::constant_of(^^Color::red));
static_assert(Miracle::meta::offsetOf(fieldInfo) == std::meta::offset_of(fieldInfo));
static_assert(Miracle::meta::sizeOf(intType) == std::meta::size_of(intType));
static_assert(Miracle::meta::alignmentOf(intType) == std::meta::alignment_of(intType));
static_assert(Miracle::meta::bitSizeOf(intType) == std::meta::bit_size_of(intType));
static_assert(Miracle::meta::rank(arrayType) == std::meta::rank(arrayType));
static_assert(Miracle::meta::extent(arrayType) == std::meta::extent(arrayType));
static_assert(Miracle::meta::removeConst(constType) == std::meta::remove_const(constType));
static_assert(Miracle::meta::removeVolatile(volatileType) == std::meta::remove_volatile(volatileType));
static_assert(Miracle::meta::removeCv(^^const volatile int) == std::meta::remove_cv(^^const volatile int));
static_assert(Miracle::meta::addConst(intType) == std::meta::add_const(intType));
static_assert(Miracle::meta::addVolatile(intType) == std::meta::add_volatile(intType));
static_assert(Miracle::meta::addCv(intType) == std::meta::add_cv(intType));
static_assert(Miracle::meta::removeReference(lvalueRefType) == std::meta::remove_reference(lvalueRefType));
static_assert(Miracle::meta::addLvalueReference(intType) == std::meta::add_lvalue_reference(intType));
static_assert(Miracle::meta::addRvalueReference(intType) == std::meta::add_rvalue_reference(intType));
static_assert(Miracle::meta::makeSigned(unsignedType) == std::meta::make_signed(unsignedType));
static_assert(Miracle::meta::makeUnsigned(intType) == std::meta::make_unsigned(intType));
static_assert(Miracle::meta::removeExtent(arrayType) == std::meta::remove_extent(arrayType));
static_assert(Miracle::meta::removeAllExtents(^^int[2][3]) == std::meta::remove_all_extents(^^int[2][3]));
static_assert(Miracle::meta::removePointer(pointerType) == std::meta::remove_pointer(pointerType));
static_assert(Miracle::meta::addPointer(intType) == std::meta::add_pointer(intType));
static_assert(Miracle::meta::removeCvref(^^const int &) == std::meta::remove_cvref(^^const int &));
static_assert(Miracle::meta::decay(^^const int &) == std::meta::decay(^^const int &));
static_assert(Miracle::meta::underlyingType(enumType) == std::meta::underlying_type(enumType));
static_assert(Miracle::meta::unwrapReference(^^std::reference_wrapper<int>) ==
              std::meta::unwrap_reference(^^std::reference_wrapper<int>));
static_assert(Miracle::meta::unwrapRefDecay(^^std::reference_wrapper<int>) ==
              std::meta::unwrap_ref_decay(^^std::reference_wrapper<int>));
static_assert(
    Miracle::meta::tupleSize(^^std::tuple<int, long>) == std::meta::tuple_size(^^std::tuple<int, long>));
static_assert(Miracle::meta::tupleElement(1, ^^std::tuple<int, long>) ==
              std::meta::tuple_element(1, ^^std::tuple<int, long>));
static_assert(Miracle::meta::variantSize(^^std::variant<int, long>) ==
              std::meta::variant_size(^^std::variant<int, long>));
static_assert(Miracle::meta::variantAlternative(1, ^^std::variant<int, long>) ==
              std::meta::variant_alternative(1, ^^std::variant<int, long>));
static_assert(Miracle::meta::returnTypeOf(functionInfo) == std::meta::return_type_of(functionInfo));

static_assert(
    Miracle::meta::isAssignableType(^^int &, ^^int) == std::meta::is_assignable_type(^^int &, ^^int));
static_assert(Miracle::meta::isSwappableWithType(^^int &, ^^int &) ==
              std::meta::is_swappable_with_type(^^int &, ^^int &));
static_assert(Miracle::meta::isTriviallyAssignableType(^^int &, ^^int) ==
              std::meta::is_trivially_assignable_type(^^int &, ^^int));
static_assert(Miracle::meta::isNothrowAssignableType(^^int &, ^^int) ==
              std::meta::is_nothrow_assignable_type(^^int &, ^^int));
static_assert(Miracle::meta::isNothrowSwappableWithType(^^int &, ^^int &) ==
              std::meta::is_nothrow_swappable_with_type(^^int &, ^^int &));
static_assert(Miracle::meta::isSameType(intType, ^^int) == std::meta::is_same_type(intType, ^^int));
static_assert(Miracle::meta::typeOrder(intType, doubleType) == std::meta::type_order(intType, doubleType));
static_assert(
    Miracle::meta::isBaseOfType(baseType, sampleType) == std::meta::is_base_of_type(baseType, sampleType));
static_assert(Miracle::meta::isVirtualBaseOfType(baseType, sampleType) ==
              std::meta::is_virtual_base_of_type(baseType, sampleType));
static_assert(Miracle::meta::isConvertibleType(intType, doubleType) ==
              std::meta::is_convertible_type(intType, doubleType));
static_assert(Miracle::meta::isNothrowConvertibleType(intType, doubleType) ==
              std::meta::is_nothrow_convertible_type(intType, doubleType));

constexpr std::array constructorArguments{^^int};
constexpr std::array noArguments = std::array<std::meta::info, 0>{};
static_assert(Miracle::meta::isConstructibleType(sampleType, constructorArguments) ==
              std::meta::is_constructible_type(sampleType, constructorArguments));
static_assert(Miracle::meta::isTriviallyConstructibleType(intType, noArguments) ==
              std::meta::is_trivially_constructible_type(intType, noArguments));
static_assert(Miracle::meta::isNothrowConstructibleType(intType, noArguments) ==
              std::meta::is_nothrow_constructible_type(intType, noArguments));
constexpr std::array invokeArguments{^^int};
static_assert(Miracle::meta::isInvocableType(^^decltype(function), invokeArguments) ==
              std::meta::is_invocable_type(^^decltype(function), invokeArguments));
static_assert(Miracle::meta::isNothrowInvocableType(^^decltype(function), invokeArguments) ==
              std::meta::is_nothrow_invocable_type(^^decltype(function), invokeArguments));
static_assert(Miracle::meta::invokeResult(^^decltype(function), invokeArguments) ==
              std::meta::invoke_result(^^decltype(function), invokeArguments));
constexpr std::array commonArguments{^^int, ^^long};
static_assert(Miracle::meta::commonType(commonArguments) == std::meta::common_type(commonArguments));
static_assert(
    Miracle::meta::commonReference(commonArguments) == std::meta::common_reference(commonArguments));

consteval auto sequenceOperationsMatch() -> bool {
  if (!sequenceParity(
          Miracle::meta::templateArgumentsOf(^^Box<int>), std::meta::template_arguments_of(^^Box<int>))) {
    return false;
  }
  const auto access = Miracle::Access::unchecked();
  if (!sequenceParity(
          Miracle::meta::membersOf(sampleType, access), std::meta::members_of(sampleType, access))) {
    return false;
  }
  if (!sequenceParity(Miracle::meta::basesOf(sampleType, access), std::meta::bases_of(sampleType, access))) {
    return false;
  }
  if (!sequenceParity(
          Miracle::meta::subobjectsOf(sampleType, access), std::meta::subobjects_of(sampleType, access))) {
    return false;
  }
  if (!sequenceParity(Miracle::meta::staticDataMembersOf(sampleType, access),
          std::meta::static_data_members_of(sampleType, access))) {
    return false;
  }
  if (!sequenceParity(Miracle::meta::nonstaticDataMembersOf(sampleType, access),
          std::meta::nonstatic_data_members_of(sampleType, access))) {
    return false;
  }
  if (!sequenceParity(Miracle::meta::enumeratorsOf(enumType), std::meta::enumerators_of(enumType))) {
    return false;
  }
  if (!sequenceParity(Miracle::meta::parametersOf(functionInfo), std::meta::parameters_of(functionInfo))) {
    return false;
  }
  if (!sequenceParity(Miracle::meta::annotationsOf(sampleType), std::meta::annotations_of(sampleType))) {
    return false;
  }
  return true;
}
static_assert(sequenceOperationsMatch());

consteval auto currentFunctionParity() -> bool {
  return Miracle::meta::currentFunction() == std::meta::current_function();
}
static_assert(currentFunctionParity());

struct CurrentClassProbe final {
  static consteval auto parity() -> bool {
    return Miracle::meta::currentClass() == std::meta::current_class();
  } // namespace Tests::metaStdParity
};
static_assert(CurrentClassProbe::parity());

namespace CurrentNamespaceProbe {
consteval auto parity() -> bool {
  return Miracle::meta::currentNamespace() == std::meta::current_namespace();
}
static_assert(parity());
} // namespace CurrentNamespaceProbe

// Query accepts the raw standard callables and the Miracle wrappers through the same adaptive invocation
// path.
constexpr std::array querySubjects{^^AnyType, ^^Box, ^^int};
static_assert(Miracle::meta::query(querySubjects).count(std::meta::is_concept) ==
              Miracle::meta::query(querySubjects).count(Miracle::meta::isConcept));
static_assert(Miracle::meta::query(querySubjects).filter(std::meta::is_template).count() == 2);
constexpr std::array singleField{fieldInfo};
static_assert(Miracle::meta::query(singleField).map(std::meta::type_of).first().value() == ^^int);
static_assert(Miracle::meta::query(singleField).map(Miracle::meta::typeOf).first().value() == ^^int);

} // namespace Tests::metaStdParity

auto main() -> int {
}
