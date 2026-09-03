export module Miracle:StaticString;

import std;
import :Types;

// NOLINTBEGIN(readability-identifier-naming, cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
namespace Miracle {

struct StaticStringAccess;

};

export namespace Miracle {

template <class Char, usize Capacity>
struct StaticStringSplit;

/// A structural, fixed-capacity string value designed for constant evaluation.
///
/// `Capacity` is the maximum number of code units excluding the terminating zero. `length` stores the active
/// number of code units. The public representation is required for structural NTTP use; normal callers should
/// prefer the member API instead of mutating it directly.
template <class Char, usize Capacity>
struct BasicStaticString final {
  static_assert(std::is_trivially_copyable_v<Char> and std::is_trivially_default_constructible_v<Char> and
                    std::is_standard_layout_v<Char>,
      "BasicStaticString requires a trivial standard-layout character type");

  /// Character/code-unit type stored by this string.
  using Value = Char;

  /// Unsigned size/index type used by Miracle strings.
  using Size = usize;

  /// Sentinel retnred by failed search operations.
  static constexpr Size npos = std::numeric_limits<Size>::max(); // NOLINT(readability-identifier-naming)

  /// Compile-time storage capacity excluding the terminating zero.
  static constexpr Size capacityValue = Capacity; // NOLINT(readability-identifier-naming)

  /// Structural storage. `storage[length]` is always the terminating zero when the value is produced through
  /// the public API.
  std::array<Char, Capacity + 1> storage{};

  /// Number of active code units in `storage`, excluding the terminator.
  Size length{};

  /// Constructs an empty static string.
  constexpr BasicStaticString() noexcept = default;

  /// Constructs from a null-terminated character array whose contents fit in this instance's capacity.
  template <usize N>
    requires(N > 0 and N - 1 <= Capacity)
  constexpr BasicStaticString(const Char (&value)[N]) noexcept
      : length(N - 1) {
    std::ranges::copy(value, storage.data());
    storage[length] = Char{};
  }

  /// Constructs from an array of code units without requiring a terminator.
  template <usize N>
    requires(N <= Capacity)
  constexpr explicit BasicStaticString(const std::array<Char, N> &value) noexcept
      : length(N) {
    std::ranges::copy(value, storage.begin());
    storage[length] = Char{};
  }

  /// Returns the number of active code units.
  [[nodiscard]] constexpr auto size() const noexcept -> Size {
    return length;
  }

  /// Returns the maximum number of active code units this value can hold.
  [[nodiscard]] static consteval auto capacity() noexcept -> Size {
    return Capacity;
  }

  /// Returns true when the string contains no active code units.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return length == 0;
  }

  /// Returns a pointer to the null-terminated storage.
  [[nodiscard]] constexpr auto data() const noexcept -> const Char * {
    return storage.data();
  }

  /// Returns the null-terminated storage for C-style APIs.
  [[nodiscard]] constexpr auto cStr() const noexcept -> const Char * {
    return data();
  }

  /// Returns a non-owning view of the acitve code units.
  [[nodiscard]] constexpr auto view() const noexcept -> std::basic_string_view<Char> {
    return {storage.data(), length};
  }

  /// Returns an iterator to the first active code unit.
  [[nodiscard]] constexpr auto begin() const noexcept -> const Char * {
    return storage.data();
  }

  /// Returns an iterator one past the final active code unit.
  [[nodiscard]] constexpr auto end() const noexcept -> const Char * {
    return storage.data() + length;
  }

  /// Returns a const iterator to the first active code unit.
  [[nodiscard]] constexpr auto cbegin() const noexcept -> const Char * {
    return begin();
  }

  /// Returns a const iterator one past the final active code unit.
  [[nodiscard]] constexpr auto cend() const noexcept -> const Char * {
    return end();
  }

  /// Returns a reverse iterator to the final active code unit.
  [[nodiscard]] constexpr auto rbegin() const noexcept {
    return std::reverse_iterator{end()};
  }

  /// Returns a reverse iterator one before the first active code unit.
  [[nodiscard]] constexpr auto rend() const noexcept {
    return std::reverse_iterator{begin()};
  }

  /// Returns the code unit at `index`. The caller must keep `index < size()`.
  [[nodiscard]] constexpr auto operator[](Size index) const noexcept -> const Char & {
    return storage[index];
  }

