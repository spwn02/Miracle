export module Miracle.Format:Message;

import std;
import Miracle.Numeric;

import :Types;
import :Style;
import :Fragment;
import :Code;
import :Text;
import :Emit;

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
namespace Miracle::format::detail {

/// Reaching this means a type-level capacity bound was wrong. During constant evaluation it is a compile
/// error; at run time it traps. It never writes out of bounds.
[[noreturn]] constexpr auto capacityViolated() noexcept -> void {
  __builtin_trap();
}

/// Overflow-checked static addition for type-level capacities.
[[nodiscard]] consteval auto checkedAdd(const usize lhs, const usize rhs) -> usize {
  if (rhs > std::numeric_limits<usize>::max() - lhs)
    capacityViolated(); // not a constant expression: a wrapped bound must never become a tiny buffer
  return lhs + rhs;
}

/// Worst-case bytes of a heading: severity word, optional `[code]`, `: `, escaped summary and the LF.
[[nodiscard]] consteval auto headingCapacity(const usize prefixBytes, const usize minimumDigits, const usize summaryBytes)
    -> usize {
  constexpr usize severityBytes = 7; // "warning"
  usize total = severityBytes;
  if (prefixBytes > 0)
    total = checkedAdd(total, 2 + escapedBound(prefixBytes) + std::max(minimumDigits, maxU64Digits));
  return checkedAdd(checkedAdd(total, 2 + 1), escapedBound(summaryBytes));
}

/// Worst-case bytes of one grammar line with `bodyBytes` escaped body bytes and `fixedBytes` of layout.
[[nodiscard]] consteval auto lineCapacity(const usize fixedBytes, const usize bodyBytes) -> usize {
  return checkedAdd(fixedBytes, escapedBound(bodyBytes));
}

// Fixed layout bytes per line, including the optional blank separator and the LF.
inline constexpr usize contextFixedBytes = 1 + 2 + 3 + 1 + 1;     // "\n" "  " "-->" " " ... "\n"
inline constexpr usize fieldFixedBytes = 1 + 2 + 2 + 1;           // "\n" "  " ... ": " ... "\n"
inline constexpr usize noteFixedBytes = 1 + 3 + 7 + 1 + 1;        // "\n" "   " "= note:" " " ... "\n"

} // namespace Miracle::format::detail

export namespace Miracle::format {

/// Half-open byte range `[begin, end)` of a message with an associated style. Ranges are ordered,
/// non-overlapping, never empty and measured in bytes.
struct StyleSpan final {
  /// First styled byte.
  usize begin{};
  /// One past the last styled byte.
  usize end{};
  /// Style of the range.
  Style style{};

  constexpr auto operator<=>(const StyleSpan &) const noexcept -> std::strong_ordering = default;
};

/// Immutable compile-time message: owned plain text plus style ranges. Extending a message returns a new value
/// of a larger type and leaves the original unchanged. `Capacity` and `Runs` are inferred upper bounds, never
/// supplied by callers. In diagnostic mode (`Diagnostic`), `Current` records the last completed section and the
/// grammar can only advance.
///
/// The text and run storage are public so the type stays a structural literal type usable as a reflection
/// annotation value; treat them as read-only and use `view()`, `data()` and `styles()`.
template <usize Capacity, usize Runs, bool Diagnostic = false, Section Current = Section::Plain>
struct Message final {
  static_assert(Capacity < std::numeric_limits<usize>::max(),
      "format::Message capacity must leave room for the terminating NUL without wrapping");

  /// Plain text. `text[length] == '\0'`; interior NULs are legal.
  std::array<char, Capacity + 1> text{};
  /// Style ranges; the first `styleCount` entries are active.
  std::array<StyleSpan, Runs> styleStorage{};
  /// Number of active text bytes.
  usize length{};
  /// Number of active style ranges.
  usize styleCount{};

