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

namespace Miracle::meta::detail {

/// Promotes a temporary compile-time reflection sequence into static storage.
///
/// Miracle uses `std::span<const Info>` as its intentionally temporary source carrier. The returned span
/// therefore never refers to the transient `std::vector`; `std::define_static_array` owns the persistent
/// backing storage. In the future Miracle replaces repeated source construction with canonical category
/// caches without changing this public vocabulary. Compile-time work and promoted storage are linear in
/// `values.size()`.
[[nodiscard]] consteval auto makeInfoSequence(const std::vector<Info> &values) -> std::span<const Info> {
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
[[nodiscard]] consteval auto filteredMembers(Info subject, Access access, Predicate predicate) {
  auto result = std::vector<Info>{};
  for (const Info member : std::meta::members_of(subject, access)) {
    if (predicate(member)) {
      result.push_back(member);
    }
  }
  return makeInfoSequence(result);
}

/// Structural marker used only to constrain the predicate-composition operators.
///
/// A marker avoids inheritance or a public predicate base class: any data-oriented callable carrying this
/// member can participate in `!`, `&&`, and `||` composition without runtime polymorphism.
template <class T>
inline constexpr bool metaPredicate = requires { std::remove_cvref_t<T>::miracleMetaPredicate_; } and
                                      std::remove_cvref_t<T>::miracleMetaPredicate_;

} // namespace Miracle::meta::detail

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
/// Unline `name`, absence is a semantic error: constant evaluation throws `std::meta::exception` carrying the
/// offending reflection. This makes the operation appropriate for pipelines whose contract requires a source
/// identifier.
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
/// the speculative consteval `try`/`catch` control-flow model frozen for now.
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
/// the future.
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
/// throw `std::meta::exception` with `subject` attached. Miracle now performs one standard member query per
/// call; In the future it replaces repeated construction with canonical caches.
[[nodiscard]] consteval auto members(Info subject, Access access = Access::current()) {
  detail::requireMemberScope(subject, "members() requires a class, union, or namespace reflection");
  return detail::makeInfoSequence(std::meta::members_of(subject, access));
}

/// Type-oriented overload of `members(Info, Access)` reflecting `T` without changing source ordering or
/// access rules.
template <class T>
[[nodiscard]] consteval auto members(Access access = Access::current()) {
  return members(^^T, access);
}

/// Returns non-static data members visible to the supplied access context.
///
/// Only class/union reflection are valid. Declaration order is preserved, the result has static backing
/// storage, and an invalid subject throws `std::meta::exception`.
[[nodiscard]] consteval auto fields(Info subject, Access access = Access::current()) {
  detail::requireRecord(subject, "fields() requires a class or union reflection");
  return detail::makeInfoSequence(std::meta::nonstatic_data_members_of(subject, access));
}

/// Type-oriented overload of `fields(Info, Access)` for class/union type `T`.
template <class T>
[[nodiscard]] consteval auto fields(Access access = Access::current()) {
  return fields(^^T, access);
}

/// Returns static data members visible to the supplied access context.
///
/// Only class/union reflection are valid. Declaration order is preserved, the result has static backing
/// storage, and an invalid subject throws `std::meta::exception`.
[[nodiscard]] consteval auto staticFields(Info subject, Access access = Access::current()) {
  detail::requireRecord(subject, "staticFields() requires a class or union reflection");
  return detail::makeInfoSequence(std::meta::static_data_members_of(subject, access));
}

/// Type-oriented overload of `staticFields(Info, Access)` for class/union type `T`.
template <class T>
[[nodiscard]] consteval auto staticFields(Access access = Access::current()) {
  return staticFields(^^T, access);
}

/// Returns ordinary functions/function templates, excluding constructors.
///
/// Accepts namespaces, classes, and unions. The standard member sequence is filtered once, preserving
/// declaration order and `access`; invalid subjects throw `std::meta::exception`.
[[nodiscard]] consteval auto functions(Info subject, Access access = Access::current()) {
  detail::requireMemberScope(subject, "functions() requires a class, union, or namespace reflection");
  return detail::filteredMembers(subject, access, [](Info member) consteval -> bool {
    return (std::meta::is_function(member) or std::meta::is_function_template(member)) and
           not(std::meta::is_constructor(member) or std::meta::is_constructor_template(member));
  });
}

/// Type-oriented overload of `functions(Info, Access)` reflecting the scope represented by `T`.
template <class T>
[[nodiscard]] consteval auto functions(Access access = Access::current()) {
  return functions(^^T, access);
}

/// Returns constructors and constructor templates visible to the supplied access context.
///
/// Only class reflections are valid; unions and namespaces are rejected. Results preserve declaration order
/// and use static backing storage.
[[nodiscard]] consteval auto constructors(Info subject, Access access = Access::current()) {
  detail::requireClass(subject, "constructors() requires a class reflection");
  return detail::filteredMembers(subject, access, [](Info member) consteval -> bool {
    return std::meta::is_constructor(member) or std::meta::is_constructor_template(member);
  });
}

/// Type-oriented overload of `constructors(Info, Access)` for class type `T`.
template <class T>
[[nodiscard]] consteval auto constructors(Access access = Access::current()) {
  return constructors(^^T, access);
}

/// Returns direct base-specifier reflections in declaration order.
///
/// Only class reflections are valid. `access` determines which base specifiers are visible; no
/// transitive-base expansion or reordering is performed.
[[nodiscard]] consteval auto bases(Info subject, Access access = Access::current()) {
  detail::requireClass(subject, "bases() requires a class reflection");
  return detail::makeInfoSequence(std::meta::bases_of(subject, access));
}

/// Type-oriented overload of `bases(Info, Access)` for class type `T`.
template <class T>
[[nodiscard]] consteval auto bases(Access access = Access::current()) {
  return bases(^^T, access);
}

/// Returns enumerators of an enum in declaration order.
///
/// `subject` must reflect an enum type. The result has static backing storage; non-enum subjects throw
/// `std::meta::exception` rather than leaking the lower-level `<meta>` precondition.
[[nodiscard]] consteval auto enumerators(Info subject) {
  if (not std::meta::is_type(subject) or not std::meta::is_enum_type(subject)) {
    throw std::meta::exception{"enumerators() requires an enum reflection", subject};
  }
  return detail::makeInfoSequence(std::meta::enumerators_of(subject));
}

/// Type-oriented enum source. The constraint makes non-enum misuse an overload-formation error instead of a
/// runtime-like semantic probe; use the raw-`Info` overload when the reflected category is only known during
/// constant evaluation.
template <class E>
  requires std::is_enum_v<E>
[[nodiscard]] consteval auto enumerators() {
  return enumerators(^^E);
}

/// Parameter-source callable for reflected functions, function types, and function templates.
///
/// Results preserve source order and use static backing storage. Other reflection categories throw
/// `std::meta::exception`. A callable object is used instead of a plain function so a later Miracle can pass
/// `meta::parameters` directly to `flatMap` without changing the established call syntax.
struct Parameters final {
  /// Returns the statically backed parameter sequence for `subject`.
  [[nodiscard]] consteval auto operator()(Info subject) const {
    const bool functionType = std::meta::is_type(subject) and std::meta::is_function_type(subject);
    if (not std::meta::is_function(subject) and not functionType and
        not std::meta::is_function_template(subject)) {
      throw std::meta::exception{"parameters() requires a function reflection", subject};
    }
    return detail::makeInfoSequence(std::meta::parameters_of(subject));
  }
};
/// Stateless callable parameter source used as `meta::parameters(info)` and as a future Query projection.
inline constexpr Parameters parameters{};

/// Returns all annotations attached to a reflected entity in source order.
///
/// Each occurrence remains a distinct reflection even when two annotation values compare equally. The
/// returned span has static backing storage; value-level deduplication is intentionally left to the future
/// Query algebra.
[[nodiscard]] consteval auto annotations(Info subject) {
  return detail::makeInfoSequence(std::meta::annotations_of(subject));
}

/// Type-oriented overload returning all annotations attached to `T`.
template <class T>
[[nodiscard]] consteval auto annotations() {
  return annotations(^^T);
}

/// Returns annotations whose annotation object has type `A`, preserving source order and annotation identity.
template <class A>
[[nodiscard]] consteval auto annotations(Info subject) {
  return detail::makeInfoSequence(std::meta::annotations_of_with_type(subject, ^^A));
}

/// Type-oriented typed-annotation source equivalent to `annotations<A>(^^T)`.
template <class A, class T>
[[nodiscard]] consteval auto annotations() {
  return annotations<A>(^^T);
}

/// Template-argument source callable preserving source order and exact reflection identity.
///
/// Subjects without template arguments throw `std::meta::exception`. The callable form is intentional: later
/// Miracle can use `meta::templateArguments` directly in `map`/`flatMap` without introducing another wrapper.
struct TemplateArguments final {
  /// Returns the statically backed template-argument sequence for `subject`.
  [[nodiscard]] consteval auto operator()(Info subject) const {
    if (not std::meta::has_template_arguments(subject)) {
      throw std::meta::exception{
          "templateArguments() requires a reflection with template arguments", subject};
    }
    return detail::makeInfoSequence(std::meta::template_arguments_of(subject));
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
  requires detail::metaPredicate<PredicateType>
[[nodiscard]] consteval auto operator!(PredicateType predicate) {
  return Predicate{[predicate](Info info) consteval -> bool { return not predicate(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical AND.
template <class Left, class Right>
  requires(detail::metaPredicate<Left> and detail::metaPredicate<Right>)
[[nodiscard]] consteval auto operator&&(Left left, Right right) {
  return Predicate{[left, right](Info info) consteval -> bool { return left(info) and right(info); }};
}

/// Composes two Meta predicates with a short-circuiting logical OR.
template <class Left, class Right>
  requires(detail::metaPredicate<Left> and detail::metaPredicate<Right>)
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
    [](Info info) consteval -> bool { return not std::meta::annotations_of_with_type(info, ^^A).empty(); }};

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
  /// Contructs the unique valid façade state for `T`.
  consteval Reflect()
      : subject_(^^T) {
  }

  /// Prevents callers from violating the invariant that this façade always denotes exactly `^^T`.
  Reflect(meta::Info) = delete ("Reflect<T> doesn't support construction from reflection.");

  /// Returns the underlying standard reflection value without wrapping or copying metadata.
  [[nodiscard]] consteval auto raw() const -> meta::Info {
    return subject_;
  }

  /// Returns direct members of `T`, honoring the caller-sensitive access context.
  [[nodiscard]] consteval auto members(Access access = Access::current()) const {
    return meta::members(subject_, access);
  }

  /// Returns non-static data members of `T`; semantic validity is identical to `meta::fields(raw(), access)`.
  [[nodiscard]] consteval auto fields(Access access = Access::current()) const {
    return meta::fields(subject_, access);
  }

  /// Returns static data members of `T`, preserving declaration order and `access`.
  [[nodiscard]] consteval auto staticFields(Access access = Access::current()) const {
    return meta::staticFields(subject_, access);
  }

  /// Returns ordinary functions/function templates of `T`, excluding constructors.
  [[nodiscard]] consteval auto functions(Access access = Access::current()) const {
    return meta::functions(subject_, access);
  }

  /// Returns constructors/constructor templates of a class type `T`.
  [[nodiscard]] consteval auto constructors(Access access = Access::current()) const {
    return meta::constructors(subject_, access);
  }

  /// Returns direct base specifiers of class type `T` under the requested access context.
  [[nodiscard]] consteval auto bases(Access access = Access::current()) const {
    return meta::bases(subject_, access);
  }

  /// Returns enumerators of enum type `T`; the constraint prevents this member from existing for non-enums.
  [[nodiscard]] consteval auto enumerators() const
    requires std::is_enum_v<T>
  {
    return meta::enumerators(subject_);
  }

  /// Returns all annotations attached directly to `T` in source order.
  [[nodiscard]] consteval auto annotations() const {
    return meta::annotations(subject_);
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