  /// Returns the first code unit. The string must not be empty.
  [[nodiscard]] constexpr auto front() const noexcept -> const Char & {
    return storage[0];
  }

  /// Returns the final active code unit. The string must not be empty.
  [[nodiscard]] constexpr auto back() const noexcept -> const Char & {
    return storage[length - 1];
  }

  /// Implicitly converts to a non-owning standard string view.
  [[nodiscard]] constexpr operator std::basic_string_view<Char>() const noexcept {
    return view();
  }

  /// Implicitly converts to the null-terminated character representation.
  [[nodiscard]] constexpr operator const Char *() const noexcept {
    return cStr();
  }

  /// Explicitly creates an owning standard string.
  [[nodiscard]] explicit operator std::basic_string<Char>() const {
    return std::basic_string<Char>(view());
  }

  /// Explicitly creates an owning standard string.
  [[nodiscard]] auto toString() const -> std::basic_string<Char> {
    return std::basic_string<Char>{view()};
  }

  /// Returns a compile-time-capacity substring as another static string.
  template <Size Position, Size Count = npos>
    requires(Position <= Capacity)
  [[nodiscard]] constexpr auto substr() const noexcept {
    constexpr Size remainingCapacity = Capacity - Position;
    constexpr Size resultCapacity = Count == npos or Count > remainingCapacity ? remainingCapacity : Count;

    BasicStaticString<Char, resultCapacity> result;
    if (Position >= length) {
      return result;
    }

    const Size actual = Count == npos ? length - Position : std::min(Count, length - Position);
    result.appendUnchecked({storage.data() + Position, actual});
    return result;
  }

  /// Returns a runtime substring view without allocating or copying.
  [[nodiscard]] constexpr auto substr(Size position, Size count = npos) const noexcept
      -> std::basic_string_view<Char> {
    if (position >= length) {
      return {};
    }

    const Size actual = count == npos ? length - position : std::min(count, length - position);
    return {storage.data() + position, actual};
  }

  /// Returns whether this string begin with `prefix`.
  [[nodiscard]] constexpr auto startsWith(std::basic_string_view<Char> prefix) const noexcept -> bool {
    return view().starts_with(prefix);
  }

  /// Returns whether this string begins with `prefix`.
  [[nodiscard]] constexpr auto startsWith(Char prefix) const noexcept -> bool {
    return not empty() and front() == prefix;
  }

  /// Returns whether this string ends with `suffix`.
  [[nodiscard]] constexpr auto endsWith(std::basic_string_view<Char> suffix) const noexcept -> bool {
    return view().ends_with(suffix);
  }

  /// Returns whether this string ends with `suffix`.
  [[nodiscard]] constexpr auto endsWith(Char suffix) const noexcept -> bool {
    return not empty() and back() == suffix;
  }

  /// Returns whether this string contains `needle`.
  [[nodiscard]] constexpr auto contains(std::basic_string_view<Char> needle) const noexcept -> bool {
    return find(needle) != npos;
  }

  /// Returns whether this string contains `needle`.
  [[nodiscard]] constexpr auto contains(Char needle) const noexcept -> bool {
    return find(needle) != npos;
  }

  /// Returns the first position of `needle`, or `npos` when absent.
  [[nodiscard]] constexpr auto find(std::basic_string_view<Char> needle, Size position = 0) const noexcept
      -> Size {
    const auto found = view().find(needle, position);
    return found != std::basic_string_view<Char>::npos ? found : npos;
  }

  /// Returns the first position of `needle`, or `npos` when absent.
  [[nodiscard]] constexpr auto find(Char needle, Size position = 0) const noexcept -> Size {
    const auto found = view().find(needle, position);
    return found != std::basic_string_view<Char>::npos ? found : npos;
  }

  /// Returns the final position of `needle`, or `npos` when absent.
  [[nodiscard]] constexpr auto rfind(std::basic_string_view<Char> needle, Size position = npos) const noexcept
      -> Size {
    const auto actualPosition = position != npos ? position : std::basic_string_view<Char>::npos;
    const auto found = view().rfind(needle, actualPosition);
    return found != std::basic_string_view<Char>::npos ? found : npos;
  }

  /// Returns the final position of `needle`, or `npos` when absent.
  [[nodiscard]] constexpr auto rfind(Char needle, Size position = npos) const noexcept -> Size {
    const auto actualPosition = position != npos ? position : std::basic_string_view<Char>::npos;
    const auto found = view().rfind(needle, actualPosition);
    return found != std::basic_string_view<Char>::npos ? found : npos;
  }

