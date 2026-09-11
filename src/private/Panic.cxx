module Miracle;

import std;
import :Types;
import :Diagnostic;
import :Error;
import :Panic;

namespace Miracle {

namespace {

auto writePanicFallback(StringView fallback) noexcept -> void {
  try {
    std::ostream output{std::cerr.rdbuf()};
    constexpr StringView prefix{"panic: "};
    output.write(prefix.data(), static_cast<std::streamsize>(prefix.size()));
    output.write(fallback.data(), static_cast<std::streamsize>(fallback.size()));
    output.put('\n');
    output.flush();
  } catch (...) {
    // Best effort only; leave the process stream in a non-throwing state before finishPanic performs the
    // debugger/terminate tail.
    std::cerr.exceptions(std::ios::goodbit);
    std::cerr.clear();
  }
}

[[noreturn]] auto finishPanic(Diagnostic diagnostic, PanicOptions options) noexcept -> void {
  try {
    if (options.trace == TraceMode::Current) {
      diagnostic = std::move(diagnostic).trace(std::stacktrace::current(1));
    }
    render(diagnostic, std::cerr, options.render);
    std::cerr.flush();
  } catch (...) {
    // The panic path must remain terminal even when stacktrace capture or structured rendering fails. The
    // fallback disables iostream exceptions and avoid dynamic formatting before making a final best-effort
    // write.
    writePanicFallback(diagnostic.messageText());
  }

  if (options.breakpoint) {
    std::breakpoint_if_debugging();
  }
  std::terminate();
}

} // namespace

[[noreturn]] auto panic(Diagnostic diagnostic, PanicOptions options) noexcept -> void {
  finishPanic(std::move(diagnostic), options);
}

// The locked panic API is noexcept and terminal. Error adaptation may allocate; allocation failure therefore
// terminates by noexcept before structured output. NOLINTNEXTLINE(bugprone-exception-escape)
[[noreturn]] auto panic(const Error &error, PanicOptions options) noexcept -> void {
  finishPanic(diagnose(error), options);
}

// The locked panic API is noexcept and terminal. Constructing its owning diagnostic may allocate; allocation
// failure intentionally terminates.
// NOLINTNEXTLINE(bugprone-exception-escape)
[[noreturn]] auto panic(StringView message, PanicOptions options, std::source_location location) noexcept
    -> void {
  auto diagnostic = Diagnostic::error(PanicDiagnosticCode::Panic, location).message(String{message});
  finishPanic(std::move(diagnostic), options);
}

} // namespace Miracle
