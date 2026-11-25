export module Miracle.Format:Code;

import std;
import Miracle.Numeric;

import :Text;
import :Emit;

// NOLINTBEGIN(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
export namespace Miracle::format {

/// Presentation value of a diagnostic code: an owned `PrefixCharacters`-byte prefix and a number padded to at
/// least `MinDigits` digits. It is not a code registry; the producer of the code stays its authority.
template <usize PrefixCharacters, usize MinDigits = 3>
class Code final {
public:
  /// Worst-case number of bytes in the formatted code.
  static constexpr usize capacity = PrefixCharacters + std::max(MinDigits, detail::maxU64Digits);

  /// Builds `<prefix><zero-padded number>`; the number is never truncated.
  constexpr Code(const char (&prefix)[PrefixCharacters + 1], const u64 value) noexcept // NOLINT
      : numeric_(value) {
    std::ranges::copy_n(static_cast<const char *>(prefix), static_cast<isize>(PrefixCharacters), text_.begin());

    std::array<char, detail::maxU64Digits> digits{};
    const usize count = detail::encodeDecimal(value, digits);
    const usize zeros = MinDigits > count ? MinDigits - count : 0;

    usize at = PrefixCharacters;
    for (usize pad{}; pad < zeros; ++pad)
      text_[at++] = '0';
    for (usize index{}; index < count; ++index)
      text_[at++] = digits[index];
    length_ = at;
  }

  /// Returns the number of bytes in the formatted code.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return length_;
  }

  /// Returns the formatted code; not NUL-terminated.
  [[nodiscard]] constexpr auto data() const noexcept -> const char * {
    return text_.data();
  }

  /// Returns the formatted code, e.g. `ITR004`; borrows this object.
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
    return {text_.data(), length_};
  }

  /// Returns the borrowed projection used by the shared header encoder.
  [[nodiscard]] constexpr auto codeView() const noexcept -> CodeView {
    return CodeView{
        .prefix = std::string_view{text_.data(), PrefixCharacters}, .numeric = numeric_, .minimumDigits = MinDigits};
  }

private:
  std::array<char, capacity> text_{};
  usize length_{};
  u64 numeric_{};
};

template <usize N>
Code(const char (&)[N], u64) -> Code<N - 1, 3>;

/// Builds a code with prefix `prefix` and number `value`, padded to at least `Width` digits.
template <usize Width = 3, usize N>
[[nodiscard]] constexpr auto code(const char (&prefix)[N], const u64 value) noexcept -> Code<N - 1, Width> {
  return Code<N - 1, Width>{prefix, value};
}

} // namespace Miracle::format
// NOLINTEND(cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
