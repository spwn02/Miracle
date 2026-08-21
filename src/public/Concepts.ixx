export module Miracle:Concepts;

import :Types;

export namespace Miracle {

template <class T>
concept StringLike = requires(const T &value) { StringView{value}; };

template <class T>
concept OptionalLike = requires(const T &value) {
  { value.has_value() } -> std::same_as<bool>;
  *value;
};

template <class T>
concept Aggregate = std::is_aggregate_v<T> and not std::is_union_v<T>;

template <class T>
concept Enum = std::is_enum_v<T>;

} // namespace Miracle
