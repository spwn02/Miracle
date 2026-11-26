# Static string

`Miracle::BasicStaticString` is Miracle's structural fixed-capacity string value for constant evaluation, reflection metadata, annotations, generated names, feature identities, and compile-time diagnostics.

It is intentionally not an owning runtime text container. The future redesigned `Miracle::String` owns UTF-8 text; `BasicStaticString` instead provides a value that can participate directly in non-type template parameters while remaining pleasant to use in ordinary `constexpr` code.

## Public types

```cpp
template<class Char, usize Capacity>
struct BasicStaticString;

template<usize Capacity>
using StaticString = BasicStaticString<char, Capacity>;

template<class Char, usize Capacity>
struct StaticStringSplit;
```

`Capacity` excludes the terminating zero. The string also stores a logical `length`, so transformations such as trimming and removal do not need to invent a different runtime type merely because fewer code units remain active.

A literal uses CTAD:

```cpp
constexpr auto name = BasicStaticString{"Miracle"};
static_assert(decltype(name)::capacityValue == 7);
static_assert(name.size() == 7);
```

Explicit spare capacity is also legal:

```cpp
constexpr StaticString<32> name{"Miracle"};
static_assert(name.capacity() == 32);
static_assert(name.size() == 7);
```

## Structural representation

C++ structural NTTP rules require structural data members to be public. Therefore `BasicStaticString` exposes:

```cpp
std::array<Char, Capacity + 1> storage;
usize length;
```

Values produced through Miracle's API maintain the invariant:

```text
length <= Capacity
storage[length] == Char{}
```

Callers should treat the representation as low-level state rather than normal mutation API. Public algorithms do not expose unchecked append primitives; those remain module-local implementation details.

The representation allows direct NTTP use:

```cpp
template <BasicStaticString Name>
struct Named {};

using MiracleType = Named<"Miracle">;
```

## Construction and conversions

`BasicStaticString` constructs from a null-terminated character array when it fits inside the destination capacity, or explicitly from `std::array<Char, N>`.

Non-owning conversions are implicit:

```cpp
std::string_view view = BasicStaticString{"Miracle"};
const char* text = BasicStaticString{"Miracle"};
```

Owning conversion is explicit:

```cpp
auto text = static_cast<std::string>(BasicStaticString{"Miracle"});
auto same = BasicStaticString{"Miracle"}.toString();
```

This follows Miracle's project-wide rule: non-owning, lossless and unambigious conversions may be implicit; ownership-producing conversions are explicit.

## Observers and iteration

The type provides:

- `size()`;
- `capacity()`;
- `empty()`;
- `data()` / `cStr()`;
- `view()`;
- `begin()` / `end()`;
- `cbegin()` / `cend()`;
- `rbegin()` / `rend()`;
- `operator[]`;
- `front()` / `back()`.

Indexing, `front()` and `back()` are low-level unchecked operations, matching the cost model of the underlying fixed storage. Checked container/string indexing is a separate concern for the future runtime `String`/container redesign.

## Comparison

`operator==`, `operator<=>`, and `compare()` compare active contents, not capacity. This is important because spare compile-time capacity is a storage property, not part of string's semantic identity:

```cpp
constexpr auto a = BasicStaticString{"name"};
constexpr StaticString<64> b{"name"};
static_assert(a == b);
static_assert(a.compare("name") == 0);
```

Ordering against a raw `const Char*` is intentionally rejected so a null pointer cannot become undefined behavior and C++ cannot silently fall back to address ordering. Convert string literals or pointers to a non-owning string view when ordering is required. Equality with a null pointer is safely false.

## Substrings

There are two substring forms.

A template substring returns another structural static string:

```cpp
constexpr auto value = BasicStaticString{"0123456789"};
constexpr auto part = value.substr<2, 4>();
static_assert(part = BasicStaticString{"2345"});
```

A runtime-index substring returns a non-owning view and performs no allocation or copying:

```cpp
auto aprt = value.substr(2, 4); // std::string_view-like result
```

Positions beyond the active size yield an empty view/static string.

## Searching

The following operations mirror the familiar string vocabulary while using Miracle camelCase where the standard spelling is not already canonical:

```text
startsWith
endsWith
contains
find
rfind
```

Search failures return `BasicStaticString::npos`.

## Trimming

Default trimming removes ASCII whitespace:

