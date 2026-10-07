export module Miracle.Meta;

import std;

export namespace Miracle {

/// Caller-sensitive access policy forwarded to C++26 reflection queries.
///
/// Miracle deliberately aliases the standard type instead of wrapping it so callers can use the full standard
/// access vocabulary (`current`, `unchecked`, `via`, and future additions) without conversion or adapter
/// state.
using Access = std::meta::access_context;

} // namespace Miracle

export namespace Miracle::meta {

/// Fundamental C++26 reflection value used throughout Miracle.Meta.
///
/// `Info` is an alias rather than a wrapper: reflection identity, splicing, NTTP use, and direct
/// interoperation with `<meta>` remain exactly the standard semantics.
using Info = std::meta::info;

} // namespace Miracle::meta

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
/// Namespaces and record types share the `members()` / `functions()` source path; keeping this predicate
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
  requireRecord(subject, "fields() requires a class or union reflection");
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
  requireClass(subject, "constructors() requires a class reflection");
  return filteredMembers(subject, Access::unchecked(), [](Info member) consteval -> bool {
    return std::meta::is_constructor(member) or std::meta::is_constructor_template(member);
  });
}

/// Builds the canonical unchecked direct-base universe.
[[nodiscard]] consteval auto buildBases(Info subject) -> std::span<const Info> {
  requireClass(subject, "bases() requires a class reflection");
  return promoteVector(std::meta::bases_of(subject, Access::unchecked()));
}

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
  Members,
  Fields,
  StaticFields,
  Functions,
  Constructors,
  Bases,
  Enumerators,
  Parameters,
  Annotations,
  TemplateArguments,
};

/// Validates one value-oriented cache request before any NTTP cache specialization is named.
///
/// This is deliberately redundant with the public source preconditions and the defensive builders. The public
/// checks provide operation-specific diagnostics; this bridge-level check is the fail-closed invariant that
/// prevents a future internal caller from letting `std::meta::exception` escape a `constexpr` cache
/// initializer, where it would become a hard constant-expression failure instead of catchable semantic flow.
consteval auto validateCacheRequest(Info subject, CacheCategory category) -> void {
  if (category == CacheCategory::Members or category == CacheCategory::Functions) {
    requireMemberScope(subject,
        category == CacheCategory::Members ? "members() requires a class, union, or namespace reflection"
                                           : "functions() requires a class, union, or namespace reflection");
    return;
  }
  if (category == CacheCategory::Fields or category == CacheCategory::StaticFields) {
    requireRecord(subject,
        category == CacheCategory::Fields ? "fields() requires a class or union reflection"
                                          : "staticFields() requires a class or union reflection");
    return;
  }
  if (category == CacheCategory::Constructors) {
    requireClass(subject, "constructors() requires a class reflection");
    return;
  }
  if (category == CacheCategory::Bases) {
    requireClass(subject, "bases() requires a class reflection");
    return;
  }
  if (category == CacheCategory::Enumerators) {
    if (not std::meta::is_type(subject) or not std::meta::is_enum_type(subject)) {
      throw std::meta::exception{"enumerators() requires an enum reflection", subject};
    }
    if (not std::meta::is_enumerable_type(subject)) {
      throw std::meta::exception{"enumerators() requires an enumerable enum definition", subject};
    }
    return;
  }
  if (category == CacheCategory::Parameters) {
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
  if (category == CacheCategory::TemplateArguments and not std::meta::has_template_arguments(subject)) {
    throw std::meta::exception{"templateArguments() requires a reflection with template arguments", subject};
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
  } else if constexpr (Category == CacheCategory::Enumerators) {
    return buildEnumerators(Subject);
  } else if constexpr (Category == CacheCategory::Parameters) {
    const bool functionType = std::meta::is_type(Subject) and std::meta::is_function_type(Subject);
    if (not std::meta::is_function(Subject) and not functionType) {
      throw std::meta::exception{"parameters() requires a function declaration or function type", Subject};
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
/// Public APIs must establish semantic validity before naming this specialization so invalid input can
/// still throw/catch `std::meta::exception` normally.
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
  using Evaluator = InfoSequenceRef (*)();
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  const auto result = evaluator();
  return {result.data, result.size};
}

/// Statically backed visible subset for one canonical subject/category/access triple.
///
/// Access is part of the private cache key because it is observable reflection semantics. Repeated queries
/// from the same caller context therefore pay the linear accessibility filter once, while unrelated access
/// contexts remain isolated. This intentionally trades one specialization per *used* access context for
/// eliminating repeated `is_accessible` passes; Compiler-cost benchmarks verify that the trade is
/// profitable on the reference toolchain.
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
/// access bypasses this bridge entirely because the canonical cache is already the exact requested
/// universe. The specialization is extracted and called directly rather than routed through `reflect_invoke`;
/// this is measurably cheaper on the pinned reference snapshot.
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
  using Evaluator = InfoSequenceRef (*)();
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  const auto result = evaluator();
  return {result.data, result.size};
}

/// Type-oriented access-filtered cache lookup.
///
/// Keeping this helper separate preserves the fast direct NTTP path for typed unchecked queries while
/// sending all other access contexts through the same access-cache specialization used by raw-`Info` calls.
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

/// value-oriented bridge into the `(subject, annotation-type)` NTTP cache.
[[nodiscard]] consteval auto cachedAnnotationsOfType(Info subject, Info annotationType)
    -> std::span<const Info> {
  if (not std::meta::is_type(annotationType)) {
    throw std::meta::exception{
        "annotations<A>() requires an annotation object type reflection", annotationType};
  }
  const Info specialization = std::meta::substitute(^^typedAnnotationsCacheRef,
      {
          std::meta::reflect_constant(subject), std::meta::reflect_constant(annotationType)});
  using Evaluator = InfoSequenceRef (*)();
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  const auto result = evaluator();
  return {result.data, result.size};
}

/// Structural marker used only to constrain the predicate-composition operators.
///
/// A marker avoids inheritance or a public predicate base class: any data-oriented callable carrying this
/// member can participate in `!`, `&&`, and `||` composition without runtime polymorphism.
template <class T>
inline constexpr bool metaPredicate = requires { std::remove_cvref_t<T>::miracleMetaPredicate_; } and
                                      std::remove_cvref_t<T>::miracleMetaPredicate_;

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Identity/escape-hatch projection for obtaining the underlying standard reflection value.
///
/// Passing `Info` is a no-op. Objects exposing `raw()` (notably `Reflect<T>`) are unwrapped without
/// introducing another reflection representation. This operation cannot fail and does not create storage.
struct Raw final {
  /// Returns `info` unchanged.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return info;
  }

  /// Returns `value.raw()` for façade-like objects exposing the standard reflection identity.
  template <class T>
    requires requires(const T &value) { value.raw(); }
  [[nodiscard]] consteval auto operator()(const T &value) const -> Info {
    return value.raw();
  }
};
/// Stateless callable instance of `Raw`, suitable for direct calls and future Meta Query projections.
inline constexpr Raw raw{};

/// Optional declared-identifier projection.
///
/// Returns `nullopt` for reflections without an identifier instead of triggering the standard precondition on
/// `identifier_of`. The returned `string_view` refers to compiler-managed static reflection text.
struct Name final {
  /// Returns the declared identifier when one exists; otherwise returns an empty optional.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::optional<std::string_view> {
    if (not std::meta::has_identifier(info)) {
      return std::nullopt;
    }
    return std::meta::identifier_of(info);
  }
};
/// Stateless callable instance of `Name`.
inline constexpr Name name{};

/// Strict declared-identifier projection.
///
/// Unlike `name`, absence is a semantic error: constant evaluation throws `std::meta::exception` carrying
/// the offending reflection. This makes the operation appropriate for pipelines whose contract requires a
/// source identifier.
struct RequireName final {
  /// Returns the declared identifier or throws `std::meta::exception` when the reflection has none.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::string_view {
    if (not std::meta::has_identifier(info)) {
      throw std::meta::exception{"requireName() requires a reflection with an identifier", info};
    }
    return std::meta::identifier_of(info);
  }
};
/// Stateless callable instance of `RequireName`.
inline constexpr RequireName requireName{};

/// Descriptive reflection spelling intended for diagnostics rather than source-identifier logic.
///
/// Delegates to `std::meta::display_string_of`, so entities without identifiers still receive a printable
/// description.
struct DisplayName final {
  /// Returns the standard descriptive spelling of `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::string_view {
    return std::meta::display_string_of(info);
  }
};
/// Stateless callable instance of `DisplayName`.
inline constexpr DisplayName displayName{};

/// Projects a reflection to its reflected type.
///
/// Type reflections are returned unchanged; other supported entities delegate to `std::meta::type_of`.
/// Standard semantic failures propagate as compile-time reflection failures rathr than being converted into
/// sentinel values.
struct Type final {
  /// Returns `info` for type reflections, otherwise returns the reflected entity's type.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (std::meta::is_type(info)) {
      return info;
    }
    return std::meta::type_of(info);
  }
};
/// Stateless callable instance of `Type`.
inline constexpr Type type{};

/// Projects a function/function-type reflection to its return type.
///
/// Accepts reflected functions and reflected function types. Any other subject throws `std::meta::exception`
/// before the lower-level standard precondition is evaluated, preserving Miracle's stable failure boundary.
struct ReturnType final {
  /// Returns the reflected return type of a function or function type.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    const bool functionType = std::meta::is_type(info) and std::meta::is_function_type(info);
    if (not std::meta::is_function(info) and not functionType) {
      throw std::meta::exception{"returnType() requires a function reflection", info};
    }
    return std::meta::return_type_of(info);
  }
};
/// Stateless callable instance of `ReturnType`.
inline constexpr ReturnType returnType{};

/// Returns the lexical/semantic parent reflection of an entity.
///
/// Subjects without a parent are rejected with `std::meta::exception` carrying the original reflection.
struct Parent final {
  /// Returns the reflected parent or throws when the subject has no parent.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::has_parent(info)) {
      throw std::meta::exception{"parent() requires a reflection with a parent", info};
    }
    return std::meta::parent_of(info);
  }
};
/// Stateless callable instance of `Parent`.
inline constexpr Parent parent{};

/// Resolves a standard reflection alias/proxy to its canonical underlying entity.
struct Dealias final {
  /// Applies the standard canonical dealiasing operation to `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::dealias(info);
  }
};
/// Stateless callable instance of `Dealias`.
inline constexpr Dealias dealias{};

/// Removes top-level const/volatile qualification from a reflected type.
///
/// Validation and structural trait discovery frequently need to normalize a member/parameter type without
/// changing its reference category. Alias removal remains an explicit preceding `dealias` step so callers do
/// not accidentally conflate two different semantic transformations. Non-type reflections are rejected with
/// Miracle's ordinary strict reflection failure.
struct RemoveCv final {
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::is_type(info)) {
      throw std::meta::exception{"removeCv() requires a type reflection", info};
    }
    return std::meta::remove_cv(info);
  }
};
/// Stateless callable instance of `RemoveCv`.
inline constexpr RemoveCv removeCv{};

/// Removes one reference layer from a reflected type while preserving the cv qualification of the referred
/// type.
struct RemoveReference final {
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::is_type(info)) {
      throw std::meta::exception{"removeReference() requires a type reflection", info};
    }
    return std::meta::remove_reference(info);
  }
};
/// Stateless callable instance of `RemoveReference`.
inline constexpr RemoveReference removeReference{};

/// Removes both reference and top-level cv qualification from a reflected type.
///
/// This is the canonical shape-discovery normalization used by later Traits: it is deliberately separate from
/// `dealias`, because an alias identity and cvref normalization are independently observable reflection
/// choices.
struct RemoveCvref final {
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::is_type(info)) {
      throw std::meta::exception{"removeCvref() requires a type reflection", info};
    }
    return std::meta::remove_cvref(info);
  }
};
/// Stateless callable instance of `RemoveCvref`.
inline constexpr RemoveCvref removeCvref{};

/// Projects an enum type reflection to its underlying integral type.
///
/// Traits needs this as a primitive for enum-shape/capability discovery. The operation intentionally accepts
/// only enum *type* reflections; enumerator declarations remain ordinary entities and can first be projected
/// through `meta::type` when that is the caller's intent.
struct UnderlyingType final {
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::is_type(info) or not std::meta::is_enum_type(info)) {
      throw std::meta::exception{"underlyingType() requires an enum type reflection", info};
    }
    return std::meta::underlying_type(info);
  }
};
/// Stateless callable instance of `UnderlyingType`.
inline constexpr UnderlyingType underlyingType{};

/// Returns the originating template for a reflection that carries template arguments.
///
/// Non-template-instantitation subjects are rejected with `std::meta::exception`; callers asking only whether
/// a subject is templated should use an appropriate predicate instead of exception-driven probing.
struct TemplateOf final {
  /// Returns the originating template of an instantiated/template-argument-bearing reflection.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    if (not std::meta::has_template_arguments(info)) {
      throw std::meta::exception{"templateOf() requires a reflection with template arguments", info};
    }
    return std::meta::template_of(info);
  }
};
/// Stateless callable instance of `TemplateOf`.
inline constexpr TemplateOf templateOf{};

/// Non-throwing probe for whether `arguments` can instantiate the reflected template `templ`.
///
/// The argument range must yield `Info` values. No argument storage is retained after constant evaluation.
struct CanSubstitute final {
  /// Returns whether `arguments` form a valid substitution for `templ` without throwing on substitution
  /// failure.
  template <std::ranges::input_range Range>
    requires std::same_as<std::remove_cvref_t<std::ranges::range_value_t<Range>>, Info>
  [[nodiscard]] consteval auto operator()(Info templ, Range &&arguments) const -> bool {
    return std::meta::can_substitute(templ, std::forward<Range>(arguments));
  }
};
/// Stateless callable instance of `CanSubstitute`.
inline constexpr CanSubstitute canSubstitute{};

