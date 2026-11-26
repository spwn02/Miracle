export module Miracle.Meta:Operators;

import std;
import :Types;

export namespace Miracle::meta {

/// Exact callable wrapper returning the reflected operator category of an operator entity.
inline constexpr MetaFunction operatorOf{std::meta::operator_of};

/// Exact callable wrapper returning the source token for a reflected operator category.
inline constexpr MetaFunction symbolOf{std::meta::symbol_of};

/// Exact UTF-8 callable wrapper returning the source token for a reflected operator category.
inline constexpr MetaFunction u8SymbolOf{std::meta::u8symbol_of};

} // namespace Miracle::meta