  /// Compares active contents lexicographically with `other`.
  ///
  /// The return value is negative, zero, or positive when this value is respectively less than, equal to, or
  /// greater than `other`.
  [[nodiscard]] constexpr auto compare(std::basic_string_view<Char> other) const noexcept -> int {
    return view().compare(other);
  }

  /// Removes leading ASCII whitespace and returns a new static string.
  [[nodiscard]] constexpr auto trimStart() const noexcept -> BasicStaticString {
    Size first{};
    while (first < length and asciiWhitespace(storage[first])) {
      ++first;
    }
    return sliceIntoSameCapacity(first, length);
  }

  /// Removes trailing ASCII whitespace and returns a new static string.
  [[nodiscard]] constexpr auto trimEnd() const noexcept -> BasicStaticString {
    Size last = length;
    while (last > 0 and asciiWhitespace(storage[last - 1])) {
      --last;
    }
    return sliceIntoSameCapacity(0, last);
  }

  /// Removes leading and trailing ASCII whitespace.
  [[nodiscard]] constexpr auto trim() const noexcept -> BasicStaticString {
    Size first{};
    Size last = length;

    while (first < last and asciiWhitespace(storage[first])) {
      ++first;
    }

    while (last > first and asciiWhitespace(storage[last - 1])) {
      --last;
    }

    return sliceIntoSameCapacity(first, last);
  }

  /// Removes leading code units while `predicate` returns true.
  template <class Predicate>
    requires std::predicate<Predicate &, Char>
  [[nodiscard]] constexpr auto trimStartBy(Predicate predicate) const
      noexcept(std::is_nothrow_invocable_r_v<bool, Predicate &, Char>) -> BasicStaticString {
    Size first{};
    while (first < length and std::invoke(predicate, storage[first])) {
      ++first;
    }
    return sliceIntoSameCapacity(first, length);
  }

  /// Removes trailing code units while `predicate` returns true.
  template <class Predicate>
    requires std::predicate<Predicate &, Char>
  [[nodiscard]] constexpr auto trimEndBy(Predicate predicate) const
      noexcept(std::is_nothrow_invocable_r_v<bool, Predicate &, Char>) -> BasicStaticString {
    Size last = length;
    while (last > 0 and std::invoke(predicate, storage[last - 1])) {
      --last;
    }
    return sliceIntoSameCapacity(0, last);
  }

  /// Removes leading and trailing code units while `predicate` returns true.
  template <class Predicate>
    requires std::predicate<Predicate &, Char>
  [[nodiscard]] constexpr auto trimBy(Predicate predicate) const
      noexcept(std::is_nothrow_invocable_r_v<bool, Predicate &, Char>) -> BasicStaticString {
    Size first{};
    Size last = length;
    while (first < last and std::invoke(predicate, storage[first])) {
      ++first;
    }
    while (last > first and std::invoke(predicate, storage[last - 1])) {
      --last;
    }
    return sliceIntoSameCapacity(first, last);
  }

  /// Converts ASCII uppercase code units to lowercase.
  [[nodiscard]] constexpr auto lower() const noexcept -> BasicStaticString {
    BasicStaticString result = *this;
    for (Size index{}; index < result.length; ++index) {
      if (result.storage[index] >= static_cast<Char>('A') and
          result.storage[index] <= static_cast<Char>('Z')) {
        result.storage[index] =
            static_cast<Char>(result.storage[index] + (static_cast<Char>('a') - static_cast<Char>('A')));
      }
    }
    return result;
  }

  /// Converts ASCII lowercase code units to uppercase.
  [[nodiscard]] constexpr auto upper() const noexcept -> BasicStaticString {
    BasicStaticString result = *this;
    for (Size index{}; index < result.length; ++index) {
      if (result.storage[index] >= static_cast<Char>('a') and
          result.storage[index] <= static_cast<Char>('z')) {
        result.storage[index] =
            static_cast<Char>(result.storage[index] - (static_cast<Char>('a') - static_cast<Char>('A')));
      }
    }
    return result;
  }

  /// Replaces every matching code unit without changing capacity.
  [[nodiscard]] constexpr auto replace(Char from, Char to) const noexcept -> BasicStaticString { // NOLINT
    BasicStaticString result = *this;
    for (Size index{}; index < result.length; ++index) {
      if (result.storage[index] == from) {
        result.storage[index] = to;
      }
    }
    return result;
  }

