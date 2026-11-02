export module Miracle.Concepts;

import std;

export namespace Miracle {

template <class T>
concept StringLike = requires(const T &value) { std::string_view{value}; };

template <class T>
concept OptionalLike = requires(const T &value) {
  { value.has_value() } -> std::same_as<bool>;
  *value;
};

template <class T>
concept Numeric = std::is_arithmetic_v<T> and not std::same_as<std::remove_cv_t<T>, bool>;

} // namespace Miracle