/// Instantiates a reflected template from a range of reflection arguments.
///
/// Invalid substitution deliberately propagates `std::meta::exception`, enabling both strict diagnostics and
/// the speculative consteval `try`/`catch` control-flow model defined for Meta.
struct Substitute final {
  /// Returns the instantiated reflection; invalid substitution propagates `std::meta::exception`.
  template <std::ranges::input_range Range>
    requires std::same_as<std::remove_cvref_t<std::ranges::range_value_t<Range>>, Info>
  [[nodiscard]] consteval auto operator()(Info templ, Range &&arguments) const -> Info {
    return std::meta::substitute(templ, std::forward<Range>(arguments));
  }
};
/// Stateless callable instance of `Substitute`.
inline constexpr Substitute substitute{};

/// Converts an entity/annotation reflection to the reflection of its constant value.
///
/// This is intentionally a callable projection so annotation queries can compose as `.map(meta::constant)` in
/// Query.
struct Constant final {
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::constant_of(info);
  }
};
/// Stateless callable instance of `Constant`.
inline constexpr Constant constant{};

/// Typed extraction projection for a reflected constant.
///
/// `T` is the requested C++ value type. Standard extraction preconditions are preserved and diagnosed during
/// constant evaluation; the callable stores no state.
template <class T>
struct Extract final {
  /// Extracts the reflected constant as `T`.
  [[nodiscard]] consteval auto operator()(Info info) const -> T {
    return std::meta::extract<T>(info);
  }
};

/// Lowercase callable variable template used as `meta::extract<T>(info)` or a future Query projection.
template <class T>
inline constexpr Extract<T> extract{};

/// Returns the source location associated with a reflected declaration/entity.
///
/// The result is the standard `std::source_location` and can be fed directly into Miracle diagnostics in
/// future diagnostic work.
struct SourceLocation final {
  /// Returns the standard source location attached to `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> std::source_location {
    return std::meta::source_location_of(info);
  }
};
/// Stateless callable instance of `SourceLocation`.
inline constexpr SourceLocation sourceLocation{};

/// Returns the direct members of a class, union, or namespace in declaration order.
///
/// `access` is evaluated at the public call site and controls which access-sensitive members are visible. The
/// returned span has static backing storage and is valid for the remainder of the program. Invalid subjects
/// throw `std::meta::exception` with `subject` attached. The raw reflection is bridged into the same private
/// NTTP-backed canonical member universe used by type-oriented and `Reflect<T>` queries; only caller-access
/// filtering is per call.
[[nodiscard]] consteval auto members(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireMemberScope(subject, "members() requires a class, union, or namespace reflection");
  return cachedVisibleCategory(subject, CacheCategory::Members, access);
}

/// Type-oriented overload of `members(Info, Access)` backed by the canonical per-type member cache.
///
/// `Reflect<T>::members()` delegates here as well, and the raw-`Info` overload reaches the same
/// specialization through the private reflective bridge. The cache itself is instantiated only when this
/// category is requested for `T`.
template <class T>
[[nodiscard]] consteval auto members(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::is_class_v<T> and not std::is_union_v<T>) {
    throw std::meta::exception{"members() requires a class or union type", ^^T};
  } else {
    return cachedVisibleCategory<T, CacheCategory::Members>(access);
  }
}

/// Returns non-static data members visible to the supplied access context.
///
/// Only class/union reflection are valid. Declaration order is preserved, the result has static backing
/// storage, and an invalid subject throws `std::meta::exception`.
[[nodiscard]] consteval auto fields(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireRecord(subject, "fields() requires a class or union reflection");
  return cachedVisibleCategory(subject, CacheCategory::Fields, access);
}

/// Type-oriented overload of backed by independently lazy canonical field cache for `T`.
template <class T>
[[nodiscard]] consteval auto fields(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::is_class_v<T> and not std::is_union_v<T>) {
    throw std::meta::exception{"fields() requires a class or union type", ^^T};
  } else {
    return cachedVisibleCategory<T, CacheCategory::Fields>(access);
  }
}

/// Returns static data members visible to the supplied access context.
///
/// Only class/union reflection are valid. Declaration order is preserved, the result has static backing
/// storage, and an invalid subject throws `std::meta::exception`.
[[nodiscard]] consteval auto staticFields(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireRecord(subject, "staticFields() requires a class or union reflection");
  return cachedVisibleCategory(subject, CacheCategory::StaticFields, access);
}

/// Type-oriented overload backed by the independently lazy canonical static-field cache for `T`.
template <class T>
[[nodiscard]] consteval auto staticFields(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::is_class_v<T> and not std::is_union_v<T>) {
    throw std::meta::exception{"staticFields() requires a class or union type", ^^T};
  } else {
    return cachedVisibleCategory<T, CacheCategory::StaticFields>(access);
  }
}

/// Returns ordinary functions/function templates, excluding constructors.
///
/// Accepts namespaces, classes, and unions. The standard member sequence is filtered once, preserving
/// declaration order and `access`; invalid subjects throw `std::meta::exception`.
[[nodiscard]] consteval auto functions(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireMemberScope(subject, "functions() requires a class, union, or namespace reflection");
  return cachedVisibleCategory(subject, CacheCategory::Functions, access);
}

/// Type-oriented overload of `functions(Info, Access)` reflecting the scope represented by `T`.
template <class T>
[[nodiscard]] consteval auto functions(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::is_class_v<T> and not std::is_union_v<T>) {
    throw std::meta::exception{"functions() requires a class or union type", ^^T};
  } else {
    return cachedVisibleCategory<T, CacheCategory::Functions>(access);
  }
}

/// Returns constructors and constructor templates visible to the supplied access context.
///
/// Only class reflections are valid; unions and namespaces are rejected. Results preserve declaration order
/// and use static backing storage.
[[nodiscard]] consteval auto constructors(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireClass(subject, "constructors() requires a class reflection");
  return cachedVisibleCategory(subject, CacheCategory::Constructors, access);
}

/// Type-oriented overload backed by the independently lazy canonical constructor cache for `T`.
template <class T>
[[nodiscard]] consteval auto constructors(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::is_class_v<T>) {
    throw std::meta::exception{"constructors() requires a class type", ^^T};
  } else {
    return cachedVisibleCategory<T, CacheCategory::Constructors>(access);
  }
}

/// Returns direct base-specifier reflections in declaration order.
///
/// Only class reflections are valid. `access` determines which base specifiers are visible; no
/// transitive-base expansion or reordering is performed.
[[nodiscard]] consteval auto bases(Info subject, Access access = Access::current()) -> std::span<const Info> {
  requireClass(subject, "bases() requires a class reflection");
  return cachedVisibleCategory(subject, CacheCategory::Bases, access);
}

/// Type-oriented overload of `bases(Info, Access)` for class type `T`.
template <class T>
[[nodiscard]] consteval auto bases(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::is_class_v<T>) {
    throw std::meta::exception{"bases() requires a class type", ^^T};

  } else {
    return cachedVisibleCategory<T, CacheCategory::Bases>(access);
  }
}

/// Returns enumerators of an enum in declaration order.
///
/// `subject` must reflect an enum type. The result has static backing storage; non-enum subjects throw
/// `std::meta::exception` rather than leaking the lower-level `<meta>` precondition.
[[nodiscard]] consteval auto enumerators(Info subject) -> std::span<const Info> {
  if (not std::meta::is_type(subject) or not std::meta::is_enum_type(subject)) {
    throw std::meta::exception{"enumerators() requires an enum reflection", subject};
  }
  if (not std::meta::is_enumerable_type(subject)) {
    throw std::meta::exception{"enumerators() requires an enumerable enum definition", subject};
  }
  return cachedCategory(subject, CacheCategory::Enumerators);
}

/// Type-oriented enum source. The constraint makes non-enum misuse an overload-formation error instead of a
/// runtime-like semantic probe; use the raw-`Info` overload when the reflected category is only known during
/// constant evaluation.
template <class E>
  requires std::is_enum_v<E>
[[nodiscard]] consteval auto enumerators() -> std::span<const Info> {
  if constexpr (not std::meta::is_enumerable_type(^^E)) {
    throw std::meta::exception{"enumerators() requires an enumerable enum definition", ^^E};
  } else {
    return categoryCache<^^E, CacheCategory::Enumerators>;
  }
}

/// Parameter-source callable for reflected function declarations/specializations and function types.
///
/// A function-template reflection itself has no concrete parameter declaration sequence in the C++26
/// parameter-reflection facility; reflect a specialization (for example `^^fn<int>`) when concrete parameters
/// are required. Results preserve source order and use static backing storage. Other reflection categories
/// throw `std::meta::exception`. A callable object is used instead of a plain function so a later Miracle can
/// pass `meta::parameters` directly to `flatMap` without changing the established call syntax.
struct Parameters final {
  /// Returns the statically backed parameter sequence for `subject`.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::span<const Info> {
    const bool functionType = std::meta::is_type(subject) and std::meta::is_function_type(subject);
    if (std::meta::is_function_template(subject)) {
      throw std::meta::exception{
          "parameters() requires a function declaration/specialization, not a function-template reflection",
          subject};
    }
    if (not std::meta::is_function(subject) and not functionType) {
      throw std::meta::exception{"parameters() requires a function declaration or function type", subject};
    }
    return cachedCategory(subject, CacheCategory::Parameters);
  }
};
/// Stateless callable parameter source used as `meta::parameters(info)` and as a future Query projection.
inline constexpr Parameters parameters{};

/// Returns all annotations attached to a reflected entity in source order.
///
/// Each occurrence remains a distinct reflection even when two annotation values compare equally. The
/// returned span has static backing storage; value-level deduplication is intentionally left to the future
/// Query algebra.
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
  return typedAnnotationsCache<^^T, ^^A>;
}

/// Template-argument source callable preserving source order and exact reflection identity.
///
/// Subjects without template arguments throw `std::meta::exception`. The callable form is intentional: Query
/// can use `meta::templateArguments` directly in `map`/`flatMap` without introducing another wrapper.
struct TemplateArguments final {
  /// Returns the statically backed template-argument sequence for `subject`.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::span<const Info> {
    if (not std::meta::has_template_arguments(subject)) {
      throw std::meta::exception{
          "templateArguments() requires a reflection with template arguments", subject};
    }
    return cachedCategory(subject, CacheCategory::TemplateArguments);
  }
};
/// Stateless callable template-argument source.
inline constexpr TemplateArguments templateArguments{};

/// Data-oriented wrapper for a consteval reflection predicate.
///
/// Predicate state (including constant-evaluable captures) is stored directly with empty-state optimization.
/// The marker enables structural composition without inheritance, virtual dispatch, or a public predicate
/// base class.
template <class Function>
struct Predicate final {
  /// Structural tag consumed by the private composition constraint; it carries no runtime state.
  static constexpr bool miracleMetaPredicate_{true};

  /// Concrete predicate callable/capture state; empty callables consume no storage.
  [[no_unique_address]] Function function;

  /// Evaluates the predicate for one reflection and normalizes the result to `bool`.
  [[nodiscard]] consteval auto operator()(Info info) const -> bool {
    return static_cast<bool>(function(info));
  }
};

/// Deduction guide preserving the concrete callable/capture type as predicate value state.
template <class Function>
Predicate(Function) -> Predicate<Function>;

/// Negates a Meta predicate while retaining its captured state by value.
template <class PredicateType>
  requires metaPredicate<PredicateType>
