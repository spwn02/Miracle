import std;
import Miracle.Meta;

namespace Tests::metaSurfaceParity {

struct Constructible final {
  constexpr Constructible() = default;
  constexpr explicit Constructible(int value) noexcept {
    (void)value;
  }
};

using Function = long(int);

static_assert(Miracle::meta::isConstructibleType(^^Constructible,
                  {
                  }) == std::meta::is_constructible_type(^^Constructible,
                            {
                            }));
static_assert(Miracle::meta::isTriviallyConstructibleType(^^int,
                  {
                  }) == std::meta::is_trivially_constructible_type(^^int,
                            {
                            }));
static_assert(Miracle::meta::isNothrowConstructibleType(^^Constructible,
                  {
                      ^^int}) == std::meta::is_nothrow_constructible_type(^^Constructible,
                                     {
                                         ^^int}));
static_assert(Miracle::meta::isInvocableType(^^Function,
                  {
                      ^^int}) == std::meta::is_invocable_type(^^Function,
                                     {
                                         ^^int}));
static_assert(Miracle::meta::isInvocableRType(^^long,
                  ^^Function,
                  {
                      ^^int}) == std::meta::is_invocable_r_type(^^long,
                                     ^^Function,
                                     {
                                         ^^int}));
static_assert(Miracle::meta::isNothrowInvocableType(^^Function,
                  {
                      ^^int}) == std::meta::is_nothrow_invocable_type(^^Function,
                                     {
                                         ^^int}));
static_assert(Miracle::meta::isNothrowInvocableRType(^^long,
                  ^^Function,
                  {
                      ^^int}) == std::meta::is_nothrow_invocable_r_type(^^long,
                                     ^^Function,
                                     {
                                         ^^int}));

static_assert(Miracle::meta::commonType({^^int, ^^long}) == std::meta::common_type({^^int, ^^long}));
static_assert(Miracle::meta::commonReference({^^int &, ^^const int &}) ==
              std::meta::common_reference({^^int &, ^^const int &}));
static_assert(Miracle::meta::invokeResult(^^Function,
                  {
                      ^^int}) == std::meta::invoke_result(^^Function,
                                     {
                                         ^^int}));

} // namespace Tests::metaSurfaceParity

auto main() -> int {
}
