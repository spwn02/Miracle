export module Miracle.Format:Types;

import std;
import Miracle.Numeric;

export namespace Miracle {

/// Diagnostic importance used by rendering and filtering layers.
enum class Severity : u8 {
  /// Developer-oriented diagnostic information.
  Debug,
  /// Informational diagnostic context.
  Note,
  /// Recoverable or cautionary condition.
  Warning,
  /// Failure requiring caller attention.
  Error,
};

/// Controls ANSI colour emission.
enum class ColorMode : u8 {
  /// Follow the presentation policy: terminal presentation allows colour, plain presentation does not.
  Automatic,
  /// Request colour for terminal presentation.
  Always,
  /// Suppress ANSI colour unconditionally.
  Never,
};

/// Selects plain text versus terminal-oriented presentation.
enum class Presentation : u8 {
  /// Stable text presentation with no terminal escape sequences.
  Plain,
  /// Terminal-oriented presentation eligible for ANSI styling.
  Terminal,
};

/// Returns the lowercase spelling of a diagnostic severity level.
[[nodiscard]] constexpr auto severityName(const Severity severity) noexcept -> std::string_view {
  switch (severity) {
    case Severity::Debug: return "debug";
    case Severity::Note: return "note";
    case Severity::Warning: return "warning";
    case Severity::Error: return "error";
  }
  std::unreachable();
}

} // namespace Miracle
