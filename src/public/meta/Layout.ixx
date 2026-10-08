export module Miracle.Meta:Layout;

import std;
import :Types;

export namespace Miracle::meta {

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction offsetOf{std::meta::offset_of};

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction sizeOf{std::meta::size_of};

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction alignmentOf{std::meta::alignment_of};

/// Exact callable wrapper for `std::meta::offset_of`.
inline constexpr MetaFunction bitSizeOf{std::meta::bit_size_of};

} // namespace Miracle::meta
