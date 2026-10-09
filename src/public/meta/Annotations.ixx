export module Miracle.Meta:Annotations;

import std;
import :Types;
import :Cache;
import :Predicates;

export namespace Miracle::meta {

/// Exact callable wrapper returning an attribute's namespace spelling.
inline constexpr MetaFunction attributeNamespaceOf{std::meta::attribute_namespace_of};

/// Exact callable wrapper returning an attribute's token spelling.
inline constexpr MetaFunction attributeTokenOf{std::meta::attribute_token_of};

/// Exact UTF-8 callable wrapper returning an attribute's namespace spelling.
inline constexpr MetaFunction u8AttributeNamespaceOf{std::meta::u8attribute_namespace_of};

/// Exact UTF-8 callable wrapper returning an attribute's token spelling.
inline constexpr MetaFunction u8AttributeTokenOf{std::meta::u8attribute_token_of};

/// Callable wrapper covering both standard `annotations_of` overloads.
struct AnnotationsOf final {
  /// Returns every annotation directly to `subject` in the standard vector representation.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::vector<Info> {
    return std::meta::annotations_of(subject);
  }

  /// Returns annotations selected by the additional standard reflection selector.
  [[nodiscard]] consteval auto operator()(Info subject, Info selector) const -> std::vector<Info> {
    return std::meta::annotations_of(subject, selector);
  }
};

/// Exact overload-preserving wrapper for `std::meta::annotations_of`.
inline constexpr AnnotationsOf annotationsOf{};

/// Exact callable wrapper for `std::meta::annotations_of_with_type`.
inline constexpr MetaFunction annotationsOfWithType{std::meta::annotations_of_with_type};

/// Typed callable wrapper for `std::meta::annotation_of_type<T>`.
template <class T>
struct AnnotationOfType final {
  /// Returns the optional extracted annotation value of type `T` using the standard consistency rules.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::optional<T> {
    return std::meta::annotation_of_type<T>(subject);
  }
};

/// Variable template exposing `std::meta::annotation_of_type<T>` as a Query-compatible callable.
template <class T>
inline constexpr AnnotationOfType<T> annotationOfType{};

/// Exact callable wrapper for attaching a reflected annotation value to an entity reflection.
inline constexpr MetaFunction annotate{std::meta::annotate};

/// Callable wrapper covering both standard `has_attribute` comparison forms.
struct HasAttribute final {
  /// Tests exact attribute presence on `entity`.
  [[nodiscard]] consteval auto operator()(Info entity, Info attribute) const -> bool {
    return std::meta::has_attribute(entity, attribute);
  }

  /// Tests attribute presence using the supplied standard comparison policy.
  [[nodiscard]] consteval auto operator()(Info entity,
      Info attribute,
      std::meta::attribute_comparison policy) const -> bool {
    return std::meta::has_attribute(entity, attribute, policy);
  }
};

/// Exact overload-preserving wrapper for `std::meta::has_attribute`.
inline constexpr HasAttribute hasAttribute{};

/// Callable wrapper covering both standard `attributes_of` forms without losing overloads to function-pointer
/// decay.
struct AttributesOf final {
  /// Returns every attribute attached directly to `subject`.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::vector<Info> {
    return std::meta::attributes_of(subject);
  }

  /// Returns attributes matching `name` and the requested namespace category.
  [[nodiscard]] consteval auto operator()(Info subject,
      std::string_view name,
      AttributeNamespace attributeNamespace = AttributeNamespace::None) const -> std::vector<Info> {
    return std::meta::attributes_of(subject, name, attributeNamespace);
  }
};

/// Exact overload-preserving wrapper for `std::meta::attributes_of`.
inline constexpr AttributesOf attributesOf{};

/// Returns all annotations attached to a reflected entity in source order.
///
/// Each occurrence remains a distinct reflection even when two annotation values compare equally. The
/// returned span has static backing storage; callers that want value-level deduplication can compose the
/// result through `query(...)` and `uniqueBy(...)`.
[[nodiscard]] consteval auto annotations(Info subject) -> std::span<const Info> {
  return cachedCategory(subject, CacheCategory::Annotations);
}

/// Type-oriented overload returning the independently lazy canonical annotation cache for `T`.
template <class T>
[[nodiscard]] consteval auto annotations() -> std::span<const Info> {
  return categoryCache<^^T, CacheCategory::Annotations>;
}

/// Returns annotations whose annotation object has type `A`, preserving source order and annotation identity.
template <class A>
[[nodiscard]] consteval auto annotations(Info subject) -> std::span<const Info> {
  return cachedAnnotationsOfType(subject, ^^A);
}

/// Type-oriented typed-annotation source backed by an independent `(T, A)` canonical state.
template <class A, class T>
[[nodiscard]] consteval auto annotations() -> std::span<const Info> {
  return cachedAnnotationsOfType(^^T, ^^A);
}

/// Matches individual annotation reflections; exact wrapper for `std::meta::is_annotation`.
inline constexpr Predicate isAnnotation{std::meta::is_annotation};

/// Matches attribute reflections; exact wrapper for `std::meta::is_attribute`.
inline constexpr Predicate isAttribute{std::meta::is_attribute};

/// Matches attributes carrying a namespace; exact wrapper for `std::meta::has_attribute_namespace`.
inline constexpr Predicate hasAttributeNamespace{std::meta::has_attribute_namespace};

/// Matches unscoped attribute reflections; exact wrapper for `std::meta::is_unscoped_attribute`.
inline constexpr Predicate isUnscopedAttribute{std::meta::is_unscoped_attribute};

/// Matches Clang-namespaced attribute reflections; exact wrapper for `std::meta::is_clang_attribute`.
inline constexpr Predicate isClangAttribute{std::meta::is_clang_attribute};

/// Matches GCC-namespaced attribute reflections; exact wrapper for `std::meta::is_gcc_attribute`.
inline constexpr Predicate isGccAttribute{std::meta::is_gcc_attribute};

/// Matches MSVC-namespaced attribute reflections; exact wrapper for `std::meta::is_msvc_attribute`.
inline constexpr Predicate isMsvcAttribute{std::meta::is_msvc_attribute};

/// Matches entities carrying at least one annotation whose annotation object has type `A`.
///
/// Multiplicity is intentionally ignored here; callers needing the concrete annotation reflections use
/// `annotations<A>`.
template <class A>
inline constexpr Predicate annotated{
    [](Info info) consteval -> bool { return not annotations<A>(info).empty(); }};

} // namespace Miracle::meta
