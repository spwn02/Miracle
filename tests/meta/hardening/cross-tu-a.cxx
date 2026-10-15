import std;
import Miracle.Meta;
namespace SharedSnapshot {
struct First {};
} // namespace SharedSnapshot
static_assert(Miracle::meta::members(^^SharedSnapshot).size() == 1);
auto otherTranslationUnit() -> bool;
auto main() -> int {
  return otherTranslationUnit() ? 0 : 1;
}
