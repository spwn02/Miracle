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
[[deprecated("parity")]] auto attributedFunction() -> void;

/// Compares two reflection ranges by size, source order, and reflection identity.
template <Miracle::meta::ReflectionRange Left, Miracle::meta::ReflectionRange Right>
consteval auto sameSequence(const Left &left, const Right &right) -> bool {
  return std::ranges::equal(left, right);
}

inline constexpr auto subject = ^^Annotated;

static_assert(sameSequence(Miracle::meta::annotationsOf(subject), std::meta::annotations_of(subject)));
static_assert(sameSequence(Miracle::meta::annotationsOf(subject, ^^Marker),
    std::meta::annotations_of(subject, ^^Marker)));
static_assert(sameSequence(Miracle::meta::annotationsOfWithType(subject, ^^Marker),
    std::meta::annotations_of_with_type(subject, ^^Marker)));
static_assert(sameSequence(Miracle::meta::annotations(subject), std::meta::annotations_of(subject)));
static_assert(sameSequence(Miracle::meta::annotations<Marker>(subject),
    std::meta::annotations_of_with_type(subject, ^^Marker)));
static_assert(sameSequence(Miracle::meta::annotations<Annotated>(), std::meta::annotations_of(subject)));
static_assert(sameSequence(Miracle::meta::annotations<Marker, Annotated>(),
    std::meta::annotations_of_with_type(subject, ^^Marker)));

static_assert(
    Miracle::meta::annotationOfType<Marker>(subject) == std::meta::annotation_of_type<Marker>(subject));

consteval auto attributeParity() -> bool {
  const auto standard = std::meta::attributes_of(^^attributedFunction);
  const auto miracle = Miracle::meta::attributesOf(^^attributedFunction);
  if (not sameSequence(miracle, standard)) {
    return false;
  }
  if (standard.empty()) {
    return true;
  }

  const auto attribute = standard.front();
  if (Miracle::meta::isAttribute(attribute) != std::meta::is_attribute(attribute) or
      Miracle::meta::hasAttributeNamespace(attribute) != std::meta::has_attribute_namespace(attribute) or
      Miracle::meta::attributeTokenOf(attribute) != std::meta::attribute_token_of(attribute) or
      Miracle::meta::u8AttributeTokenOf(attribute) != std::meta::u8attribute_token_of(attribute) or
      Miracle::meta::hasAttribute(^^attributedFunction, attribute) !=
          std::meta::has_attribute(^^attributedFunction, attribute)) {
    return false;
  }

  if (std::meta::has_attribute_namespace(attribute)) {
    return Miracle::meta::attributeNamespaceOf(attribute) == std::meta::attribute_namespace_of(attribute) and
           Miracle::meta::u8AttributeNamespaceOf(attribute) == std::meta::u8attribute_namespace_of(attribute);
  }
  return true;
}
static_assert(attributeParity());

} // namespace Tests::metaParityAnnotations

auto main() -> int {
}
