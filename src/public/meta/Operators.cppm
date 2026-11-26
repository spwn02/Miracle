export module Miracle.Meta:Operators;

import std;
import :Types;

export namespace Miracle::meta {

// C++29 adds token overloads, so the overloaded standard name is given the info/operators signature explicitly.

/// Exact callable wrapper returning the reflected operator category of an operator entity.
inline constexpr MetaFunction operatorOf{
    static_cast<std::meta::operators (*)(std::meta::info)>(std::meta::operator_of)};

/// Exact callable wrapper returning the source token for a reflected operator category.
inline constexpr MetaFunction symbolOf{
    static_cast<std::string_view (*)(std::meta::operators)>(std::meta::symbol_of)};

/// Exact UTF-8 callable wrapper returning the source token for a reflected operator category.
inline constexpr MetaFunction u8SymbolOf{
    static_cast<std::u8string_view (*)(std::meta::operators)>(std::meta::u8symbol_of)};

} // namespace Miracle::meta
