import std;
import Miracle.Meta;

namespace Tests::metaPredicateParity {

/// Empty aggregate used to exercise class and structural type predicates.
struct Empty final {};

/// Ordinary aggregate used to exercise aggregate/layout/property predicates.
struct Aggregate final {
  /// Stored payload keeps the aggregate non-empty.
  int value{};
};

/// Polymorphic base used to exercise polymorphic/virtual-destructor predicates.
struct Polymorphic {
  /// Creates an ordinary polymorphic test object.
  Polymorphic() = default;
  /// Copies the reflection fixture without changing its semantics.
  Polymorphic(const Polymorphic &) = default;
  /// Moves the reflection fixture without changing its semantics.
  Polymorphic(Polymorphic &&) = default;
  /// Copies the reflection fixture state.
  auto operator=(const Polymorphic &) -> Polymorphic & = default;
  /// Moves the reflection fixture state.
  auto operator=(Polymorphic &&) -> Polymorphic & = default;
  /// Virtual destructor makes the reflected type polymorphic.
  virtual ~Polymorphic() = default;
};

/// Abstract class used to exercise abstract-type predicates.
struct Abstract {
  /// Creates the abstract reflection fixture through derived objects.
  Abstract() = default;
  /// Copies the abstract fixture state.
  Abstract(const Abstract &) = default;
  /// Moves the abstract fixture state.
  Abstract(Abstract &&) = default;
  /// Copies the abstract fixture state.
  auto operator=(const Abstract &) -> Abstract & = default;
  /// Moves the abstract fixture state.
  auto operator=(Abstract &&) -> Abstract & = default;
  /// Pure virtual operation makes the reflected type abstract.
  virtual auto operation() -> void = 0;
  /// Virtual destructor permits ordinary derived cleanup.
  virtual ~Abstract() = default;
};

/// Union subject for primary/composite category coverage.
union Union {
  /// Integral alternative.
  int integer;
  /// Floating-point alternative.
  double floating;
};

/// Scoped enumeration subject for enum-property coverage.
enum class Scoped : std::uint8_t { first, second };

/// Member-pointer owner used for member pointer categories.
struct MemberOwner final {
  /// Member-object target.
  int value{};
  /// Member-function target.
  auto function() -> void {
  }
};

// Raw array/function aliases are intentional reflection subjects for exact type-category coverage.
using Array = int[3]; // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
using Function = int(int);
using MemberObjectPointer = int MemberOwner::*;
using MemberFunctionPointer = void (MemberOwner::*)();

/// Shared corpus containing only complete types on which every canonical unary type predicate is defined.
inline constexpr std::array typeCorpus{
    ^^void,
    ^^decltype(nullptr),
    ^^bool,
    ^^int,
    ^^unsigned,
    ^^double,
    ^^const int,
    ^^volatile int,
    ^^int *,
    ^^int &,
    ^^int &&,
    ^^Array,
    ^^MemberObjectPointer,
    ^^MemberFunctionPointer,
    ^^Scoped,
    ^^Union,
    ^^Empty,
    ^^Aggregate,
    ^^Polymorphic,
    ^^Abstract,
    ^^Function,
    ^^decltype(^^int),
};

/// Verifies one Miracle predicate against its standard counterpart across every supplied reflection.
template <auto MiraclePredicate, auto StandardPredicate, Miracle::meta::ReflectionRange Range>
consteval auto unaryParity(const Range &subjects) -> bool {
  for (const auto subject : subjects) {
    if (MiraclePredicate(subject) != StandardPredicate(subject)) {
      return false;
    }
  }
  return true;
}

// Primary type categories.
static_assert(unaryParity<Miracle::meta::isVoidType, std::meta::is_void_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isNullPointerType, std::meta::is_null_pointer_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isIntegralType, std::meta::is_integral_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isFloatingPointType, std::meta::is_floating_point_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isArrayType, std::meta::is_array_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isPointerType, std::meta::is_pointer_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isLvalueReferenceType, std::meta::is_lvalue_reference_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isRvalueReferenceType, std::meta::is_rvalue_reference_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isMemberObjectPointerType, std::meta::is_member_object_pointer_type>(
    typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isMemberFunctionPointerType, std::meta::is_member_function_pointer_type>(
        typeCorpus));
static_assert(unaryParity<Miracle::meta::isEnumType, std::meta::is_enum_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isUnionType, std::meta::is_union_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isClassType, std::meta::is_class_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isFunctionType, std::meta::is_function_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isReflectionType, std::meta::is_reflection_type>(typeCorpus));

// Composite type categories.
static_assert(unaryParity<Miracle::meta::isReferenceType, std::meta::is_reference_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isArithmeticType, std::meta::is_arithmetic_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isFundamentalType, std::meta::is_fundamental_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isObjectType, std::meta::is_object_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isScalarType, std::meta::is_scalar_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isCompoundType, std::meta::is_compound_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isMemberPointerType, std::meta::is_member_pointer_type>(typeCorpus));

// Type properties.
static_assert(unaryParity<Miracle::meta::isConstType, std::meta::is_const_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isVolatileType, std::meta::is_volatile_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isTriviallyCopyableType, std::meta::is_trivially_copyable_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isStandardLayoutType, std::meta::is_standard_layout_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isEmptyType, std::meta::is_empty_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isPolymorphicType, std::meta::is_polymorphic_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isAbstractType, std::meta::is_abstract_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isFinalType, std::meta::is_final_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isAggregateType, std::meta::is_aggregate_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isStructuralType, std::meta::is_structural_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isSignedType, std::meta::is_signed_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isUnsignedType, std::meta::is_unsigned_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isBoundedArrayType, std::meta::is_bounded_array_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isScopedEnumType, std::meta::is_scoped_enum_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isDefaultConstructibleType, std::meta::is_default_constructible_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isCopyConstructibleType, std::meta::is_copy_constructible_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isMoveConstructibleType, std::meta::is_move_constructible_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isCopyAssignableType, std::meta::is_copy_assignable_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isMoveAssignableType, std::meta::is_move_assignable_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isSwappableType, std::meta::is_swappable_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isDestructibleType, std::meta::is_destructible_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isTriviallyDefaultConstructibleType,
    std::meta::is_trivially_default_constructible_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isTriviallyCopyConstructibleType,
    std::meta::is_trivially_copy_constructible_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isTriviallyMoveConstructibleType,
    std::meta::is_trivially_move_constructible_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isTriviallyCopyAssignableType, std::meta::is_trivially_copy_assignable_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isTriviallyMoveAssignableType, std::meta::is_trivially_move_assignable_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isTriviallyDestructibleType, std::meta::is_trivially_destructible_type>(
        typeCorpus));
static_assert(unaryParity<Miracle::meta::isNothrowDefaultConstructibleType,
    std::meta::is_nothrow_default_constructible_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isNothrowCopyConstructibleType, std::meta::is_nothrow_copy_constructible_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isNothrowMoveConstructibleType, std::meta::is_nothrow_move_constructible_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isNothrowCopyAssignableType, std::meta::is_nothrow_copy_assignable_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isNothrowMoveAssignableType, std::meta::is_nothrow_move_assignable_type>(
        typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isNothrowSwappableType, std::meta::is_nothrow_swappable_type>(typeCorpus));
static_assert(unaryParity<Miracle::meta::isNothrowDestructibleType, std::meta::is_nothrow_destructible_type>(
    typeCorpus));
static_assert(
    unaryParity<Miracle::meta::isImplicitLifetimeType, std::meta::is_implicit_lifetime_type>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::hasVirtualDestructor, std::meta::has_virtual_destructor>(typeCorpus));
static_assert(
    unaryParity<Miracle::meta::hasUniqueObjectRepresentations, std::meta::has_unique_object_representations>(
        typeCorpus));

// Unbounded arrays are intentionally isolated because several unrelated property predicates require
// complete types.
using UnboundedArray = int[]; // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
static_assert(Miracle::meta::isUnboundedArrayType(^^UnboundedArray) ==
              std::meta::is_unbounded_array_type(^^UnboundedArray));

// Miracle convenience predicates retain their intentionally broader safe-domain semantics.
static_assert(Miracle::meta::isType(^^int));
static_assert(Miracle::meta::isComplete(^^int));
static_assert(Miracle::meta::isEnumerable(^^Aggregate));
static_assert(Miracle::meta::isClass(^^Aggregate));
static_assert(Miracle::meta::isUnion(^^Union));
static_assert(Miracle::meta::isEnum(^^Scoped));

// Query accepts both Miracle predicates and raw standard predicates over the same source.
inline constexpr std::array queryCorpus{^^int, ^^double, ^^Aggregate, ^^Scoped};
static_assert(Miracle::meta::query(queryCorpus).filter(Miracle::meta::isIntegralType).count() ==
              Miracle::meta::query(queryCorpus).filter(std::meta::is_integral_type).count());

} // namespace Tests::metaPredicateParity

auto main() -> int {
}
