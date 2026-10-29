import std;
import Miracle.Meta;

namespace Tests::metaParityLayout {

struct Subject final {
  char prefix{};
  int value{};
};

inline constexpr auto subject = ^^Subject;
inline constexpr auto member = ^^Subject::value;

static_assert(Miracle::meta::offsetOf(member) == std::meta::offset_of(member));
static_assert(Miracle::meta::sizeOf(subject) == std::meta::size_of(subject));
static_assert(Miracle::meta::alignmentOf(subject) == std::meta::alignment_of(subject));
static_assert(Miracle::meta::bitSizeOf(subject) == std::meta::bit_size_of(subject));

} // namespace Tests::metaParityLayout

auto main() -> int {
}
