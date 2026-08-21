export module Miracle:Debug;

import std;
import :Types;
import :Meta;
import :Concepts;

// NOLINTBEGIN(readability-identifier-naming, bugprone-reserved-identifier)
namespace Miracle::debug {

/// Marks an aggregate or enumeration for reflection-based debug formatting.
export struct DebugDerive final {};

/// Hides one reflected member or enumerator from debug formatting.
export struct DebugHide final {};

/// Selects the fallback enumerator when an enum value has no visible exact match.
export struct DebugPrefer final {};

/// Replaces the reflected identifier used by debug formatting and enumName().
export template <usize Size>
struct DebugRename final {
  meta::StaticString<Size> name_{};

  explicit constexpr DebugRename(meta::StaticString<Size> name)
      : name_(std::move(name)) {
  }

  [[nodiscard]] constexpr auto apply() const noexcept -> StringView {
    return name_.apply();
  }
};

export inline constexpr DebugDerive derive{};
export inline constexpr DebugHide hide{};
export inline constexpr DebugPrefer prefer{};

export template <usize Size>
consteval auto rename(const char (&name)[Size]) -> DebugRename<Size> { // NOLINT
  return DebugRename<Size>{meta::StaticString<Size>{name}};
}

namespace detail {

template <class>
inline constexpr bool is_debug_rename_v{};
template <usize Size>
inline constexpr bool is_debug_rename_v<DebugRename<Size>>{true};

template <class T>
concept DebuggableAggr = Miracle::meta::has_annotation<DebugDerive, ^^T>() and not Enum<T>;

template <class T>
concept DebuggableEnum = Miracle::meta::has_annotation<DebugDerive, ^^T>() and Enum<T>;

template <std::meta::info Info>
consteval auto debug_hidden() -> bool {
  return Miracle::meta::has_annotation<DebugHide, Info>();
}

template <std::meta::info Info>
consteval auto debug_name() -> StringView {
  template for (constexpr std::meta::info annotation : meta::annotations<Info>) {
    using Annotation = meta::TypeObject<annotation>;

    if constexpr (is_debug_rename_v<Annotation>)
      return std::meta::extract<Annotation>(annotation).apply();
  }

  return meta::identifier<Info>;
}

struct DebugMetadata {
  StringView name;
  bool skipped{};
  bool preferred{};
};

template <std::meta::info Info>
constexpr auto debug_metadata() -> DebugMetadata {
  return DebugMetadata{
      .name = debug_name<Info>(),
      .skipped = debug_hidden<Info>(),
      .preferred = meta::has_annotation<DebugPrefer, Info>(),
  };
}

template <Enum T>
constexpr auto enum_name(T value) -> StringView {
  StringView preferred{"<unnamed>"};

  template for (constexpr std::meta::info enumerator : meta::enumerators<^^T>) {
    constexpr DebugMetadata metadata = debug_metadata<enumerator>();

    if constexpr (metadata.skipped)
      continue;

    if ([:enumerator:] == std::remove_cvref_t<T>(value))
      return metadata.name;

    if constexpr (metadata.preferred)
      preferred = metadata.name;
  }

  return preferred;
}

template <class Obj>
constexpr auto format_fields(const Obj &obj, bool pretty = false, usize level = 0) -> String {
  Hive<String> fields{};
  const String indent_outer(pretty ? level * 2 : 0, ' ');
  const String indent_inner(pretty ? (level + 1) * 2 : 0, ' ');

  template for (constexpr std::meta::info mem : meta::nsMembers<^^Obj, meta::AccessContext::unchecked()>) {
    constexpr DebugMetadata metadata = debug_metadata<mem>();
    if constexpr (metadata.skipped)
      continue;

    using Type = meta::TypeObject<mem>;
    const meta::Type<mem> &value = obj.[:mem:];

    String field{};
    if constexpr (DebuggableAggr<Type>) {
      field = std::format("{}: {}", metadata.name, format_fields(value, pretty, level + 1));
    } else if constexpr (DebuggableEnum<Type>) {
      field = std::format("{}: {}", metadata.name, enum_name(value));
    } else {
      static_assert(std::formattable<Type, char>,
          "Debug formatter error: reflected member is not std::formattable. Provide std::formatter for the "
          "member type or annotate that type with [[= debug::derive]].");

      if constexpr (StringLike<Type>) {
        field = std::format("{}: \"{}\"", metadata.name, value);
      } else {
        field = std::format("{}: {}", metadata.name, value);
      }
    }

    fields.insert(std::move(field.insert(0, indent_inner)));
  }

  constexpr StringView name = meta::identifier<^^Obj>;
  if (fields.empty())
    return String{name};

  const String joined =
      fields | std::views::join_with(StringView{pretty ? ",\n" : ", "}) | std::ranges::to<String>();
  if (pretty)
    return std::format("{} {{\n{},\n{}}}", name, joined, indent_outer);

  return std::format("{} {{ {} }}", name, joined);
}

} // namespace detail

export template <class T>
concept DebuggableAggr = detail::DebuggableAggr<T>;

export template <class T>
concept DebuggableEnum = detail::DebuggableEnum<T>;

export template <class T>
concept Debuggable = DebuggableAggr<T> or DebuggableEnum<T>;

/// Returns the reflected spelling of an enum value.
///
/// Exact visible enumerators win. If no exact enumerator matches, the single visible `[[= debug::prefer]]`
/// enumerator is returned. Otherwise the stable `<unnamed>` spelling is returned. Aliases naturally resolve
/// to the first reflected enumerator.
export template <Enum T>
constexpr auto enumName(T value) -> StringView {
  return detail::enum_name(value);
}

} // namespace Miracle::debug

export template <Miracle::debug::DebuggableAggr T>
struct std::formatter<T> : std::formatter<Miracle::String> {
  bool pretty{};

  template <typename ParseContext>
  constexpr auto parse(ParseContext &ctx) -> typename ParseContext::iterator {
    auto iter = ctx.begin();

    if (iter == ctx.end())
      return iter;

    if (*iter == '#') {
      pretty = true;
      ++iter;
    }

    if (iter != ctx.end() && *iter != '}')
      throw std::format_error("Invalid format args for Debug.");

    return iter;
  }

  constexpr auto format(const T &obj, format_context &ctx) const -> std::format_context::iterator {
    return std::formatter<Miracle::String>::format(Miracle::debug::detail::format_fields(obj, pretty), ctx);
  }
};

export template <Miracle::debug::DebuggableEnum T>
struct std::formatter<T> : std::formatter<Miracle::StringView> {
  constexpr auto format(const T &obj, format_context &ctx) const -> std::format_context::iterator {
    return std::formatter<Miracle::StringView>::format(Miracle::debug::enumName(obj), ctx);
  }
};
// NOLINTEND(readability-identifier-naming, bugprone-reserved-identifier)