[[nodiscard]] consteval auto operator!(PredicateType predicate) {
  return Predicate{[predicate](Info info) consteval -> bool { return not predicate(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical AND.
template <class Left, class Right>
  requires(metaPredicate<Left> and metaPredicate<Right>)
[[nodiscard]] consteval auto operator&&(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) and right(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical OR.
template <class Left, class Right>
  requires(metaPredicate<Left> and metaPredicate<Right>)
[[nodiscard]] consteval auto operator||(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) or right(info); }};
}

/// Matches reflections carrying a declared identifier.
///
/// Use this (or the optional `meta::name` projection) for ordinary Boolean branching; `meta::requireName` is
/// the strict semantic operation for callers that require an identifier and want a diagnostic failure.
inline constexpr Predicate hasIdentifier{
    [](Info info) consteval -> bool { return std::meta::has_identifier(info); }};
/// Matches reflections with an associated semantic/lexical parent.
inline constexpr Predicate hasParent{[](Info info) consteval -> bool { return std::meta::has_parent(info); }};
/// Matches reflections carrying a recoverable template-argument sequence.
inline constexpr Predicate hasTemplateArguments{
    [](Info info) consteval -> bool { return std::meta::has_template_arguments(info); }};
/// Matches reflections denoting C++ types.
inline constexpr Predicate isType{[](Info info) consteval -> bool { return std::meta::is_type(info); }};
/// Matches complete reflected types. Non-type reflections are simply false.
inline constexpr Predicate isComplete{[](Info info) consteval -> bool {
  return std::meta::is_type(info) and std::meta::is_complete_type(info);
}};
/// Matches types whose member/enumerator set is available for enumeration.
inline constexpr Predicate isEnumerable{[](Info info) consteval -> bool {
  return std::meta::is_type(info) and std::meta::is_enumerable_type(info);
}};
/// Matches reflections denoting class types.
inline constexpr Predicate isClass{
    [](Info info) consteval -> bool { return std::meta::is_type(info) and std::meta::is_class_type(info); }};
/// Matches reflections denoting union types.
inline constexpr Predicate isUnion{
    [](Info info) consteval -> bool { return std::meta::is_type(info) and std::meta::is_union_type(info); }};
/// Matches reflections denoting enum types.
inline constexpr Predicate isEnum{
    [](Info info) consteval -> bool { return std::meta::is_type(info) and std::meta::is_enum_type(info); }};
/// Matches function declaration reflections, excluding function templates.
inline constexpr Predicate isFunction{
    [](Info info) consteval -> bool { return std::meta::is_function(info); }};
/// Matches reflections denoting function types rather than function declarations.
inline constexpr Predicate isFunctionType{[](Info info) consteval -> bool {
  return std::meta::is_type(info) and std::meta::is_function_type(info);
}};
/// Matches function-template reflections.
inline constexpr Predicate isFunctionTemplate{
    [](Info info) consteval -> bool { return std::meta::is_function_template(info); }};
/// Matches non-static data members and static member variables.
inline constexpr Predicate isField{[](Info info) consteval -> bool {
  return std::meta::is_nonstatic_data_member(info) or
         (std::meta::is_variable(info) and std::meta::is_static_member(info));
}};
/// Matches static class members.
inline constexpr Predicate isStatic{
    [](Info info) consteval -> bool { return std::meta::is_static_member(info); }};
/// Matches non-static data members only.
inline constexpr Predicate isInstanceData{
    [](Info info) consteval -> bool { return std::meta::is_nonstatic_data_member(info); }};
/// Matches constructor declaration reflections.
inline constexpr Predicate isConstructor{
    [](Info info) consteval -> bool { return std::meta::is_constructor(info); }};
/// Matches namespace reflections.
inline constexpr Predicate isNamespace{
    [](Info info) consteval -> bool { return std::meta::is_namespace(info); }};
/// Matches base-specifier reflections returned by `bases`.
inline constexpr Predicate isBase{[](Info info) consteval -> bool { return std::meta::is_base(info); }};
/// Matches enumerator reflections.
inline constexpr Predicate isEnumerator{
    [](Info info) consteval -> bool { return std::meta::is_enumerator(info); }};
/// Matches variable declaration reflections.
inline constexpr Predicate isVariable{
    [](Info info) consteval -> bool { return std::meta::is_variable(info); }};
/// Matches template reflections supported by the standard reflection predicate.
inline constexpr Predicate isTemplate{
    [](Info info) consteval -> bool { return std::meta::is_template(info); }};
/// Matches type-alias reflections without automatically dealiasing them.
inline constexpr Predicate isTypeAlias{
    [](Info info) consteval -> bool { return std::meta::is_type_alias(info); }};
/// Matches namespace-alias declarations without automatically resolving the target namespace.
inline constexpr Predicate isNamespaceAlias{
    [](Info info) consteval -> bool { return std::meta::is_namespace_alias(info); }};
/// Matches concept reflections.
inline constexpr Predicate isConcept{[](Info info) consteval -> bool { return std::meta::is_concept(info); }};
/// Matches individual annotation reflections.
inline constexpr Predicate isAnnotation{
    [](Info info) consteval -> bool { return std::meta::is_annotation(info); }};
/// Matches reflections whose declared access is public.
inline constexpr Predicate isPublic{[](Info info) consteval -> bool { return std::meta::is_public(info); }};
/// Matches reflections whose declared access is protected.
inline constexpr Predicate isProtected{
    [](Info info) consteval -> bool { return std::meta::is_protected(info); }};
/// Matches reflections whose declared access is private.
inline constexpr Predicate isPrivate{[](Info info) consteval -> bool { return std::meta::is_private(info); }};

/// Matches entities carrying at least one annotation whose annotation object has type `A`.
///
/// Multiplicity is intentionally ignored here; callers needing the concrete annotation reflections use
/// `annotations<A>`.
template <class A>
inline constexpr Predicate annotated{
    [](Info info) consteval -> bool { return not annotations<A>(info).empty(); }};

} // namespace Miracle::meta

export namespace Miracle {

/// Advanced type-oriented reflection façade. It always represents exactly `^^T`.
///
/// `Reflect<T>` is a compile-time value, not a runtime reflection object. It stores the standard `Info` only
/// so all member operations can share value-oriented implmentation machinery; its invariant forbids
/// substituting a different subject. The façade owns no dynamic storage and all operations are immediate
/// (`consteval`).
template <class T>
struct Reflect final {
  /// Constructs the unique valid façade state for `T`.
  consteval Reflect()
      : subject_(^^T) {
  }

  /// Prevents callers from violating the invariant that this façade always denotes exactly `^^T`.
  Reflect(meta::Info) = delete (
      "Reflect<T> always denotes ^^T; use meta::* for arbitrary reflection subjects");

  /// Returns the underlying standard reflection value without wrapping or copying metadata.
  [[nodiscard]] consteval auto raw() const -> meta::Info {
    return subject_;
  }

  /// Returns direct members of `T`, honoring the caller-sensitive access context.
  [[nodiscard]] consteval auto members(Access access = Access::current()) const
      -> std::span<const meta::Info> {
    return meta::members<T>(access);
  }

  /// Returns non-static data members of `T`; semantic validity is identical to `meta::fields(raw(), access)`.
  [[nodiscard]] consteval auto fields(Access access = Access::current()) const
      -> std::span<const meta::Info> {
    return meta::fields<T>(access);
  }

  /// Returns static data members of `T`, preserving declaration order and `access`.
  [[nodiscard]] consteval auto staticFields(Access access = Access::current()) const
      -> std::span<const meta::Info> {
    return meta::staticFields<T>(access);
  }

  /// Returns ordinary functions/function templates of `T`, excluding constructors.
  [[nodiscard]] consteval auto functions(Access access = Access::current()) const
      -> std::span<const meta::Info> {
    return meta::functions<T>(access);
  }

  /// Returns constructors/constructor templates of a class type `T`.
  [[nodiscard]] consteval auto constructors(Access access = Access::current()) const
      -> std::span<const meta::Info> {
    return meta::constructors<T>(access);
  }

  /// Returns direct base specifiers of class type `T` under the requested access context.
  [[nodiscard]] consteval auto bases(Access access = Access::current()) const -> std::span<const meta::Info> {
    return meta::bases<T>(access);
  }

  /// Returns enumerators of enum type `T`; the constraint prevents this member from existing for non-enums.
  [[nodiscard]] consteval auto enumerators() const -> std::span<const meta::Info>
    requires std::is_enum_v<T>
  {
    return meta::enumerators<T>();
  }

  /// Returns all annotations attached directly to `T` in source order.
  [[nodiscard]] consteval auto annotations() const -> std::span<const meta::Info> {
    return meta::annotations<T>();
  }

private:
  /// Standard reflection identity cached as value state; constructor/deleted-overload preserve `subject_ ==
  /// ^^T`.
  meta::Info subject_{};
};

/// Convenience constructor for the sole advanced type-oriented façade.
///
/// Returns a value rather than a singleton/reference because `Reflect<T>` has no runtime identity or mutable
/// state.
template <class T>
[[nodiscard]] consteval auto reflect() -> Reflect<T> {
  return {};
}

} // namespace Miracle

export namespace Miracle::meta {

template <class State>
struct Query;

} // namespace Miracle::meta

namespace Miracle::meta {

/// Private bridge used by `flatten` to evaluate a nested Query without exposing its state representation.
template <class State, class Function>
consteval auto evaluateQuery(const Query<State> &query, Function &&function) -> bool;

/// Homogeneous value-level opcode used by the Query program IR.
///
/// Operation topology is data: adding another logical operation must not require another stage class-template
/// specialization. Typed/cardinality boundaries remain separate compiler concerns and are intentionally not
/// encoded here.
enum class QueryOp : unsigned char {
  Map,
  Filter,
  FilterMap,
  Flatten,
  Inspect,
  Take,
  Skip,
  TakeWhile,
  SkipWhile,
  Slice,
  Enumerate,
};

/// Compact structural instruction carried by the immutable Query program.
///
/// `operand` identifies structural callable/configuration state when C++26 can represent it directly. Scalar
/// fields carry compact operation parameters or future evaluation-state slot indices. Non-structural capture
/// state is deliberately not stored here.
struct Instruction final {
  QueryOp op{};
  Info operand{};
  std::size_t a{};
  std::size_t b{};

  constexpr auto operator==(const Instruction &) const -> bool = default;
};

/// Persistent reflected program node.
///
/// Each append reflects one structural `(previous, instruction)` value. Equivalent prefixes therefore inherit
/// the compiler's native structural identity instead of requiring a Miracle-owned global memo table.
struct ProgramNode final {
  Info previous{};
  Instruction instruction{};

  constexpr auto operator==(const ProgramNode &) const -> bool = default;
};

/// Appends one instruction without normalization.
[[nodiscard]] consteval auto appendProgramRaw(Info previous, Instruction instruction) -> Info {
  return std::meta::reflect_constant(ProgramNode{.previous = previous, .instruction = instruction});
}

/// Appends one structural instruction and performs only locally provable, observation-preserving
/// normalization.
///
/// Rewrites are intentionally conservative. In particular, arithmetic that would overflow keeps the two
/// original instructions, and `Take(0)` is not allowed to erase an observable prefix: earlier
/// predicates/inspection may still be evaluated before the zero-take boundary is reached.
[[nodiscard]] consteval auto appendProgram(Info previous, Instruction instruction) -> Info {
  if (instruction.op == QueryOp::Skip and instruction.a == 0) {
    return previous;
  }

  if (previous == Info{}) {
    return appendProgramRaw(previous, instruction);
  }

  const auto tail = std::meta::extract<ProgramNode>(previous);
  const auto prior = tail.instruction;

  if (prior.op == QueryOp::Take and instruction.op == QueryOp::Take) {
    return appendProgramRaw(
        tail.previous, Instruction{.op = QueryOp::Take, .a = std::min(prior.a, instruction.a)});
  }

  if (prior.op == QueryOp::Skip and instruction.op == QueryOp::Skip) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(tail.previous, Instruction{.op = QueryOp::Skip, .a = prior.a + instruction.a});
    }
    return appendProgramRaw(previous, instruction);
  }

  if (prior.op == QueryOp::Skip and instruction.op == QueryOp::Take) {
    return appendProgramRaw(
        tail.previous, Instruction{.op = QueryOp::Slice, .a = prior.a, .b = instruction.a});
  }

  // Once a bounded Slice exists, a later Take can only tighten its output limit. Source traversal stops at
  // the same logical item in both formulations, including the zero-take case where Slice deliberately
  // preserves the established observable prefix traversal.
  if (prior.op == QueryOp::Slice and instruction.op == QueryOp::Take) {
    return appendProgramRaw(tail.previous,
        Instruction{.op = QueryOp::Slice, .a = prior.a, .b = std::min(prior.b, instruction.a)});
  }

  // Prefix skipping before a Slice is just a larger Slice offset when that offset is representable. Overflow
  // is intentionally left as two instructions so the original stepwise size_t semantics remain observable.
  if (prior.op == QueryOp::Skip and instruction.op == QueryOp::Slice) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(
          tail.previous, Instruction{.op = QueryOp::Slice, .a = prior.a + instruction.a, .b = instruction.b});
    }
    return appendProgramRaw(previous, instruction);
  }

  // Skipping inside an already-bounded Slice can be folded only while the second skip remains strictly inside
  // the first Slice's take window. At the exact/end boundary, replacing the pair with Slice(..., 0) would ask
  // the source for one additional item under the established Slice semantics, so that case stays separate.
  if (prior.op == QueryOp::Slice and instruction.op == QueryOp::Skip and instruction.a < prior.b) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(tail.previous,
          Instruction{.op = QueryOp::Slice, .a = prior.a + instruction.a, .b = prior.b - instruction.a});
    }
    return appendProgramRaw(previous, instruction);
  }

  // Two adjacent bounded slices compose exactly while the second Slice begins inside the first take window.
  // The strict inequality is intentional for the same source-observation reason as Slice + Skip above.
  if (prior.op == QueryOp::Slice and instruction.op == QueryOp::Slice and instruction.a < prior.b) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(tail.previous,
          Instruction{.op = QueryOp::Slice,
              .a = prior.a + instruction.a,
              .b = std::min(prior.b - instruction.a, instruction.b)});
    }
    return appendProgramRaw(previous, instruction);
  }

  return appendProgramRaw(previous, instruction);
}

/// Counts one reflected program at compile time without creating an intermediate container.
template <Info Program>
[[nodiscard]] consteval auto programInstructionCount() -> std::size_t {
  std::size_t count{};
  Info cursor = Program;
  while (cursor != Info{}) {
    ++count;
    cursor = std::meta::extract<ProgramNode>(cursor).previous;
  }
  return count;
}

/// Linearizes a persistent program into declaration order for homogeneous segment compilation.
///
/// The array extent is derived directly from the reflected program NTTP. This deliberately avoids temporary
/// `vector`/`define_static_array` machinery on the evaluator hot path: the structural probe showed that the
/// fixed template-library cost dwarfed the actual value-IR stage growth.
template <Info Program>
[[nodiscard]] consteval auto programInstructions() {
  std::array<Instruction, programInstructionCount<Program>()> result{};
  Info cursor = Program;
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<ProgramNode>(cursor);
    result[--index] = node.instruction;
    cursor = node.previous;
  }
  return result;
}