  /// Replaces every non-overlapping occurence of `from` with `to`.
  ///
  /// The result capacity is a compile-time worst-case bound. An empty `from` string intentionally leaves the
  /// value unchanged.
  template <usize FromCapacity, usize ToCapacity>
  [[nodiscard]] constexpr auto replace(const BasicStaticString<Char, FromCapacity> &from,
      const BasicStaticString<Char, ToCapacity> &to) const noexcept { // NOLINT
    constexpr Size expansion = ToCapacity == 0 ? 1 : ToCapacity;
    BasicStaticString<Char, Capacity * expansion> result;

    if (from.empty()) {
      result.appendUnchecked(view());
      return result;
    }

    Size position{};
    while (position < length) {
      const bool matches =
          position + from.size() <= length and
          std::basic_string_view<Char>{storage.data() + position, from.size()} == from.view();

      if (matches) {
        result.appendUnchecked(to.view());
        position += from.size();
      } else {
        result.pushUnchecked(storage[position]);
        ++position;
      }
    }

    return result;
  }

  /// Removes every matching code unit.
  [[nodiscard]] constexpr auto remove(Char needle) const noexcept -> BasicStaticString {
    BasicStaticString result;
    for (Size index{}; index < length; ++index) {
      if (storage[index] != needle) {
        result.pushUnchecked(storage[index]);
      }
    }
    return result;
  }

  /// Removes every non-overlapping occurence of `needle`.
  template <usize NeedleCapacity>
  [[nodiscard]] constexpr auto remove(const BasicStaticString<Char, NeedleCapacity> &needle) const noexcept
      -> BasicStaticString {
    if (needle.empty()) {
      return *this;
    }

    BasicStaticString result;
    Size position{};
    while (position < length) {
      const bool matches =
          position + needle.size() <= length and
          std::basic_string_view<Char>{storage.data() + position, needle.size()} == needle.view();

      if (matches) {
        position += needle.size();
      } else {
        result.pushUnchecked(storage[position]);
        ++position;
      }
    }
    return result;
  }

  /// Splits the value on one delimiter and owns every resulting part.
  [[nodiscard]] constexpr auto split(Char delimiter) const noexcept -> StaticStringSplit<Char, Capacity>;

  /// Splits the value on a non-empty delimiter and owns every resulting part.
  template <usize DelimiterCapacity>
  [[nodiscard]] constexpr auto split(
      const BasicStaticString<Char, DelimiterCapacity> &delimiter) const noexcept
      -> StaticStringSplit<Char, Capacity>;

  /// Computes a stable FNV-1a hash over the acitve code units.
  [[nodiscard]] constexpr auto hash() const noexcept -> u64 {
    u64 value = 14695981039346656037ULL; // NOLINT
    using UnsignedChar = std::make_unsigned_t<Char>;

    for (Size index{}; index < length; ++index) {
      value ^= static_cast<u64>(static_cast<UnsignedChar>(storage[index]));
      value *= 1099511628211ULL; // NOLINT
    }
    return value;
  }

private:
  template <class OtherChar, usize OtherCapacity>
  friend struct BasicStaticString;

  friend struct StaticStringAccess;

  /// Returns whether `value` is one of the ASCII whitespace code units used by the default trim family.
  [[nodiscard]] static constexpr auto asciiWhitespace(Char value) noexcept -> bool {
    return value == static_cast<Char>(' ') or value == static_cast<Char>('\t') or
           value == static_cast<Char>('\n') or value == static_cast<Char>('\r') or
           value == static_cast<Char>('\f') or value == static_cast<Char>('\v');
  }

  /// Appends `value` after the active contents without a capacity branch.
  ///
  /// Every caller computes a result capacity that proves the write fits. Keeping this primitive private
  /// prevents ordinary code from violating the structural representation invariant.
  constexpr auto appendUnchecked(std::basic_string_view<Char> value) noexcept -> void {
    std::ranges::copy(value, storage.begin() + static_cast<isize>(length));
    length += value.size();
    storage[length] = Char{};
  }

  /// Appends one code unit after the active contents without a capacity branch.
  constexpr auto pushUnchecked(Char value) noexcept -> void {
    storage[length] = value;
    ++length;
    storage[length] = Char{};
  }