  /// Ordinary text from a string literal.
  template <usize N>
  constexpr Message(const char (&literal)[N]) noexcept // NOLINT
    requires(not Diagnostic and N - 1 <= Capacity)
  {
    emitInto(*this, Event{.kind = EventKind::Text, .text = detail::literalView(literal)}, RenderState{});
  }

  /// Diagnostic heading from a severity, a code and a literal summary (rendered bold).
  template <usize PrefixCharacters, usize MinDigits, usize N>
  constexpr Message(const Severity severity, const Code<PrefixCharacters, MinDigits> code, const char (&summary)[N]) noexcept
    requires(Diagnostic and Current == Section::Heading)
  {
    emitInto(*this,
        Event{.kind = EventKind::Heading,
            .text = detail::literalView(summary),
            .style = Style{.bold = true},
            .severity = severity,
            .code = code.codeView()},
        RenderState{});
  }

  /// Diagnostic heading from a severity, a code and a styled summary fragment.
  template <usize PrefixCharacters, usize MinDigits, usize N>
  constexpr Message(const Severity severity, const Code<PrefixCharacters, MinDigits> code, const Fragment<N> summary) noexcept
    requires(Diagnostic and Current == Section::Heading)
  {
    emitInto(*this,
        Event{.kind = EventKind::Heading,
            .text = summary.view(),
            .style = summary.style(),
            .severity = severity,
            .code = code.codeView()},
        RenderState{});
  }

  /// Returns the capacity bound in bytes, excluding the terminating NUL.
  [[nodiscard]] static constexpr auto capacity() noexcept -> usize {
    return Capacity;
  }

  /// Returns the number of active bytes.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return length;
  }

