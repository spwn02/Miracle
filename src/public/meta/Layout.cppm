export module Miracle.Meta:Layout;

import std;
import :Types;

export namespace Miracle::meta {

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction offsetOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::offset_of(std::forward<decltype(arguments)>(arguments)...); }};

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction sizeOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::size_of(std::forward<decltype(arguments)>(arguments)...); }};

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction alignmentOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::alignment_of(std::forward<decltype(arguments)>(arguments)...); }};

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction bitSizeOf{[](auto &&...arguments) consteval -> decltype(auto) { return std::meta::bit_size_of(std::forward<decltype(arguments)>(arguments)...); }};

} // namespace Miracle::meta
