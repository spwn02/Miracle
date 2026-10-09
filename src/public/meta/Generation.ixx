export module Miracle.Meta:Generation;

import std;
import :Types;
import :Predicates;

export namespace Miracle::meta {

/// Callable wrapper for `std::meta::enumerator_spec`.
struct EnumeratorSpec final {
  /// Creates one enumerator specification from the standard option record.
  [[nodiscard]] consteval auto operator()(std::meta::enumerator_options options) const -> Info {
    return std::meta::enumerator_spec(std::move(options));
  }
};

/// Stateless exact wrapper for `std::meta::enumerator_spec`.
inline constexpr EnumeratorSpec enumeratorSpec{};

/// Callable wrapper for `std::meta::define_enum`.
struct DefineEnum final {
  /// Completes a scoped enum from the supplied range of enumerator specifications.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info enumType, Range &&members) const -> Info {
    return std::meta::define_enum(enumType, std::forward<Range>(members));
  }

  /// Preserves the standard default-range call form for a braced enumerator list.
  [[nodiscard]] consteval auto operator()(Info enumType, std::initializer_list<Info> members) const -> Info {
    return std::meta::define_enum(enumType, members);
  }
};

/// Stateless exact wrapper for `std::meta::define_enum`.
inline constexpr DefineEnum defineEnum{};

/// Callable wrapper for `std::meta::define_unscoped_enum`.
struct DefineUnscopedEnum final {
  /// Completes an unscoped enum from the supplied range of enumerator specifications.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info enumType, Range &&members) const -> Info {
    return std::meta::define_unscoped_enum(enumType, std::forward<Range>(members));
  }

  /// Preserves the standard default-range call form for a braced enumerator list.
  [[nodiscard]] consteval auto operator()(Info enumType, std::initializer_list<Info> members) const -> Info {
    return std::meta::define_unscoped_enum(enumType, members);
  }
};

/// Stateless exact wrapper for `std::meta::define_unscoped_enum`.
inline constexpr DefineUnscopedEnum defineUnscopedEnum{};

/// Callable wrapper for `std::meta::data_member_spec` retaining its default options argument.
struct DataMemberSpec final {
  /// Creates one data-member specification for `memberType` using the supplied standard options.
  [[nodiscard]] consteval auto operator()(Info memberType, std::meta::data_member_options options = {}) const
      -> Info {
    return std::meta::data_member_spec(memberType, std::move(options));
  }
};

/// Stateless exact wrapper for `std::meta::data_member_spec`.
inline constexpr DataMemberSpec dataMemberSpec{};

/// Callable wrapper for `std::meta::define_aggregate`.
struct DefineAggregate final {
  /// Completes `classType` from the supplied reflection range of data-member specifications.
  template <ReflectionRange Range>
  [[nodiscard]] consteval auto operator()(Info classType, Range &&members) const -> Info {
    return std::meta::define_aggregate(classType, std::forward<Range>(members));
  }

  /// Preserves the standard default-range call form for a braced member-specification list.
  [[nodiscard]] consteval auto operator()(Info classType, std::initializer_list<Info> members) const -> Info {
    return std::meta::define_aggregate(classType, members);
  }
};

/// Stateless exact wrapper for `std::meta::define_aggregate`.
inline constexpr DefineAggregate defineAggregate{};

/// Matches data-member specification reflections; exact wrapper for `std::meta::is_data_member_spec`.
inline constexpr Predicate isDataMemberSpec{std::meta::is_data_member_spec};

/// Matches enumerator specification reflections; exact wrapper for `std::meta::is_enumerator_spec`.
inline constexpr Predicate isEnumeratorSpec{std::meta::is_enumerator_spec};

} // namespace Miracle::meta
