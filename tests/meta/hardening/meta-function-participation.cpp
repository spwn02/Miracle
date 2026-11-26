import std;
import Miracle.Meta;
template <class T>
concept CanCallIdentifier = requires(T value) { Miracle::meta::identifierOf(value); };
static_assert(!CanCallIdentifier<int>);

auto main() -> int {
}
