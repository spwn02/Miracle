import std;
import Miracle.Meta;

namespace Tests::metaReflectParity {

struct Base {
  int baseValue;
};

struct Subject final : Base {
  int field;
  static inline int staticField{};
  Subject() = default;
  explicit Subject(int input)
      : Base{}
      , field(input) {
  }
  [[nodiscard]] auto method() const -> int {
    return field;
  }
};

enum class Choice : std::uint8_t { first, second };

constexpr auto reflected = Miracle::reflect<Subject>();
static_assert(reflected.raw() == ^^Subject);
static_assert(reflected.members(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::members<Subject>(std::meta::access_context::unchecked()).size());
static_assert(reflected.fields(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::fields<Subject>(std::meta::access_context::unchecked()).size());
static_assert(reflected.staticFields(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::staticFields<Subject>(std::meta::access_context::unchecked()).size());
static_assert(reflected.functions(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::functions<Subject>(std::meta::access_context::unchecked()).size());
static_assert(reflected.constructors(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::constructors<Subject>(std::meta::access_context::unchecked()).size());
static_assert(reflected.bases(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::bases<Subject>(std::meta::access_context::unchecked()).size());
static_assert(reflected.subobjects(std::meta::access_context::unchecked()).size() ==
              Miracle::meta::subobjects<Subject>(std::meta::access_context::unchecked()).size());
static_assert(Miracle::reflect<Choice>().enumerators().size() == Miracle::meta::enumerators<Choice>().size());

} // namespace Tests::metaReflectParity

auto main() -> int {
}
