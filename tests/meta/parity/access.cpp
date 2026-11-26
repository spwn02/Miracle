import std;
import Miracle.Meta;

namespace Tests::metaParityAccess {

/// Private base used to verify access-sensitive base/subobject queries.
struct HiddenBase {};

class Subject : private HiddenBase {
public:
  /// Public data member used as the accessible comparison subject.
  int visible{};

private:
  /// Private data member used as the inaccessible comparison subject.
  [[maybe_unused]] int hidden_{};
};

/// Returns the non-static data member named `name` using unchecked standard reflection.
consteval auto memberNamed(std::string_view name) -> Miracle::meta::Info {
  for (const auto member : std::meta::nonstatic_data_members_of(^^Subject, Miracle::Access::unchecked())) {
    if (std::meta::identifier_of(member) == name) {
      return member;
    }
  }
  return {};
}

inline constexpr auto visibleMember = memberNamed("visible");
inline constexpr auto hiddenMember = memberNamed("hidden_");
inline constexpr auto unprivileged = Miracle::Access::unprivileged();
inline constexpr auto unchecked = Miracle::Access::unchecked();

static_assert(std::same_as<Miracle::meta::AccessContext, std::meta::access_context>);
static_assert(Miracle::meta::isAccessible(visibleMember, unprivileged) ==
              std::meta::is_accessible(visibleMember, unprivileged));
static_assert(Miracle::meta::isAccessible(hiddenMember, unprivileged) ==
              std::meta::is_accessible(hiddenMember, unprivileged));
static_assert(Miracle::meta::isAccessible(hiddenMember, unchecked) ==
              std::meta::is_accessible(hiddenMember, unchecked));

static_assert(Miracle::meta::hasInaccessibleNonstaticDataMembers(^^Subject, unprivileged) ==
              std::meta::has_inaccessible_nonstatic_data_members(^^Subject, unprivileged));
static_assert(Miracle::meta::hasInaccessibleBases(^^Subject, unprivileged) ==
              std::meta::has_inaccessible_bases(^^Subject, unprivileged));
static_assert(Miracle::meta::hasInaccessibleSubobjects(^^Subject, unprivileged) ==
              std::meta::has_inaccessible_subobjects(^^Subject, unprivileged));

consteval auto currentFunctionParity() -> bool {
  return Miracle::meta::currentFunction() == std::meta::current_function();
}
static_assert(currentFunctionParity());

struct CurrentClassProbe final {
  /// Compares Miracle and standard caller-sensitive class reflection in the same lexical class scope.
  static consteval auto parity() -> bool {
    return Miracle::meta::currentClass() == std::meta::current_class();
  }
};
static_assert(CurrentClassProbe::parity());

namespace CurrentNamespaceProbe {

consteval auto parity() -> bool {
  return Miracle::meta::currentNamespace() == std::meta::current_namespace();
}
static_assert(parity());

} // namespace CurrentNamespaceProbe

} // namespace Tests::metaParityAccess

auto main() -> int {
}