/// One compiled same-type segment plus its optional typed continuation boundary.
///
/// The body is represented as the persistent half-open program range `(bodyBegin, bodyEnd]`. `boundary` is
/// empty for the terminal segment or names the single instruction that changes the logical item type before
/// evaluation continues in the next segment. Type reflections are kept as data rather than template
/// parameters.
struct SegmentNode final {
  Info previous{};
  Info bodyBegin{};
  Info bodyEnd{};
  Info boundary{};
  Info inputType{};
  Info outputType{};

  constexpr auto operator==(const SegmentNode &) const -> bool = default;
};

/// Mutable-by-value planning cursor used only while compiling immutable Program IR into typed segments.
///
/// `segments` contains already-closed segments. The remaining fields describe the currently open same-type
/// body. Every update returns a new structural value; no persistent traversal state leaks into Query
/// evaluation.
struct SegmentPlan final {
  Info segments{};
  Info bodyBegin{};
  Info bodyEnd{};
  Info valueType{};

  constexpr auto operator==(const SegmentPlan &) const -> bool = default;
};

/// Starts typed planning at one source logical type and one already-built program prefix.
[[nodiscard]] consteval auto beginSegmentPlan(Info program, Info valueType) -> SegmentPlan {
  return SegmentPlan{.bodyBegin = program, .bodyEnd = program, .valueType = valueType};
}

/// Appends one closed segment to the persistent segment chain.
[[nodiscard]] consteval auto appendSegment(Info previous,
    Info bodyBegin,
    Info bodyEnd,
    Info boundary,
    Info inputType,
    Info outputType) -> Info {
  return std::meta::reflect_constant(SegmentNode{
      .previous = previous,
      .bodyBegin = bodyBegin,
      .bodyEnd = bodyEnd,
      .boundary = boundary,
      .inputType = inputType,
      .outputType = outputType,
  });
}

/// Returns whether one IR operation may legitimately change the logical item type.
[[nodiscard]] consteval auto permitsTypedBoundary(QueryOp op) // NOLINT(readability-identifier-length)
    -> bool {
  return op == QueryOp::Map or op == QueryOp::FilterMap or op == QueryOp::Flatten or op == QueryOp::Enumerate;
}

/// Advances typed planning after one normalized IR append.
///
/// Same-type operations merely extend the open body. A genuine logical type transition closes that body with
/// the new instruction as its typed boundary, then starts a fresh body after the boundary. Comparing
/// reflected exact logical types means a `map<T -> T>` or `filterMap<T -> T>` remains inside the homogeneous
/// segment while reference/cv changes correctly form a boundary.
[[nodiscard]] consteval auto advanceSegmentPlan(SegmentPlan plan,
    Info programBefore,
    Info programAfter,
    Info outputType) -> SegmentPlan {
  if (plan.bodyEnd != programBefore) {
    throw std::meta::exception{
        "Miracle Meta Query typed-segment plan is not aligned with the normalized program", programBefore};
  }

  // A normalized no-op such as Skip(0) adds no instruction and therefore cannot create a segment boundary.
  if (programAfter == programBefore) {
    return plan;
  }

  if (plan.valueType == outputType) {
    plan.bodyEnd = programAfter;
    return plan;
  }

  const auto boundaryNode = std::meta::extract<ProgramNode>(programAfter);
  if (not permitsTypedBoundary(boundaryNode.instruction.op)) {
    throw std::meta::exception{
        "Miracle Meta Query non-transforming instruction changed the logical item type", programAfter};
  }

  plan.segments =
      appendSegment(plan.segments, plan.bodyBegin, programBefore, programAfter, plan.valueType, outputType);
  plan.bodyBegin = programAfter;
  plan.bodyEnd = programAfter;
  plan.valueType = outputType;
  return plan;
}

/// Finalizes the current homogeneous body, including an intentionally empty continuation body after a
/// terminal typed boundary. Keeping that final segment explicit gives the evaluator one uniform sink
/// continuation shape.
[[nodiscard]] consteval auto finishSegmentPlan(SegmentPlan plan) -> Info {
  return appendSegment(plan.segments, plan.bodyBegin, plan.bodyEnd, Info{}, plan.valueType, plan.valueType);
}

/// Counts persistent typed segments without constructing an auxiliary type pack.
template <Info Segments>
[[nodiscard]] consteval auto segmentCount() -> std::size_t {
  std::size_t count{};
  Info cursor = Segments;
  while (cursor != Info{}) {
    ++count;
    cursor = std::meta::extract<SegmentNode>(cursor).previous;
  }
  return count;
}

/// Linearizes persistent segment metadata in source/evaluation order.
template <Info Segments>
[[nodiscard]] consteval auto programSegments() {
  std::array<SegmentNode, segmentCount<Segments>()> result{};
  Info cursor = Segments;
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<SegmentNode>(cursor);
    result[--index] = node;
    cursor = node.previous;
  }
  return result;
}

/// Counts instructions in the persistent half-open program range `(Begin, End]`.
template <Info Begin, Info End>
[[nodiscard]] consteval auto programInstructionCountBetween() -> std::size_t {
  std::size_t count{};
  Info cursor = End;
  while (cursor != Begin) {
    if (cursor == Info{}) {
      throw std::meta::exception{
          "Miracle Meta Query segment does not belong to its declared program prefix", End};
    }
    ++count;
    cursor = std::meta::extract<ProgramNode>(cursor).previous;
  }
  return count;
}

/// Linearizes one homogeneous segment body in declaration order.
template <Info Begin, Info End>
[[nodiscard]] consteval auto programInstructionsBetween() {
  std::array<Instruction, programInstructionCountBetween<Begin, End>()> result{};
  Info cursor = End;
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<ProgramNode>(cursor);
    result[--index] = node.instruction;
    cursor = node.previous;
  }
  return result;
}

/// Visits one homogeneous body with a single C++26 expansion statement.
///
/// This helper is useful for validation/initialization that only consumes instruction values. Actual
/// evaluator lowering keeps the expansion directly in the typed segment function so `instruction` remains a
/// local `constexpr` and opcode dispatch can use `if constexpr` without creating one callback specialization
/// per logical operation.
template <Info Begin, Info End, class Function>
consteval auto forEachSegmentInstruction(Function &&function) -> void {
  template for (constexpr auto instruction : programInstructionsBetween<Begin, End>()) {
    function(instruction);
  }
}

/// One instruction paired with its ordinal in the normalized immutable program.
///
/// Evaluation-state slots are assigned from this normalized order rather than while fluent adaptors are being
/// appended. That is important: local normalization may remove or replace stateful instructions, so
/// allocating a counter before normalization would leave holes or stale slots.
struct IndexedInstruction final {
  Instruction instruction{};
  std::size_t ordinal{};

  constexpr auto operator==(const IndexedInstruction &) const -> bool = default;
};

/// Linearizes a homogeneous body together with each instruction's global normalized-program ordinal.
template <Info Begin, Info End>
[[nodiscard]] consteval auto programIndexedInstructionsBetween() {
  std::array<IndexedInstruction, programInstructionCountBetween<Begin, End>()> result{};
  Info cursor = End;
  const auto firstOrdinal = programInstructionCount<Begin>();
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<ProgramNode>(cursor);
    --index;
    result[index] = IndexedInstruction{node.instruction, firstOrdinal + index};
    cursor = node.previous;
  }
  return result;
}

inline constexpr std::size_t noEvaluationStateSlot = std::numeric_limits<std::size_t>::max();

/// Compact slot assigned to one normalized instruction. The opcode determines whether the index addresses the
/// counter pool or flag pool; stateless instructions retain `noEvaluationStateSlot`.
struct EvaluationStateSlot final {
  std::size_t index{noEvaluationStateSlot};

  constexpr auto operator==(const EvaluationStateSlot &) const -> bool = default;
};

/// Immutable evaluation-state layout compiled from normalized value IR.
///
/// This is data, not one C++ type per stateful stage. `N` contributes only an array extent; the mutable
/// evaluator itself is parameterized by the two primitive pool sizes.
template <std::size_t N>
struct EvaluationStatePlan final {
  std::array<EvaluationStateSlot, N> slots{};
  std::size_t counterCount{};
  std::size_t flagCount{};

  constexpr auto operator==(const EvaluationStatePlan &) const -> bool = default;
};

[[nodiscard]] consteval auto counterStateWidth(QueryOp op) // NOLINT(readability-identifier-length)
    -> std::size_t {
  if (op == QueryOp::Take or op == QueryOp::Skip or op == QueryOp::Enumerate) {
    return 1;
  }
  if (op == QueryOp::Slice) {
    return 2;
  }
  return 0;
}

[[nodiscard]] consteval auto flagStateWidth(QueryOp op) // NOLINT(readability-identifier-length)
    -> std::size_t {
  return op == QueryOp::TakeWhile or op == QueryOp::SkipWhile ? 1 : 0;
}

/// Assigns dense primitive-state slots after all local IR normalization has completed.
template <Info Program>
[[nodiscard]] consteval auto compileEvaluationStatePlan() {
  constexpr auto instructions = programInstructions<Program>();
  EvaluationStatePlan<instructions.size()> result{};

  for (std::size_t ordinal{}; ordinal < instructions.size(); ++ordinal) {
    const auto counterWidth = counterStateWidth(instructions[ordinal].op);
    const auto flagWidth = flagStateWidth(instructions[ordinal].op);
    if (counterWidth != 0 and flagWidth != 0) {
      throw std::meta::exception{"Miracle Meta Query instruction requested incompatible state pools", Info{}};
    }
    if (counterWidth != 0) {
      result.slots[ordinal].index = result.counterCount;
      result.counterCount += counterWidth;
    } else if (flagWidth != 0) {
      result.slots[ordinal].index = result.flagCount;
      result.flagCount += flagWidth;
    }
  }
  return result;
}

/// Ephemeral mutable traversal state. Query and Program stay immutable; only these primitive pools change
/// while a terminal walks one source.
template <std::size_t CounterCount, std::size_t FlagCount>
struct Evaluation final {
  std::array<std::size_t, CounterCount> counters{};
  std::array<bool, FlagCount> flags{};
};

/// Initializes primitive traversal state directly from normalized instruction data.
template <Info Program>
[[nodiscard]] consteval auto makeEvaluation() {
  constexpr auto instructions = programInstructions<Program>();
  constexpr auto plan = compileEvaluationStatePlan<Program>();
  Evaluation<plan.counterCount, plan.flagCount> result{};

  for (std::size_t ordinal{}; ordinal < instructions.size(); ++ordinal) {
    const auto instruction = instructions[ordinal];
    const auto slot = plan.slots[ordinal].index;
    switch (instruction.op) {
      case QueryOp::Take:
      case QueryOp::Skip: result.counters[slot] = instruction.a; break;
      case QueryOp::Slice:
        result.counters[slot] = instruction.a;
        result.counters[slot + 1] = instruction.b;
        break;
      case QueryOp::Enumerate: result.counters[slot] = 0; break;
      case QueryOp::TakeWhile:
      case QueryOp::SkipWhile: result.flags[slot] = true; break;
      default: break;
    }
  }
  return result;
}

/// One independently addressable slot in the flat non-structural callable environment.
///
/// `Index` deliberately participates in the slot type so two occurrences of the same closure type remain
/// distinct.
/// `[[no_unique_address]]` also lets empty captured-state wrappers avoid unnecessary storage when the
/// language permits.
template <std::size_t Index, class Capture>
struct CaptureSlot {
  [[no_unique_address]] Capture value;
};

/// Flat implementation selected by an index sequence so no capture points at a predecessor environment.
template <class Indices, class... Captures>
struct CaptureEnvironmentImpl;

template <std::size_t... Indices, class... Captures>
struct CaptureEnvironmentImpl<std::index_sequence<Indices...>, Captures...> final
    : CaptureSlot<Indices, Captures>... {
  static constexpr std::size_t size_ = sizeof...(Captures);
};

/// Stores only irreducible non-structural Query callable values.
///
/// Captureless/structural callables belong directly in `Instruction::operand`; this environment
/// therefore grows with M non-structural captured stages rather than N total Query operations. The
/// representation is a single flat pack of indexed slots: there is no recursive
/// `Environment<Previous, Capture>` chain and no tuple-backed stage list.
///
/// A reflected `define_aggregate` representation was measured before selecting this form. Both
/// representations were semantically viable, but the synthesized aggregate imposed substantially
/// higher fixed and scaling frontend cost on the reference probes.
template <class... Captures>
using CaptureEnvironment = CaptureEnvironmentImpl<std::index_sequence_for<Captures...>, Captures...>;

/// Constructs one flat environment directly from its captured callable values.
template <class... Captures>
[[nodiscard]] consteval auto makeCaptureEnvironment(Captures... captures) -> CaptureEnvironment<Captures...> {
  return [&]<std::size_t... Indices>(
             std::index_sequence<Indices...>) consteval -> CaptureEnvironment<Captures...> {
    return CaptureEnvironment<Captures...>{CaptureSlot<Indices, Captures>{std::move(captures)}...};
  }(std::index_sequence_for<Captures...>{});
}

/// Returns one captured callable by the compact slot index stored in the Query instruction.
template <std::size_t Index, class... Captures>
[[nodiscard]] consteval auto captureAt(CaptureEnvironment<Captures...> &environment) -> decltype(auto) {
  static_assert(Index < sizeof...(Captures), "Miracle Meta Query capture slot is out of range");
  using Capture = Captures...[Index];
  return (static_cast<CaptureSlot<Index, Capture> &>(environment).value);
}

