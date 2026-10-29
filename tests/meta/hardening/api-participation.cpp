import std;
import Miracle.Meta;

using namespace Miracle::meta;

template <class Character>
struct ToLiteral final {
  const Character *text;
  consteval operator const Character *() const {
    return text;
  }
};

struct AmbiguousLiteral final {
  consteval operator const char *() const {
    return "text";
  }
  consteval operator const wchar_t *() const {
    return L"text";
  }
};

template <class T>
concept StandardLiteralArgument = requires(T value) { std::is_string_literal(value); };
template <class T>
concept MiracleLiteralArgument = requires(T value) { isStringLiteral(value); };

static_assert(isStringLiteral(ToLiteral{"text"}));
static_assert(isStringLiteral(ToLiteral{L"text"}));
static_assert(isStringLiteral(ToLiteral{u8"text"}));
static_assert(isStringLiteral(ToLiteral{u"text"}));
static_assert(isStringLiteral(ToLiteral{U"text"}));
static_assert(not MiracleLiteralArgument<int *>);
static_assert(not MiracleLiteralArgument<signed char *>);
static_assert(not MiracleLiteralArgument<unsigned char *>);
static_assert(not MiracleLiteralArgument<AmbiguousLiteral>);
static_assert(not StandardLiteralArgument<AmbiguousLiteral>);
static_assert(not MiracleLiteralArgument<std::nullptr_t>);
static_assert(not StandardLiteralArgument<std::nullptr_t>);
static_assert(std::invocable<AnnotationsOf, Info>);
static_assert(not std::invocable<AnnotationsOf, Info, Info>);
static_assert(not std::invocable<DataMemberSpec, Info>);
static_assert(std::invocable<DataMemberSpec, Info, DataMemberOptions>);

template <class T>
concept Negatable = requires(T value) { !value; };
template <class T>
concept Conjoinable = requires(T value) {
  value && isClass;
  isClass && value;
};
template <class T>
concept Disjoinable = requires(T value) {
  value || isClass;
  isClass || value;
};
static_assert(not Negatable<decltype(identifierOf)>);
static_assert(not Conjoinable<decltype(identifierOf)>);
static_assert(not Disjoinable<decltype(identifierOf)>);
static_assert((isType && !isEnum)(^^int));
struct PredicateRecord {};
static_assert((isClass || isEnum)(^^PredicateRecord));

struct Structural final {
  int value;
  constexpr explicit Structural(int input)
      : value(input) {
  }
  constexpr Structural(const Structural &other)
      : value(other.value + 1) {
  }
};
constexpr Structural original{0};
constexpr auto lvalue = std::meta::reflect_constant(original);
constexpr auto prvalue = std::meta::reflect_constant(Structural{0});
static_assert(std::meta::extract<const Structural &>(lvalue).value == 1);
static_assert(std::meta::extract<const Structural &>(prvalue).value == 0);

struct CopyOnly final {
  int value;
  constexpr explicit CopyOnly(int input)
      : value(input) {
  }
  constexpr CopyOnly(const CopyOnly &) = default;
  constexpr CopyOnly(CopyOnly &&) = delete;
};
constexpr auto copyOnly = std::meta::reflect_constant(CopyOnly{7});
static_assert(std::meta::extract<const CopyOnly &>(copyOnly).value == 7);

auto main() -> int {
}
