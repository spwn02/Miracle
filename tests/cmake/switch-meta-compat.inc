// Miracle's Meta vocabulary intentionally removed the variable-template helpers.
// The pinned Switch test dependency still targets that historical surface, so
// keep its adapter local to Switch rather than re-exporting deprecated names
// from Miracle.Meta.
export namespace Switch::compat {

using AccessContext = Miracle::Access;

template <class...>
inline constexpr bool always_false_v{};

template <std::meta::info Info>
inline constexpr Miracle::StringView identifier = std::meta::identifier_of(Info);

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

template <Miracle::usize N>
class StaticString final {
public:
  const char *ptr_{};

  consteval StaticString(const char (&str)[N])
      : ptr_(std::define_static_string(Miracle::StringView{str, N - 1})) {
  }

  [[nodiscard]] constexpr auto apply() const -> Miracle::StringView {
    return Miracle::StringView{ptr_, N - 1};
  }

  [[nodiscard]] constexpr operator Miracle::StringView() const {
    return apply();
  }
};

} // namespace Switch::compat
