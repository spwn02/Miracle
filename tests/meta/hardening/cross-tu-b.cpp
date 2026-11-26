import std;
import Miracle.Meta;
namespace SharedSnapshot {
struct First {};
struct Second {};
} // namespace SharedSnapshot
static_assert(Miracle::meta::members(^^SharedSnapshot).size() == 2);
auto otherTranslationUnit() -> bool {
  return true;
}
