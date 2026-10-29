export module Miracle:Panic;

import std;
import :Types;
import :Diagnostic;
import :Error;

namespace Miracle {

// Declared in the interface partition so reflection metadata is seriaized for same-module implementation
// units without coupling Diagnostic to Panic.
enum class[[= diagnostics]] PanicDiagnosticCode : u8 {
  Panic = 1,
};

} // namespace Miracle

export namespace Miracle {

/// Controls stacktrace capture for terminal panic reporting.
enum class TraceMode : u8 {
  /// Do not capture a panic-time stacktrace.
  None,
  /// Capture the current stack, excluding the panic helper frame.
  Current,
};

/// Policy controlling panic rendering, trace capture, and debugger breakpoints.
struct PanicOptions final {
  /// Structured diagnostic rendering policy.
  RenderOptions render{};
  /// Stacktrace capture policy.
  TraceMode trace{TraceMode::Current};
  /// Whether to invoke `std::breakpoint_if_debugging()` after rendering.
  bool breakpoint{true};
};

/// Renders an already structured diagnostic and terminates the process.
[[noreturn]] auto panic(Diagnostic diagnostic, PanicOptions options = {}) noexcept -> void;
/// Adapts an Error into a diagnostic, renders it, and terminates the process.
[[noreturn]] auto panic(const Error &error, PanicOptions options = {}) noexcept -> void;
/// Creates a panic diagnostic for a message, renders it, and terminates the process.
[[noreturn]] auto panic(StringView message,
    PanicOptions = {},
    std::source_location location = std::source_location::current()) noexcept -> void;

} // namespace Miracle
