# Color and Format

`Miracle.Color` is a standard-library-and-`Miracle.Numeric`-only value module. `Miracle.Format` imports only `Miracle.Color`, `Miracle.Numeric` and the standard library. It is the single presentation foundation shared by compile-time messages and the runtime `Diagnostic` renderer. See [Diagnostics and panic](DIAGNOSTICS.md) for the runtime side and [Phase M design](phases/M/DESIGN.md) for the contract.

## Color

```cpp
import Miracle.Color;
using namespace Miracle;

constexpr Color orange = Color::rgb(255, 159, 67);          // typed channels, no validation
static_assert(Color::checkedRgb(0, 256, 0) == std::nullopt); // any integral type, whole range checked
static_assert(Color::hex(0xFF9F43) == orange);               // 0xRRGGBB, higher bits -> nullopt
static_assert(Color::parseHex("#f93") == Color::rgb(255, 153, 51)); // #RGB or #RRGGBB, ASCII hex only
static_assert(Color::fromHsv({0, 1, 1}) == Color::rgb(255, 0, 0));   // NaN/inf/out-of-range -> nullopt
static_assert(orange.toHex() == 0xFF9F43U);
```

`Color` is three `u8` channels (sRGB-coded), trivially copyable and structural. `Hsv{hueDegrees, saturation, value}` is a conversion carrier only. HSV to RGB quantizes to nearest with halves up; RGB to HSV to RGB differs by at most one code value per channel. `toHsv()` yields hue 0 for achromatic colors.

## Format

| Header | Contents |
|---|---|
| `Severity`, `ColorMode`, `Presentation` | one definition, in namespace `Miracle`; `Diagnostics` re-exports them |
| `format::TerminalColor`, `format::Style` | default / indexed 0-255 / RGB colour plus bold, dim, italic, underline; no ANSI bytes |
| `format::Fragment<N>`, `fragment()`, `styled()` | exactly `N` owned bytes plus a style; built from a string literal or `std::array<char, N>` |
| `format::Code<P, W>`, `code<W>()` | `ITR004`-style code, zero-padded to at least `W` digits, never truncated |
| `format::Message<C, R, D, S>` | immutable owned text plus style spans; every extension returns a new type with inferred capacity |
| `format::RenderPolicy`, `render()`, `renderTo()` | plain or ANSI rendering of a message |
| `format::Event`, `emit()`, `finish()`, `RenderState`, `TextSink`, `WriteResult`, `CodeView`, `Section` | runtime event writer used by `Diagnostics`; one layout and one SGR encoder |
| `Event::escape`, `severityStyle()` | runtime renderers with their own layout (`Error::display`, `Miracle.Diagnostics`) emit `Text` events through `emit`/`finish`; `escape` routes untrusted text through the one encoder, `severityStyle` gives the severity colour |

```cpp
constexpr auto message =
    format::Message{Severity::Error, format::code<3>("ITR", 4), "callable cannot accept sequence element"}
        .context("Iter::map")
        .field("element", format::fragment("std::tuple<int, float>"))
        .note("direct call, tuple expansion and reflected aggregate expansion failed")
        .help("define a callable matching one supported invocation shape");
static_assert(false, message);  // the compiler prints the authored body
```

Rules the types enforce:

- A message never owns more than it needs: capacity and style-run bounds are deduced from literal extents (a `char[N]` is read as C text: content ends at the first NUL and never exceeds `N - 1` bytes; exact bytes use `fragment(std::array<char, N>)`), and type-level additions are overflow-checked.
- `data()` and `view()` are always plain text. ANSI appears only when rendering with `Presentation::Terminal` and a `ColorMode` other than `Never`; every styled run ends in a reset.
- Diagnostic mode follows heading, optional single context, fields, notes, help. Moving backwards is a constraint failure, not a runtime error. The context line hugs the heading; later sections are preceded by one blank line; the message ends with exactly one LF.
- Control bytes in headings, context, field keys and values, notes and help are escaped (`\n`, `\r`, `\t`, `\0`, `\xNN`); `Text` events and `append` keep their bytes verbatim.
- Runtime strings never become a `Message`. Dynamic text flows through `emit()` with `std::string_view`; a failing sink makes the state permanently failed and returns `WriteResult::SinkFailed`.
- No `std::formatter`, no `{}` grammar, no floating-point formatting. Render functions are not `noexcept`.

## Diagnostic code domains

Miracle-authored hard compile-time errors are `error[CODE]: summary` messages built with this module.

| Domain | Codes |
|---|---|
| `SSTR` (StaticString) | 001 invalid character type, 002 `staticFormat` needs a `char` format, 003 unmatched or unsupported braces, 004 placeholder count mismatch |
| `FEA` (Feature) | 001-006 resolution failures (duplicate, missing feature, missing build feature, missing capability, cycle, conflict), 007 feature not enabled, 008 duplicate build-feature name, 009-012 descriptor option misuse |
| `ITR` (Iter) | 001 item callable, 002 accumulator callable, 003 tuple elements not invocable, 004-007 `Peekable::nextIfMap` contract |

Meta Query hard errors and `DiagnosticCode` concept failures keep their original text.
