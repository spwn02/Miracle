export module Miracle.Meta:Reflect;

import std;
import :Types;
import :Members;
import :Annotations;

export namespace Miracle {

/// Advanced type-oriented reflection façade. It always represents exactly `^^T`.
///
/// `Reflect<T>` is a compile-time value, not a runtime reflection object. It stores the standard `Info` only
/// so all member operations can share value-oriented implementation machinery; its invariant forbids
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

  /// Returns class subobjects of `T`: direct bases followed by non-static data members.
  [[nodiscard]] consteval auto subobjects(Access access = Access::current()) const
      -> std::span<const meta::Info> {
    return meta::subobjects<T>(access);
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
