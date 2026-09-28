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
}

/// Enforces a class-only semantic precondition while preserving the offending reflection in the diagnostic.
consteval auto requireClass(Info subject, std::string_view message) -> void {
  if (not std::meta::is_type(subject) or not std::meta::is_class_type(subject)) {
    throw std::meta::exception{message, subject};
  }
}

/// Enforces a namespace-or-record semantic precondition for member-source operations.
consteval auto requireMemberScope(Info subject, std::string_view message) -> void {
  if (not isMemberScope(subject)) {
    throw std::meta::exception{message, subject};
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
    if (not std::meta::is_function(Subject) and not functionType and
        not std::meta::is_function_template(Subject)) {
      throw std::meta::exception{"parameters() requires a function reflection", Subject};
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
/// `categoryCacheRef`, invoke that specialization during constant evaluation, then extract its structural
/// pointer/size result. This lets `meta::fields(^^T)` and `meta::fields<T>()` converge on the exact same
/// cache specialization while the public API stays value-oriented. The bridge performs no reflection source
/// query itself and assumes the public source API has already validated `subject`; validation cannot be
/// deferred into a failing cache variable initializer without changing error semantics.
[[nodiscard]] consteval auto cachedCategory(Info subject, CacheCategory category) -> std::span<const Info> {
  const std::array arguments{std::meta::reflect_constant(subject), std::meta::reflect_constant(category)};
  const Info specialization = std::meta::substitute(^^categoryCacheRef, arguments);
  const Info reflectedResult = std::meta::reflect_invoke(specialization, {});
  const auto result = std::meta::extract<InfoSequenceRef>(reflectedResult);
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
/// `access_context` is structural in C+26, so the same reflection/substitution technique used by
/// `cachedCategory` can specialize by caller access without exposing template-state publicly. Unchecked
/// access bypasses this bridge entirely because the canonical cache is already the exact requested universe.
[[nodiscard]] consteval auto cachedVisibleCategory(Info subject, CacheCategory category, Access access)
    -> std::span<const Info> {
  if (isUnchecked(access)) {
    return cachedCategory(subject, category);
  }

  const std::array arguments{std::meta::reflect_constant(subject),
      std::meta::reflect_constant(category),
      std::meta::reflect_constant(access)};
  const Info specialization = std::meta::substitute(^^visibleCategoryCacheRef, arguments);
  const Info reflectedResult = std::meta::reflect_invoke(specialization, {});
  const auto result = std::meta::extract<InfoSequenceRef>(reflectedResult);
  return {result.data, result.size};
}

/// Type-oriented access-filtered cache lookup.
///
/// Keeping this helper separate preserves the fast direct NTTP path for typed unchecked queries while sending
/// all other access contexts through the same access-cache specialization used by raw-`Info` calls.
template <class T, CacheCategory Category>
[[nodiscard]] consteval auto cachedVisibleCategory(Access access) -> std::span<const Info> {
  if (isUnchecked(access)) {
    return categoryCache<^^T, Category>;
  }
  return cachedVisibleCategory(^^T, Category, access);
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
  const std::array arguments{
      std::meta::reflect_constant(subject), std::meta::reflect_constant(annotationType)};
  const Info specialization = std::meta::substitute(^^typedAnnotationsCacheRef, arguments);
  const Info reflectedResult = std::meta::reflect_invoke(specialization, {});
  const auto result = std::meta::extract<InfoSequenceRef>(reflectedResult);
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

/// Removes one standard reflection alias layer while preserving exact standard reflection identity semantics.
struct Dealias final {
  /// Applies the standard dealiasing operation to `info`.
  [[nodiscard]] consteval auto operator()(Info info) const -> Info {
    return std::meta::dealias(info);
  }
};
/// Stateless callable instance of `Dealias`.
inline constexpr Dealias dealias{};

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
/// later phases.
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
  return cachedCategory(subject, CacheCategory::Enumerators);
}

/// Type-oriented enum source. The constraint makes non-enum misuse an overload-formation error instead of a
/// runtime-like semantic probe; use the raw-`Info` overload when the reflected category is only known during
/// constant evaluation.
template <class E>
  requires std::is_enum_v<E>
[[nodiscard]] consteval auto enumerators() -> std::span<const Info> {
  return categoryCache<^^E, CacheCategory::Enumerators>;
}

/// Parameter-source callable for reflected functions, function types, and function templates.
///
/// Results preserve source order and use static backing storage. Other reflection categories throw
/// `std::meta::exception`. A callable object is used instead of a plain function so a later Miracle can pass
/// `meta::parameters` directly to `flatMap` without changing the established call syntax.
struct Parameters final {
  /// Returns the statically backed parameter sequence for `subject`.
  [[nodiscard]] consteval auto operator()(Info subject) const -> std::span<const Info> {
    const bool functionType = std::meta::is_type(subject) and std::meta::is_function_type(subject);
    if (not std::meta::is_function(subject) and not functionType and
        not std::meta::is_function_template(subject)) {
      throw std::meta::exception{"parameters() requires a function reflection", subject};
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
  static constexpr bool miracleMetaPredicate_ = true;

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

/// Matches reflections denoting C++ types.
inline constexpr Predicate isType{[](Info info) consteval -> bool { return std::meta::is_type(info); }};
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
/// Matches concept reflections.
inline constexpr Predicate isConcept{[](Info info) consteval -> bool { return std::meta::is_concept(info); }};
/// Matches individual annotation reflections.
inline constexpr Predicate isAnnotation{
    [](Info info) consteval -> bool { return std::meta::is_annotation(info); }};
/// Matches reflections whose declared access is public.
inline constexpr Predicate isPublic{[](Info info) consteval -> bool { return std::meta::is_public(info); }};
/// Matches reflections whose declared access is protected.
inline constexpr Predicate isProjected{
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

/// Computes the logical element type produced by flattening either an input range or another Query.
///
/// Lvalue ranges preserve their reference type; rvalue ranges use their value type so no Query can retain a
/// reference into a temporary range after the flattening call completes.
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

/// Flat value-state representation carried by `Query<State>`.
///
/// The source is stored once and every lazy adaptor appends one compact stage value to `stages`; no adaptor
/// wraps the previous Query type recursively. `Value` is the exact logical element type after all recorded
/// stages.
template <class Source, class Value, class... Stages>
struct QueryState final {
  using source_type = Source;
  using value_type = Value;
  using stages_tuple = std::tuple<Stages...>;

  Source source;
  std::tuple<Stages...> stages;
};

/// Borrowed static-span source. Query never extends the lifetime of `values`.
template <class Element>
struct SpanSource final {
  std::span<Element> values;
  using value_type = Element &;
};

/// Stateless-or-captured projection stage; callable state is stored inline with empty-state optimization.
template <class Function>
struct MapStage final {
  [[no_unique_address]] Function function;
};
/// Predicate stage that drops individual values without terminating source traversal.
template <class Predicate>
struct FilterStage final {
  [[no_unique_address]] Predicate predicate;
};
/// Optional-like projection stage combining transformation and rejection in one source pass.
template <class Function>
struct FilterMapStage final {
  [[no_unique_address]] Function function;
};
/// Marker stage expanding nested Query/range values into the surrounding pipeline.
struct FlatenStage final {};
template <class Function>
/// Observation stage whose callable runs without changing the logical value.
struct InspectStage final {
  [[no_unique_address]] Function function;
};
/// Stateful prefix stage; `active` becomes false permanently after the first predicate failure.
template <class Predicate>
struct TakeWhileStage final {
  [[no_unique_address]] Predicate predicate;
  bool active{true};
};
/// Stateful prefix-skipping stage; `skipping` becomes false permanently at the first predicate rejection.
template <class Predicate>
struct SkipWhileStage final {
  [[no_unique_address]] Predicate predicate;
  bool skipping{true};
};
/// Fixed cardinality limiter; `remaining` also drives upstream short-circuiting.
struct TakeStage final {
  std::size_t remaining{};
};
/// Fixed prefix drop counter.
struct SkipStage final {
  std::size_t remaining{};
};
/// Index state counting only values that reach this stage.
struct EnumerateStage final {
  std::size_t index{};
};
/// Fused `[start, end)` state avoiding separate skip/take stage objects.
struct SliceStage final {
  std::size_t skip{};
  std::size_t take{};
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
/// Reference-valued queries store pointers and recover the original references through `unwrap`; owning
/// values are stored directly. The source therefore preserves Query's logical `Value` even across a
/// persistence barrier.
template <class Value, class Stored>
struct StoredSource final {
  std::span<const Stored> values;
  using value_type = Value;
};

/// Compile-time stage classifiers keep `emit` as one flat dispatcher without introducing virtual/tagged
/// hierarchy.
template <class>
struct IsMap : std::false_type {};
template <class F>
struct IsMap<MapStage<F>> : std::true_type {
  using Function = F;
};
template <class>
struct IsFilter : std::false_type {};
template <class F>
struct IsFilter<FilterStage<F>> : std::true_type {
  using Function = F;
};
template <class>
struct IsFilterMap : std::false_type {};
template <class F>
struct IsFilterMap<FilterMapStage<F>> : std::true_type {
  using Function = F;
};
template <class>
struct IsFlatten : std::false_type {};
template <>
struct IsFlatten<FlatenStage> : std::true_type {};
template <class>
struct IsInspect : std::false_type {};
template <class F>
struct IsInspect<InspectStage<F>> : std::true_type {
  using Function = F;
};
template <class>
struct IsTakeWhile : std::false_type {};
template <class F>
struct IsTakeWhile<TakeWhileStage<F>> : std::true_type {
  using Predicate = F;
};
template <class>
struct IsSkipWhile : std::false_type {};
template <class F>
struct IsSkipWhile<SkipWhileStage<F>> : std::true_type {
  using Predicate = F;
};

/// Invokes a Query callable using the richest shape the callable explicitly accepts.
///
/// Dispatch is ordered deliberately: an exact logical-value overload wins first, tuple protocol expansion is
/// second, and reflected aggregate-field expansion is the final structural fallback. This prevents a callable
/// that already accepts the object itself from being unexpectedly decomposed and keeps the adaptation local
/// to Meta Query.
template <class Function, class Item>
consteval auto adaptiveInvoke(Function &&function, Item &&item) -> decltype(auto) {
  if constexpr (std::invocable<Function, Item>) {
    // Preserve the logical value/category when the callable already understands it; decomposition would only
    // add compiler work and could select a different overload set.
    return std::invoke(std::forward<Function>(function), std::forward<Item>(item));
  } else if constexpr (requires { typename std::tuple_size<std::remove_cvref_t<Item>>::type; }) {
    // Tuple-like values use their established tuple protocol before reflection. This covers pair/tuple/custom
    // tuple-like values without requires them to be aggregates or exposing implementation members.
    constexpr auto count = std::tuple_size_v<std::remove_cvref_t<Item>>;
    return [&]<std::size_t... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      static_assert(std::invocable<Function, decltype(std::get<Index>(std::declval<Item>()))...>,
          "Miracle Meta Query invocation requires a callable accepting the item or its tuple elements");
      return std::invoke(std::forward<Function>(function), std::get<Index>(std::forward<Item>(item))...);
    }(std::make_index_sequence<count>{});
  } else if constexpr (std::is_aggregate_v<std::remove_cvref_t<Item>>) {
    // Reuse Miracle's reflection façade instead of issuing a parallel raw <meta> query. `fields()` is the
    // exact semantic match for aggregate decomposition: only non-static data members participate, in
    // declaration order.
    constexpr auto fields = reflect<std::remove_cvref_t<Item>>().fields();
    return [&]<std::size_t... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      // Validate the fully expanded call before splicing. The generated indices are bounded by fields.size(),
      // so the reflected member accesses below cannot escape the cached field universe.
      static_assert(
          requires(Function &&function, Item &&value) {
            std::invoke(std::forward<Function>(function),
                std::forward<Item>(value).[:fields[Index
            ]:]...); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
                     // - Index is generated by make_index_sequence.
          },
          "Miracle Meta Query invocation requires a callable accepting the item, its tuple elements, or its "
          "reflected members");
      return std::invoke(std::forward<Function>(function),
          std::forward<Item>(item)
              .[:fields[Index]:]...); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    }(std::make_index_sequence<fields.size()>{});
  } else {
    // Reaching this branch means no supported invocation shape exists. Keep the diagnostic at the adaptation
    // boundary instead of allowing deeper std::invoke substitution failure to obscure the Query operation
    // that caused it.
    static_assert(std::invocable<Function, Item>,
        "Miracle Meta Query invocation requires a callable accepting the item, its tuple elements, or its "
        "reflected members");
  }
}

/// Invokes a binary Query callable while preserving both logical value categories.
///
/// Binary operations currently have no tuple/aggregate decomposition semantics; keeping that boundary
/// explicit avoids surprising comparator/reducer behavior and leaves room for a separately designed binary
/// adaptation policy later.
template <class Function, class Left, class Right>
consteval auto adaptiveInvokeBinary(Function &&function, Left &&left, Right &&right) -> decltype(auto) {
  if constexpr (std::invocable<Function, Left, Right>) {
    // Forward both values exactly so reference-sensitive comparators and reducers observe the Query's logical
    // types.
    return std::invoke(
        std::forward<Function>(function), std::forward<Left>(left), std::forward<Right>(right));
  } else {
    // Diagnose at the Query boundary rather than surfacing an implementation-level std::invoke substitution
    // trace.
    static_assert(std::invocable<Function, Left, Right>,
        "Miracle Meta Query binary invocation requires a callable accepting both logical values");
  }
}

/// Minimal optional protocol required by `filterMap`; deliberately avoids coupling the Query engine to one
/// wrapper type.
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

/// C++26 `std::optional<T&>` exposes `value_type` as `T`, so the reference must be restored explicitly for
/// Query's logical value type. Ordinary `std::optional<T>` retains its owning `T` result type.
template <class T>
struct OptionalResult<std::optional<T &>> final {
  using type = T &;
};

template <class T>
using OptionalResultT = OptionalResult<std::remove_cvref_t<T>>::type;

/// Chooses the static-storage representation for one logical Query value.
///
/// References become pointers so persistence never copies or silently owns the referent; object values remain
/// values.
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
    // Persist only the address; copying the referent here would violate Query's exact-reference contract.
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

/// Enforces the exact conditions under which Query may expose a persistent standard span.
///
/// References are intentionally rejected even though the engine can internally persist them as pointers:
/// public `materialize()` must not change `T&` into `T*` or copy the referent behind the caller's back.
template <class T>
constexpr auto requireMaterializable() -> void {
  if constexpr (std::is_reference_v<T> or not std::is_object_v<std::remove_cvref_t<T>> or
                not std::copy_constructible<std::remove_cvref_t<T>> or
                not std::meta::is_structural_type(^^std::remove_cvref_t<T>)) {
    throw std::meta::exception{"Query::materialize() requires a copy-constructible object value", {}};
  }
}

template <class State, class Sink>
consteval auto evaluateState(State state, Sink &&sink) -> bool;

/// Pushes one logical value through the remaining flat Query stages.
///
/// The recursive template index is compile-time stage dispatch, not input recursion: each instantiation
/// represents one position in the finite stage tuple. Runtime-like state (`remaining`, `skipping`, `active`,
/// indices) lives in the copied stage values, allowing one source traversal without constructing intermediate
/// compile-time containers. Returning `false` propagates terminal short-circuiting back to the source
/// traversal.
template <std::size_t Index, class Tuple, class Item, class Sink>
// NOLINTBEGIN(readability-function-cognitive-complexity, readability-function-size) - one dispatcher keeps
// the normalized state machine flat.
consteval auto emit(Tuple &stages, Item &&item, Sink &sink) -> bool {
  if constexpr (Index == std::tuple_size_v<std::remove_reference_t<Tuple>>) {
    // The terminal sink receives one logical value and decides whether upstream source traversal continues.
    return static_cast<bool>(sink(std::forward<Item>(item)));
  } else {
    auto &stage = std::get<Index>(stages);
    using Stage = std::remove_cvref_t<decltype(stage)>;

    if constexpr (IsMap<Stage>::value) {
      // Map replaces the logical value but does not materialize it; decltype(auto) forwarding preserves
      // references.
      auto &&mapped = adaptiveInvoke(stage.function, std::forward<Item>(item));
      return emit<Index + 1>(stages, std::forward<decltype(mapped)>(mapped), sink);
    } else if constexpr (IsFilter<Stage>::value) {
      // Rejection consumes only this value; acceptance forwards the unchanged logical value.
      if (static_cast<bool>(adaptiveInvoke(stage.predicate, item)))
        return emit<Index + 1>(stages, std::forward<Item>(item), sink);
      return true;
    } else if constexpr (IsFilterMap<Stage>::value) {
      // Empty optionals skip one value; engaged C++26 optional references must remain references.
      auto result = adaptiveInvoke(stage.function, std::forward<Item>(item));
      static_assert(OptionalLike<decltype(result)>, "Query::filterMap() requires an Optional-like result");
      if (not result.has_value())
        return true;
      using ResultValue = OptionalResultT<decltype(result)>;
      if constexpr (std::is_reference_v<ResultValue>) {
        // Dereferencing optional<T&> yields the original referent; forward that lvalue to preserve
        // Query::Value == T&.
        return emit<Index + 1>(stages, *result, sink);
      } else {
        // Owning optional-like results may transfer their contained value into the remainder of the pipeline.
        return emit<Index + 1>(stages, std::move(*result), sink);
      }
    } else if constexpr (IsFlatten<Stage>::value) {
      // Nested Queries/ranges stream directly into later stages without an intermediate flattened sequence.
      if constexpr (requires { evaluateQuery(std::declval<Item &>(), sink); }) {
        // Nested Query evaluation preserves the same downstream short-circuiting signal.
        return evaluateQuery(std::forward<Item>(item), [&](auto &&nested) consteval -> bool {
          return emit<Index + 1>(stages, std::forward<decltype(nested)>(nested), sink);
        });
      } else if constexpr (std::ranges::input_range<std::remove_cvref_t<Item>>) {
        // Range flattening forwards directly and stops consuming the nested range on downstream termination.
        for (auto &&nested : std::forward<Item>(item)) {
          if (not emit<Index + 1>(stages, std::forward<decltype(nested)>(nested), sink)) {
            return false;
          }
        }
        return true;
      } else {
        static_assert(std::ranges::input_range<std::remove_cvref_t<Item>>,
            "Query::flatten() requires a Query or an input range value");
      }
    } else if constexpr (IsInspect<Stage>::value) {
      // Inspect observes the current logical value for consteval side effects and then forwards that same
      // value unchanged.
      adaptiveInvoke(stage.function, item);
      return emit<Index + 1>(stages, std::forward<Item>(item), sink);
    } else if constexpr (IsTakeWhile<Stage>::value) {
      // Once the predicate first fails, the stage becomes permanently inactive and terminates the entire
      // source walk.
      if (not stage.active)
        return false;
      if (static_cast<bool>(adaptiveInvoke(stage.predicate, item)))
        return emit<Index + 1>(stages, std::forward<Item>(item), sink);
      stage.active = false;
      return false;
    } else if constexpr (IsSkipWhile<Stage>::value) {
      // The first rejection permanently ends prefix skipping; every later value flows through.
      if (stage.skipping and static_cast<bool>(adaptiveInvoke(stage.predicate, item))) {
        return true;
      }
      stage.skipping = false;
      return emit<Index + 1>(stages, std::forward<Item>(item), sink);
    } else if constexpr (std::same_as<Stage, TakeStage>) {
      // `remaining` is both a per-value gate and a source short-circuit budget; zero means no later source
      // value matters.
      if (stage.remaining == 0)
        return false;
      --stage.remaining;
      const bool keepGoing = emit<Index + 1>(stages, std::forward<Item>(item), sink);
      return keepGoing and stage.remaining != 0;
    } else if constexpr (std::same_as<Stage, SkipStage>) {
      // Consume the requested prefix locally, then forward every subsequent value unchanged.
      if (stage.remaining != 0) {
        --stage.remaining;
        return true;
      }
      return emit<Index + 1>(stages, std::forward<Item>(item), sink);
    } else if constexpr (std::same_as<Stage, EnumerateStage>) {
      // Enumeration state advances only for values that reach this stage, matching adaptor order rather than
      // source index.
      auto pair = std::pair<std::size_t, Item>{stage.index++, std::forward<Item>(item)};
      return emit<Index + 1>(stages, std::move(pair), sink);
    } else if constexpr (std::same_as<Stage, SliceStage>) {
      // Slice fuses skip + bounded take into one state object so it needs no intermediate Query or second
      // traversal.
      if (stage.skip != 0) {
        --stage.skip;
        return true;
      }
      if (stage.take == 0)
        return false;
      --stage.take;
      const bool keepGoing = emit<Index + 1>(stages, std::forward<Item>(item), sink);
      return keepGoing and stage.take != 0;
    }
  }
}
// NOLINTEND(readability-function-cognitive-complexity, readability-function-size)

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

/// Emits a persistent source after restoring each stored representation to its logical value/reference.
template <class Value, class Stored, class Sink>
consteval auto emitSource(StoredSource<Value, Stored> source, Sink &sink) -> bool {
  for (auto &stored : source.values) {
    if (not sink(unwrap(stored))) {
      return false;
    }
  }
  return true;
}

/// Emits chained query states lazily; downstream termination prevents evaluation of the remaining/right
/// source.
template <class LeftState, class RightState, class Sink>
consteval auto emitSource(ChainSource<LeftState, RightState> source, Sink &sink) -> bool {
  if (not evaluateState(std::move(source.left), sink)) {
    return false;
  }
  return evaluateState(std::move(source.right), sink);
}

/// Evaluates both zip inputs into structural static storage, then emits pairs up to the shorter cardinality.
///
/// Zip needs random paired access across two independent source machines, so this is an intentional
/// persistence barrier rather than an intermediate created by ordinary element-wise fusion.
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
  const auto leftValues = promoteVector(left);
  const auto rightValues = promoteVector(right);
  const auto count = std::min(leftValues.size(), rightValues.size());
  for (std::size_t index{}; index < count; ++index) {
    auto item = std::pair<LeftValue, RightValue>{unwrap(leftValues[index]), unwrap(rightValues[index])};
    if (not sink(std::move(item))) {
      return false;
    }
  }
  return true;
}

/// Evaluates one Query state by copying mutable stage counters and streaming the source through the flat
/// dispatcher.
///
/// Copying state makes every Query terminal observationally independent: evaluating `take`, `skipWhile`, or
/// enumerate never mutates the reusable Query value held by the caller.
template <class State, class Sink>
consteval auto evaluateState(State state, Sink &&sink) -> bool {
  auto stages = state.stages;
  auto emitSink = [&](auto &&item) consteval -> bool {
    return emit<0>(stages, std::forward<decltype(item)>(item), sink);
  };
  return emitSource(state.source, emitSink);
}

/// Type-level append operation for the flat state tuple; this is linear state description, not recursive
/// Query nesting.
template <class State, class NewValue, class... NewStages>
struct StateRebind;

template <class Source, class OldValue, class... OldStages, class NewValue, class... NewStages>
struct StateRebind<QueryState<Source, OldValue, OldStages...>, NewValue, NewStages...> {
  using type = QueryState<Source, NewValue, OldStages..., NewStages...>;
};

template <class State, class NewValue, class Stage>
using AppendState = StateRebind<State, NewValue, Stage>::type;

/// Declared before the public Query definition because ordering/set operations call it from exported template
/// bodies.
template <class Value>
consteval auto makeStoredQuery(std::vector<StoredT<Value>> values);

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Finite, compile-time-only metadata query algebra.
///
/// `State` contains one source value and one flat tuple of normalized stages. The query is not a runtime
/// range and deliberately does not inherit from a range/view hierarchy. Every public operation is immediate
/// and returns a new value-state query. Logical reference types are preserved throughout fused stages.
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
  template <class Function>
  [[nodiscard]] consteval auto map(Function function) const {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(
        not std::is_void_v<Result>, "Query::map() cannot return void; use forEach() for terminal work");
    using NextState = AppendState<State, Result, MapStage<Function>>;
    return Query<NextState>{NextState{
        state_.source, std::tuple_cat(state_.stages, std::tuple{MapStage<Function>{std::move(function)}})}};
  }

  /// Keeps values accepted by a consteval predicate while preserving the exact logical value type.
  template <class Predicate>
  [[nodiscard]] consteval auto filter(Predicate predicate) const {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(std::convertible_to<Result, bool>, "Query::filter() predicate must be boolean-convertible");
    using NextState = AppendState<State, Value, FilterStage<Predicate>>;
    return Query<NextState>{NextState{state_.source,
        std::tuple_cat(state_.stages, std::tuple{FilterStage<Predicate>{std::move(predicate)}})}};
  }

  /// Maps values to Optional-like results and emits only engaged results.
  /// C++26 `std::optional<T&>` results preserve `T&` as the next logical value; ordinary owning optionals
  /// produce their `value_type`.
  template <class Function>
  [[nodiscard]] consteval auto filterMap(Function function) const {
    using Maybe = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(OptionalLike<Maybe>, "Query::filterMap() requires an Optional-like result");
    using Result = OptionalResultT<Maybe>;
    using NextState = AppendState<State, Result, FilterMapStage<Function>>;
    return Query<NextState>{NextState{state_.source,
        std::tuple_cat(state_.stages, std::tuple{FilterMapStage<Function>{std::move(function)}})}};
  }

  /// Flattens a map result that is itself a finite range or another Query.
  template <class Function>
  [[nodiscard]] consteval auto flatMap(Function function) const {
    return map(std::move(function)).flatten();
  }

  /// Flattens one level of Query or input-range values while keeping the operation in the same fused state
  /// tuple.
  [[nodiscard]] consteval auto flatten() const {
    static_assert(requires(Value value) { value; }, "Query::flatten() requires a logical value type");
    using Result = FlattenValue<Value>::type;
    using NextState = AppendState<State, Result, FlatenStage>;
    return Query<NextState>{
        NextState{state_.source, std::tuple_cat(state_.stages, std::tuple{FlatenStage{}})}};
  }

  /// Observes each logical value without changing the sequence.
  template <class Function>
  [[nodiscard]] consteval auto inspect(Function function) const {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(std::same_as<Result, void>, "Query::inspect() callback must return void");
    using NextState = AppendState<State, Value, InspectStage<Function>>;
    return Query<NextState>{NextState{state_.source,
        std::tuple_cat(state_.stages, std::tuple{InspectStage<Function>{std::move(function)}})}};
  }

  /// Stops after `count` logical values. The operation remains fused with preceding stages.
  [[nodiscard]] consteval auto take(std::size_t count) const {
    using NextState = AppendState<State, Value, TakeStage>;
    return Query<NextState>{
        NextState{state_.source, std::tuple_cat(state_.stages, std::tuple{TakeStage{count}})}};
  }

  /// Skips the first `count` logical values.
  [[nodiscard]] consteval auto skip(std::size_t count) const {
    using NextState = AppendState<State, Value, SkipStage>;
    return Query<NextState>{
        NextState{state_.source, std::tuple_cat(state_.stages, std::tuple{SkipStage{count}})}};
  }

  /// Takes values while the predicate remains true; the first failure terminates traversal.
  template <class Predicate>
  [[nodiscard]] consteval auto takeWhile(Predicate predicate) const {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::takeWhile() predicate must be boolean-convertible");
    using NextState = AppendState<State, Value, TakeWhileStage<Predicate>>;
    return Query<NextState>{NextState{state_.source,
        std::tuple_cat(state_.stages, std::tuple{TakeWhileStage<Predicate>{std::move(predicate)}})}};
  }

  /// Skips values while the predicate remains true, then passes every later value.
  template <class Predicate>
  [[nodiscard]] consteval auto skipWhile(Predicate predicate) const {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::skipWhile() predicate must be boolean-convertible");
    using NextState = AppendState<State, Value, SkipWhileStage<Predicate>>;
    return Query<NextState>{NextState{state_.source,
        std::tuple_cat(state_.stages, std::tuple{SkipWhileStage<Predicate>{std::move(predicate)}})}};
  }

  /// Selects a half-open positional slice `[start, stop)` using the same semantics as Miracle Range.
  [[nodiscard]] consteval auto slice(std::size_t start, std::size_t stop) const {
    using NextState = AppendState<State, Value, SliceStage>;
    return Query<NextState>{NextState{
        state_.source, std::tuple_cat(state_.stages, std::tuple{SliceStage{.skip = start, .take = stop}})}};
  }

  /// Accepts any finite integral endpoint range, including Miracle's `Range`, as `[start, stop)`.
  template <std::ranges::input_range Range>
    requires std::integral<std::ranges::range_value_t<Range>>
  [[nodiscard]] consteval auto slice(Range range) const {
    auto iterator = std::ranges::begin(range);
    const auto last = std::ranges::end(range);
    if (iterator == last) {
      return slice(0, 0);
    }
    const auto start = static_cast<std::size_t>(*iterator);
    ++iterator;
    if (iterator == last) {
      return slice(start, start);
    }
    return slice(start, static_cast<std::size_t>(*iterator));
  }

  /// Enumerates logical values as `(index, value)` pairs without materializing the source.
  [[nodiscard]] consteval auto enumerate() const {
    using Result = std::pair<std::size_t, Value>;
    using NextState = AppendState<State, Result, EnumerateStage>;
    return Query<NextState>{
        NextState{state_.source, std::tuple_cat(state_.stages, std::tuple{EnumerateStage{}})}};
  }

  /// Concatenates two queries with identical logical value types. Each source is evaluated only when
  /// traversal reaches it.
  template <class OtherState>
  [[nodiscard]] consteval auto chain(Query<OtherState> other) const
    requires std::same_as<Value, typename Query<OtherState>::Value>
  {
    using Source = ChainSource<State, OtherState>;
    using NextState = QueryState<Source, Value>;
    return Query<NextState>{NextState{Source{state_, other.state_}, {}}};
  }

  /// Zips two queries until either source is exhausted. The pair preserves both logical element types
  /// exactly.
  template <class OtherState>
  [[nodiscard]] consteval auto zip(Query<OtherState> other) const {
    using Result = std::pair<Value, typename Query<OtherState>::Value>;
    using Source = ZipSource<State, OtherState>;
    using NextState = QueryState<Source, Result>;
    return Query<NextState>{NextState{Source{state_, other.state_}, {}}};
  }

  /// Returns the first logical value, preserving references through C++26 `optional<T&>`.
  [[nodiscard]] consteval auto first() const -> std::optional<Value> {
    std::optional<Value> result{};
    visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return false;
    });
    return result;
  }

  /// Returns the last logical value, preserving references where the logical value is a reference.
  [[nodiscard]] consteval auto last() const -> std::optional<Value> {
    std::optional<Value> result{};
    visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return true;
    });
    return result;
  }

  /// Returns the zero-based `index`th logical value, preserving the exact logical value type.
  [[nodiscard]] consteval auto nth(std::size_t index) const -> std::optional<Value> {
    std::optional<Value> result{};
    std::size_t current{};
    visit([&](auto &&item) consteval -> bool {
      if (current++ == index) {
        result.emplace(std::forward<decltype(item)>(item));
        return false;
      }
      return true;
    });
    return result;
  }

  /// Finds the first value accepted by `predicate`.
  template <class Predicate>
  [[nodiscard]] consteval auto find(Predicate predicate) const -> std::optional<Value> {
    return filter(std::move(predicate)).first();
  }

  /// Returns the first value produced by an Optional-like mapper.
  template <class Function>
  [[nodiscard]] consteval auto findMap(Function function) const {
    return filterMap(std::move(function)).first();
  }

  /// Returns whether any logical value compares equal to `value`.
  template <class T>
  [[nodiscard]] consteval auto contains(const T &value) const -> bool {
    bool found{};
    visit([&](auto &&item) consteval -> bool {
      if (item == value) {
        found = true;
        return false;
      }
      return true;
    });
    return found;
  }

  /// Returns the first logical index accepted by `predicate`.
  template <class Predicate>
  [[nodiscard]] consteval auto position(Predicate predicate) const -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, item))) {
        result = index;
        return false;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns the greatest logical index accepted by `predicate`, or no value when none matches.
  template <class Predicate>
  [[nodiscard]] consteval auto rposition(Predicate predicate) const -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, item))) {
        result = index;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns whether every logical value satisfies `predicate`.
  template <class Predicate>
  [[nodiscard]] consteval auto all(Predicate predicate) const -> bool {
    bool result{true};
    visit([&](auto &&item) consteval -> bool {
      if (not static_cast<bool>(adaptiveInvoke(predicate, item))) {
        result = false;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether at least one logical value satisfies `predicate`.
  template <class Predicate>
  [[nodiscard]] consteval auto any(Predicate predicate) const -> bool {
    bool result{};
    visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, item))) {
        result = true;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether no logical value satisfies `predicate`.
  template <class Predicate>
  [[nodiscard]] consteval auto none(Predicate predicate) const -> bool {
    return not any(std::move(predicate));
  }

  /// Counts logical values by evaluating the fused pipeline.
  [[nodiscard]] consteval auto count() const -> std::size_t {
    std::size_t result{};
    visit([&](auto &&) consteval -> bool {
      ++result;
      return true;
    });
    return result;
  }

  /// Returns whether the logical sequence contains no values.
  [[nodiscard]] consteval auto isEmpty() const -> bool {
    return not first().has_value();
  }

  /// Returns the exact source cardinality when every stage preserves it structurally.
  ///
  /// A filtered or otherwise data-dependent query deliberately has no `size()` member; callers use `count()`
  /// when evaluation is required. This implementation recognizes only the statically obvious zero-cost cases.
  [[nodiscard]] consteval auto size() const
    requires(std::same_as<typename State::stages_tuple, std::tuple<>>)
  {
    return state_.source.values.size();
  }

  /// Reverses the logical sequence while preserving references. This is an eager barrier only at the
  /// operation itself; later stages are still fused into the resulting Query state.
  [[nodiscard]] consteval auto reverse() const {
    std::vector<StoredT<Value>> values;
    visit([&](auto &&item) consteval -> bool {
      values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
      return true;
    });
    std::ranges::reverse(values);
    return makeStoredQuery<Value>(std::move(values));
  }

  /// Sorts logical values using ordinary ordering. Reference values are sorted by referent without changing
  /// their identity.
  [[nodiscard]] consteval auto sort() const {
    return sortBy(std::less<>{});
  }

  /// Sorts logical values with a binary comparator. The comparator receives the exact logical value types.
  template <class Compare>
  [[nodiscard]] consteval auto sortBy(Compare compare) const {
    using Stored =
        std::conditional_t<std::same_as<std::remove_cvref_t<Value>, bool>, std::uint8_t, StoredT<Value>>;
    std::vector<Stored> values;
    visit([&](auto &&item) consteval -> bool {
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
        if (not static_cast<bool>(adaptiveInvokeBinary(compare, unwrap(previous), unwrap(current)))) {
          break;
        }
        values[position] = std::move(previous);
        --position;
      }
      values[position] = std::move(current);
    }
    return makeStoredQuery<Value>(std::move(values));
  }

  /// Sorts logical values by an adaptive key projection while retaining the original logical elements.
  template <class Projection>
  [[nodiscard]] consteval auto sortByKey(Projection projection) const {
    auto comparator = [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
      return adaptiveInvoke(projection, unwrap(left)) < adaptiveInvoke(projection, unwrap(right));
    };
    return sortBy(std::move(comparator));
  }

  /// Returns the minimum logical value under ordinary ordering.
  [[nodiscard]] consteval auto min() const -> std::optional<Value> {
    return minBy(std::less<>{});
  }

  /// Returns the maximum logical value under ordinary ordering.
  [[nodiscard]] consteval auto max() const -> std::optional<Value> {
    return maxBy(std::less<>{});
  }

  /// Returns the logical minimum according to a binary ordering predicate.
  template <class Compare>
  [[nodiscard]] consteval auto minBy(Compare compare) const -> std::optional<Value> {
    std::optional<Value> result{};
    visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, item, *result))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical maximum according to a binary ordering predicate.
  template <class Compare>
  [[nodiscard]] consteval auto maxBy(Compare compare) const -> std::optional<Value> {
    std::optional<Value> result{};
    visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, *result, item))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical minimum according to an adaptive key projection.
  template <class Projection>
  [[nodiscard]] consteval auto minByKey(Projection projection) const -> std::optional<Value> {
    return minBy([projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
      return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
    });
  }

  /// Returns the logical maximum according to an adaptive key projection.
  template <class Projection>
  [[nodiscard]] consteval auto maxByKey(Projection projection) const -> std::optional<Value> {
    return maxBy([projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
      return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
    });
  }

  /// Removes consecutive duplicate logical values. The operation is fused by keeping only the previous value.
  [[nodiscard]] consteval auto unique() const {
    return uniqueBy(std::identity{});
  }

  /// Removes every later logical element whose projected key has already occured, preserving encounter order.
  template <class Projection>
  [[nodiscard]] consteval auto uniqueBy(Projection projection) const {
    // `unique` is implemented as a stage-like eager normalization so arbitrary reference values remain
    // references.
    using Key = std::remove_cvref_t<decltype(adaptiveInvoke(projection, std::declval<Value>()))>;
    static_assert(std::copy_constructible<Key>, "Query::uniqueBy() requires a copy-constructible key");
    std::vector<Key> keys;
    std::vector<StoredT<Value>> values;
    visit([&](auto &&item) consteval -> bool {
      auto key = adaptiveInvoke(projection, item);
      bool duplicate{};
      for (const Key &existing : keys) {
        if (existing == key) {
          duplicate = true;
          break;
        }
      }
      if (duplicate) {
        return true;
      }
      keys.push_back(key);
      values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
      return true;
    });
    return makeStoredQuery<Value>(std::move(values));
  }

  /// Splits logical values into two independently persistent queries while preserving encounter order.
  template <class Predicate>
  [[nodiscard]] consteval auto partition(Predicate predicate) const {
    using Stored = StoredT<Value>;
    std::vector<Stored> yes;
    std::vector<Stored> rejected;
    visit([&](auto &&item) consteval -> bool {
      auto stored = storeValueAs<Value>(std::forward<decltype(item)>(item));
      if (static_cast<bool>(adaptiveInvoke(predicate, unwrap(stored)))) {
        yes.push_back(std::move(stored));
      } else {
        rejected.push_back(std::move(stored));
      }
      return true;
    });
    return std::pair{makeStoredQuery<Value>(std::move(yes)), makeStoredQuery<Value>(std::move(rejected))};
  }

  /// Executes a compile-time callback for every logical value and returns no runtime result.
  template <class Function>
  consteval auto forEach(Function function) const -> void {
    visit([&](auto &&item) consteval -> bool {
      adaptiveInvoke(function, std::forward<decltype(item)>(item));
      return true;
    });
  }

  /// Promotes a value query into static standard storage. Reference-valued queries cannot be silently
  /// converted into a different element representation, so they fail instead of manufacturing a wrapper type.
  [[nodiscard]] consteval auto materialize() const {
    requireMaterializable<Value>();
    using Element = std::remove_cvref_t<Value>;
    std::vector<Element> values;
    visit([&](auto &&item) consteval -> bool {
      values.emplace_back(std::forward<decltype(item)>(item));
      return true;
    });
    return promoteVector(std::move(values));
  }

private:
  template <class StateType>
  friend struct Query;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(const Query<StateType> &, Function &&) -> bool;

  template <class Function>
  consteval auto visit(Function function) const -> bool {
    return evaluateState(state_, std::move(function));
  }

  State state_;
};

} // namespace Miracle::meta

