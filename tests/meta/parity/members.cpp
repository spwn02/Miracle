import std;
import Miracle.Meta;

namespace Tests::metaParityMembers {

struct Base {
  /// Data member used to make the direct base a concrete subobject source.
  int baseValue{};
};

struct Subject final : Base {
  /// First non-static data member used for source-order parity.
  int first{};
  /// Second non-static data member used for source-order parity.
  int second{};
  /// Static data member used to verify static-member reflection.
  static inline int shared{};

  /// Provides a default constructor for constructor-reflection parity.
  Subject() = default;
  /// Provides a second constructor for declaration-order parity.
  explicit Subject(int value)
      : first{value} {
  }

  /// Ordinary member function used to verify function/member classification.
  [[nodiscard]] auto function(int value) const -> int {
    return first + value;
  }
};

enum class Color : unsigned char {
  /// First enumerator used to verify source-order reflection.
  red,
  /// Middle enumerator used to verify source-order reflection.
  green,
  ///  Final enumerator used to verify source-order reflection.
  blue,
};

/// Class-template subject used for template-argument reflection parity.
template <class T>
struct Box {};

auto sampleFunction(int first, double second) -> long;

/// Compares two reflection ranges by size, source order, and reflection identity.
template <Miracle::meta::ReflectionRange Left, Miracle::meta::ReflectionRange Right>
consteval auto sameSequence(const Left &left, const Right &right) -> bool {
  return std::ranges::equal(left, right);
}

inline constexpr auto subject = ^^Subject;
inline constexpr auto access = Miracle::Access::unchecked();

static_assert(
    sameSequence(Miracle::meta::membersOf(subject, access), std::meta::members_of(subject, access)));
static_assert(sameSequence(Miracle::meta::basesOf(subject, access), std::meta::bases_of(subject, access)));
static_assert(
    sameSequence(Miracle::meta::subobjectsOf(subject, access), std::meta::subobjects_of(subject, access)));
static_assert(sameSequence(Miracle::meta::staticDataMembersOf(subject, access),
    std::meta::static_data_members_of(subject, access)));
static_assert(sameSequence(Miracle::meta::nonstaticDataMembersOf(subject, access),
    std::meta::nonstatic_data_members_of(subject, access)));
static_assert(sameSequence(Miracle::meta::enumeratorsOf(^^Color), std::meta::enumerators_of(^^Color)));
static_assert(sameSequence(Miracle::meta::templateArgumentsOf(^^Box<int>),
    std::meta::template_arguments_of(^^Box<int>)));
static_assert(
    sameSequence(Miracle::meta::parametersOf(^^sampleFunction), std::meta::parameters_of(^^sampleFunction)));

static_assert(sameSequence(Miracle::meta::members(subject, access), std::meta::members_of(subject, access)));
static_assert(sameSequence(Miracle::meta::fields(subject, access),
    std::meta::nonstatic_data_members_of(subject, access)));
static_assert(sameSequence(Miracle::meta::staticFields(subject, access),
    std::meta::static_data_members_of(subject, access)));
static_assert(sameSequence(Miracle::meta::bases(subject, access), std::meta::bases_of(subject, access)));
static_assert(
    sameSequence(Miracle::meta::subobjects(subject, access), std::meta::subobjects_of(subject, access)));
static_assert(
    sameSequence(Miracle::meta::subobjects<Subject>(access), std::meta::subobjects_of(subject, access)));
static_assert(
    sameSequence(Miracle::reflect<Subject>().subobjects(access), std::meta::subobjects_of(subject, access)));
static_assert(sameSequence(Miracle::meta::enumerators(^^Color), std::meta::enumerators_of(^^Color)));
static_assert(
    sameSequence(Miracle::meta::parameters(^^sampleFunction), std::meta::parameters_of(^^sampleFunction)));
static_assert(
    sameSequence(Miracle::meta::templateArguments(^^Box<int>), std::meta::template_arguments_of(^^Box<int>)));

static_assert(
    Miracle::meta::subobjects(subject, access).data() == Miracle::meta::subobjects<Subject>(access).data());

} // namespace Tests::metaParityMembers

auto main() -> int {
}
