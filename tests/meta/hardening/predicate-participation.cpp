import std;
import Miracle.Meta;
using namespace Miracle::meta;
template <class T>
concept Negatable = requires(T value) { !value; };
static_assert(Negatable<decltype(isClass)>);
static_assert(not Negatable<decltype(identifierOf)>);

auto main() -> int {
}