namespace Miracle::meta {

template <class State, class Function>
consteval auto evaluateQuery(const Query<State> &query, Function &&function) -> bool {
  return evaluateState(query.state_, std::forward<Function>(function));
}

/// Promotes a reordered/persisted logical sequence and returns a new Query sourcing that static backing.
///
/// Internal reference storage is allowed through pointers, but the stored representation itself must still be
/// structural so `std::define_static_array` can legally provide program-lifetime backing.
template <class Value>
consteval auto makeStoredQuery(std::vector<StoredT<Value>> values) {
  using Stored = StoredT<Value>;
  if constexpr (not std::meta::is_structural_type(^^Stored)) {
    throw std::meta::exception{
        "Query reorder/persistence requires a structurally promotable stored element", {}};
  }
  auto span = promoteVector(std::move(values));
  using Source = StoredSource<Value, Stored>;
  using QueryStateType = QueryState<Source, Value>;
  return Query<QueryStateType>{QueryStateType{Source{span}, {}}};
}

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Creates a Meta Query over a finite static span without copying its source.
/// The span follows ordinary `std::span` lifetime rules: its referenced storage must remain valid for every
/// Query evaluation that uses it. Query does not take ownership of external span storage.
template <class T, std::size_t Extent>
[[nodiscard]] consteval auto query(std::span<T, Extent> source) {
  using Source = SpanSource<T>;
  using QueryStateType = QueryState<Source, T &>;
  return Query<QueryStateType>{QueryStateType{Source{std::span<T>{source}}, {}}};
}

/// Creates a Meta Query over a static array without copying its source.
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
