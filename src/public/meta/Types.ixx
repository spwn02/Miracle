export module Miracle.Meta:Types;

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

/// Standard C+26 reflection exception type, re-exported under its standard spelling.
///
/// Keeping the exact type means callers can catch failures from Miracle and raw `<meta>` operations with the
/// same handler and inspect `what()`, `u8what()`, `from()`, and `where()` without adaptation.
using exception = std::meta::exception;

/// Miracle-style type spelling for the standard reflection exception.
using Exception = exception;

/// Standard access-checking context used by caller-sensitive reflection queries.
using AccessContext = std::meta::access_context;

/// A range whose value and reference types are both reflection values.
///
/// This deliberately aliases the standard C++26 semantic concept rather than rebuilding it from range traits,
/// avoiding duplicate template machinery and ensuring future standard refinements propagate automatically.
template <class Range>
concept ReflectionRange = std::meta::reflection_range<Range>;

/// Standard reflected operator category; aliased so Miracle and `<meta>` APIs exchange values without
/// conversion.
using Operators = std::meta::operators;
/// Standard aggregate-member description options used by `dataMemberSpec`/`defineAggregate`.
using DataMemberOptions = std::meta::data_member_options;
/// Standard byte/bit member offset result returned by `offsetOf`.
using MemberOffset = std::meta::member_offset;

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Lightweight callable façade for a standard or user-provided immediate Meta operation.
///
/// The callable is stored directly with empty-state optimization and invoked with exact forwarding.
/// This is the common representation used by one-to-one `std::meta` wrappers whose semantics do not require
/// Miracle-specific caching, validation, or failure adaptation. Richer wrappers remain separate when Miracle
/// intentionally adds those semantics.
template <class Function>
struct MetaFunction final {
  /// Underlying immediate callable. Function pointers and empty function objects occupy no unnecessary state.
  [[no_unique_address]] Function function;

  /// Invokes the wrapped Meta operation with the exact argument/value categories supplied by the caller.
  template <class... Arguments>
    requires std::invocable<const Function &, Arguments...>
  [[nodiscard]] consteval auto operator()(Arguments &&...arguments) const -> decltype(auto) {
    return function(std::forward<Arguments>(arguments)...);
  }
};

/// Deduction guide preserving the concrete standard/user callable type without type erasure.
template <class Function>
MetaFunction(Function) -> MetaFunction<Function>;

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

/// Stateless callable instance of `Raw`, suitable for direct calls and Meta Query projections.
inline constexpr Raw raw{};

} // namespace Miracle::meta
