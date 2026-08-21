export module Miracle:Bitflags;

import std;
import :Types;
import :Meta;

// NOLINTBEGIN(bugprone-reserved-identifier)
namespace Miracle {

struct Bitflags {};
export inline constexpr Bitflags bitflags{};

template <typename E>
concept UnevaluatedBitflagsConcept = std::is_enum_v<E> and
    (std::meta::is_unsigned_type(^^std::underlying_type_t<E>)) and Miracle::meta::has_annotation<Bitflags, ^^E>();

template <UnevaluatedBitflagsConcept E>
consteval auto is_flags_enum_v() -> bool { // NOLINT(readability-identifier-naming)
  template for (constexpr std::meta::info enumerator : meta::enumerators<^^E>) {
    const auto value = std::to_underlying([:enumerator:]);
    if (value > 0 and not std::has_single_bit(value)) {
      return false;
    }
  }

  return true;
}

template <typename E>
concept BitflagsConcept = is_flags_enum_v<E>();

export template <Miracle::BitflagsConcept B>
constexpr auto bits(B flags) noexcept {
  using U = std::underlying_type_t<std::remove_cvref_t<B>>;
  return static_cast<U>(flags);
}

} // namespace Miracle

template <Miracle::BitflagsConcept B>
struct std::formatter<B> : std::formatter<Miracle::String> {
  enum class Mode : Miracle::u8 {
    None = 0,
    Binary = 1,
    Hex = 2,
  };
  Mode mode{Mode::None};
  bool prefix{};

  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) -> typename ParseContext::iterator {
    auto iter = ctx.begin();

    if (iter == ctx.end())
      return iter;

    if (*iter == 'b') {
      mode = Mode::Binary;
      ++iter;
    }

    if (*iter == 'x') {
      if (mode != Mode::None)
        throw std::format_error(std::format("Invalid format args for displaying Bitflags: {}. Attempted to "
                                            "enable both binary and hex simultaneously",
            Miracle::meta::identifier<^^B>));
      mode = Mode::Hex;
      ++iter;
    }

    if (*iter == 'p') {
      prefix = true;
      ++iter;
    }

    if (iter != ctx.end() && *iter != '}')
      throw std::format_error(
          std::format("Invalid format args for Bitflags: {}.", Miracle::meta::identifier<^^B>));

    return iter;
  }

  constexpr auto format(const B &flags, format_context &ctx) const -> std::format_context::iterator {
    Miracle::String result{};
    auto decimal = Miracle::bits(flags);

    switch (mode) {
      case Mode::None: result = std::format("{}", decimal); break;
      case Mode::Binary:
        result = std::format("{:08b}", decimal);
        if (prefix)
          result.insert(0, "0b");
        break;
      case Mode::Hex:
        result = std::format("{:x}", decimal);
        if (prefix)
          result.insert(0, "0x");
        break;
    }

    return std::formatter<Miracle::String, char>::format(result, ctx);
  }
};

export template <Miracle::BitflagsConcept B>
constexpr auto operator|(B lhs, B rhs) noexcept -> B {
  using U = std::underlying_type_t<B>;
  return static_cast<B>(static_cast<U>(lhs) | static_cast<U>(rhs));
}

export template <Miracle::BitflagsConcept B>
constexpr auto operator&(B lhs, B rhs) noexcept -> B {
  using U = std::underlying_type_t<B>;
  return static_cast<B>(static_cast<U>(lhs) & static_cast<U>(rhs));
}

export template <Miracle::BitflagsConcept B>
constexpr auto operator~(B val) noexcept -> B {
  using U = std::underlying_type_t<B>;
  return static_cast<B>(~static_cast<U>(val));
}

export template <Miracle::BitflagsConcept B>
constexpr auto operator|=(B &lhs, B rhs) noexcept -> B {
  return lhs = lhs | rhs;
}

export template <Miracle::BitflagsConcept B>
constexpr auto operator&=(B &lhs, B rhs) noexcept -> B {
  return lhs = lhs & rhs;
}

export namespace Miracle {

template <BitflagsConcept B>
constexpr auto all() noexcept -> B {
  B result{};

  template for (constexpr std::meta::info enumerator : meta::enumerators<^^B>) {
    result |= [:enumerator:];
  }

  return result;
}

template <BitflagsConcept B>
constexpr auto has(B flags, B flag) noexcept -> bool {
  return bits(flags & flag) != 0;
}

} // namespace Miracle
// NOLINTEND(bugprone-reserved-identifier)
