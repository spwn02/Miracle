import std;
import Miracle.Meta;

[[= 11]] void miracle_phase56_annotation_probe();

static_assert(std::meta::annotations_of(^^miracle_phase56_annotation_probe).size() == 1);
static_assert(Miracle::meta::annotations(^^miracle_phase56_annotation_probe).size() == 1);

[[= 22]] void miracle_phase56_annotation_probe();

static_assert(std::meta::annotations_of(^^miracle_phase56_annotation_probe).size() == 2);
static_assert(Miracle::meta::annotations(^^miracle_phase56_annotation_probe).size() == 2);

auto main() -> int {
}
