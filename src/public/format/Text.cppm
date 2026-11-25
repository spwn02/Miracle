export module Miracle.Format:Text;

import std;
import Miracle.Numeric;

/// Byte-level encoders shared by the compile-time Message and the runtime emitter. Nothing here allocates.
namespace Miracle::format::detail {

/// Maximum number of decimal digits of a `u64`.
inline constexpr usize maxU64Digits = 20;
/// Longest escape spelling of one control byte (`\xNN`).
inline constexpr usize maxEscapeBytes = 4;

/// Writes the decimal digits of `value` into `out` and returns their count. Zero yields one digit.
constexpr auto encodeDecimal(u64 value, std::array<char, maxU64Digits> &out) noexcept -> usize {
  std::array<char, maxU64Digits> reversed{};
  usize count{};
  do {
    reversed[count++] = static_cast<char>('0' + (value % 10U)); // NOLINT(readability-magic-numbers)
    value /= 10U;                                               // NOLINT(readability-magic-numbers)
  } while (value != 0);

  for (usize index{}; index < count; ++index)
    out[index] = reversed[count - 1 - index];
  return count;
}

/// Returns true when `byte` is a C0 control or DEL and must be escaped inside single-line diagnostic fields.
[[nodiscard]] constexpr auto needsEscape(const unsigned char byte) noexcept -> bool {
  return byte < 0x20 or byte == 0x7F; // NOLINT(readability-magic-numbers)
}

/// Encodes one control byte as `\n`, `\r`, `\t`, `\0`, `\x1B` or uppercase `\xNN`; returns the spelling length.
constexpr auto encodeEscape(const unsigned char byte, std::array<char, maxEscapeBytes> &out) noexcept -> usize {
  switch (byte) {
    case '\n': out = {'\\', 'n'}; return 2;
    case '\r': out = {'\\', 'r'}; return 2;
    case '\t': out = {'\\', 't'}; return 2;
    case '\0': out = {'\\', '0'}; return 2;
    default: break;
  }
  constexpr std::string_view digits = "0123456789ABCDEF";
  out = {'\\', 'x', digits[byte >> 4U], digits[byte & 0x0FU]}; // NOLINT(readability-magic-numbers)
  return 4;
}

/// Worst-case output size of escaping `bytes` source bytes.
[[nodiscard]] consteval auto escapedBound(const usize bytes) noexcept -> usize {
  return bytes * maxEscapeBytes;
}

} // namespace Miracle::format::detail
