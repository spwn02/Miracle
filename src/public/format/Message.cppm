export module Miracle.Format:Message;

import std;
import Miracle.Numeric;
import Miracle.Concepts;

import :Style;
import :Code;
import :Fragment;

using namespace std::string_view_literals;

export namespace Miracle::format {

/// Represents a section of a formatted message, such as a heading, context, or detail.
enum class Section : u8 {
  /// Plain text section with no special formatting.
  Plain,
  /// Heading section with severity, code, and summary.
  Heading,
  /// Context section with source location or snippet information.
  Context,
  /// Detail section with key-value pairs of additional information.
  Fields,
  /// Note section with additional information or suggestions.
  Notes,
  /// Help section with guidance or instructions for resolving the issue.
  Help,
};

/// Represents a span of text with an associated style in a formatted message.
struct StyleSpan final {
  /// The starting index of the span in the message text.
  usize begin{};
  /// The ending index of the span in the message text.
  usize end{};
  /// The style associated with the span of text.
  Style style{};

  constexpr auto operator<=>(const StyleSpan &) const noexcept -> std::strong_ordering = default;
};

/// Represents a formatted message with text, styles, and optional diagnostic sections.
template <usize Capacity, usize Styles, bool Diagnostic = false, Section Current = Section::Plain>
struct Message final {

private:
  static constexpr std::string_view indent_ = "\n   "sv;
  static constexpr std::string_view contextPrefix_ = "\n  --> "sv;
  static constexpr std::string_view fieldsSeparator_ = ": "sv;
  static constexpr std::string_view notePrefix_ = "= note: "sv;
  static constexpr std::string_view helpPrefix_ = "= help: "sv;

public:
  /// The text content of the message, null-terminated for C-style APIs.
  std::array<char, Capacity + 1> text{};
  /// The style spans associated with the message text.
  std::array<StyleSpan, Styles> styleStorage{};
  /// The number of active characters in the message text, excluding the null terminator.
  usize length{};
  /// The number of active style spans in the message.
  usize styleCount{};

  /// Constructs a formatted message with the specified text and optional style spans.
  template <usize N>
  constexpr Message(const char (&text)[N], const std::array<StyleSpan, Styles> styles = {}) noexcept // NOLINT
    requires(not Diagnostic)
      : length(N - 1)
      , styleCount(styles.size()) {
    std::ranges::copy(text, this->text.begin());
    std::ranges::copy(styles, styleStorage.begin());
  }

  template <usize PrefixN, usize Width, usize SummaryN>
  constexpr Message(const Severity severity,
      const Code<PrefixN, Width> code,
      const char (&summary)[SummaryN]) noexcept // NOLINT
    requires(Diagnostic)
  {
    initializeDiagnostic(severity, code, std::string_view{static_cast<const char *>(summary), SummaryN});
  }

  template <usize PrefixN, usize Width, usize SummaryN>
  constexpr Message(const Severity severity,
      const Code<PrefixN, Width> code,
      const Fragment<SummaryN> summary) noexcept
    requires(Diagnostic)
  {
    initializeDiagnostic(severity, code, summary.view());
  }

  /// Returns the total capacity of the message text, including null terminator.
  [[nodiscard]] static constexpr auto capacity() noexcept -> usize {
    return Capacity;
  }

  /// Returns the number of active characters in the message text, excluding the null terminator.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return length;
  }