/// Appends one non-structural callable while keeping the resulting environment flat.
template <class... Captures, class Capture>
[[nodiscard]] consteval auto appendCapture(CaptureEnvironment<Captures...> environment, Capture capture)
    -> CaptureEnvironment<Captures..., Capture> {
  return [&]<std::size_t... Indices>(
             std::index_sequence<Indices...>) consteval -> CaptureEnvironment<Captures..., Capture> {
    return CaptureEnvironment<Captures..., Capture>{
        CaptureSlot<Indices, Captures>{std::move(captureAt<Indices>(environment))}...,
        CaptureSlot<sizeof...(Captures), Capture>{std::move(capture)}};
  }(std::index_sequence_for<Captures...>{});
}

/// The immutable instruction plus the capture environment resulting from binding one callable.
template <class Environment>
struct CallableBinding final {
  Instruction instruction;
  Environment environment;
};

/// Binds a callable without duplicating opcode semantics into static/stateful stage types.
///
/// Structural values are reflected directly into the instruction. A genuinely non-structural closure
/// receives the next compact environment slot instead; `Instruction::a` records that slot and
/// `operand` remains empty.
template <class... Captures, class Function>
[[nodiscard]] consteval auto bindCallable(QueryOp op, // NOLINT(readability-identifier-length)
    CaptureEnvironment<Captures...> environment,
    Function function) {
  using Callable = std::remove_cvref_t<Function>;
  if constexpr (std::meta::is_structural_type(^^Callable)) {
    return CallableBinding<CaptureEnvironment<Captures...>>{
        Instruction{.op = op, .operand = std::meta::reflect_constant(function)}, std::move(environment)};
  } else {
    constexpr auto slot = sizeof...(Captures);
    auto nextEnvironment = appendCapture(std::move(environment), std::move(function));
    return CallableBinding<decltype(nextEnvironment)>{
        Instruction{.op = op, .a = slot}, std::move(nextEnvironment)};
  }
}

/// Computes the logical element type produced by flattening either an input range or another Query.
///
/// Lvalue ranges preserve their reference type; rvalue ranges use their value type so no Query can
/// retain a reference into a temporary range after the flattening call completes.
template <class T, bool IsRange = std::ranges::input_range<std::remove_cvref_t<T>>>
struct FlattenValue;

template <class T>
struct FlattenValue<T, true> {
  using type = std::conditional_t<std::is_reference_v<T>,
      std::ranges::range_reference_t<T>,
      std::ranges::range_value_t<std::remove_cvref_t<T>>>;
};

template <class T>
  requires(not std::ranges::input_range<std::remove_cvref_t<T>>)
struct FlattenValue<T, false> {
  using type = std::remove_cvref_t<T>::Value;
};

/// Data-oriented public Query state used by the Query façade.
///
/// Operation topology is not encoded in the C++ type. `program` and `segmentPlan` are immutable structural
/// values, while `environment` contains only irreducible non-structural callable state. Consequently the
/// state type changes only when the logical value/source type changes or the capture environment genuinely
/// grows.
template <class Source, class Value, class Environment = CaptureEnvironment<>>
struct QueryPlanState final {
  using source_type = Source;
  using value_type = Value;
  using environment_type = Environment;

  Source source;
  [[no_unique_address]] Environment environment;
  Info program{};
  SegmentPlan segmentPlan{};
};

/// Constructs an empty compiled Query state for one source/logical value type.
template <class Value, class Source>
[[nodiscard]] consteval auto makeInitialQueryPlan(Source source) {
  using State = QueryPlanState<Source, Value>;
  return State{std::move(source), CaptureEnvironment<>{}, Info{}, beginSegmentPlan(Info{}, ^^Value)};
}

/// Appends one already-bound IR instruction and updates typed-segment planning after normalization.
template <class Result, class Source, class Value, class Environment>
[[nodiscard]] consteval auto appendQueryInstruction(QueryPlanState<Source, Value, Environment> state,
    Instruction instruction) {
  const auto before = state.program;
  const auto after = appendProgram(before, instruction);
  const auto nextPlan = advanceSegmentPlan(state.segmentPlan, before, after, ^^Result);
  using NextState = QueryPlanState<Source, Result, Environment>;
  return NextState{std::move(state.source), std::move(state.environment), after, nextPlan};
}

/// Appends one callable-backed instruction. Structural callables stay in IR; only genuinely non-structural
/// callable values grow the flat environment type.
template <class Result, class Source, class Value, class Environment, class Function>
[[nodiscard]] consteval auto appendQueryCallable(QueryPlanState<Source, Value, Environment> state,
    QueryOp op, // NOLINT(readability-identifier-length)
    Function function) {
  auto binding = bindCallable(op, std::move(state.environment), std::move(function));
  const auto before = state.program;
  const auto after = appendProgram(before, binding.instruction);
  const auto nextPlan = advanceSegmentPlan(state.segmentPlan, before, after, ^^Result);
  using NextEnvironment = decltype(binding.environment);
  using NextState = QueryPlanState<Source, Result, NextEnvironment>;
  return NextState{std::move(state.source), std::move(binding.environment), after, nextPlan};
}

/// Borrowed static-span source. Query never extends the lifetime of `values`.
template <class Element>
struct SpanSource final {
  std::span<Element> values;
  using value_type = Element &;
};

/// Composite source that evaluates the right query only after the left query is exhausted.
template <class LeftState, class RightState>
struct ChainSource final {
  LeftState left;
  RightState right;
  using value_type = LeftState::value_type;
};

/// Composite source pairing two independently evaluated query states until the shorter side ends.
template <class LeftState, class RightState>
struct ZipSource final {
  LeftState left;
  RightState right;
  using left_value_type = LeftState::value_type;
  using right_value_type = RightState::value_type;
  using value_type = std::pair<left_value_type, right_value_type>;
};

/// Static-storage source used after operations that must persist/reorder logical values.
///
/// Reference-valued queries store pointers and recover the original references through `unwrap`;
/// owning values are stored directly. The source therefore preserves Query's logical `Value` even
/// across a persistence barrier.
template <class Value, class Stored>
struct StoredSource final {
  std::span<const Stored> values;
  using value_type = Value;
};

/// Invokes a Query callable using the richest shape the callable explicitly accepts.
///
/// Dispatch is ordered deliberately: an exact logical-value overload wins first, tuple protocol
/// expansion is second, and reflected aggregate-field expansion is the final structural fallback.
/// This prevents a callable that already accepts the object itself from being unexpectedly decomposed
/// and keeps the adaptation local to Meta Query.
template <class Function, class Item>
consteval auto adaptiveInvoke(Function &&function, Item &&item) -> decltype(auto) {
  if constexpr (std::invocable<Function, Item>) {
    // Preserve the logical value/category when the callable already understands it; decomposition
    // would only add compiler work and could select a different overload set.
    return std::invoke(std::forward<Function>(function), std::forward<Item>(item));
  } else if constexpr (requires { typename std::tuple_size<std::remove_cvref_t<Item>>::type; }) {
    // Tuple-like values use their established tuple protocol before reflection. This covers
    // pair/tuple/custom tuple-like values without requires them to be aggregates or exposing
    // implementation members.
    constexpr auto count = std::tuple_size_v<std::remove_cvref_t<Item>>;
    return [&]<std::size_t... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      static_assert(std::invocable<Function, decltype(std::get<Index>(std::declval<Item>()))...>,
          "Miracle Meta Query invocation requires a callable accepting the item or its tuple "
          "elements");
      return std::invoke(std::forward<Function>(function), std::get<Index>(std::forward<Item>(item))...);
    }(std::make_index_sequence<count>{});
  } else if constexpr (std::is_aggregate_v<std::remove_cvref_t<Item>>) {
    // Reuse Miracle's reflection façade instead of issuing a parallel raw <meta> query. `fields()` is
    // the exact semantic match for aggregate decomposition: only non-static data members participate,
    // in declaration order.
    constexpr auto fields = reflect<std::remove_cvref_t<Item>>().fields();
    return [&]<std::size_t... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      // Validate the fully expanded call before splicing. The generated indices are bounded by
      // fields.size(), so the reflected member accesses below cannot escape the cached field
      // universe.
      static_assert(
          requires(Function &&function, Item &&value) {
            std::invoke(std::forward<Function>(function),
                std::forward<Item>(value).[:fields[Index
            ]:]...); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
                     // - Index is generated by make_index_sequence.
          },
          "Miracle Meta Query invocation requires a callable accepting the item, its tuple elements, "
          "or its reflected members");
      return std::invoke(std::forward<Function>(function),
          std::forward<Item>(item)
              .[:fields[Index]:]...); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    }(std::make_index_sequence<fields.size()>{});
  } else {
    // Reaching this branch means no supported invocation shape exists. Keep the diagnostic at the
    // adaptation boundary instead of allowing deeper std::invoke substitution failure to obscure the
    // Query operation that caused it.
    static_assert(std::invocable<Function, Item>,
        "Miracle Meta Query invocation requires a callable accepting the item, its tuple elements, "
        "or its reflected members");
  }
}

/// Invokes a binary Query callable while preserving both logical value categories.
///
/// Binary operations currently have no tuple/aggregate decomposition semantics; keeping that boundary
/// explicit avoids surprising comparator/reducer behavior and leaves room for a separately designed
/// binary adaptation policy later.
template <class Function, class Left, class Right>
consteval auto adaptiveInvokeBinary(Function &&function, Left &&left, Right &&right) -> decltype(auto) {
  if constexpr (std::invocable<Function, Left, Right>) {
    // Forward both values exactly so reference-sensitive comparators and reducers observe the Query's
    // logical types.
    return std::invoke(
        std::forward<Function>(function), std::forward<Left>(left), std::forward<Right>(right));
  } else {
    // Diagnose at the Query boundary rather than surfacing an implementation-level std::invoke
    // substitution trace.
    static_assert(std::invocable<Function, Left, Right>,
        "Miracle Meta Query binary invocation requires a callable accepting both logical values");
  }
}

/// Minimal optional protocol required by `filterMap`; deliberately avoids coupling the Query engine
/// to one wrapper type.
template <class T>
concept OptionalLike = requires(T value) {
  typename std::remove_cvref_t<T>::value_type;
  { value.has_value() } -> std::convertible_to<bool>;
  *value;
};

/// Extracts the logical engaged value from an Optional-like result.
template <class T>
struct OptionalResult final {
  using type = std::remove_cvref_t<T>::value_type;
};

/// C++26 `std::optional<T&>` exposes `value_type` as `T`, so the reference must be restored
/// explicitly for Query's logical value type. Ordinary `std::optional<T>` retains its owning `T`
/// result type.
template <class T>
struct OptionalResult<std::optional<T &>> final {
  using type = T &;
};

template <class T>
using OptionalResultT = OptionalResult<std::remove_cvref_t<T>>::type;

/// Presents one engaged Optional-like result to its continuation using the declared logical result type.
///
/// Standard optional values already expose `T&&` when the wrapper is moved. More general Optional-like
/// wrappers may expose proxy dereference values instead; those are materialized once into the declared
/// `value_type` rather than requiring std::optional-specific dereference semantics. Reference results are
/// stricter because manufacturing a temporary would destroy identity.
template <class ResultValue, class Optional, class Function>
consteval auto withOptionalLogical(Optional &result, Function &&function) -> decltype(auto) {
  if constexpr (std::is_reference_v<ResultValue>) {
    static_assert(std::same_as<decltype(*result), ResultValue>,
        "Query::filterMap() reference Optional-like results must dereference to the exact reference type");
    return std::forward<Function>(function)(static_cast<ResultValue>(*result));
  } else if constexpr (std::same_as<decltype(std::move(*result)), ResultValue &&>) {
    return std::forward<Function>(function)(std::move(*result));
  } else {
    ResultValue value(std::move(*result));
    return std::forward<Function>(function)(std::move(value));
  }
}

/// Chooses the static-storage representation for one logical Query value.
///
/// References become pointers so persistence never copies or silently owns the referent; object
/// values remain values.
template <class T>
struct Stored final {
  using Raw = std::remove_reference_t<T>;
  using type = std::conditional_t<std::is_reference_v<T>, Raw *, std::remove_cvref_t<T>>;
};

template <class T>
using StoredT = Stored<T>::type;

/// Converts one logical value to its persistence representation while preserving reference identity.
template <class Value, class Item>
constexpr auto storeValueAs(Item &&value) -> StoredT<Value> {
  if constexpr (std::is_reference_v<Value>) {
    // Persist only the address; copying the referent here would violate Query's exact-reference
    // contract.
    return std::addressof(value);
  } else {
    // Owning logical values can be transferred directly into the static-storage representation.
    return std::forward<Item>(value);
  }
}

/// Recovers the logical value from a persisted representation.
template <class T>
constexpr auto unwrap(T &value) -> decltype(auto) {
  if constexpr (std::is_pointer_v<T>) {
    // Pointer storage represents a logical reference; dereference without changing ownership.
    return *value;
  } else {
    // Owning storage already is the logical value.
    return value;
  }
}

/// Presents one persisted representation to a synchronous continuation using the Query's exact logical
/// category.
///
/// References are recovered by identity. Owning values are reconstructed locally and forwarded as xvalues so
/// immutable/static backing never leaks `const T&` into a Query whose logical value is `T`. The continuation
/// must not retain the owning value beyond the call.
template <class Value, class Stored, class Function>
consteval auto withStoredLogical(Stored &&stored, Function &&function) -> decltype(auto) {
  if constexpr (std::is_reference_v<Value>) {
    static_assert(std::is_pointer_v<std::remove_cvref_t<Stored>>,
        "Miracle Meta Query reference persistence must store an address");
    return std::forward<Function>(function)(static_cast<Value>(*stored));
  } else {
    using Object = std::remove_reference_t<Value>;
    Object value(std::forward<Stored>(stored));
    return std::forward<Function>(function)(std::move(value));
  }
}