  /// Copies `[first, last)` into a value retaining this specialization's capacity. Trim operations use this
  /// to avoid value-dependent result types.
  [[nodiscard]] constexpr auto sliceIntoSameCapacity(Size first, Size last) const noexcept
      -> BasicStaticString {
    BasicStaticString result;
    if (first < last) {
      result.appendUnchecked({storage.data() + first, last - first});
    }
    return result;
  }
};

} // namespace Miracle

namespace Miracle {

/// Module-private bridge used by exported static-string algorithms without exposing mutation primitives in
/// the public API.
struct StaticStringAccess final {
  template <class Char, usize Capacity>
  static constexpr auto append(BasicStaticString<Char, Capacity> &target,
      std::basic_string_view<Char> value) noexcept -> void {
    target.appendUnchecked(value);
  }

  template <class Char, usize Capacity>
  static constexpr auto push(BasicStaticString<Char, Capacity> &target, Char value) noexcept -> void {
    target.pushUnchecked(value);
  }
};

} // namespace Miracle

export namespace Miracle {

/// Deduces static capacity from a null-terminated character array literal.
template <class Char, usize N>
BasicStaticString(const Char (&)[N]) -> BasicStaticString<Char, N - 1>;

/// Convenience alias for the ordinary UTF-8/code-unit `char` static string.
template <usize Capacity>
using StaticString = BasicStaticString<char, Capacity>;

// Owned result of a compile-time/static-string split operation.
template <class Char, usize Capacity>
struct StaticStringSplit final {
  /// Maximum number of produced parts for a string of this capacity.
  static constexpr usize maxParts = Capacity + 1;

  /// Structural storage for split parts.
  std::array<BasicStaticString<Char, Capacity>, maxParts> parts{};

  /// Number of valid entries in `parts`.
  usize count{};

  /// Returns the number of split parts.
  [[nodiscard]] constexpr auto size() const noexcept -> usize {
    return count;
  }

  /// Returns true when no split parts were produced.
  [[nodiscard]] constexpr auto empty() const noexcept -> bool {
    return count == 0;
  }

  /// Returns the split part at `index`.
  [[nodiscard]] constexpr auto operator[](usize index) const noexcept
      -> const BasicStaticString<Char, Capacity> & {
    return parts[index];
  }

  /// Returns an iterator to the first split part.
  [[nodiscard]] constexpr auto begin() const noexcept {
    return parts.begin();
  }