  /// Returns true when no bytes are active.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return length == 0;
  }

  /// Returns the plain text, NUL-terminated at `size()`. Never contains ANSI. Borrows this message.
  [[nodiscard]] constexpr auto data() const noexcept -> const char * {
    return text.data();
  }

  /// Returns the plain text. Never contains ANSI. Borrows this message.
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
    return {text.data(), length};
  }

  /// Returns the active style ranges.
  [[nodiscard]] constexpr auto styles() const noexcept -> std::span<const StyleSpan> {
    return {styleStorage.data(), styleCount};
  }

  /// Appends literal text byte-for-byte.
  template <usize N>
  [[nodiscard]] constexpr auto append(const char (&literal)[N]) const
    requires(not Diagnostic)
  {
    using Result = Message<detail::checkedAdd(Capacity, N - 1), Runs, false, Current>;
    return extendWith<Result>(Event{.kind = EventKind::Text, .text = detail::literalView(literal)});
  }

  /// Appends a styled fragment byte-for-byte; an empty or default-styled fragment adds no style range.
  template <usize N>
  [[nodiscard]] constexpr auto append(const Fragment<N> fragment) const
    requires(not Diagnostic)
  {
    using Result = Message<detail::checkedAdd(Capacity, N), detail::checkedAdd(Runs, 1), false, Current>;
    return extendWith<Result>(Event{.kind = EventKind::Text, .text = fragment.view(), .style = fragment.style()});
  }

  /// Appends the decimal spelling of a non-bool, non-character integer.
  template <std::integral Integer>
  [[nodiscard]] constexpr auto append(const Integer number) const
    requires(not Diagnostic and sizeof(Integer) <= sizeof(u64) and not std::same_as<Integer, bool> and
             not std::same_as<Integer, char> and
             not std::same_as<Integer, wchar_t> and not std::same_as<Integer, char8_t> and
             not std::same_as<Integer, char16_t> and not std::same_as<Integer, char32_t>)
  {
    using Unsigned = std::make_unsigned_t<Integer>;
    constexpr usize digitBound = std::numeric_limits<Unsigned>::digits10 + 3;
    using Result = Message<detail::checkedAdd(Capacity, digitBound), Runs, false, Current>;

    // Magnitude in the unsigned type through modular arithmetic: no `-min` overflow.
    const bool negative = number < Integer{};
    const auto magnitude = negative ? static_cast<Unsigned>(Unsigned{} - static_cast<Unsigned>(number))
                                    : static_cast<Unsigned>(number);
    std::array<char, detail::maxU64Digits> digits{};
    const usize count = detail::encodeDecimal(static_cast<u64>(magnitude), digits);

    std::array<char, detail::maxU64Digits + 1> spelling{};
    usize at{};
    if (negative)
      spelling[at++] = '-';
    for (usize index{}; index < count; ++index)
      spelling[at++] = digits[index];
    return extendWith<Result>(Event{.kind = EventKind::Text, .text = std::string_view{spelling.data(), at}});
  }

  /// Adds the single `--> context` line, directly after the heading.
  template <usize N>
  [[nodiscard]] constexpr auto context(const char (&literal)[N]) const
    requires(Diagnostic and Current == Section::Heading)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::contextFixedBytes, N - 1)),
        detail::checkedAdd(Runs, 2),
        true,
        Section::Context>;
    return extendWith<Result>(Event{.kind = EventKind::Context, .text = detail::literalView(literal)});
  }

  /// Adds the single `--> context` line from a styled fragment, e.g. a reflected name copied into an owned fragment.
  template <usize N>
  [[nodiscard]] constexpr auto context(const Fragment<N> fragment) const
    requires(Diagnostic and Current == Section::Heading)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::contextFixedBytes, N)),
        detail::checkedAdd(Runs, 2),
        true,
        Section::Context>;
    return extendWith<Result>(
        Event{.kind = EventKind::Context, .text = fragment.view(), .style = fragment.style()});
  }

  /// Adds a `key: value` line. The key must be non-empty.
  template <usize K, usize V>
  [[nodiscard]] constexpr auto field(const char (&key)[K], const char (&value)[V]) const
    requires(Diagnostic and Current <= Section::Fields and K > 1)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::fieldFixedBytes, K - 1 + V - 1)),
        detail::checkedAdd(Runs, 1),
        true,
        Section::Fields>;
    return extendWith<Result>(Event{.kind = EventKind::Field,
        .text = detail::literalView(value),
        .key = detail::literalView(key)});
  }

  /// Adds a `key: value` line with a styled value. The key must be non-empty.
  template <usize K, usize V>
  [[nodiscard]] constexpr auto field(const char (&key)[K], const Fragment<V> value) const
    requires(Diagnostic and Current <= Section::Fields and K > 1)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::fieldFixedBytes, K - 1 + V)),
        detail::checkedAdd(Runs, 1),
        true,
        Section::Fields>;
    return extendWith<Result>(Event{.kind = EventKind::Field,
        .text = value.view(),
        .key = detail::literalView(key),
        .style = value.style()});
  }

  /// Adds a `= note:` line.
  template <usize N>
  [[nodiscard]] constexpr auto note(const char (&literal)[N]) const
    requires(Diagnostic and Current <= Section::Notes)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::noteFixedBytes, N - 1)),
        detail::checkedAdd(Runs, 2),
        true,
        Section::Notes>;
    return extendWith<Result>(Event{.kind = EventKind::Note, .text = detail::literalView(literal)});
  }

  /// Adds a `= note:` line from a styled fragment.
  template <usize N>
  [[nodiscard]] constexpr auto note(const Fragment<N> fragment) const
    requires(Diagnostic and Current <= Section::Notes)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::noteFixedBytes, N)),
        detail::checkedAdd(Runs, 2),
        true,
        Section::Notes>;
    return extendWith<Result>(Event{.kind = EventKind::Note, .text = fragment.view(), .style = fragment.style()});
  }

  /// Adds a `= help:` line.
  template <usize N>
  [[nodiscard]] constexpr auto help(const char (&literal)[N]) const
    requires(Diagnostic and Current <= Section::Help)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::noteFixedBytes, N - 1)),
        detail::checkedAdd(Runs, 2),
        true,
        Section::Help>;
    return extendWith<Result>(Event{.kind = EventKind::Help, .text = detail::literalView(literal)});
  }

  /// Adds a `= help:` line from a styled fragment.
  template <usize N>
  [[nodiscard]] constexpr auto help(const Fragment<N> fragment) const
    requires(Diagnostic and Current <= Section::Help)
  {
    using Result = Message<detail::checkedAdd(Capacity, detail::lineCapacity(detail::noteFixedBytes, N)),
        detail::checkedAdd(Runs, 2),
        true,
        Section::Help>;
    return extendWith<Result>(Event{.kind = EventKind::Help, .text = fragment.view(), .style = fragment.style()});
  }