/// Enforces the exact conditions under which Query may expose a persistent standard span.
///
/// References are intentionally rejected even though the engine can internally persist them as
/// pointers: public `materialize()` must not change `T&` into `T*` or copy the referent behind the
/// caller's back.
template <class T>
constexpr auto requireMaterializable() -> void {
  if constexpr (std::is_reference_v<T> or not std::is_object_v<std::remove_cvref_t<T>> or
                not std::copy_constructible<std::remove_cvref_t<T>> or
                not std::meta::is_structural_type(^^std::remove_cvref_t<T>)) {
    throw std::meta::exception{"Query::materialize() requires a copy-constructible object value", {}};
  }
}

/// Returns whether one reorder/grouping barrier can create reusable static backing for its logical value.
///
/// This predicate is used in `if constexpr` at the barrier boundary. A consteval throw by itself is
/// insufficient: code after that throw would still be instantiated and could leak `define_static_array` hard
/// diagnostics before Miracle's intended semantic exception is observed.
template <class Value, class Stored = StoredT<Value>>
inline constexpr bool persistentQueryBacking =
    (std::is_reference_v<Value> or std::copy_constructible<std::remove_cvref_t<Value>>) and
    std::meta::is_structural_type(^^Stored);

template <class Value, class Stored = StoredT<Value>>
consteval auto rejectPersistentQueryBacking(Info source = {}) -> void {
  if constexpr (not std::is_reference_v<Value> and not std::copy_constructible<std::remove_cvref_t<Value>>) {
    throw std::meta::exception{
        "Query persistence barrier requires a copy-constructible owning logical value", source};
  } else {
    static_assert(not std::meta::is_structural_type(^^Stored));
    throw std::meta::exception{
        "Query persistence barrier requires a structurally promotable stored element", source};
  }
}

template <class State, class Sink>
consteval auto evaluateState(State state, Sink &&sink) -> bool;

/// Emits a direct span-like source in order, stopping as soon as the downstream sink returns false.
template <class Source, class Sink>
consteval auto emitSource(Source source, Sink &sink) -> bool {
  for (auto &&item : source.values) {
    if (not sink(std::forward<decltype(item)>(item))) {
      return false;
    }
  }
  return true;
}

/// Emits a persistent source after restoring each stored representation to its exact logical category.
///
/// Static promoted backing is necessarily const. Reference-valued queries therefore unwrap their persisted
/// pointers directly, while owning queries reconstruct one fresh logical value for each visit and forward it
/// as an xvalue. This keeps a barrier from silently changing `T` into `const T&` merely because its backing
/// storage is immutable.
template <class Value, class Stored, class Sink>
consteval auto emitSource(StoredSource<Value, Stored> source, Sink &sink) -> bool {
  for (const auto &stored : source.values) {
    const bool keepGoing = withStoredLogical<Value>(
        stored, [&](auto &&value) consteval -> bool { return sink(std::forward<decltype(value)>(value)); });
    if (not keepGoing)
      return false;
  }
  return true;
}

/// Emits a chained source while distinguishing child-query termination from downstream cancellation.
///
/// `evaluateState()` returns false both when a stateful adaptor such as `take` naturally terminates its child
/// query and when the terminal sink requests cancellation. Only the latter may suppress the right-hand
/// source.
template <class LeftState, class RightState, class Sink>
consteval auto emitSource(ChainSource<LeftState, RightState> source, Sink &sink) -> bool {
  bool downstreamStopped{};
  auto trackedSink = [&](auto &&item) consteval -> bool {
    const bool keepGoing = sink(std::forward<decltype(item)>(item));
    downstreamStopped = not keepGoing;
    return keepGoing;
  };

  static_cast<void>(evaluateState(std::move(source.left), trackedSink));
  if (downstreamStopped)
    return false;

  return evaluateState(std::move(source.right), sink);
}

/// Emits `zip` using evaluation-local pairing buffers.
///
/// Query preserves the established observable source-composition semantics: both child queries are evaluated
/// before pairs are emitted, and the shorter logical sequence sets the result cardinality. The old
/// implementation then promoted both temporary buffers into permanent static arrays on every compiled zip
/// path. Those promotions are unnecessary: the buffers already outlive the synchronous pairing loop. Keeping
/// them local removes hidden static-storage materialization without changing established evaluation order.
template <class LeftState, class RightState, class Sink>
consteval auto emitSource(ZipSource<LeftState, RightState> source, Sink &sink) -> bool {
  using LeftValue = LeftState::value_type;
  using RightValue = RightState::value_type;
  std::vector<StoredT<LeftValue>> left;
  std::vector<StoredT<RightValue>> right;
  evaluateState(std::move(source.left), [&]<class Item>(Item &&item) consteval -> bool {
    left.push_back(storeValueAs<LeftValue>(std::forward<Item>(item)));
    return true;
  });
  evaluateState(std::move(source.right), [&]<class Item>(Item &&item) consteval -> bool {
    right.push_back(storeValueAs<RightValue>(std::forward<Item>(item)));
    return true;
  });

  const auto count = std::min(left.size(), right.size());
  for (std::size_t index{}; index < count; ++index) {
    bool keepGoing{};
    withStoredLogical<LeftValue>(std::move(left[index]), [&](auto &&leftValue) consteval {
      withStoredLogical<RightValue>(std::move(right[index]), [&](auto &&rightValue) consteval {
        auto item = std::pair<LeftValue, RightValue>{
            std::forward<decltype(leftValue)>(leftValue), std::forward<decltype(rightValue)>(rightValue)};
        keepGoing = sink(std::move(item));
      });
    });
    if (not keepGoing)
      return false;
  }
  return true;
}

/// Two-slot local storage for same-type value transformations inside one compiled segment.
///
/// A `map<T -> T>` must not become a fake typed boundary merely to keep a transformed prvalue alive. The
/// inactive slot is constructed directly from the map prvalue, so guaranteed copy elision preserves immovable
/// values and no additional Query-visible move/copy is introduced. Reference-valued segments do not
/// instantiate this storage at all. Both slots are tracked explicitly so exception unwinding destroys every
/// live object.
template <class T>
  requires(std::is_object_v<T> and not std::is_reference_v<T>)
struct SegmentValueStorage final {
  union Slot {
    unsigned char inactive;
    T value;

    constexpr Slot()
        : inactive{} {
    }

    constexpr ~Slot() {
    }
  };

  Slot slots[2]{};
  bool alive[2]{};
  int active{-1};

  constexpr ~SegmentValueStorage() {
    for (std::size_t index{}; index < 2; ++index) {
      if (alive[index]) {
        std::destroy_at(std::addressof(slots[index].value));
      }
    }
  }

  [[nodiscard]] constexpr auto next() -> T * {
    return std::addressof(slots[active == 0 ? 1 : 0].value);
  }

  [[nodiscard]] static constexpr auto writableAddress(T *value) -> void * {
    return const_cast<void *>(static_cast<const volatile void *>(value));
  }

  /// Commits an already-constructed inactive slot and retires the previously owned slot, if any.
  constexpr auto commit(T *value) -> void {
    const auto nextIndex = value == std::addressof(slots[0].value) ? 0 : 1;
    alive[nextIndex] = true;

    const auto previous = active;
    active = nextIndex;
    if (previous >= 0) {
      alive[static_cast<std::size_t>(previous)] = false;
      std::destroy_at(std::addressof(slots[static_cast<std::size_t>(previous)].value));
    }
  }
};

/// Forwards one current segment object using the segment's exact logical value category.
template <class Logical>
[[nodiscard]] consteval auto forwardSegmentValue(std::remove_reference_t<Logical> &value) -> decltype(auto) {
  if constexpr (std::is_lvalue_reference_v<Logical>) {
    return static_cast<Logical>(value);
  } else {
    return static_cast<std::remove_reference_t<Logical> &&>(value);
  }
}

/// Emits one value through one compiled homogeneous segment.
///
/// Most operations execute directly in one C++26 expansion statement. Same-type owning `map` results use
/// `SegmentValueStorage`, avoiding per-map continuation templates while preserving immovable values. A
/// same-type `filterMap` re-enters this same function specialization with an ordinary suffix index so its
/// Optional-like wrapper stays alive; the suffix is ordinary evaluation state rather than an NTTP, so it does
/// not create per-operation template recursion.
template <Info Program,
    Info Segments,
    std::size_t SegmentIndex,
    class Environment,
    class EvaluationState,
    class Item,
    class Sink>
