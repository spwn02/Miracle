export module Miracle.Meta:Operators;

import std;
import :Types;

export namespace Miracle::meta {

// C++29 adds token overloads, so the overloaded standard name is given the info/operators signature explicitly.

/// Exact callable wrapper returning the reflected operator category of an operator entity.
inline constexpr MetaFunction operatorOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::operator_of(std::forward<decltype(arguments)>(arguments)...); }};

/// Exact callable wrapper returning the source token for a reflected operator category.
inline constexpr MetaFunction symbolOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::symbol_of(std::forward<decltype(arguments)>(arguments)...); }};

/// Exact UTF-8 callable wrapper returning the source token for a reflected operator category.
inline constexpr MetaFunction u8SymbolOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::u8symbol_of(std::forward<decltype(arguments)>(arguments)...); }};

} // namespace Miracle::meta