  /// Returns true when the message text contains no active characters.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return length == 0;
  }

  /// Returns a pointer to the message text, null-terminated for C-style APIs.
  [[nodiscard]] constexpr auto data() const noexcept -> const char * {
    return text.data();
  }

  /// Returns a string view of the message text, excluding the null terminator.
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
    return {text.data(), length};
  }

  /// Returns a span of the active style spans in the message.
  [[nodiscard]] constexpr auto styles() const noexcept -> std::span<const StyleSpan> {
    return {styleStorage.data(), styleCount};
  }

  template <usize N>
  [[nodiscard]] constexpr auto append(const char (&text)[N]) const && // NOLINT
      noexcept
    requires(not Diagnostic)
  {
    Message<Capacity + N - 1, Styles, false, Current> result{};
    result.appendUnchecked(view());
    result.appendUnchecked({static_cast<const char *>(text), N - 1});
    std::ranges::move(styleStorage, result.styleStorage.begin());
    result.styleCount = styleCount;

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto append(const Fragment<N> text) const && noexcept
    requires(not Diagnostic)
  {
    constexpr usize newStyles = Styles > 0 ? Styles + 1 : 0;
    Message<Capacity + N - 1, newStyles, false, Current> result{};
    result.appendUnchecked(view());

    const usize textBegin = result.length;
    result.appendUnchecked(text.view(), N - 1);
    const usize textEnd = result.length;

    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = textBegin,
          .end = textEnd,
          .style = text.style,
      };
      ++result.styleCount;
    }

    return std::forward<decltype(result)>(result);
  }

  template <Numeric T>
  [[nodiscard]] constexpr auto append(const T number) const &&
    requires(not Diagnostic)
  {
    constexpr usize maxDigits = std::numeric_limits<std::make_unsigned_t<T>>::digits10 + 3;
    Message<Capacity + maxDigits, Styles, false, Current> result{};
    result.appendUnchecked(view());
    result.appendUnchecked(std::to_string(number));
    std::ranges::move(styleStorage, result.styleStorage.begin());
    result.styleCount = styleCount;

    return std::forward<decltype(result)>(result);
  }

  [[nodiscard]] constexpr auto append(bool value) const &&
    requires(not Diagnostic)
  {
    constexpr usize maxDigits = 5; // "false"
    Message<Capacity + maxDigits, Styles, false, Current> result{};
    result.appendUnchecked(view());
    result.appendUnchecked(value ? "true" : "false");
    std::ranges::move(styleStorage, result.styleStorage.begin());
    result.styleCount = styleCount;

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto context(const char (&text)[N]) const && // NOLINT
    requires(Diagnostic and Current <= Section::Context)
  {
    static constexpr usize contextPrefixBytes_ = Current < Section::Context ? contextPrefix_.length() : 0;
    static constexpr usize newStyles_ = Styles > 0 ? Styles + (Current < Section::Context ? 1 : 0) : 0;

    Message<Capacity + N + contextPrefixBytes_ - 1, newStyles_, true, Section::Context> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current < Section::Context) {
      const usize arrowBegin = result.length;
      result.appendUnchecked(contextPrefix_);
      const usize arrowEnd = result.length;

      if constexpr (newStyles_ > 0) {
        *styleIter = StyleSpan{
            .begin = arrowBegin,
            .end = arrowEnd,
            .style = severityStyle(Severity::Note),
        };
        ++result.styleCount;
      }
    }

    result.appendUnchecked({static_cast<const char *>(text), N - 1});

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto context(const Fragment<N> text) const && noexcept
    requires(Diagnostic and Current <= Section::Context)
  {
    static constexpr usize contextPrefixBytes_ = Current < Section::Context ? contextPrefix_.length() : 0;
    static constexpr usize newStyles_ = Styles > 0 ? Styles + (Current < Section::Context ? 1 : 0) + 1 : 0;

    Message<Capacity + N + contextPrefixBytes_ - 1, newStyles_, true, Section::Context> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current < Section::Context) {
      const usize arrowBegin = result.length;
      result.appendUnchecked(contextPrefix_);
      const usize arrowEnd = result.length;

      if constexpr (Styles > 0) {
        *styleIter++ = StyleSpan{
            .begin = arrowBegin,
            .end = arrowEnd,
            .style = severityStyle(Severity::Note),
        };
        ++result.styleCount;
      }
    }

    const usize textBegin = result.length;
    result.appendUnchecked(text.view(), N - 1);
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = textBegin,
          .end = result.length,
          .style = text.style,
      };
      ++result.styleCount;
    }

    return std::forward<decltype(result)>(result);
  }

  template <usize K, usize V>
  [[nodiscard]] constexpr auto field(const char (&key)[K], const char (&value)[V]) const && // NOLINT
    requires(Diagnostic and Current <= Section::Fields)
  {
    static constexpr usize punctuationBytes_ =
        indent_.length() + fieldsSeparator_.length() + (Current == Section::Context ? 1 : 0);

    Message<Capacity + K + V + punctuationBytes_ - 2, Styles, true, Section::Fields> result{};
    result.appendUnchecked(view());
    std::ranges::move(styleStorage, result.styleStorage.begin());
    result.styleCount = styleCount;

    if constexpr (Current == Section::Context) {
      result.appendUnchecked("\n");
    }
    result.appendUnchecked(indent_);
    result.appendUnchecked({static_cast<const char *>(key), K - 1});
    result.appendUnchecked(fieldsSeparator_);
    result.appendUnchecked({static_cast<const char *>(value), V - 1});

    return std::forward<decltype(result)>(result);
  }

  template <usize K, usize V>
  [[nodiscard]] constexpr auto field(const char (&key)[K], const Fragment<V> value) const && // NOLINT
    requires(Diagnostic and Current <= Section::Fields)
  {
    static constexpr usize punctuationBytes_ =
        indent_.length() + fieldsSeparator_.length() + (Current == Section::Context ? 1 : 0);
    static constexpr usize newStyles_ = Styles > 0 ? Styles + 1 : 0;

    Message<Capacity + K + V + punctuationBytes_ - 2, newStyles_, true, Section::Fields> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Context) {
      result.appendUnchecked("\n");
    }

    result.appendUnchecked(indent_);
    result.appendUnchecked({static_cast<const char *>(key), K - 1});
    result.appendUnchecked(fieldsSeparator_);

    const usize textBegin = result.length;
    result.appendUnchecked(value.view(), V - 1);
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = textBegin,
          .end = result.length,
          .style = value.style,
      };
      ++result.styleCount;
    }

    return std::forward<decltype(result)>(result);
  }

  template <usize K, usize V>
  [[nodiscard]] constexpr auto field(const Fragment<K> key, const char (&value)[V]) const && // NOLINT
    requires(Diagnostic and Current <= Section::Fields)
  {
    static constexpr usize punctuationBytes_ =
        indent_.length() + fieldsSeparator_.length() + (Current == Section::Context ? 1 : 0);
    static constexpr usize newStyles_ = Styles > 0 ? Styles + 1 : 0;

    Message<Capacity + K + V + punctuationBytes_ - 2, newStyles_, true, Section::Fields> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Context) {
      result.appendUnchecked("\n");
    }

    result.appendUnchecked(indent_);

    const usize textBegin = result.length;
    result.appendUnchecked(key.view(), K - 1);
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = textBegin,
          .end = result.length,
          .style = key.style,
      };
      ++result.styleCount;
    }

    result.appendUnchecked(fieldsSeparator_);
    result.appendUnchecked({static_cast<const char *>(value), V - 1});

    return std::forward<decltype(result)>(result);
  }

  template <usize K, usize V>
  [[nodiscard]] constexpr auto field(const Fragment<K> key, const Fragment<V> value) const && // NOLINT
    requires(Diagnostic and Current <= Section::Fields)
  {
    static constexpr usize punctuationBytes_ =
        indent_.length() + fieldsSeparator_.length() + (Current == Section::Context ? 1 : 0);
    static constexpr usize newStyles_ = Styles > 0 ? Styles + 2 : 0;

    Message<Capacity + K + V + punctuationBytes_ - 2, newStyles_, true, Section::Fields> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Context) {
      result.appendUnchecked("\n");
    }

    result.appendUnchecked(indent_);

    const usize keyBegin = result.length;
    result.appendUnchecked(key.view(), K - 1);
    if constexpr (Styles > 0) {
      *styleIter++ = StyleSpan{
          .begin = keyBegin,
          .end = result.length,
          .style = key.style,
      };
      ++result.styleCount;
    }

    result.appendUnchecked(fieldsSeparator_);

    const usize valueBegin = result.length;
    result.appendUnchecked(value.view(), V - 1);
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = valueBegin,
          .end = result.length,
          .style = value.style,
      };
      ++result.styleCount;
    }

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto note(const char (&text)[N]) const && // NOLINT
    requires(Diagnostic and Current <= Section::Notes)
  {
    static constexpr usize newlineBytes_ = Current == Section::Fields ? 1 : 0;
    static constexpr usize notePrefixBytes_ =
        Current < Section::Notes ? indent_.length() + notePrefix_.length() + newlineBytes_ : 0;
    static constexpr usize newStyles_ = Styles > 0 ? Styles + (Current < Section::Notes ? 1 : 0) : 0;

    Message<Capacity + N + notePrefixBytes_ - 1, newStyles_, true, Section::Notes> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Fields) {
      result.appendUnchecked("\n");
    }

    if constexpr (Current < Section::Notes) {
      result.appendUnchecked(indent_);
      const usize noteBegin = result.length;

      result.appendUnchecked(notePrefix_);
      const usize noteEnd = result.length;

      if constexpr (newStyles_ > 0) {
        *styleIter = StyleSpan{
            .begin = noteBegin,
            .end = noteEnd,
            .style = severityStyle(Severity::Note),
        };
        ++result.styleCount;
      }
    }

    result.appendUnchecked({static_cast<const char *>(text), N - 1});

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto note(const Fragment<N> value) const &&
    requires(Diagnostic and Current <= Section::Notes)
  {
    static constexpr usize newlineBytes_ = Current == Section::Fields ? 1 : 0;
    static constexpr usize notePrefixBytes_ =
        Current < Section::Notes ? indent_.length() + notePrefix_.length() + newlineBytes_ : 0;
    static constexpr usize newStyles_ = Styles > 0 ? Styles + (Current < Section::Notes ? 1 : 0) + 1 : 0;

    Message<Capacity + N + notePrefixBytes_ - 1, newStyles_, true, Section::Notes> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Fields) {
      result.appendUnchecked("\n");
    }

    if constexpr (Current < Section::Notes) {
      result.appendUnchecked(indent_);
      const usize noteBegin = result.length;

      result.appendUnchecked(notePrefix_);
      const usize noteEnd = result.length;

      if constexpr (Styles > 0) {
        *styleIter++ = StyleSpan{
            .begin = noteBegin,
            .end = noteEnd,
            .style = severityStyle(Severity::Note),
        };
        ++result.styleCount;
      }
    }

    const usize valueBegin = result.length;
    result.appendUnchecked(value.view(), N - 1);
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = valueBegin,
          .end = result.length,
          .style = value.style,
      };
      ++result.styleCount;
    }

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto help(const char (&text)[N]) const && // NOLINT
    requires(Diagnostic and Current <= Section::Help)
  {
    static constexpr usize newlineBytes_ = Current == Section::Fields ? 1 : 0;
    static constexpr usize helpPrefixBytes_ =
        Current < Section::Help ? indent_.length() + helpPrefix_.length() + newlineBytes_ : 0;
    static constexpr usize newStyles_ = Styles > 0 ? Styles + (Current < Section::Help ? 1 : 0) : 0;

    Message<Capacity + N + helpPrefixBytes_ - 1, newStyles_, true, Section::Help> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Fields) {
      result.appendUnchecked("\n");
    }

    if constexpr (Current < Section::Help) {
      result.appendUnchecked(indent_);
      const usize helpBegin = result.length;

      result.appendUnchecked(helpPrefix_);
      const usize helpEnd = result.length;

      if constexpr (newStyles_ > 0) {
        *styleIter = StyleSpan{
            .begin = helpBegin,
            .end = helpEnd,
            .style = severityStyle(Severity::Note),
        };
        ++result.styleCount;
      }
    }

    result.appendUnchecked({static_cast<const char *>(text), N - 1});

    return std::forward<decltype(result)>(result);
  }

  template <usize N>
  [[nodiscard]] constexpr auto help(const Fragment<N> value) const &&
    requires(Diagnostic and Current <= Section::Help)
  {
    static constexpr usize newlineBytes_ = Current == Section::Fields ? 1 : 0;
    static constexpr usize helpPrefixBytes_ =
        Current < Section::Help ? indent_.length() + helpPrefix_.length() + newlineBytes_ : 0;
    static constexpr usize newStyles_ = Styles > 0 ? Styles + (Current < Section::Help ? 1 : 0) + 1 : 0;

    Message<Capacity + N + helpPrefixBytes_ - 1, newStyles_, true, Section::Help> result{};
    result.appendUnchecked(view());
    [[maybe_unused]] auto styleIter = std::ranges::move(styleStorage, result.styleStorage.begin()).out;
    result.styleCount = styleCount;

    if constexpr (Current == Section::Fields) {
      result.appendUnchecked("\n");
    }

    if constexpr (Current < Section::Help) {
      result.appendUnchecked(indent_);
      const usize helpBegin = result.length;

      result.appendUnchecked(helpPrefix_);
      const usize helpEnd = result.length;

      if constexpr (Styles > 0) {
        *styleIter++ = StyleSpan{
            .begin = helpBegin,
            .end = helpEnd,
            .style = severityStyle(Severity::Note),
        };
        ++result.styleCount;
      }
    }

    const usize valueBegin = result.length;
    result.appendUnchecked(value.view(), N - 1);
    if constexpr (Styles > 0) {
      *styleIter = StyleSpan{
          .begin = valueBegin,
          .end = result.length,
          .style = value.style,
      };
      ++result.styleCount;
    }

    return std::forward<decltype(result)>(result);
  }

