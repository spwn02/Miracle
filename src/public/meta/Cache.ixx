export module Miracle.Meta:Cache;

import std;
import :Types;

namespace Miracle::meta {

/// Promotes a temporary compile-time vector into static program storage.
///
/// `std::define_static_array` owns the resulting backing storage, so the returned span never refers to the
/// transient vector. Reflection caches, reordered Query results, and explicit materialization all share this
/// one persistence primitive instead of maintaining category-specific copies. Compile-time work and promoted
/// storage are O(n).
template <class T>
[[nodiscard]] consteval auto promoteVector(const std::vector<T> &values) -> std::span<const T> {
  return std::define_static_array(values);
}

/// Returns whether `subject` denotes a class or union type, the scopes accepted by data-member queries.
[[nodiscard]] consteval auto isRecord(Info subject) -> bool {
  return std::meta::is_type(subject) and
         (std::meta::is_class_type(subject) or std::meta::is_union_type(subject));
}

/// Returns whether `subject` can own ordinary reflected members.
///
/// Namespaces and record types share the `members()` / `function()` source path; keeping this predicate
/// centralized prevents subtly different precondition checks across those public APIs.
[[nodiscard]] consteval auto isMemberScope(Info subject) -> bool {
  return std::meta::is_namespace(subject) or isRecord(subject);
}

/// Enforces a record-only semantic precondition while preserving the offending reflection in the diagnostic.
consteval auto requireRecord(Info subject, std::string_view message) -> void {
  if (not isRecord(subject)) {
    throw std::meta::exception{message, subject};
  }
  if (not std::meta::is_complete_type(subject)) {
    throw std::meta::exception{"reflection member source requires a complete class or union type", subject};
  }
}

/// Enforces a class-only semantic precondition while preserving the offending reflection in the diagnostic.
consteval auto requireClass(Info subject, std::string_view message) -> void {
  if (not std::meta::is_type(subject) or not std::meta::is_class_type(subject)) {
    throw std::meta::exception{message, subject};
  }
  if (not std::meta::is_complete_type(subject)) {
    throw std::meta::exception{"reflection member source requires a complete class type", subject};
  }
}

/// Enforces a namespace-or-record semantic precondition for member-source operations.
consteval auto requireMemberScope(Info subject, std::string_view message) -> void {
  if (not isMemberScope(subject)) {
    throw std::meta::exception{message, subject};
  }
  if (isRecord(subject) and not std::meta::is_complete_type(subject)) {
    throw std::meta::exception{"reflection member source requires a complete class or union type", subject};
  }
}

/// Filters the standard member sequence once and promotes only the accepted reflections.
///
/// `std::meta::members_of` applies the requested access context before the semantic predicate runs.
/// Declaration order is preserved, no secondary sort is introduced, and the temporary vector exists only
/// during constant evaluation. This helper is used for categories such as functions/constructors that do not
/// have a single exact standard source query. Compile-time complexity is O(m) for `m` visible members and
/// promoted storage is O(k) for `k` matches.
template <class Predicate>
[[nodiscard]] consteval auto filteredMembers(Info subject, Access access, Predicate predicate)
    -> std::span<const Info> {
  auto result = std::vector<Info>{};
  for (const Info member : std::meta::members_of(subject, access)) {
    if (predicate(member)) {
      result.push_back(member);
    }
  }
  return promoteVector(result);
}

/// Returns whether `access` is the standard unrestricted reflection context.
///
/// `access_context` is a structural standard type whose unchecked spelling is represented by empty
/// scope/designating reflections. Recognizing it lets typed queries return their canonical backing span
/// directly instead of allocating a second, content-identical static array.
[[nodiscard]] consteval auto isUnchecked(Access access) -> bool {
  return access.scope() == Info{} and access.designating_class() == Info{};
}

/// Applies caller-sensitive access to an already-canonical unchecked category universe.
///
/// Declaration order is stable because filtering never reorders the cached sequence. The unchecked path is
/// O(1) and returns the canonical span itself; other access contexts perform one linear `is_accessible` pass
/// and statically promote the visible subset. This preserves the standard access model without repeating the
/// expensive source query.
[[nodiscard]] consteval auto visibleFrom(std::span<const Info> canonical, Access access)
    -> std::span<const Info> {
  if (isUnchecked(access)) {
    return canonical;
  }

  std::vector<Info> result{};
  result.reserve(canonical.size());
  for (const Info item : canonical) {
    if (std::meta::is_accessible(item, access)) {
      result.push_back(item);
    }
  }
  return promoteVector(result);
}

/// Builds the canonical unchecked member universe for one reflected subject.
///
/// The validation here is defensive: public source APIs validate *before* crossing into a variable-template
/// cache. An exception escaping a `constexpr` variable initializer is a hard constant-expression failure and
/// cannot serve as Miracle's catchable semantic-failure channel. Keeping the builder check as well protects
/// private direct use.
[[nodiscard]] consteval auto buildMembers(Info subject) -> std::span<const Info> {
  requireMemberScope(subject, "members() requires a class, union, or namespace reflection");
  return promoteVector(std::meta::members_of(subject, Access::unchecked()));
}

/// Builds the canonical unchecked non-static-data-member universe.
[[nodiscard]] consteval auto buildFields(Info subject) -> std::span<const Info> {
  requireMemberScope(subject, "fields() requires a class or union reflection");
  return promoteVector(std::meta::nonstatic_data_members_of(subject, Access::unchecked()));
}

/// Builds the canonical unchecked static-data-member universe.
[[nodiscard]] consteval auto buildStaticFields(Info subject) -> std::span<const Info> {
  requireRecord(subject, "staticFields() requires a class or union reflection");
  return promoteVector(std::meta::static_data_members_of(subject, Access::unchecked()));
}

/// Builds the canonical unchecked ordinary-function universe, excluding constructors.
[[nodiscard]] consteval auto buildFunctions(Info subject) -> std::span<const Info> {
  requireMemberScope(subject, "functions() requires a class, union, or namespace reflection");
  return filteredMembers(subject, Access::unchecked(), [](Info member) consteval -> bool {
    return (std::meta::is_function(member) or std::meta::is_function_template(member)) and
           not(std::meta::is_constructor(member) or std::meta::is_constructor_template(member));
  });
}

/// Builds the canonical unchecked constructor/constructor-template universe.
[[nodiscard]] consteval auto buildConstructors(Info subject) -> std::span<const Info> {
  requireClass(subject, "bases() requires a class reflection");
  return filteredMembers(subject, Access::unchecked(), [](Info member) consteval -> bool {
    return std::meta::is_constructor(member) or std::meta::is_constructor_template(member);
  });
}

/// Builds the canonical unchecked direct-base universe.
[[nodiscard]] consteval auto buildBases(Info subject) -> std::span<const Info> {
  requireClass(subject, "bases() requires a class reflection");
  return promoteVector(std::meta::bases_of(subject, Access::unchecked()));
}

/// Builds the canonical unchecked class-subobject universe.
///
/// The standard sequence contains direct bases followed by non-static data members, preserving each source
/// category's Declaration order. Only class types are accepted by `std::meta::subobjects_of`.
[[nodiscard]] consteval auto buildSubobjects(Info subject) -> std::span<const Info> {
  requireClass(subject, "subobjects() requires a class reflection");
  return promoteVector(std::meta::subobjects_of(subject, Access::unchecked()));
};

/// Builds the canonical enumerator universe for an enum reflection.
[[nodiscard]] consteval auto buildEnumerators(Info subject) -> std::span<const Info> {
  if (not std::meta::is_type(subject) or not std::meta::is_enum_type(subject)) {
    throw std::meta::exception{"enumerators() requires an enum reflection", subject};
  }
  if (not std::meta::is_enumerable_type(subject)) {
    throw std::meta::exception{"enumerators() requires an enumerable enum definition", subject};
  }
  return promoteVector(std::meta::enumerators_of(subject));
}

/// Builds the canonical all-annotation universe for one reflected subject.
[[nodiscard]] consteval auto buildAnnotations(Info subject) -> std::span<const Info> {
  return promoteVector(std::meta::annotations_of(subject));
}

/// Builds the canonical annotation universe restricted to one annotation object type.
[[nodiscard]] consteval auto buildAnnotationsOfType(Info subject, Info annotationType)
    -> std::span<const Info> {
  return promoteVector(std::meta::annotations_of_with_type(subject, annotationType));
}

/// Reflection source categories with independently lazy canonical cache specializations.
enum class CacheCategory : unsigned char {
  /// Direct members of a class, union, or namespace.
  Members,
  /// Non-static data members of a class or union.
  Fields,
  /// Static data members of a class or union.
  StaticFields,
  /// Ordinary functions/function templates, excluding constructors.
  Functions,
  /// Constructors and constructor templates of a class.
  Constructors,
  /// Direct base-specifier reflection of a class.
  Bases,
  /// Direct bases and non-static data members of a class.
  Subobjects,
  /// Enumerators of an enumerable enum reflection.
  Enumerators,
  /// Parameters of a function declaration, specialization, or function type.
  Parameters,
  /// Annotation reflections attached to an entity.
  Annotations,
  /// Template arguments carried by a specialization-like reflection.
  TemplateArguments,
};

/// Validates one value-oriented cache request before any NTTP cache specialization is named.
///
/// This is deliberately redundant with the public source preconditions and the defensive builders. The public
/// checks provide operation-specific diagnostic; this bridge-level check is the fail-closed invariant that
/// prevents a future internal caller from letting `std::meta::exception` escape a `constexpr` cache
/// initializer, where it would become a hard constant-expression failure instead of a catchable semantic
/// flow.
consteval auto validateCacheRequest(Info subject, CacheCategory category) -> void {
  switch (category) {
    case CacheCategory::Members:
      requireMemberScope(subject, "members() requires a class, union, or namespace reflection");
      return;
    case CacheCategory::Functions:
      requireMemberScope(subject, "functions() requires a class, union, or namespace reflection");
      return;
    case CacheCategory::Fields:
      requireRecord(subject, "functions() requires a class or union reflection");
      return;
    case CacheCategory::StaticFields:
      requireRecord(subject, "staticFields() requires a class or union reflection");
      return;
    case CacheCategory::Constructors:
      requireClass(subject, "constructors() requires a class reflection");
      return;
    case CacheCategory::Bases: requireClass(subject, "bases() requires a class reflection"); return;
    case CacheCategory::Subobjects: requireClass(subject, "subobjects() requires a class reflection"); return;
    case CacheCategory::Enumerators:
      if (not std::meta::is_type(subject) or not std::meta::is_enum_type(subject)) {
        throw std::meta::exception{"enumerators() requires an enum reflection", subject};
      }
      if (not std::meta::is_enumerable_type(subject)) {
        throw std::meta::exception{"enumerators() requires an enumerable enum definition", subject};
      }
      return;
    case CacheCategory::Parameters: {
      const bool functionType = std::meta::is_type(subject) and std::meta::is_function_type(subject);
      if (std::meta::is_function_template(subject)) {
        throw std::meta::exception{
            "parameters() requires a function declaration/specialization, not a function-template reflection",
            subject};
      }
      if (not std::meta::is_function(subject) and not functionType) {
        throw std::meta::exception{"parameters() requires a function declaration or function type", subject};
      }
      return;
    }
    case CacheCategory::TemplateArguments:
      if (not std::meta::has_template_arguments(subject)) {
        throw std::meta::exception{
            "templateArguments() requires a reflection with template arguments", subject};
      }
      return;
    case CacheCategory::Annotations: return;
  }
}

/// Builds exactly one requested source category for `Subject`.
///
/// A `(Subject, Category)` specialization is the unit of laziness: the `if constexpr` chain discards every
/// unrelated source query before instantiation. This prevents asking for fields from forcing
/// functions/bases/annotations and keeps compiler work proportional to the categories a translation unit
/// actually consumes.
template <Info Subject, CacheCategory Category>
[[nodiscard]] consteval auto buildCategory() -> std::span<const Info> {
  if constexpr (Category == CacheCategory::Members) {
    return buildMembers(Subject);
  } else if constexpr (Category == CacheCategory::Fields) {
    return buildFields(Subject);
  } else if constexpr (Category == CacheCategory::StaticFields) {
    return buildStaticFields(Subject);
  } else if constexpr (Category == CacheCategory::Functions) {
    return buildFunctions(Subject);
  } else if constexpr (Category == CacheCategory::Constructors) {
    return buildConstructors(Subject);
  } else if constexpr (Category == CacheCategory::Bases) {
    return buildBases(Subject);
  } else if constexpr (Category == CacheCategory::Subobjects) {
    return buildSubobjects(Subject);
  } else if constexpr (Category == CacheCategory::Enumerators) {
    return buildEnumerators(Subject);
  } else if constexpr (Category == CacheCategory::Parameters) {
    const bool functionType = std::meta::is_type(Subject) and std::meta::is_function_type(Subject);
    if (not std::meta::is_function(Subject) and not functionType) {
      throw std::meta::exception{"parameters() requires a function declaration of function type", Subject};
    }
    return promoteVector(std::meta::parameters_of(Subject));
  } else if constexpr (Category == CacheCategory::Annotations) {
    return buildAnnotations(Subject);
  } else {
    static_assert(Category == CacheCategory::TemplateArguments);
    if (not std::meta::has_template_arguments(Subject)) {
      throw std::meta::exception{
          "templateArguments() requires a reflection with template arguments", Subject};
    }
    return promoteVector(std::meta::template_arguments_of(Subject));
  }
}

/// Canonical static backing for one valid reflected subject/category pair.
///
/// `std::meta::info` is intentionally used as an NTTP only here, at the private cache boundary. Repeated
/// type-rooted queries and raw-Info queries that resolve to the same subject/category specialization
/// therefore share one reflected universe rather than rebuilding `std::vector<Info>` state on each call.
/// Public APIs must establish semantic validity before naming this specialization so invalid input can still
/// throw/catch `std::meta::exception` normally.
template <Info Subject, CacheCategory Category>
inline constexpr std::span<const Info> categoryCache = buildCategory<Subject, Category>();

/// Structural pointer/size representation used to cross the reflective invocation bridge.
///
/// `std::span` itself is not structural and therefore cannot be the result of `std::meta::reflect_invoke`.
/// This tiny representation is structural, contains only a pointer into canonical static storage plus its
/// extent, and converts back to `span` immediately; it is never exposed publicly.
struct InfoSequenceRef final {
  const Info *data{};
  std::size_t size{};

