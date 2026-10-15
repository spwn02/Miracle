import std;
import Miracle.Meta;

namespace Tests::metaParityOperators {

struct Subject final {
  auto operator+(int value) const -> int {
    return value;
  }
};

inline constexpr auto plusOperator = ^^Subject::operator+;

static_assert(Miracle::meta::operatorOf(plusOperator) == std::meta::operator_of(plusOperator));
static_assert(Miracle::meta::symbolOf(std::meta::operators::op_plus) ==
              std::meta::symbol_of(std::meta::operators::op_plus));
static_assert(Miracle::meta::u8SymbolOf(std::meta::operators::op_plus) ==
              std::meta::u8symbol_of(std::meta::operators::op_plus));

} // namespace Tests::metaParityOperators

auto main() -> int {
}