private:
  template <usize C, usize S, bool D, Section Sec>
  friend struct Message;

  constexpr Message() noexcept = default;

  template <usize PrefixN, usize Width>
  constexpr auto initializeDiagnostic(const Severity severity,
      const Code<PrefixN, Width> code,
      const std::string_view summary) noexcept -> void
    requires(Diagnostic)
  {
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    auto textIter = text.begin();
    auto styleIter = styleStorage.begin();

    // Append newline
    *textIter++ = '\n';

    // Format and append severity string
    const std::string_view sevStr = severityName(severity);
    const auto sevBegin = textIter;
    textIter = std::ranges::copy(sevStr, textIter).out;

    // Append code
    *textIter++ = '[';
    textIter = std::ranges::copy(code.view(), textIter).out;
    *textIter++ = ']';
    const auto codeEnd = textIter;

    // Record style span for severity
    if constexpr (Styles > 0) {
      *styleIter++ = StyleSpan{
          .begin = static_cast<usize>(std::distance(text.begin(), sevBegin)),
          .end = static_cast<usize>(std::distance(text.begin(), textIter)),
          .style = severityStyle(severity),
      };
    }

    // Append summary separator
    *textIter++ = ':';
    *textIter++ = ' ';

    // Append summary
    textIter = std::ranges::copy_n(summary.begin(), summary.length() - 1, textIter).out;
    const auto summaryEnd = textIter;

    // Record style span for summary
    if constexpr (Styles > 1) {
      *styleIter++ = StyleSpan{
          .begin = static_cast<usize>(std::distance(text.begin(), codeEnd)),
          .end = static_cast<usize>(std::distance(text.begin(), summaryEnd)),
          .style = Style{.bold = true},
      };
    }

    // Null-terminate
    *textIter = '\0';
    length = static_cast<usize>(std::distance(text.begin(), textIter));
    styleCount = static_cast<usize>(std::distance(styleStorage.begin(), styleIter));
    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  }

  /// Appends the specified text to the message without bounds checking, updating the length and
  /// null-terminating the text.
  constexpr auto appendUnchecked(std::string_view value) noexcept -> void {
    auto destination = std::next(text.begin(), static_cast<isize>(length));
    auto out = std::ranges::copy(value, destination).out;
    length += value.length();
    out = {};
  }

  constexpr auto appendUnchecked(std::string_view value, usize len) noexcept -> void {
    auto destination = std::next(text.begin(), static_cast<isize>(length));
    auto out = std::ranges::copy_n(value.begin(), len, destination).out;
    length += len;
    out = {};
  }
};

} // namespace Miracle::format

