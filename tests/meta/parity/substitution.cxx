import std;
import Miracle.Meta;

namespace Tests::metaSubstitutionParity {

template <class T>
struct Box final {};

static_assert(Miracle::meta::templateOf(^^Box<int>) == std::meta::template_of(^^Box<int>));
static_assert(Miracle::meta::canSubstitute(^^Box,
                  {
                      ^^int}) == std::meta::can_substitute(^^Box,
                                     {
                                         ^^int}));
static_assert(Miracle::meta::substitute(^^Box,
                  {
                      ^^int}) == std::meta::substitute(^^Box,
                                     {
                                         ^^int}));
static_assert(Miracle::meta::substitute(^^Box,
                  {
                      ^^int}) == ^^Box<int>);

template <auto Operation>
consteval auto failureSource(std::meta::info subject) -> std::meta::info {
  try {
    (void)Operation(subject);
  } catch (const std::meta::exception &failure) {
    return failure.from();
  }
  return {};
}

static_assert(
    failureSource<Miracle::meta::templateOf>(^^int) == failureSource<std::meta::template_of>(^^int));

} // namespace Tests::metaSubstitutionParity

auto main() -> int {
}
