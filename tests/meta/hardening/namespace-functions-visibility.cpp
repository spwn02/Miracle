import std;
import Miracle.Meta;

namespace miracle_phase56_function_cache_probe {
void before();
} // namespace miracle_phase56_function_cache_probe

static_assert(
    std::meta::members_of(^^miracle_phase56_function_cache_probe, std::meta::access_context::unchecked())
        .size() == 1);
static_assert(
    Miracle::meta::functions(^^miracle_phase56_function_cache_probe, std::meta::access_context::unchecked())
        .size() == 1);

namespace miracle_phase56_function_cache_probe {
void after();
} // namespace miracle_phase56_function_cache_probe

static_assert(
    std::meta::members_of(^^miracle_phase56_function_cache_probe, std::meta::access_context::unchecked())
        .size() == 2);
static_assert(
    Miracle::meta::functions(^^miracle_phase56_function_cache_probe, std::meta::access_context::unchecked())
        .size() == 2);

auto main() -> int {
}