```cpp
constexpr auto value = BasicStaticString{"  Miracle\n"};
static_assert(value.trim() == BasicStaticString{"Miracle"});
```

Available operations are:

```text
trimStart()
trimEnd()
trim()
```

Predicate variants allows arbitrary compile-time policies:

```text
trimStartBy(predicate)
trimEndBy(predicate)
trimBy(predicate)
```

The default operation is intentionally ASCII/code-unit based. Unicode normalization grapheme segmentation and Unicode case mapping belong to the future runtime text layer rather than being silently approximated here.

## Case conversion

`lower()` and `upper()` perform ASCII case conversion and preserve the same static capacity:

```cpp
static_assert(BasicStaticString{"Miracle"}.upper() == 
              BasicStaticString{"MIRACLE"});
```

Non-ASCII code units are preserved unchanged.

## Replacement and removal

Single-code-unit replacement/removal preserves the original capacity:

```cpp
constexpr auto value = BasicStaticString{"a-b-c"};
static_assert(value.replace('-', '_') == BasicStaticString{"a_b_c"});
static_assert(value.remove('-') == BasicStaticString{"abc"});
```

Static-string pattern replacement is also supported:

```cpp
constexpr auto value = BasicStaticString{"a--b--c"};
static_assert(value.replace(BasicStaticString{"--"}, BasicStaticString{" / "}) == 
              BasicStaticString{"a / b / c"}));
```

The result type reserves a compile-time worst-case capacity so replacement never allocates. An empty search pattern is defined as a no-op rather than creating an ambiguous infinite insertion rule.

Pattern removal similarly removes non-overlapping matches and keeps the original capacity.

## Concatenation and join

`operator+` concatenates two values and gives the result the sum of their facilities:

```cpp
constexpr auto name = BasicStaticString{"Miracle"} + BasicStaticString{".Meta"};
```

`join` accepts heterogeneous static-string capacities:

```cpp
constexpr auto name = join(BasicStaticString{"::"},
                           BasicStaticString{"Miracle"},
                           BasicStaticString{"Meta"},
                           BasicStaticString{"Trait"});
```

No intermediate runtime strings or allocations are created.

## Split

`split` owns its result so it remains valid independently of temporary inputs:

```cpp
constexpr auto parts = BasicStaticString{"a,b,,c"}.split(',');
static_assert(parts.size() == 4);
static_assert(parts[2].empty());
```

Both code-unit and static-string delimiters are supported. Empty fields are preserved. An empty string delimiter produces one part containing the original value instead of inventing character-boundary semantics.

`StaticStringSplit<Char, Capacity>` is itself structural. It stores at most `Capacity + 1` parts and exposes `size`, `empty`, `operator[]`, `begin` and `end`. The deliberately bounded representation avoids dynamic allocation during constant evaluation.

## Stable hashing

`hash()` computes 64-bit FNV-1a over active code units. The result is stable across processes and independent of spare capacity, making it suitable for compile-time metadata keys.

It is not a cryptographic hash and does not promise compatibility with the future Miracle `Hash` trait's selected runtime hashing policy.

## Compile-time diagnostic formatting

`staticFormat<Format>(args...)` provides a deliberately small structural formatter:

```cpp
constexpr auto message = 
    staticFormat<"member {} failed with code {}">(
        BasicStaticString{"position"}, -7);
```

Supported syntax is:

```text
{}  next argument
{{  literal {
}}  literal }
```

Supported argument categories are:

- `BasicStaticString<char, N>`;
- character arrays/string literals;
- `char`;
- `bool`;
- integral types;
- enums, formatted through their underlying integral type.

Placeholder count and brace structure are validated during constant evaluation. The function is meant for generated compiler diagnostics and metadata. It does not compete with the much broader runtime `std::format` grammar.

## `std::formatter`

Miracle specializes `std::formatter` for `BasicStaticString`, delegating formatting to the standard `basic_string_view` formatter. All normal string alignment/width formatting therefore comes from the standard formatter rather than a parallel Miracle implementation.

## Performance model

`BasicStaticString` performs no heap allocation. Operations are `constexpr` and `noexcept` whenever their callable predicates permit it. Returned transformed values are fixed-capacity objects whose storage requirements are determined at compile time.

The design intentionally accepts compile-time storage in exchange for structural NTTP compatibility. Large split/replacement values should therefore be treated as compile-time metadata tools, not as replacements for runtime dynamic strings.