consteval auto emitCompiledSegment(Environment &environment,
    EvaluationState &evaluation,
    Item &&item,
    Sink &sink,
    std::size_t startInstruction = 0,
    bool stopAfterCurrent = false) -> bool {
  constexpr auto segments = programSegments<Segments>();
  static_assert(SegmentIndex < segments.size(), "Miracle Meta Query compiled segment index is out of range");
  constexpr auto segment = segments[SegmentIndex];
  constexpr auto statePlan = compileEvaluationStatePlan<Program>();
  using Logical = [:segment.inputType:];
  using Object = std::remove_reference_t<Logical>;

  using ExpectedItem =
      std::conditional_t<std::is_lvalue_reference_v<Logical>, Logical, std::remove_reference_t<Logical> &&>;
  static_assert(std::same_as<decltype(std::forward<Item>(item)), ExpectedItem>,
      "Miracle Meta Query compiled segment received an object of the wrong exact logical type");

  Object *current = std::addressof(item);
  using Storage =
      std::conditional_t<std::is_reference_v<Logical>, std::monostate, SegmentValueStorage<Object>>;
  Storage storage{};
  std::size_t ordinal{};

  template for (constexpr auto indexed :
      programIndexedInstructionsBetween<segment.bodyBegin, segment.bodyEnd>()) {
    constexpr auto instruction = indexed.instruction;
    constexpr auto stateSlot = statePlan.slots[indexed.ordinal].index;
    const auto currentOrdinal = ordinal++;
    if (currentOrdinal >= startInstruction) {
      if constexpr (instruction.op == QueryOp::Map) {
        if constexpr (std::is_reference_v<Logical>) {
          if constexpr (instruction.operand != Info{}) {
            auto &&mapped = adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
            static_assert(std::same_as<decltype(mapped), Logical>,
                "Miracle Meta Query same-type map changed its exact logical reference type");
            current = std::addressof(mapped);
          } else {
            auto &&mapped =
                adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
            static_assert(std::same_as<decltype(mapped), Logical>,
                "Miracle Meta Query same-type captured map changed its exact logical reference type");
            current = std::addressof(mapped);
          }
        } else {
          auto *target = storage.next();
          if constexpr (instruction.operand != Info{}) {
            using Result =
                decltype(adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
            static_assert(std::same_as<Result, Logical>,
                "Miracle Meta Query same-type map changed its exact logical value type");
            ::new (Storage::writableAddress(target))
                Logical(adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
          } else {
            using Result = decltype(adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
            static_assert(std::same_as<Result, Logical>,
                "Miracle Meta Query same-type captured map chnaged its exact logical value type");
            ::new (Storage::writableAddress(target)) Logical(adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
          }
          storage.commit(target);
          current = target;
        }
      } else if constexpr (instruction.op == QueryOp::FilterMap) {
        auto result = [&] consteval {
          if constexpr (instruction.operand != Info{}) {
            return adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
          } else {
            return adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
          }
        }();
        static_assert(OptionalLike<decltype(result)>, "Query::filterMap() requires an Optional-like result");
        using ResultValue = OptionalResultT<decltype(result)>;
        static_assert(std::same_as<ResultValue, Logical>,
            "Miracle Meta Query same-type filterMap changed its exact logical value type");
        if (not result.has_value()) {
          return not stopAfterCurrent;
        }
        return withOptionalLogical<ResultValue>(result, [&](auto &&engaged) consteval -> bool {
          return emitCompiledSegment<Program, Segments, SegmentIndex>(environment,
              evaluation,
              std::forward<decltype(engaged)>(engaged),
              sink,
              currentOrdinal + 1,
              stopAfterCurrent);
        });
      } else if constexpr (instruction.op == QueryOp::Filter) {
        bool accepted{};
        if constexpr (instruction.operand != Info{}) {
          accepted = static_cast<bool>(
              adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
        } else {
          accepted = static_cast<bool>(
              adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
        }
        if (not accepted)
          return not stopAfterCurrent;
      } else if constexpr (instruction.op == QueryOp::Inspect) {
        if constexpr (instruction.operand != Info{}) {
          adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
        } else {
          adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
        }
      } else if constexpr (instruction.op == QueryOp::Take) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query Take instruction has no counter-state slot");
        auto &remaining = evaluation.counters[stateSlot];
        if (remaining == 0)
          return false;
        --remaining;
        stopAfterCurrent = stopAfterCurrent or remaining == 0;
      } else if constexpr (instruction.op == QueryOp::Skip) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query Skip instruction has no counter-state slot");
        auto &remaining = evaluation.counters[stateSlot];
        if (remaining != 0)
          --remaining;
        return not stopAfterCurrent;
      } else if constexpr (instruction.op == QueryOp::TakeWhile) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query TakeWhile instruction has no flag-state slot");
        auto &active = evaluation.flags[stateSlot];
        if (not active)
          return false;
        bool accepted{};
        if constexpr (instruction.operand != Info{}) {
          accepted = static_cast<bool>(
              adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
        } else {
          accepted = static_cast<bool>(
              adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
        }
        if (not accepted) {
          active = false;
          return false;
        }
      } else if constexpr (instruction.op == QueryOp::SkipWhile) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query SkipWhile instruction has no flag-state slot");
        auto &skipping = evaluation.flags[stateSlot];
        if (skipping) {
          bool rejected{};
          if constexpr (instruction.operand != Info{}) {
            rejected = not static_cast<bool>(
                adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
          } else {
            rejected = not static_cast<bool>(adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
          }
          if (not rejected) {
            return not stopAfterCurrent;
          }
          skipping = false;
        }
      } else if constexpr (instruction.op == QueryOp::Slice) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query Slice instruction has no counter-state slot");
        auto &skip = evaluation.counters[stateSlot];
        auto &take = evaluation.counters[stateSlot + 1];
        if (skip != 0) {
          --skip;
          return not stopAfterCurrent;
        }
        if (take == 0)
          return false;
        --take;
        stopAfterCurrent = stopAfterCurrent or take == 0;
      } else {
        static_assert(instruction.op != QueryOp::Flatten and instruction.op != QueryOp::Enumerate,
            "Miracle Meta Query typed-boundary instruction escaped into a homogeneous segment body");
      }
    }
  }

  if constexpr (segment.boundary == Info{}) {
    const bool keepGoing = static_cast<bool>(sink(forwardSegmentValue<Logical>(*current)));
    return keepGoing and not stopAfterCurrent;
  } else {
    constexpr auto boundaryNode = std::meta::extract<ProgramNode>(segment.boundary);
    constexpr auto instruction = boundaryNode.instruction;
    constexpr auto boundaryOrdinal = programInstructionCount<segment.boundary>() - 1;
    constexpr auto stateSlot = statePlan.slots[boundaryOrdinal].index;
    static_assert(SegmentIndex + 1 < segments.size(),
        "Miracle Meta Query boundary must be followed by a typed continuation segment");

    if constexpr (instruction.op == QueryOp::Map) {
      bool keepGoing{};
      if constexpr (instruction.operand != Info{}) {
        auto &&mapped = adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
        keepGoing = emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(mapped)>(mapped), sink);
      } else {
        auto &&mapped =
            adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
        keepGoing = emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(mapped)>(mapped), sink);
      }
      return keepGoing and not stopAfterCurrent;
    } else if constexpr (instruction.op == QueryOp::FilterMap) {
      auto result = [&] consteval {
        if constexpr (instruction.operand != Info{}) {
          return adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
        } else {
          return adaptiveInvoke(
              captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
        }
      }();
      static_assert(OptionalLike<decltype(result)>, "Query::filterMap() requires an Optional-like result");
      if (not result.has_value()) {
        return not stopAfterCurrent;
      }
      using ResultValue = OptionalResultT<decltype(result)>;
      const bool keepGoing = withOptionalLogical<ResultValue>(result, [&](auto &&engaged) consteval -> bool {
        return emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(engaged)>(engaged), sink);
      });
      return keepGoing and not stopAfterCurrent;
    } else if constexpr (instruction.op == QueryOp::Flatten) {
      auto forwardNested = [&](auto &&nested) consteval -> bool {
        return emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(nested)>(nested), sink);
      };

      if constexpr (requires { evaluateQuery(forwardSegmentValue<Logical>(*current), forwardNested); }) {
        bool downstreamStopped{};
        auto trackedForward = [&](auto &&nested) consteval -> bool {
          const bool keepGoing = forwardNested(std::forward<decltype(nested)>(nested));
          downstreamStopped = not keepGoing;
          return keepGoing;
        };
        static_cast<void>(evaluateQuery(forwardSegmentValue<Logical>(*current), trackedForward));
        return not downstreamStopped and not stopAfterCurrent;
      } else if constexpr (std::ranges::input_range<std::remove_cvref_t<Logical>>) {
        using NextLogical = [:segments[SegmentIndex + 1].inputType:];
        for (auto &&nested : forwardSegmentValue<Logical>(*current)) {
          bool keepGoing{};
          if constexpr (std::is_reference_v<NextLogical>) {
            static_assert(std::same_as<decltype(nested), NextLogical>,
                "Miracle Meta Query flatten changed the exact nested reference type");
            keepGoing = forwardNested(nested);
          } else {
            using NextObject = std::remove_reference_t<NextLogical>;
            if constexpr (std::same_as<decltype(std::move(nested)), NextObject &&>) {
              // The owning range itself stays alive for this continuation, so forwarding its element as an
              // xvalue preserves value semantics without manufacturing an intermediate object.
              keepGoing = forwardNested(std::move(nested));
            } else {
              // A const/proxy range element cannot be forwarded as the exact owning logical type. Materialize
              // the one element locally; it lives until the synchronous downstream continuation returns.
              NextObject ownedNested(std::forward<decltype(nested)>(nested));
              keepGoing = forwardNested(std::move(ownedNested));
            }
          }
          if (not keepGoing)
            return false;
        }
        return not stopAfterCurrent;
      } else {
        static_assert(std::ranges::input_range<std::remove_cvref_t<Logical>>,
            "Query::flatten() requires a Query or an input range value");
      }
    } else if constexpr (instruction.op == QueryOp::Enumerate) {
      static_assert(stateSlot != noEvaluationStateSlot,
          "Miracle Meta Query Enumerate instruction has no counter-state slot");
      using Enumerated = std::pair<std::size_t, Logical>;
      using NextLogical = [:segments[SegmentIndex + 1].inputType:];
      static_assert(std::same_as<Enumerated, NextLogical>,
          "Miracle Meta Query enumerate boundary has the wrong continuation type");
      Enumerated pair{evaluation.counters[stateSlot]++, forwardSegmentValue<Logical>(*current)};
      const bool keepGoing = emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
          environment, evaluation, std::move(pair), sink);
      return keepGoing and not stopAfterCurrent;
    } else {
      static_assert(instruction.op == QueryOp::Map or instruction.op == QueryOp::FilterMap or
                        instruction.op == QueryOp::Flatten or instruction.op == QueryOp::Enumerate,
          "Miracle Meta Query unsupported typed segment boundary");
      return false;
    }
  }
}

/// Static compiled-program entry point used by the value-IR execution bridge.
///
/// `Program` and `Segments` are the only reflected value identities promoted to NTTPs. Source values,
/// non-structural captures, and terminal state remain ordinary function parameters.
template <Info Program, Info Segments, class Source, class Environment, class Sink>
consteval auto evaluateCompiledProgramStatic(Source source, Environment environment, Sink &sink) -> bool {
  static_assert(
      segmentCount<Segments>() != 0, "Miracle Meta Query compiled plan must contain a final segment");
  constexpr auto segments = programSegments<Segments>();
  static_assert(segments.back().bodyEnd == Program,
      "Miracle Meta Query compiled segment plan does not terminate at the supplied program");

  auto evaluation = makeEvaluation<Program>();
  auto emitSink = [&](auto &&item) consteval -> bool {
    return emitCompiledSegment<Program, Segments, 0>(
        environment, evaluation, std::forward<decltype(item)>(item), sink);
  };
  return emitSource(std::move(source), emitSink);
}

/// Reifies one structural program/segment pair once, extracts the specialized evaluator, then crosses back
/// into ordinary C++ values for source, captures, and sink state. No `reflect_invoke` is used for
/// non-structural state.
template <class Source, class Environment, class Sink>
consteval auto evaluateCompiledProgram(Info program,
    Info segments,
    Source source,
    Environment environment,
    Sink &&sink) -> bool {
  using SinkType = std::remove_cvref_t<Sink>;
  const Info specialization = std::meta::substitute(^^evaluateCompiledProgramStatic,
      {
          std::meta::reflect_constant(program),
          std::meta::reflect_constant(segments),
          ^^Source,
          ^^Environment,
          ^^SinkType});
  using Evaluator = bool (*)(Source, Environment, SinkType &);
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  return evaluator(std::move(source), std::move(environment), sink);
}

/// Evaluates one public Query plan by finalizing the currently open segment and invoking the compiled
/// evaluator.
///
/// Evaluation receives state by value: reusable lvalue Queries are copied, while rvalue Queries move their
/// state forward. Every terminal therefore gets fresh evaluation-local primitive state without preventing
/// move-only capture environments from flowing through a consumed Query pipeline.
template <class State, class Sink>
consteval auto evaluateState(State state, Sink &&sink) -> bool {
  const auto segments = finishSegmentPlan(state.segmentPlan);
  return evaluateCompiledProgram(state.program,
      segments,
      std::move(state.source),
      std::move(state.environment),
      std::forward<Sink>(sink));
}

/// Declared before the public Query definition because ordering/set operations call it from exported
/// template bodies.
template <class Value, class Stored = StoredT<Value>>
consteval auto makeStoredQuery(std::vector<Stored> values);

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Finite, compile-time-only metadata query algebra.
///
/// `State` contains one source value, one flat non-structural capture environment, and immutable structural
/// Program/SegmentPlan values. The query is not a runtime range and deliberately does not inherit from a
/// range/view hierarchy. Logical reference types are preserved through the compiled lazy evaluator.
template <class State>
struct Query final {
  /// Exact logical element type of the current query; reference qualifiers are intentionally preserved.
  using Value = State::value_type;

  /// Constructs a query from its internal state. The state type is implementation vocabulary; callers
  /// normally use `query(source)` and never spell it directly.
  explicit consteval Query(State state)
      : state_(std::move(state)) {
  }

  /// Maps every logical value using the Meta-specific adaptive invocation rules.
  template <class Self, class Function>
  [[nodiscard]] consteval auto map(this Self &&self, Function function) {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(
        not std::is_void_v<Result>, "Query::map() cannot return void; use forEach() for terminal work");
    auto next =
        appendQueryCallable<Result>(std::forward<Self>(self).state_, QueryOp::Map, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Keeps values accepted by a consteval predicate while preserving the exact logical value type.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto filter(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(std::convertible_to<Result, bool>, "Query::filter() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::Filter, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Maps values to Optional-like results and emits only engaged results.
  /// C++26 `std::optional<T&>` results preserve `T&` as the next logical value; ordinary owning optionals
  /// produce their `value_type`.
  template <class Self, class Function>
  [[nodiscard]] consteval auto filterMap(this Self &&self, Function function) {
    using Maybe = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(OptionalLike<Maybe>, "Query::filterMap() requires an Optional-like result");
    using Result = OptionalResultT<Maybe>;
    auto next =
        appendQueryCallable<Result>(std::forward<Self>(self).state_, QueryOp::FilterMap, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Flattens a map result that is itself a finite range or another Query.
  template <class Self, class Function>
  [[nodiscard]] consteval auto flatMap(this Self &&self, Function function) {
    return std::forward<Self>(self).map(std::move(function)).flatten();
  }

  /// Flattens one level of Query or input-range values without materializing an intermediate sequence.
  template <class Self>
  [[nodiscard]] consteval auto flatten(this Self &&self) {
    static_assert(requires(Value value) { value; }, "Query::flatten() requires a logical value type");
    using Result = FlattenValue<Value>::type;
    auto next =
        appendQueryInstruction<Result>(std::forward<Self>(self).state_, Instruction{.op = QueryOp::Flatten});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Observes each logical value without changing the sequence.
  template <class Self, class Function>
  [[nodiscard]] consteval auto inspect(this Self &&self, Function function) {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(std::same_as<Result, void>, "Query::inspect() callback must return void");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::Inspect, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Stops after `count` logical values.
  template <class Self>
  [[nodiscard]] consteval auto take(this Self &&self, std::size_t count) {
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Take, .a = count});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Skips the first `count` logical values.
  template <class Self>
  [[nodiscard]] consteval auto skip(this Self &&self, std::size_t count) {
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Skip, .a = count});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Takes values while the predicate remains true; the first failure terminates traversal.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto takeWhile(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::takeWhile() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::TakeWhile, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Skips values while the predicate remains true, then passes every later value.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto skipWhile(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::skipWhile() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::SkipWhile, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Selects a half-open positional slice `[start, stop)` using the same semantics as Miracle Range.
  template <class Self>
  [[nodiscard]] consteval auto slice(this Self &&self, std::size_t start, std::size_t stop) {
    const auto take = stop > start ? stop - start : std::size_t{};
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Slice, .a = start, .b = take});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Accepts any finite integral endpoint range, including Miracle's `Range`, as `[start, stop)`.
  template <class Self, std::ranges::input_range Range>
    requires std::integral<std::ranges::range_value_t<Range>>
  [[nodiscard]] consteval auto slice(this Self &&self, Range range) {
    // Miracle::Range is itself an input range, but iterating it yields the values *inside* [start, stop), not
    // the two endpoints. Prefer an exposed integral start/stop pair when the range supplies one; generic
    // endpoint carriers such as std::array{start, stop} retain the established first-two-values behavior.
    if constexpr (requires {
                    range.start;
                    range.stop;
                  }) {
      using Start = std::remove_cvref_t<decltype(range.start)>;
      using Stop = std::remove_cvref_t<decltype(range.stop)>;
      if constexpr (std::integral<Start> and std::integral<Stop>) {
        return std::forward<Self>(self).slice(
            static_cast<std::size_t>(range.start), static_cast<std::size_t>(range.stop));
      }
    }

    auto iterator = std::ranges::begin(range);
    const auto last = std::ranges::end(range);
    if (iterator == last) {
      return std::forward<Self>(self).slice(0, 0);
    }
    const auto start = static_cast<std::size_t>(*iterator);
    ++iterator;
    if (iterator == last) {
      return std::forward<Self>(self).slice(start, start);
    }
    return std::forward<Self>(self).slice(start, static_cast<std::size_t>(*iterator));
  }

  /// Enumerates logical values as `(index, value)` pairs without materializing the source.
  template <class Self>
  [[nodiscard]] consteval auto enumerate(this Self &&self) {
    using Result = std::pair<std::size_t, Value>;
    auto next = appendQueryInstruction<Result>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Enumerate});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Concatenates two queries with identical logical value types.
  ///
  /// Child-query termination is local to that child: `left.take(n)` ending naturally still advances into the
  /// right source. Only downstream terminal cancellation stops the whole chain. No persistence barrier is
  /// used.
  template <class Self, class OtherState>
  [[nodiscard]] consteval auto chain(this Self &&self, Query<OtherState> other)
    requires std::same_as<Value, typename Query<OtherState>::Value>
  {
    using Source = ChainSource<State, OtherState>;
    auto next = makeInitialQueryPlan<Value>(Source{std::forward<Self>(self).state_, std::move(other.state_)});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Zips two queries until either child query is exhausted.
  ///
  /// Miracle preserves the established evaluation order in which both child queries are evaluated before
  /// pairing. Pairing storage is evaluation-local only; no child result is promoted into hidden static
  /// backing.
  template <class Self, class OtherState>
  [[nodiscard]] consteval auto zip(this Self &&self, Query<OtherState> other) {
    using Result = std::pair<Value, typename Query<OtherState>::Value>;
    using Source = ZipSource<State, OtherState>;
    auto next =
        makeInitialQueryPlan<Result>(Source{std::forward<Self>(self).state_, std::move(other.state_)});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Returns the first logical value, preserving references through C++26 `optional<T&>`.
  template <class Self>
  [[nodiscard]] consteval auto first(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return false;
    });
    return result;
  }

  /// Returns the last logical value, preserving references where the logical value is a reference.
  template <class Self>
  [[nodiscard]] consteval auto last(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return true;
    });
    return result;
  }

  /// Returns the zero-based `index`th logical value, preserving the exact logical value type.
  template <class Self>
  [[nodiscard]] consteval auto nth(this Self &&self, std::size_t index) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::size_t current{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (current++ == index) {
        result.emplace(std::forward<decltype(item)>(item));
        return false;
      }
      return true;
    });
    return result;
  }

  /// Finds the first value accepted by `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto find(this Self &&self, Predicate predicate) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).filter(std::move(predicate)).first();
  }

  /// Returns the first value produced by an Optional-like mapper.
  template <class Self, class Function>
  [[nodiscard]] consteval auto findMap(this Self &&self, Function function) {
    return std::forward<Self>(self).filterMap(std::move(function)).first();
  }

  /// Returns whether any logical value compares equal to `value`.
  template <class Self, class T>
  [[nodiscard]] consteval auto contains(this Self &&self, const T &value) -> bool {
    bool found{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (std::forward<decltype(item)>(item) == value) {
        found = true;
        return false;
      }
      return true;
    });
    return found;
  }

  /// Returns the first logical index accepted by `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto position(this Self &&self, Predicate predicate) -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = index;
        return false;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns the greatest logical index accepted by `predicate`, or no value when none matches.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto rposition(this Self &&self, Predicate predicate)
      -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = index;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns whether every logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto all(this Self &&self, Predicate predicate) -> bool {
    bool result{true};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = false;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether at least one logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto any(this Self &&self, Predicate predicate) -> bool {
    bool result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = true;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether no logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto none(this Self &&self, Predicate predicate) -> bool {
    return not std::forward<Self>(self).any(std::move(predicate));
  }

  /// Counts logical values by evaluating the fused pipeline.
  template <class Self>
  [[nodiscard]] consteval auto count(this Self &&self) -> std::size_t {
    std::size_t result{};
    std::forward<Self>(self).visit([&](auto &&) consteval -> bool {
      ++result;
      return true;
    });
    return result;
  }

  /// Returns whether the logical sequence contains no values.
  template <class Self>
  [[nodiscard]] consteval auto isEmpty(this Self &&self) -> bool {
    bool empty{true};
    std::forward<Self>(self).visit([&](auto &&) consteval -> bool {
      empty = false;
      return false;
    });
    return empty;
  }

  /// Returns the direct source cardinality before any lazy IR has been appended.
  ///
  /// Query operation topology is now value IR rather than part of the Query C++ type, so member availability
  /// can no longer encode "empty program". Calling `size()` after an adaptor is therefore a semantic error;
  /// use `count()`.
  template <class Self>
  [[nodiscard]] consteval auto size(this Self &&self)
    requires requires(const typename State::source_type &source) { source.values.size(); }
  {
    if (self.state_.program != Info{}) {
      throw std::meta::exception{
          "Query::size() is available only for an unmodified direct source; use count()",
          self.state_.program};
    }
    return self.state_.source.values.size();
  }

  /// Reverses the logical sequence while preserving references. This is an eager barrier only at the
  /// operation itself; later stages are still fused into the resulting Query state.
  template <class Self>
  [[nodiscard]] consteval auto reverse(this Self &&self) {
    if constexpr (not persistentQueryBacking<Value>) {
      rejectPersistentQueryBacking<Value>(self.state_.program);
    } else {
      std::vector<StoredT<Value>> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        return true;
      });
      std::ranges::reverse(values);
      return makeStoredQuery<Value>(std::move(values));
    }
  }

  /// Sorts logical values using ordinary ordering. Reference values are sorted by referent without
  /// changing their identity.
  template <class Self>
  [[nodiscard]] consteval auto sort(this Self &&self) {
    return std::forward<Self>(self).sortBy(std::less<>{});
  }

  /// Sorts logical values with a binary comparator. The comparator receives the exact logical value
  /// types.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto sortBy(this Self &&self, Compare compare) {
    using Stored =
        std::conditional_t<std::same_as<std::remove_cvref_t<Value>, bool>, std::uint8_t, StoredT<Value>>;
    if constexpr (not persistentQueryBacking<Value, Stored>) {
      rejectPersistentQueryBacking<Value, Stored>(self.state_.program);
    } else {
      std::vector<Stored> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        if constexpr (std::same_as<std::remove_cvref_t<Value>, bool>) {
          values.push_back(static_cast<std::uint8_t>(static_cast<bool>(item)));
        } else {
          values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        }
        return true;
      });
      for (std::size_t index{1}; index < values.size(); ++index) {
        auto current = std::move(values[index]);
        std::size_t position = index;
        while (position != 0) {
          auto &previous = values[position - 1];
          if (not static_cast<bool>(adaptiveInvokeBinary(compare, unwrap(current), unwrap(previous)))) {
            break;
          }
          values[position] = std::move(previous);
          --position;
        }
        values[position] = std::move(current);
      }
      return makeStoredQuery<Value>(std::move(values));
    }
  }

  /// Sorts logical values by an adaptive key projection while retaining the original logical elements.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto sortByKey(this Self &&self, Projection projection) {
    auto comparator = [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
      return adaptiveInvoke(projection, std::forward<decltype(left)>(left)) <
             adaptiveInvoke(projection, std::forward<decltype(right)>(right));
    };
    return std::forward<Self>(self).sortBy(std::move(comparator));
  }

  /// Returns the minimum logical value under ordinary ordering.
  template <class Self>
  [[nodiscard]] consteval auto min(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).minBy(std::less<>{});
  }

  /// Returns the maximum logical value under ordinary ordering.
  template <class Self>
  [[nodiscard]] consteval auto max(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).maxBy(std::less<>{});
  }

  /// Returns the logical minimum according to a binary ordering predicate.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto minBy(this Self &&self, Compare compare) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, item, *result))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical maximum according to a binary ordering predicate.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto maxBy(this Self &&self, Compare compare) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, *result, item))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical minimum according to an adaptive key projection.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto minByKey(this Self &&self, Projection projection) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).minBy(
        [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
          return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
        });
  }

  /// Returns the logical maximum according to an adaptive key projection.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto maxByKey(this Self &&self, Projection projection) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).maxBy(
        [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
          return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
        });
  }

  /// Removes consecutive duplicate logical values. The operation is fused by keeping only the previous
  /// value.
  template <class Self>
  [[nodiscard]] consteval auto unique(this Self &&self) {
    return std::forward<Self>(self).uniqueBy(std::identity{});
  }

  /// Removes every later logical element whose projected key has already occured, preserving encounter
  /// order.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto uniqueBy(this Self &&self, Projection projection) {
    if constexpr (not persistentQueryBacking<Value>) {
      rejectPersistentQueryBacking<Value>(self.state_.program);
    } else {
      // `unique` is implemented as a stage-like eager normalization so arbitrary reference values remain
      // references.
      using Key = std::remove_cvref_t<decltype(adaptiveInvoke(projection, std::declval<Value>()))>;
      static_assert(std::copy_constructible<Key>, "Query::uniqueBy() requires a copy-constructible key");
      std::vector<Key> keys;
      std::vector<StoredT<Value>> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        auto key = adaptiveInvoke(projection, item);
        bool duplicate{};
        for (const Key &existing : keys) {
          if (existing == key) {
            duplicate = true;
            break;
          }
        }
        if (duplicate)
          return true;
        keys.push_back(key);
        values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        return true;
      });
      return makeStoredQuery<Value>(std::move(values));
    }
  }

  /// Splits logical values into two independently persistent queries while preserving encounter order.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto partition(this Self &&self, Predicate predicate) {
    using Stored = StoredT<Value>;
    if constexpr (not persistentQueryBacking<Value, Stored>) {
      rejectPersistentQueryBacking<Value, Stored>(self.state_.program);
    } else {
      std::vector<Stored> yes;
      std::vector<Stored> rejected;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        const bool accepted = static_cast<bool>(adaptiveInvoke(predicate, item));
        auto stored = storeValueAs<Value>(std::forward<decltype(item)>(item));
        if (accepted)
          yes.push_back(std::move(stored));
        else
          rejected.push_back(std::move(stored));
        return true;
      });
      return std::pair{makeStoredQuery<Value>(std::move(yes)), makeStoredQuery<Value>(std::move(rejected))};
    }
  }

  /// Executes a compile-time callback for every logical value and returns no runtime result.
  template <class Self, class Function>
  consteval auto forEach(this Self &&self, Function function) -> void {
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      adaptiveInvoke(function, std::forward<decltype(item)>(item));
      return true;
    });
  }

  /// Promotes a value query into static standard storage. Reference-valued queries cannot be silently
  /// converted into a different element representation, so they fail instead of manufacturing a wrapper
  /// type.
  template <class Self>
  [[nodiscard]] consteval auto materialize(this Self &&self) {
    requireMaterializable<Value>();
    using Element = std::remove_cvref_t<Value>;
    std::vector<Element> values;
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      values.emplace_back(std::forward<decltype(item)>(item));
      return true;
    });
    return promoteVector(std::move(values));
  }

