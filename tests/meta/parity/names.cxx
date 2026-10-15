import std;
import Miracle.Meta;

namespace Tests::metaParityNames {

struct Subject final {
  int value{};
};

inline constexpr auto subject = ^^Subject;
inline constexpr auto member = ^^Subject::value;

/// Compares source locations field-by-field because `std::source_location` has no equality operator.
consteval auto sourceLocationParity() -> bool {
  const auto miracle = Miracle::meta::sourceLocationOf(member);
  const auto standard = std::meta::source_location_of(member);
  return std::string_view{miracle.file_name()} == standard.file_name() &&
         std::string_view{miracle.function_name()} == standard.function_name() &&
         miracle.line() == standard.line() && miracle.column() == standard.column();
}

static_assert(Miracle::meta::hasIdentifier(member) == std::meta::has_identifier(member));
static_assert(Miracle::meta::identifierOf(member) == std::meta::identifier_of(member));
static_assert(Miracle::meta::u8IdentifierOf(member) == std::meta::u8identifier_of(member));
static_assert(Miracle::meta::displayStringOf(member) == std::meta::display_string_of(member));
static_assert(Miracle::meta::u8DisplayStringOf(member) == std::meta::u8display_string_of(member));
static_assert(sourceLocationParity());
static_assert(Miracle::meta::parentOf(member) == std::meta::parent_of(member));

// Miracle's richer projections must converge on the same standard result when their stronger
// preconditions hold.
static_assert(Miracle::meta::name(member).value() == std::meta::identifier_of(member));
static_assert(Miracle::meta::requireName(member) == std::meta::identifier_of(member));
static_assert(Miracle::meta::displayName(member) == std::meta::display_string_of(member));
static_assert(Miracle::meta::parent(member) == std::meta::parent_of(member));
static_assert(Miracle::meta::name(^^int) == std::nullopt);
static_assert(subject == ^^Subject);

} // namespace Tests::metaParityNames

auto main() -> int {
}
