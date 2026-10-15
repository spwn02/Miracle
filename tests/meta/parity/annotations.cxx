import std;
import Miracle.Meta;

namespace Tests::metaParityAnnotations {

struct Marker final {
  /// Payload compared when the reflected annotation object is extracted.
  int value{};

  /// Structural equality required for reflected annotation-value comparison.
  constexpr auto operator==(const Marker &) const -> bool = default;
};

struct OtherMarker final {
  /// Payload compared when the second reflected annotation object is extracted.
  int value{};

  /// Structural equality required for reflected annotation-value comparison.
  constexpr auto operator==(const OtherMarker &) const -> bool = default;
};

/// Stable payload used by the primary annotation parity fixture.
inline constexpr int markerValue = 17;
/// Stable payload used by the secondary annotation parity fixture.
inline constexpr int otherMarkerValue = 23;

struct[[= Marker{markerValue}]][[= OtherMarker{otherMarkerValue}]] Annotated final {};

/// Compares two reflection ranges by size, source order, and reflection identity.
template <Miracle::meta::ReflectionRange Left, Miracle::meta::ReflectionRange Right>
consteval auto sameSequence(const Left &left, const Right &right) -> bool {
  return std::ranges::equal(left, right);
}

inline constexpr auto subject = ^^Annotated;

static_assert(sameSequence(Miracle::meta::annotationsOf(subject), std::meta::annotations_of(subject)));
static_assert(sameSequence(Miracle::meta::annotationsOfWithType(subject, ^^Marker),
    std::meta::annotations_of_with_type(subject, ^^Marker)));

} // namespace Tests::metaParityAnnotations

auto main() -> int {
}
