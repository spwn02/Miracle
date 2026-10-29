import std;
import Miracle.Meta;

namespace miracle_phase56_cache_probe {
inline int first;
} // namespace miracle_phase56_cache_probe

static_assert(
    std::meta::members_of(^^miracle_phase56_cache_probe, std::meta::access_context::unchecked()).size() == 1);
static_assert(
    Miracle::meta::members(^^miracle_phase56_cache_probe, std::meta::access_context::unchecked()).size() ==
    1);

constexpr auto retainedBefore = Miracle::meta::members(^^miracle_phase56_cache_probe);

namespace miracle_phase56_cache_probe {
inline int second;
} // namespace miracle_phase56_cache_probe

static_assert(
    std::meta::members_of(^^miracle_phase56_cache_probe, std::meta::access_context::unchecked()).size() == 2);
static_assert(
    Miracle::meta::members(^^miracle_phase56_cache_probe, std::meta::access_context::unchecked()).size() ==
    2);

static_assert(retainedBefore.size() == 1);
static_assert(retainedBefore.front() == ^^miracle_phase56_cache_probe::first);

namespace SnapshotAlpha {
struct Value {};
} // namespace SnapshotAlpha
namespace SnapshotBeta {
struct Value {};
} // namespace SnapshotBeta
static_assert(Miracle::meta::members(^^SnapshotAlpha).size() == 1);
static_assert(Miracle::meta::members(^^SnapshotBeta).size() == 1);
static_assert(Miracle::meta::members(^^SnapshotAlpha).front() == ^^SnapshotAlpha::Value);
static_assert(Miracle::meta::members(^^SnapshotBeta).front() == ^^SnapshotBeta::Value);
static_assert(
    Miracle::meta::members(^^SnapshotAlpha).front() != Miracle::meta::members(^^SnapshotBeta).front());

auto main() -> int {
}