namespace Miracle::format {

/// Computes the maximum number of characters needed to represent a diagnostic message with the given prefix
/// length, numeric width, and summary length.
consteval auto computeDiagnosticCapacity(const usize prefixN,
    const usize width,
    const usize summaryLen) noexcept -> usize {
  constexpr usize severityMaxBytes = 7; // "warning"
  usize prefixBytes = (prefixN > 1) ? prefixN - 1 : 0;
  // constexpr usize maxNumericBytes = 20;
  // usize numericBytes = std::max(width, maxNumericBytes);
  constexpr usize punctuationBytes = 5; // spacing, brackets, separators

  return severityMaxBytes + prefixBytes + width + punctuationBytes + summaryLen;
}

} // namespace Miracle::format

export namespace Miracle::format {

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
/// Constructs a diagnostic message with the specified summary text.
template <usize Capacity>
Message(const char (&text)[Capacity]) -> Message<Capacity, 0>;

/// Constructs a diagnostic message with the specified severity, code, and summary text.
template <usize PrefixN, usize Width, usize Capacity>
Message(Severity, Code<PrefixN, Width>, const char (&text)[Capacity])
    -> Message<computeDiagnosticCapacity(PrefixN, Width, Capacity),
        2, // Upper bound on initial style spans (severity + code, summary)
        true,
        Section::Plain // Initial completed section before context/fields/notes
        >;

template <usize Capacity>
constexpr auto message(const char (&text)[Capacity]) -> Message<Capacity, 0> {
  return Message<Capacity, 0>{text, {}};
}

template <usize Capacity, usize Styles>
constexpr auto message(const char (&text)[Capacity], const std::array<StyleSpan, Styles> styles)
    -> Message<Capacity, Styles> {
  return Message<Capacity, Styles>{text, styles};
}

template <usize PrefixN, usize Width, usize SummaryN>
constexpr auto message(const Severity severity,
    const Code<PrefixN, Width> code,
    const char (&summary)[SummaryN]) noexcept // NOLINT
{
  return Message{severity, code, summary};
}
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)

} // namespace Miracle::format
