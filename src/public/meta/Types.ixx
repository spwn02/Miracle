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
/// Standard enum-member description options used by `enumeratorSpec`/`defineEnum`.
using EnumeratorOptions = std::meta::enumerator_options;
/// Standard byte/bit member offset result returned by `offsetOf`.
using MemberOffset = std::meta::member_offset;
/// Standard reflected attribute namespace discriminator.
using AttributeNamespace = std::meta::AttributeNamespace;
/// Standard attribute comparison policy used by `hasAttribute`.
using AttributeComparison = std::meta::attribute_comparison;

} // namespace Miracle::meta
