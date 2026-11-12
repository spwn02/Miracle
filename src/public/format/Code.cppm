export module Miracle.Format:Code;

import std;
import Miracle.Numeric;

export namespace Miracle::format {

/// Represents a diagnostic code with a prefix and numeric value, formatted as a string.
template <usize PrefixN, usize Width = 3>
struct Code {
private:
  /// The maximum number of decimal digits in a u64 value with sign and null-terminator.
  static constexpr usize maxValueDigits_ = 20;
  /// The number of bytes in the prefix, excluding the null terminator.
  static constexpr usize prefixBytes_ = (PrefixN > 1) ? PrefixN - 1 : 0;
  /// The total capacity of the diagnostic code, including prefix and numeric value.
  static constexpr usize totalCapacity_ = prefixBytes_ + std::max(Width, maxValueDigits_);

public:
  /// The text representation of the diagnostic code, null-terminated for C-style APIs.
  std::array<char, totalCapacity_ + 1> text{};
  /// The number of characters in the diagnostic code, excluding the null terminator.
  usize length{};

  /// Constructs a diagnostic code with the specified prefix and numeric value.
  constexpr Code(const char (&prefix)[PrefixN], const u64 value) noexcept // NOLINT
  {
    auto ptr = text.begin();

    // Copy prefix if P > 0 (omits prefix if PrefixN <= 1)
    if constexpr (prefixBytes_ > 0) {
      ptr =
          std::ranges::copy(std::span<const char>(static_cast<const char *>(prefix), prefixBytes_), ptr).out;
    }

    // Convert u64 to string digits
    std::array<char, maxValueDigits_> buf{};
    const auto [p_end, ec] = std::to_chars(buf.begin(), buf.end(), value);
    const auto len = static_cast<usize>(p_end - buf.begin());

    // Calculate minimum width padding: max(Width, decimalDigits(number))
    const usize targetWidth = std::max(Width, len);
    const usize zeros = targetWidth - len;

    // Append zero padding
    ptr = std::ranges::fill_n(ptr, zeros, '0');

    // Append number digits
    ptr = std::ranges::copy(buf.begin(), p_end, ptr).out;

    // Null-terminate
    *ptr = '\0';

    length = static_cast<usize>(ptr - text.begin());
  }

  /// Returns the total capacity of the diagnostic code, including prefix and numeric value.
  [[nodiscard]] static constexpr auto capacity() noexcept -> usize {
    return totalCapacity_;
  }

  /// Returns the number of characters in the diagnostic code, excluding the null terminator.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return length;
  }

  /// Returns a pointer to the diagnostic code text, null-terminated for C-style APIs.
  [[nodiscard]] constexpr auto data() const noexcept -> const char * {
    return text.data();
  }

  /// Returns a string view of the diagnostic code text, excluding the null terminator.
  [[nodiscard]] constexpr auto view() const noexcept -> std::string_view {
    return {text.data(), length};
  }
};

/// Deduction guide for constructing a diagnostic code with the specified prefix and numeric value, using a
/// default width of 3.
template <usize PrefixN, usize Width = 3>
Code(const char (&prefix)[PrefixN], const u64 value) -> Code<PrefixN, Width>; // NOLINT

/// Constructs a diagnostic code with the specified prefix and numeric value, using a default width of 3.
template <usize Width = 3, usize N>
[[nodiscard]] constexpr auto code(const char (&prefix)[N], const u64 value) noexcept // NOLINT
    -> Code<N, Width> {
  return Code<N, Width>{prefix, value};
}

} // namespace Miracle::format