  constexpr auto operator==(const InfoSequenceRef &) const -> bool = default;
};

/// Returns a structural reference to a concrete category-cache specialization.
template <Info Subject, CacheCategory Category>
[[nodiscard]] consteval auto categoryCacheRef() -> InfoSequenceRef {
  constexpr auto cached = categoryCache<Subject, Category>;
  return {.data = cached.data(), .size = cached.size()};
}

/// Bridges a value-oriented `Info` subject into the canonical NTTP cache without exposing template-state
/// publicly.
///
/// C++ function parameters cannot be used directly as NTTPs even inside an immediate function. C++26
/// reflection provides a zero-runtime bridge: reflect the subject/category constants, substitute them into
/// `categoryCacheRef`, extract that consteval specialization as a function pointer, and call it directly.
/// This lets `meta::fields(^^T)` and `meta::fields<T>()` converge on the exact same cache specialization
/// while the public API stays value-oriented. The bridge performs no reflection source query itself and
/// assumes the public source API has already validated `subject`; validation cannot be deferred into a
/// failing cache variable initializer without changing error semantics.
[[nodiscard]] consteval auto cachedCategory(Info subject, CacheCategory category) -> std::span<const Info> {
  validateCacheRequest(subject, category);
  const Info specialization = std::meta::substitute(^^categoryCacheRef,
      {
          std::meta::reflect_constant(subject), std::meta::reflect_constant(category)});
  using Evaluator = std::function<InfoSequenceRef()>;
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  const auto result = evaluator();
  return {result.data, result.size};
}

/// Statically backed visible subset for one canonical subject/category/access triple.
///
/// Access is part of the private cache key because it is observable reflection semantics. Repeated queries
/// from the same caller context therefore pay the linear accessibility filter once, while unrelated access
/// contexts remain isolated. This intentionally trades one specialization per *used* access context for
/// eliminating repeated `is_accessible` passes; Compiler-cost benchmarks verify that the trade is profitable
/// on the reference toolchain.
template <Info Subject, CacheCategory Category, Access Context>
inline constexpr std::span<const Info> visibleCategoryCache =
    visibleFrom(categoryCache<Subject, Category>, Context);

/// Returns a structural reference to one access-filtered cache specialization.
template <Info Subject, CacheCategory Category, Access Context>
[[nodiscard]] consteval auto visibleCategoryCacheRef() -> InfoSequenceRef {
  constexpr auto cached = visibleCategoryCache<Subject, Category, Context>;
  return {.data = cached.data(), .size = cached.size()};
}

/// Bridges value-oriented subject/category/access state into the access-filtered NTTP cache.
///
/// `access_context` is structural in C++26, so the same reflection/substitution technique used by
/// `cachedCategory` can specialize by caller access without exposing template-state publicly. Unchecked
/// access bypasses this bridge entirely because the canonical cache is already the exact requested universe.
/// The specialization is extracted and called directly rather than routed through `reflect_invoke`; this is
/// measurably cheaper on the pinned reference snapshot.
[[nodiscard]] consteval auto cachedVisibleCategory(Info subject, CacheCategory category, Access access)
    -> std::span<const Info> {
  if (isUnchecked(access)) {
    return cachedCategory(subject, category);
  }

  validateCacheRequest(subject, category);
  const Info specialization = std::meta::substitute(^^visibleCategoryCacheRef,
      {
          std::meta::reflect_constant(subject),
          std::meta::reflect_constant(category),
          std::meta::reflect_constant(access)});
  using Evaluator = std::function<InfoSequenceRef()>;
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  const auto result = evaluator();
  return {result.data, result.size};
}

/// Type-oriented access-filtered cache lookup.
///
/// Keeping this helper separate preserves the fast direct NTTP path for typed unchecked queries while sending
/// all other access contexts through the same access-cache specialization used by raw-`Info` calls.
template <class T, CacheCategory Category>
[[nodiscard]] consteval auto cachedVisibleCategory(Access access) -> std::span<const Info> {
  if constexpr (not std::meta::is_complete_type(^^T)) {
    throw std::meta::exception{"reflection member source requires a complete class or union type", ^^T};
  } else {
    if (isUnchecked(access)) {
      return categoryCache<^^T, Category>;
    }
    return cachedVisibleCategory(^^T, Category, access);
  }
}

/// Canonical typed-annotation backing keyed by both reflected subject and annotation object type.
template <Info Subject, Info AnnotationType>
inline constexpr std::span<const Info> typedAnnotationsCache =
    buildAnnotationsOfType(Subject, AnnotationType);

/// Returns a structural reference to a concrete typed-annotation cache specialization.
template <Info Subject, Info AnnotationType>
[[nodiscard]] consteval auto typedAnnotationsCacheRef() -> InfoSequenceRef {
  constexpr auto cached = typedAnnotationsCache<Subject, AnnotationType>;
  return {.data = cached.data(), .size = cached.size()};
}

/// Value-oriented bridge into the `(subject, annotation-type)` NTTP cache.
[[nodiscard]] consteval auto cachedAnnotationsOfType(Info subject, Info annotationType)
    -> std::span<const Info> {
  if (not std::meta::is_type(annotationType)) {
    throw std::meta::exception{
        "annotations<A>() requires an annotation object type reflection", annotationType};
  }
  const Info specialization = std::meta::substitute(^^typedAnnotationsCacheRef,
      {
          std::meta::reflect_constant(subject), std::meta::reflect_constant(annotationType)});
  using Evaluator = std::function<InfoSequenceRef()>;
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  const auto result = evaluator();
  return {result.data, result.size};
}

} // namespace Miracle::meta
