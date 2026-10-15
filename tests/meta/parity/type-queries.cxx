import std;
import Miracle.Meta;

namespace Tests::metaTypeQueryParity {

/// Base class used for reflected type-relation parity.
struct Base {
  /// Creates the base reflection fixture.
  Base() = default;
  /// Copies the base reflection fixture.
  Base(const Base &) = default;
  /// Moves the base reflection fixture.
  Base(Base &&) = default;
  /// Copies the base reflection fixture state.
  auto operator=(const Base &) -> Base & = default;
  /// Moves the base reflection fixture state.
  auto operator=(Base &&) -> Base & = default;
  /// Virtual destructor enables polymorphic relationship coverage.
  virtual ~Base() = default;
};

/// Derived class used for ordinary base/derived relation parity.
struct Derived final : Base {};

/// Trivial aggregate used for constructibility and layout relation checks.
struct Aggregate final {
  /// Stored payload provides an ordinary constructible member.
  int value{};
};

/// Simple callable used for reflected invocability predicates.
struct Callable final {
  /// Callable operation used by invocability queries.
  auto operator()(int value) const noexcept -> long {
    return value;
  }
};

// Raw array alias is an intentional reflection subject for rank/extent queries.
using Matrix = int[2][3]; // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)

/// Compares a Miracle binary Meta operation with its exact standard counterpart.
template <auto MiracleOperation, auto StandardOperation>
consteval auto binaryParity(std::meta::info left, std::meta::info right) -> bool {
  return MiracleOperation(left, right) == StandardOperation(left, right);
}

inline constexpr std::array noArguments = std::array<std::meta::info, 0>{};
inline constexpr std::array intArgument{^^int};
inline constexpr std::array doubleArgument{^^double};

// Type-property queries.
static_assert(Miracle::meta::rank(^^Matrix) == std::meta::rank(^^Matrix));
static_assert(Miracle::meta::extent(^^Matrix) == std::meta::extent(^^Matrix));
static_assert(Miracle::meta::extent(^^Matrix, 1) == std::meta::extent(^^Matrix, 1));

// Binary type properties.
static_assert(binaryParity<Miracle::meta::isAssignableType, std::meta::is_assignable_type>(^^int &, ^^int));
static_assert(
    binaryParity<Miracle::meta::isTriviallyAssignableType, std::meta::is_trivially_assignable_type>(^^int &,
        ^^int));
static_assert(
    binaryParity<Miracle::meta::isNothrowAssignableType, std::meta::is_nothrow_assignable_type>(^^int &,
        ^^int));
static_assert(
    binaryParity<Miracle::meta::isSwappableWithType, std::meta::is_swappable_with_type>(^^int &, ^^int &));
static_assert(
    binaryParity<Miracle::meta::isNothrowSwappableWithType, std::meta::is_nothrow_swappable_with_type>(
        ^^int &,
        ^^int &));
static_assert(binaryParity<Miracle::meta::referenceConstructsFromTemporary,
    std::meta::reference_constructs_from_temporary>(^^const int &, ^^int));
static_assert(
    binaryParity<Miracle::meta::referenceConvertsFromTemporary, std::meta::reference_converts_from_temporary>(
        ^^const int &,
        ^^int));

// Type relations.
static_assert(binaryParity<Miracle::meta::isSameType, std::meta::is_same_type>(^^int, ^^int));
static_assert(Miracle::meta::typeOrder(^^int, ^^double) == std::meta::type_order(^^int, ^^double));
static_assert(binaryParity<Miracle::meta::isBaseOfType, std::meta::is_base_of_type>(^^Base, ^^Derived));
static_assert(
    binaryParity<Miracle::meta::isVirtualBaseOfType, std::meta::is_virtual_base_of_type>(^^Base, ^^Derived));
static_assert(
    binaryParity<Miracle::meta::isConvertibleType, std::meta::is_convertible_type>(^^int, ^^double));
static_assert(
    binaryParity<Miracle::meta::isNothrowConvertibleType, std::meta::is_nothrow_convertible_type>(^^int,
        ^^double));
static_assert(
    binaryParity<Miracle::meta::isLayoutCompatibleType, std::meta::is_layout_compatible_type>(^^int, ^^int));
static_assert(binaryParity<Miracle::meta::isPointerInterconvertibleBaseOfType,
    std::meta::is_pointer_interconvertible_base_of_type>(^^Base, ^^Derived));

// Range-argument type properties.
static_assert(Miracle::meta::isConstructibleType(^^Aggregate, noArguments) ==
              std::meta::is_constructible_type(^^Aggregate, noArguments));
static_assert(Miracle::meta::isConstructibleType(^^int, intArgument) ==
              std::meta::is_constructible_type(^^int, intArgument));
static_assert(Miracle::meta::isTriviallyConstructibleType(^^int, intArgument) ==
              std::meta::is_trivially_constructible_type(^^int, intArgument));
static_assert(Miracle::meta::isNothrowConstructibleType(^^int, intArgument) ==
              std::meta::is_nothrow_constructible_type(^^int, intArgument));

// Invocability relations use the same reflection-range argument representation as the standard API.
static_assert(Miracle::meta::isInvocableType(^^Callable, intArgument) ==
              std::meta::is_invocable_type(^^Callable, intArgument));
static_assert(Miracle::meta::isInvocableType(^^Callable, doubleArgument) ==
              std::meta::is_invocable_type(^^Callable, doubleArgument));
static_assert(Miracle::meta::isInvocableRType(^^long, ^^Callable, intArgument) ==
              std::meta::is_invocable_r_type(^^long, ^^Callable, intArgument));
static_assert(Miracle::meta::isNothrowInvocableType(^^Callable, intArgument) ==
              std::meta::is_nothrow_invocable_type(^^Callable, intArgument));
static_assert(Miracle::meta::isNothrowInvocableRType(^^long, ^^Callable, intArgument) ==
              std::meta::is_nothrow_invocable_r_type(^^long, ^^Callable, intArgument));

} // namespace Tests::metaTypeQueryParity

auto main() -> int {
}
