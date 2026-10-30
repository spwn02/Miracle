export module Miracle:Types;

export import Miracle.Numeric;
import std;

export namespace Miracle {

using String = std::string;
using StringView = std::string_view;

using Byte = std::byte;
using Path = std::filesystem::path;

template <class T, usize N>
using Array = std::array<T, N>;
template <class T>
using Vec = std::vector<T>;

template <class T>
using UPtr = std::unique_ptr<T>;
template <class T>
using Ref = std::reference_wrapper<T>;

template <class T1, class T2>
using Pair = std::pair<T1, T2>;
template <class... Values>
using Tuple = std::tuple<Values...>;

template <class T>
using Option = std::optional<T>;
inline constexpr std::nullopt_t None = std::nullopt; // NOLINT(readability-identifier-naming)

template <class T, usize Extent = std::dynamic_extent>
using Span = std::span<T, Extent>;

template <class T, class Compare = std::less<T>>
using FlatSet = std::flat_set<T, Compare, Vec<T>>;

template <class Key, class Value, class Compare = std::less<Key>>
using FlatMap = std::flat_map<Key, Value, Compare, Vec<Key>, Vec<Value>>;

template <class T, usize Capacity>
using InplaceVec = std::inplace_vector<T, Capacity>;

template <class T>
using Hive = std::hive<T>;

} // namespace Miracle
