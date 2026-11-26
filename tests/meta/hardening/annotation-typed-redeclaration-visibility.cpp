import std;
import Miracle.Meta;

struct Phase56Marker {
  int id;
};
[[= Phase56Marker{1}]] void miracle_phase56_typed_annotation_probe();
static_assert(
    std::meta::annotations_of_with_type(^^miracle_phase56_typed_annotation_probe, ^^Phase56Marker).size() ==
    1);
static_assert(
    Miracle::meta::annotations<Phase56Marker>(^^miracle_phase56_typed_annotation_probe).size() == 1);

[[= Phase56Marker{2}]] void miracle_phase56_typed_annotation_probe();
static_assert(
    std::meta::annotations_of_with_type(^^miracle_phase56_typed_annotation_probe, ^^Phase56Marker).size() ==
    2);
static_assert(
    Miracle::meta::annotations<Phase56Marker>(^^miracle_phase56_typed_annotation_probe).size() == 2);

struct[[= Phase56Marker{3}]] Marked;
static_assert(Miracle::meta::annotations<Marked>().size() == 1);
static_assert((Miracle::meta::annotations<Phase56Marker, Marked>().size() == 1));
static_assert(Miracle::reflect<Marked>().annotations().size() == 1);
struct[[= Phase56Marker{3}]] Marked {};
static_assert(std::meta::annotations_of(^^Marked).size() == 2);
static_assert(Miracle::meta::annotations<Marked>().size() == 2);
static_assert((Miracle::meta::annotations<Phase56Marker, Marked>().size() == 2));
static_assert(Miracle::reflect<Marked>().annotations().size() == 2);
static_assert(Miracle::meta::annotations<Marked>()[0] != Miracle::meta::annotations<Marked>()[1]);
static_assert(Miracle::meta::annotated<Phase56Marker>(^^Marked));

auto main() -> int {
}