private:
  template <usize C, usize R, bool D, Section S>
  friend struct Message;

  /// Message storage seen as a span writer, so the compile-time path runs the same layout code as the sink path.
  struct Writer final {
    Message &target;

    constexpr auto put(const std::string_view bytes, const Style &style) noexcept -> bool {
      if (bytes.empty())
        return true;
      if (bytes.size() > Capacity - target.length)
        return false;

      const usize begin = target.length;
      std::ranges::copy(bytes, target.text.begin() + static_cast<isize>(begin));
      target.length += bytes.size();

      if (style == Style{})
        return true;
      if (target.styleCount > 0) {
        StyleSpan &last = target.styleStorage[target.styleCount - 1];
        if (last.end == begin and last.style == style) {
          last.end = target.length;
          return true;
        }
      }
      if (target.styleCount == Runs)
        return false;
      target.styleStorage[target.styleCount++] = StyleSpan{.begin = begin, .end = target.length, .style = style};
      return true;
    }
  };

  constexpr Message() noexcept = default;

  /// Runs `event` through the shared layout, starting from the grammar position `state` of the source message.
  static constexpr auto emitInto(Message &message, const Event &event, RenderState state) noexcept -> void {
    Writer writer{message};
    if (detail::layout(writer, state, event) != WriteResult::Written)
      detail::capacityViolated();
  }

  template <class Result>
  [[nodiscard]] constexpr auto extendWith(const Event &event) const -> Result {
    Result result{};
    std::ranges::copy_n(text.begin(), static_cast<isize>(length), result.text.begin());
    std::ranges::copy_n(styleStorage.begin(), static_cast<isize>(styleCount), result.styleStorage.begin());
    result.length = length;
    result.styleCount = styleCount;
    Result::emitInto(result, event, RenderState{.section = Current, .started = Diagnostic});
    return result;
  }
};

template <usize N>
Message(const char (&)[N]) -> Message<N - 1, 0>;

template <usize P, usize W, usize N>
Message(Severity, Code<P, W>, const char (&)[N])
    -> Message<detail::headingCapacity(P, W, N - 1), 2, true, Section::Heading>;

template <usize P, usize W, usize N>
Message(Severity, Code<P, W>, Fragment<N>) -> Message<detail::headingCapacity(P, W, N), 2, true, Section::Heading>;

/// Constructs an ordinary message from a literal.
template <usize N>
[[nodiscard]] constexpr auto message(const char (&text)[N]) noexcept -> Message<N - 1, 0> {
  return Message<N - 1, 0>{text};
}

/// Constructs a diagnostic message from a severity, a code and a literal summary.
template <usize P, usize W, usize N>
[[nodiscard]] constexpr auto message(const Severity severity, const Code<P, W> code, const char (&summary)[N]) noexcept {
  return Message{severity, code, summary};
}

/// Constructs a diagnostic message from a severity, a code and a styled summary fragment.
template <usize P, usize W, usize N>
[[nodiscard]] constexpr auto message(const Severity severity, const Code<P, W> code, const Fragment<N> summary) noexcept {
  return Message{severity, code, summary};
}

} // namespace Miracle::format
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