  /// Returns an iterator one past the final valid split part.
  [[nodiscard]] constexpr auto end() const noexcept {
    return parts.begin() + static_cast<isize>(count);
  }
};

template <class Char, usize Capacity>
constexpr auto BasicStaticString<Char, Capacity>::split(Char delimiter) const noexcept
    -> StaticStringSplit<Char, Capacity> {
  StaticStringSplit<Char, Capacity> result;
  usize start{};

  for (usize position{}; position <= length; ++position) {
    if (position == length or storage[position] == delimiter) {
      auto &part = result.parts[result.count++];
      if (start < position) {
        part.appendUnchecked({storage.data() + start, position - start});
      }
      start = position + 1;
    }
  }
  return result;
}

template <class Char, usize Capacity>
template <usize DelimiterCapacity>
constexpr auto BasicStaticString<Char, Capacity>::split(
    const BasicStaticString<Char, DelimiterCapacity> &delimiter) const noexcept
    -> StaticStringSplit<Char, Capacity> {
  StaticStringSplit<Char, Capacity> result;
  if (delimiter.empty()) {
    result.parts[0].appendUnchecked(view());
    result.count = 1;
    return result;
  }

  usize start{};
  usize position{};
  while (position <= length) {
    const bool atEnd = position == length;
    const bool matches =
        not atEnd and position + delimiter.size() <= length and
        std::basic_string_view<Char>(storage.data() + position, delimiter.size()) == delimiter.view();

    if (atEnd or matches) {
      auto &part = result.parts[result.count++];
      if (start < position) {
        part.appendUnchecked({storage.data() + start, position - start});
      }
      if (atEnd) {
        break;
      }
      position += delimiter.size();
      start = position;
      continue;
    }
    ++position;
  }
  return result;
}

/// Concatenates two static strings without allocation.
template <class Char, usize LeftCapacity, usize RightCapacity>
[[nodiscard]] constexpr auto operator+(const BasicStaticString<Char, LeftCapacity> &left,
    const BasicStaticString<Char, RightCapacity> &right) noexcept
    -> BasicStaticString<Char, LeftCapacity + RightCapacity> {
  BasicStaticString<Char, LeftCapacity + RightCapacity> result;
  StaticStringAccess::append(result, left.view());
  StaticStringAccess::append(result, right.view());
  return result;
}

/// Compares two static strings by active contents rather than storage capacity.
template <class Char, usize LeftCapacity, usize RightCapacity>
[[nodiscard]] constexpr auto operator==(const BasicStaticString<Char, LeftCapacity> &left,
    const BasicStaticString<Char, RightCapacity> &right) noexcept -> bool {
  return left.view() == right.view();
}

/// Orders two static strings lexicographically by active contents.
template <class Char, usize LeftCapacity, usize RightCapacity>
[[nodiscard]] constexpr auto operator<=>(const BasicStaticString<Char, LeftCapacity> &left,
    const BasicStaticString<Char, RightCapacity> &right) noexcept {
  return left.view() <=> right.view();
}

/// Compares a static string with a non-owning string view by contents.
template <class Char, usize Capacity>
[[nodiscard]] constexpr auto operator==(const BasicStaticString<Char, Capacity> &left,
    std::basic_string_view<Char> right) noexcept -> bool {
  return left.view() == right;
}

/// Compares a non-owning string view with a static string by contents.
template <class Char, usize Capacity>
[[nodiscard]] constexpr auto operator==(std::basic_string_view<Char> left,
    const BasicStaticString<Char, Capacity> &right) noexcept -> bool {
  return left == right.view();
}

/// Compares a static string with a null-terminated character sequence by contents.
template <class Char, usize Capacity>
[[nodiscard]] constexpr auto operator==(const BasicStaticString<Char, Capacity> &left,
    const Char *right) noexcept -> bool {
  return right != nullptr and left.view() == std::basic_string_view<Char>{right};
}

/// Compares a null-terminated character sequence with a static string by contents.
template <class Char, usize Capacity>
[[nodiscard]] constexpr auto operator==(const Char *left,
    const BasicStaticString<Char, Capacity> &right) noexcept -> bool {
  return left != nullptr and std::basic_string_view<Char>{left} == right.view();
}

/// Orders a static string and string view lexicographically.
template <class Char, usize Capacity>
[[nodiscard]] constexpr auto operator<=>(const BasicStaticString<Char, Capacity> &left,
    std::basic_string_view<Char> right) noexcept {
  return left.view() <=> right;
}

/// Orders a string view and static string lexicographically.
template <class Char, usize Capacity>
[[nodiscard]] constexpr auto operator<=>(std::basic_string_view<Char> left,
    const BasicStaticString<Char, Capacity> &right) noexcept {
  return left <=> right.view();
}

/// Pointer ordering is deliberately rejected because a null pointer is not a string and built-in pointer
/// ordering would compare addresses, not contents.
template <class Char, usize Capacity>
auto operator<=>(const BasicStaticString<Char, Capacity> &, const Char *) = delete (
    "Pointer ordering is deliberately rejected because a null pointer is not a string and built-in pointer "
    "ordering would compare addresses, not contents.");

/// Pointer ordering is deliberately rejected because a null pointer is not a string and built-in pointer
/// ordering would compare addresses, not contents.
template <class Char, usize Capacity>
auto operator<=>(const Char *, const BasicStaticString<Char, Capacity> &) = delete (
    "Pointer ordering is deliberately rejected because a null pointer is not a string and built-in pointer "
    "ordering would compare addresses, not contents.");

/// Joins one or more static strings with `separator` without allocation.
template <class Char, usize SeparatorCapacity, usize FirstCapacity, usize... RestCapacity>
[[nodiscard]] constexpr auto join(const BasicStaticString<Char, SeparatorCapacity> &separator,
    const BasicStaticString<Char, FirstCapacity> &first,
    const BasicStaticString<Char, RestCapacity> &...rest) noexcept {
  constexpr usize resultCapacity =
      FirstCapacity + (RestCapacity + ... + 0) + SeparatorCapacity * sizeof...(rest);
  BasicStaticString<Char, resultCapacity> result;
  StaticStringAccess::append(result, first.view());
  ((StaticStringAccess::append(result, separator.view()), StaticStringAccess::append(result, rest.view())),
      ...);
  return result;
}

}; // namespace Miracle

