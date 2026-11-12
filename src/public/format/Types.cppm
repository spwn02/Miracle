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

/// Returns the string representation of a diagnostic severity level.
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