private:
  template <class StateType>
  friend struct Query;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(const Query<StateType> &, Function &&) -> bool
    requires std::copy_constructible<StateType>;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(Query<StateType> &&, Function &&) -> bool;

  template <class Self, class Function>
  consteval auto visit(this Self &&self, Function function) -> bool {
    return evaluateState(std::forward<Self>(self).state_, std::move(function));
  }

  State state_;
};

} // namespace Miracle::meta

namespace Miracle::meta {

template <class State, class Function>
consteval auto evaluateQuery(const Query<State> &query, Function &&function) -> bool
  requires std::copy_constructible<State>
{
  return evaluateState(query.state_, std::forward<Function>(function));
}

template <class State, class Function>
consteval auto evaluateQuery(Query<State> &&query, Function &&function) -> bool {
  return evaluateState(std::move(query.state_), std::forward<Function>(function));
}

/// Promotes a reordered/persisted logical sequence and returns a new Query sourcing that static backing.
///
/// Internal reference storage is allowed through pointers, but the stored representation itself must
/// still be structural so `std::define_static_array` can legally provide program-lifetime backing.
template <class Value, class Stored>
consteval auto makeStoredQuery(std::vector<Stored> values) {
  if constexpr (not persistentQueryBacking<Value, Stored>) {
    rejectPersistentQueryBacking<Value, Stored>();
  } else {
    auto span = promoteVector(values);
    using Source = StoredSource<Value, Stored>;
    auto state = makeInitialQueryPlan<Value>(Source{span});
    return Query<decltype(state)>{std::move(state)};
  }
}

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Creates a Meta Query over a finite static span without copying its source.
/// The span follows ordinary `std::span` lifetime rules: its referenced storage must remain valid for
/// every Query evaluation that uses it. Query does not take ownership of external span storage.
template <class T, std::size_t Extent>
[[nodiscard]] consteval auto query(std::span<T, Extent> source) {
  using Source = SpanSource<T>;
  auto state = makeInitialQueryPlan<T &>(Source{std::span<T>{source}});
  return Query<decltype(state)>{std::move(state)};
}

/// Creates a Meta Query over a mutable lvalue array without copying its source or discarding mutability.
template <class T, std::size_t N>
[[nodiscard]] consteval auto query(std::array<T, N> &source) {
  return query(std::span<T>{source});
}

/// Creates a Meta Query over a const lvalue array without copying its source.
template <class T, std::size_t N>
[[nodiscard]] consteval auto query(const std::array<T, N> &source) {
  return query(std::span<const T>{source});
}

template <class T, std::size_t N>
consteval auto query(std::array<T, N> &&) = delete (
    "Query cannot borrow a temporary array; keep the source alive or materialize it first");

template <class T, std::size_t N>
consteval auto query(const std::array<T, N> &&) = delete (
    "Query cannot borrow a temporary array; keep the source alive or materialize it first");

} // namespace Miracle::meta