namespace Miracle::StaticStringFormatting {

/// Detects Miracle static-string specializations without exporting a public trait solely for formatter
/// implementation.
template <class T>
struct IsBasicStaticString : std::false_type {};

/// Marks every `BasicStaticString` specialization as a supported static string.
template <class Char, usize Capacity>
struct IsBasicStaticString<BasicStaticString<Char, Capacity>> : std::true_type {};

/// Convenient normalized value form of `IsBasicStaticString`.
template <class T>
inline constexpr bool is_basic_static_string_v = IsBasicStaticString<std::remove_cvref_t<T>>::value;

/// Returns whether the diagnostic formatter has a bounded, allocation-free representation for `T`.
template <class T>
consteval auto supportedArgument() -> bool {
  using Value = std::remove_cvref_t<T>;
  return is_basic_static_string_v<Value> or
         (std::is_array_v<Value> && std::same_as<std::remove_extent_t<Value>, char>) or
         std::same_as<Value, char> or std::same_as<Value, bool> or std::integral<Value> or
         std::is_enum_v<Value>;
}

/// Computes a conservative number of output code units required for one supported formatting argument.
template <class T>
  requires(supportedArgument<T>())
consteval auto argumentCapacity() -> usize {
  using Value = std::remove_cvref_t<T>;

  if constexpr (is_basic_static_string_v<Value>) {
    return Value::capacityValue;
  } else if constexpr (std::is_array_v<Value>) {
    return std::extent_v<Value> == 0 ? 0 : std::extent_v<Value> - 1;
  } else if constexpr (std::same_as<Value, char>) {
    return 1;
  } else if constexpr (std::same_as<Value, bool>) {
    return 5; // NOLINT
  } else if constexpr (std::integral<Value>) {
    return static_cast<usize>(std::numeric_limits<Value>::digits10) + 3;
  } else {
    return argumentCapacity<std::underlying_type_t<Value>>();
  }
}

/// Validates the deliberately small brace grammar and counts sequential placeholders. `npos` represents
/// malformed syntax.
template <BasicStaticString Format>
consteval auto placeholderCount() -> usize {
  usize count{};
  for (usize index{}; index < Format.size(); ++index) {
    if (Format[index] == '{') {
      if (index + 1 < Format.size() and Format[index + 1] == '{') {
        ++index;
      } else if (index + 1 < Format.size() and Format[index + 1] == '}') {
        ++count;
        ++index;
      } else {
        return BasicStaticString<char, 0>::npos;
      }
    } else if (Format[index] == '}') {
      if (index + 1 < Format.size() and Format[index + 1] == '}') {
        ++index;
      } else {
        return BasicStaticString<char, 0>::npos;
      }
    }
  }
  return count;
}

/// Integral constraint for values accepted by `staticFormat`.
template <class T>
concept Argument = supportedArgument<T>();

/// Appends a single character argument.
template <usize Capacity>
constexpr auto appendArgument(BasicStaticString<char, Capacity> &result, char value) noexcept -> void {
  StaticStringAccess::push(result, value);
}

/// Appends `true` or `false` without routing through runtime formatting.
template <usize Capacity>
constexpr auto appendArgument(BasicStaticString<char, Capacity> &result, bool value) noexcept -> void {
  StaticStringAccess::append(result, StringView{value ? "true" : "false"});
}

/// Appends an integral value in decimal form, including the minimum signed value without overflowing while
/// computing its magnitude.
template <usize Capacity, class T>
  requires std::integral<std::remove_cvref_t<T>> and
           (not std::same_as<std::remove_cvref_t<T>, bool> and not std::same_as<std::remove_cvref_t<T>, char>)
constexpr auto appendArgument(BasicStaticString<char, Capacity> &result, T value) noexcept -> void {
  using Value = std::remove_cvref_t<T>;
  using Unsigned = std::make_unsigned_t<Value>;

  bool negative{};
  Unsigned magnitude{};
  if constexpr (std::signed_integral<Value>) {
    negative = value < 0;
    const auto encoded = static_cast<Unsigned>(value);
    magnitude = negative ? Unsigned{} - encoded : encoded;
  } else {
    magnitude = value;
  }

  std::array<char, static_cast<usize>(std::numeric_limits<Unsigned>::digits10) + 3> digits{};
  usize count{};
  // NOLINTBEGIN
  do {
    digits[count++] = static_cast<char>('0' + (magnitude % 10));
    magnitude /= 10;
  } while (magnitude != 0);
  // NOLINTEND

  if (negative) {
    StaticStringAccess::push(result, '-');
  }
  while (count > 0) {
    StaticStringAccess::push(result, digits[--count]);
  }
}

/// Appends an enum through its underlying integral representation.
template <usize Capacity, class T>
  requires std::is_enum_v<std::remove_cvref_t<T>>
constexpr auto appendArgument(BasicStaticString<char, Capacity> &result, T value) noexcept -> void {
  appendArgument(result, static_cast<std::underlying_type_t<std::remove_cvref_t<T>>>(value));
}

/// Appends another static string's active contents.
template <usize Capacity, usize StringCapacity>
constexpr auto appendArgument(BasicStaticString<char, Capacity> &result,
    const BasicStaticString<char, StringCapacity> &value) noexcept -> void {
  StaticStringAccess::append(result, value.view());
}

/// Appends a null-terminated character array without its terminator.
template <usize Capacity, usize N>
constexpr auto appendArgument(BasicStaticString<char, Capacity> &result, const char (&value)[N]) noexcept
    -> void {
  StaticStringAccess::append(result, StringView{value, N == 0 ? 0 : N - 1});
}

/// Selects the requested heterogeneous tuple argument at compile time while the format parser advances
/// through placeholders at constant evaluation.
template <usize Index = 0, usize Capacity, class Tuple>
constexpr auto appendTupleArgument(BasicStaticString<char, Capacity> &result,
    Tuple &arguments,
    usize requested) noexcept -> void {
  if constexpr (Index < std::tuple_size_v<std::remove_reference_t<Tuple>>) {
    if (Index == requested) {
      appendArgument(result, std::get<Index>(arguments));
      return;
    }
    appendTupleArgument<Index + 1>(result, arguments, requested);
  }
}

} // namespace Miracle::StaticStringFormatting

