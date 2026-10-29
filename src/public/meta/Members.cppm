export module Miracle.Meta:Members;

import std;
import :Types;
import :Cache;

export namespace Miracle::meta {

/// Exact callable wrapper returning the standard vector representation of template arguments.
inline constexpr MetaFunction templateArgumentsOf{std::meta::template_arguments_of};

/// Exact access-context-aware callable wrapper for `std::meta::members_of`
inline constexpr MetaFunction membersOf{std::meta::members_of};

/// Exact access-context-aware callable wrapper for `std::meta::bases_of`
inline constexpr MetaFunction basesOf{std::meta::bases_of};

/// Exact access-context-aware callable wrapper for `std::meta::subobjects_of`
inline constexpr MetaFunction subobjectsOf{std::meta::subobjects_of};

/// Exact access-context-aware callable wrapper for `std::meta::static_data_members_of`
inline constexpr MetaFunction staticDataMembersOf{std::meta::static_data_members_of};

/// Exact access-context-aware callable wrapper for `std::meta::nonstatic_data_members_of`
inline constexpr MetaFunction nonstaticDataMembersOf{std::meta::nonstatic_data_members_of};

/// Exact callable wrapper returning the standard vector representation of enum enumerators.
inline constexpr MetaFunction enumeratorsOf{std::meta::enumerators_of};

/// Exact callable wrapper returning function-parameter reflections in the standard vector representation.
inline constexpr MetaFunction parametersOf{std::meta::parameters_of};

/// Exact callable wrapper returning the variable associated with a function-parameter reflection.
inline constexpr MetaFunction variableOf{std::meta::variable_of};

/// Returns the direct members of a class, union, or namespace in declaration order.
///
/// `access` is evaluated at the public call site and controls which access-sensitive members are visible. The
/// returned span has static backing storage and is valid for the remainder of the program. Invalid subjects
/// throw `std::meta::exception` with `subject` attached. Raw, type-oriented, and `Reflect<T>`
/// routes use the same current standard member query. Enumeration runs at
/// each evaluation point because visible declarations and deduced-return members can change.
[[nodiscard]] consteval auto members(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireMemberScope(subject, "members() requires a class, union, or namespace reflection");
  return promoteInfoSnapshot(std::meta::members_of(subject, access));
}

/// Type-oriented overload of `members(Info, Access)` returning the current member snapshot.
///
/// `Reflect<T>::members()` delegates here as well, and the raw-`Info` overload reaches the same
/// current standard query. Static backing preserves the returned snapshot, without caching future results.
template <class T>
[[nodiscard]] consteval auto members(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::meta::is_class_type(^^T) and not std::meta::is_union_type(^^T)) {
    throw std::meta::exception{"members() requires a class or union type", ^^T};
  } else {
    return members(^^T, access);
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
  if constexpr (not std::meta::is_class_type(^^T) and not std::meta::is_union_type(^^T)) {
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
  if constexpr (not std::meta::is_class_type(^^T) and not std::meta::is_union_type(^^T)) {
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
  return filteredMembers(
      subject,
      access,
      [](Info member) consteval -> bool {
        return (std::meta::is_function(member) or std::meta::is_function_template(member)) and
               not(std::meta::is_constructor(member) or std::meta::is_constructor_template(member));
      },
      true);
}

/// Type-oriented overload of `functions(Info, Access)` reflecting the scope represented by `T`.
template <class T>
[[nodiscard]] consteval auto functions(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::meta::is_class_type(^^T) and not std::meta::is_union_type(^^T)) {
    throw std::meta::exception{"functions() requires a class or union type", ^^T};
  } else {
    return functions(^^T, access);
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
  if constexpr (not std::meta::is_class_type(^^T)) {
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
  if constexpr (not std::meta::is_class_type(^^T)) {
    throw std::meta::exception{"bases() requires a class type", ^^T};

  } else {
    return cachedVisibleCategory<T, CacheCategory::Bases>(access);
  }
}

/// Returns class subobjects visible to the supplied access context.
///
/// The sequence exactly follows `std::meta::subobjects_of`: direct bases first, then non-static data members,
/// with source order preserved inside each category. The unchecked source is cached independently and caller
/// access is applied through the same access-sensitive cache used by `members`, `fields`, and `bases`.
[[nodiscard]] consteval auto subobjects(Info subject, Access access = Access::current())
    -> std::span<const Info> {
  requireClass(subject, "subobjects() requires a class reflection");
  return cachedVisibleCategory(subject, CacheCategory::Subobjects, access);
}

/// Type-oriented overload backed by the independently lazy canonical class-subobject cache for `T`.
template <class T>
[[nodiscard]] consteval auto subobjects(Access access = Access::current()) -> std::span<const Info> {
  if constexpr (not std::meta::is_class_type(^^T)) {
    throw std::meta::exception{"subobjects() requires a class type", ^^T};
  } else {
    return cachedVisibleCategory<T, CacheCategory::Subobjects>(access);
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

/// Stateless callable parameter source used directly or as a Query projection.
inline constexpr Parameters parameters{};

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

} // namespace Miracle::meta
