import std;
import Miracle.Meta;

namespace Tests::metaTypeTransformParity {

/// Scoped enum used to exercise signedness and underlying-type transformations.
enum class Scoped : std::uint8_t { value };

/// Callable fixture used for invocation and tuple-application result transformations.
struct Callable final {
  /// Returns a distinct result type so invocation transforms have an observable answer.
  auto operator()(int value) const noexcept -> long {
    return value;
  }
};

// Raw array and function aliases are intentional reflection subjects for exact transformation coverage.
using Matrix = const int[2][3]; // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
using Vector = const int[4];    // NOLINT(cppcoreguidelines-avoid-c-arrays,modernize-avoid-c-arrays)
using Function = long(double);
using ReferenceWrapper = std::reference_wrapper<int>;
using Pair = std::pair<int, const long>;
using Tuple = std::tuple<int>;
using Variant = std::variant<int, const long>;

/// Compares a unary Miracle type transformation with its exact standard counterpart across one valid corpus.
template <auto MiracleTransformation, auto StandardTransformation, Miracle::meta::ReflectionRange Range>
consteval auto unaryParity(const Range &subjects) -> bool {
  for (const std::meta::info subject : subjects) {
    if (MiracleTransformation(subject) != StandardTransformation(subject)) {
      return false;
    }
  }
  return true;
}

/// Returns the standard reflection attached to a transformation failure, or an empty reflection on success.
template <auto Transformation>
consteval auto failureSource(std::meta::info subject) -> std::meta::info {
  try {
    (void)Transformation(subject);
  } catch (const std::meta::exception &failure) {
    return failure.from();
  }
  return {};
}

/// Non-type reflection used to verify exact failure provenance for type-only transformations.
inline constexpr int reflectedValue{};

inline constexpr std::array cvTypes{
    ^^int,
    ^^const int,
    ^^volatile long,
    ^^const volatile double,
};

inline constexpr std::array referenceTypes{
    ^^int,
    ^^const int &,
    ^^volatile long &&,
};

inline constexpr std::array signTypes{
    ^^int,
    ^^unsigned int,
    ^^long,
    ^^unsigned long,
    ^^Scoped,
};

inline constexpr std::array arrayTypes{
    ^^Matrix,
    ^^Vector,
};

inline constexpr std::array pointerTypes{
    ^^int,
    ^^int *,
    ^^const long *,
};

inline constexpr std::array normalizationTypes{
    ^^const int &,
    ^^volatile long &&,
    ^^Matrix,
    ^^Function,
};

static_assert(unaryParity<Miracle::meta::removeConst, std::meta::remove_const>(cvTypes));
static_assert(unaryParity<Miracle::meta::removeVolatile, std::meta::remove_volatile>(cvTypes));
static_assert(unaryParity<Miracle::meta::removeCv, std::meta::remove_cv>(cvTypes));
static_assert(unaryParity<Miracle::meta::addConst, std::meta::add_const>(cvTypes));
static_assert(unaryParity<Miracle::meta::addVolatile, std::meta::add_volatile>(cvTypes));
static_assert(unaryParity<Miracle::meta::addCv, std::meta::add_cv>(cvTypes));

static_assert(unaryParity<Miracle::meta::removeReference, std::meta::remove_reference>(referenceTypes));
static_assert(
    unaryParity<Miracle::meta::addLvalueReference, std::meta::add_lvalue_reference>(referenceTypes));
static_assert(
    unaryParity<Miracle::meta::addRvalueReference, std::meta::add_rvalue_reference>(referenceTypes));

static_assert(unaryParity<Miracle::meta::makeSigned, std::meta::make_signed>(signTypes));
static_assert(unaryParity<Miracle::meta::makeUnsigned, std::meta::make_unsigned>(signTypes));

static_assert(unaryParity<Miracle::meta::removeExtent, std::meta::remove_extent>(arrayTypes));
static_assert(unaryParity<Miracle::meta::removeAllExtents, std::meta::remove_all_extents>(arrayTypes));

static_assert(unaryParity<Miracle::meta::removePointer, std::meta::remove_pointer>(pointerTypes));
static_assert(unaryParity<Miracle::meta::addPointer, std::meta::add_pointer>(pointerTypes));

static_assert(unaryParity<Miracle::meta::removeCvref, std::meta::remove_cvref>(normalizationTypes));
static_assert(unaryParity<Miracle::meta::decay, std::meta::decay>(normalizationTypes));

static_assert(failureSource<Miracle::meta::removeCv>(^^reflectedValue) ==
              failureSource<std::meta::remove_cv>(^^reflectedValue));
static_assert(failureSource<Miracle::meta::removeReference>(^^reflectedValue) ==
              failureSource<std::meta::remove_reference>(^^reflectedValue));
static_assert(failureSource<Miracle::meta::removeCvref>(^^reflectedValue) ==
              failureSource<std::meta::remove_cvref>(^^reflectedValue));

inline constexpr std::array commonTypes{^^int, ^^double};
inline constexpr std::array commonReferenceTypes{^^int &, ^^const int &};
inline constexpr std::array invocationArguments{^^int};

static_assert(Miracle::meta::commonType(commonTypes) == std::meta::common_type(commonTypes));
static_assert(Miracle::meta::commonReference(commonReferenceTypes) ==
              std::meta::common_reference(commonReferenceTypes));
static_assert(Miracle::meta::underlyingType(^^Scoped) == std::meta::underlying_type(^^Scoped));
static_assert(
    failureSource<Miracle::meta::underlyingType>(^^int) == failureSource<std::meta::underlying_type>(^^int));
static_assert(Miracle::meta::invokeResult(^^Callable, invocationArguments) ==
              std::meta::invoke_result(^^Callable, invocationArguments));

static_assert(
    Miracle::meta::unwrapReference(^^ReferenceWrapper) == std::meta::unwrap_reference(^^ReferenceWrapper));
static_assert(
    Miracle::meta::unwrapRefDecay(^^ReferenceWrapper) == std::meta::unwrap_ref_decay(^^ReferenceWrapper));

static_assert(Miracle::meta::tupleElement(0, ^^Pair) == std::meta::tuple_element(0, ^^Pair));
static_assert(Miracle::meta::tupleElement(1, ^^Pair) == std::meta::tuple_element(1, ^^Pair));
static_assert(
    Miracle::meta::applyResult(^^Callable, ^^Tuple) == std::meta::apply_result(^^Callable, ^^Tuple));
static_assert(
    Miracle::meta::variantAlternative(0, ^^Variant) == std::meta::variant_alternative(0, ^^Variant));
static_assert(
    Miracle::meta::variantAlternative(1, ^^Variant) == std::meta::variant_alternative(1, ^^Variant));

// Adjacent tuple/variant/applicability/order facilities are observations, so their ownership stays in
// `TypeQueries`; 2F audits that boundary while the `Info`-returning transforms stay in `TypeTransforms`.
static_assert(Miracle::meta::tupleSize(^^Pair) == std::meta::tuple_size(^^Pair));
static_assert(Miracle::meta::variantSize(^^Variant) == std::meta::variant_size(^^Variant));
static_assert(Miracle::meta::isApplicableType(^^Callable, ^^Tuple) ==
              std::meta::is_applicable_type(^^Callable, ^^Tuple));
static_assert(Miracle::meta::isNothrowApplicableType(^^Callable, ^^Tuple) ==
              std::meta::is_nothrow_applicable_type(^^Callable, ^^Tuple));
static_assert(Miracle::meta::typeOrder(^^int, ^^double) == std::meta::type_order(^^int, ^^double));

constexpr auto miracleMapped = Miracle::meta::query(cvTypes).map(Miracle::meta::removeCv);
constexpr auto standardMapped = Miracle::meta::query(cvTypes).map(std::meta::remove_cv);
static_assert(miracleMapped.count() == standardMapped.count());
static_assert(miracleMapped.nth(0).value() == standardMapped.nth(0).value());
static_assert(miracleMapped.nth(1).value() == standardMapped.nth(1).value());
static_assert(miracleMapped.nth(2).value() == standardMapped.nth(2).value());
static_assert(miracleMapped.nth(3).value() == standardMapped.nth(3).value());

} // namespace Tests::metaTypeTransformParity

auto main() -> int {
}