export namespace Miracle {

/// Formats compile-time-friendly values into a structural staic string.
///
/// The format syntax intentionally supports only sequential `{}` placeholders plus escaped `{{` and `}}`.
/// This small constexpr formatter is designed for generated diagnostics and metadata, not as a replacement
/// for `std::format`.
template <BasicStaticString Format, class... Args>
  requires(StaticStringFormatting::Argument<Args> && ...)
[[nodiscard]] consteval auto staticFormat(Args &&...args) {
  static_assert(std::same_as<typename decltype(Format)::Value, char>,
      "staticFormat currently requires a char format string");
  constexpr usize placeholders = StaticStringFormatting::placeholderCount<Format>();
  static_assert(placeholders != BasicStaticString<char, 0>::npos,
      "staticFormat contains an unmatched or unsupported brace sequence");
  static_assert(
      placeholders == sizeof...(Args), "staticFormat placeholder count does not match argument count");

  constexpr usize resultCapacity =
      decltype(Format)::capacityValue + (StaticStringFormatting::argumentCapacity<Args>() + ... + 0);
  BasicStaticString<char, resultCapacity> result;
  auto arguments = std::forward_as_tuple(std::forward<Args>(args)...);

  usize argumentIndex{};
  for (usize index{}; index < Format.size(); ++index) {
    if (Format[index] == '{') {
      if (index + 1 < Format.size() and Format[index + 1] == '{') {
        StaticStringAccess::push(result, '{');
        ++index;
      } else {
        StaticStringFormatting::appendTupleArgument(result, arguments, argumentIndex++);
        ++index;
      }
    } else if (Format[index] == '}') {
      StaticStringAccess::push(result, '}');
      ++index;
    } else {
      StaticStringAccess::push(result, Format[index]);
    }
  }

  return result;
}

} // namespace Miracle

/// Formats a Miracle static string using the ordinary string-view formatter.
export template <class Char, Miracle::usize Capacity>
struct std::formatter<Miracle::BasicStaticString<Char, Capacity>, Char>
    : std::formatter<std::basic_string_view<Char>, Char> {
  /// Writes the active static-string contents through the standard string-view formatter, preserving the
  /// complete standard string format grammar.
  template <class FormatContext>
  constexpr auto format(const Miracle::BasicStaticString<Char, Capacity> &value,
      FormatContext &context) const {
    return std::formatter<std::basic_string_view<Char>, Char>::format(value.view(), context);
  }
};

// NOLINTEND(readability-identifier-naming, cppcoreguidelines-avoid-c-arrays, modernize-avoid-c-arrays)
