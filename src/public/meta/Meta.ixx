export module Miracle:Meta;

import std;
import :Types;

// NOLINTBEGIN(readability-identifier-naming, bugprone-reserved-identifier)
export namespace Miracle::meta {

using AccessContext = std::meta::access_context;

template <class...>
inline constexpr bool always_false_v{};

template <std::meta::info Info>
inline constexpr StringView identifier = std::meta::identifier_of(Info);

template <std::meta::info Info, std::meta::access_context Ctx>
inline constexpr auto members = std::define_static_array(std::meta::members_of(Info, Ctx));

template <std::meta::info Info, std::meta::access_context Ctx>
inline constexpr auto nsMembers = std::define_static_array(std::meta::nonstatic_data_members_of(Info, Ctx));

template <std::meta::info Info>
inline constexpr auto enumerators = std::define_static_array(std::meta::enumerators_of(Info));

template <std::meta::info Info>
inline constexpr auto annotations = std::define_static_array(std::meta::annotations_of(Info));

template <std::meta::info Info>
inline constexpr auto parameters = std::define_static_array(std::meta::parameters_of(Info));

template <std::meta::info Info>
consteval auto typeInfo() -> std::meta::info {
  if constexpr (std::meta::is_type(Info))
    return Info;
  else
    return std::meta::type_of(Info);
}

template <std::meta::info Info>
using Type = typename[:typeInfo<Info>():];

template <std::meta::info Info>
using TypeObject = std::remove_cvref_t<Type<Info>>;

template <std::meta::info Info>
using Return = typename[:std::meta::return_type_of(Info):];

template <std::meta::info Info>
using ReturnObject = std::remove_cvref_t<Return<Info>>;

template <class Annotation, std::meta::info Item>
consteval auto annotation() -> Option<Annotation> {
  template for (constexpr auto item : annotations<Item>) {
    if constexpr (std::is_same_v<TypeObject<item>, Annotation>)
      return std::meta::extract<Annotation>(item);
  }

  return None;
}

template <class Annotation, std::meta::info Item>
consteval auto has_annotation() -> bool {
  return annotation<Annotation, Item>().has_value();
}

// template <class Annotation, std::meta::info Item>
// consteval auto require_annotation() -> Annotation {
//   constexpr auto value = annotation<Annotation, Item>();
//
//   static_assert(value.has_value(), "Required reflection annotation is missing.");
//
//   return *value;
// }
//
template <usize N>
class StaticString final {
public:
  const char *ptr_{};

  consteval StaticString(const char (&str)[N]) // NOLINT
      : ptr_(std::define_static_string(StringView{str, N - 1})) {
  }

  [[nodiscard]] constexpr auto apply() const -> StringView {
    return StringView{ptr_, N - 1};
  }

  [[nodiscard]] constexpr operator const char *() const {
    return ptr_;
  }

  [[nodiscard]] constexpr operator StringView() const {
    return apply();
  }
};

template <class... Items>
struct Values;

template <>
struct Values<> final {};

template <class First, class... Rest>
struct Values<First, Rest...> final {
  First first_;
  Values<Rest...> rest_;

  constexpr explicit Values(First first, Rest... rest)
      : first_(std::move(first))
      , rest_(std::move(rest)...) {
  }
};

} // namespace Miracle::meta
// NOLINTEND(readability-identifier-naming, bugprone-reserved-identifier)
