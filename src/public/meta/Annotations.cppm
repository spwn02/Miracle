export module Miracle.Meta:Annotations;

import std;
import :Types;
import :Cache;
import :Predicates;

export namespace Miracle::meta {

/// Callable wrapper for the standard unary `annotations_of` operation.
struct AnnotationsOf final {
  /// Returns every annotation directly to `subject` in the standard vector representation.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::vector<Info> {
    return std::meta::annotations_of(subject);
  }
};

/// Exact overload-preserving wrapper for `std::meta::annotations_of`.
inline constexpr AnnotationsOf annotationsOf{};

/// Exact callable wrapper for `std::meta::annotations_of_with_type`.
inline constexpr MetaFunction annotationsOfWithType{std::meta::annotations_of_with_type};

/// Returns all annotations attached to a reflected entity in source order.
///
/// Each occurrence remains a distinct reflection even when two annotation values compare equally. The
/// returned span has static backing storage. Each call re-queries reachable annotations; callers that want
/// value-level deduplication can compose the result through `query(...)` and `uniqueBy(...)`.
[[nodiscard]] consteval auto annotations(Info subject) -> std::span<const Info> {
  return promoteInfoSnapshot(std::meta::annotations_of(subject));
}

/// Type-oriented overload querying annotations reachable at this evaluation point.
template <class T>
[[nodiscard]] consteval auto annotations() -> std::span<const Info> {
  return annotations(^^T);
}

/// Returns annotations whose annotation object has type `A`, preserving source order and annotation identity.
template <class A>
[[nodiscard]] consteval auto annotations(Info subject) -> std::span<const Info> {
  return promoteInfoSnapshot(std::meta::annotations_of_with_type(subject, ^^A));
}

/// Type-oriented typed-annotation source querying the currently reachable declarations.
template <class A, class T>
[[nodiscard]] consteval auto annotations() -> std::span<const Info> {
  return annotations<A>(^^T);
}

/// Matches individual annotation reflections; exact wrapper for `std::meta::is_annotation`.
inline constexpr Predicate isAnnotation{std::meta::is_annotation};

/// Matches entities carrying at least one annotation whose annotation object has type `A`.
///
/// Multiplicity is intentionally ignored here; callers needing the concrete annotation reflections use
/// `annotations<A>`.
template <class A>
inline constexpr Predicate annotated{
    [](Info info) consteval -> bool { return not annotations<A>(info).empty(); }};

} // namespace Miracle::meta
