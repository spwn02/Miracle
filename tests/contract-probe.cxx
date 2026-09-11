import std;
import Miracle;

using namespace Miracle;

[[nodiscard]] auto codeMatchesKind(const DiagnosticCodeDescriptor &code, std::contracts::assertion_kind kind)
    -> bool {
  using Kind = std::contracts::assertion_kind;
  switch (kind) {
    case Kind::pre: return code.enumerator == "Precondition" and code.numeric == 1;
    case Kind::post: return code.enumerator == "Postcondition" and code.numeric == 2;
    case Kind::assert:
    case Kind::manual:
    case Kind::cassert:
    case Kind::__unknown: return code.enumerator == "Assertion" and code.numeric == 3;
  }
  return false;
}

// The Contracts customization point has a standardized snake_case spelling.
// NOLINTNEXTLINE(readability-identifier-naming)
void handle_contract_violation(const std::contracts::contract_violation &violation) {
  const Diagnostic diagnostic = diagnose(violation);
  const bool mapped =
      diagnostic.code().domain == "ContractCode" and diagnostic.code().prefix == "CTR" and
      diagnostic.code().alignment == 3 and codeMatchesKind(diagnostic.code(), violation.kind()) and
      diagnostic.messageText() == violation.comment() and diagnostic.location().line() != 0 and
      diagnostic.notes().size() == 3 and diagnostic.notes().at(0) == "semantic: enforce" and
      diagnostic.notes().at(1) == "detection: predicate_false" and
      diagnostic.notes().at(2) == "terminating: true";

  const String text = render(diagnostic,
      {.color = ColorMode::Never,
          .source = SourceMode::Location,
          .detail = DetailMode::Compact,
          .presentation = Presentation::Plain});
  std::cerr << text;
  std::cerr.flush();
  constexpr int successExit = 42;
  constexpr int failureExit = 43;
  std::_Exit(mapped ? successExit : failureExit);
}

[[nodiscard]] auto parseMode(int argc, char **argv) -> int {
  if (argc <= 1) {
    return 0;
  }

  // The C main ABI exposes argv as a pointer array; this single indexed read is
  // the bounded access established by argc above.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  const StringView argument{argv[1]};
  int mode{};
  const auto result = std::from_chars(argument.begin(), argument.end(), mode);
  if (std::make_error_code(result.ec) or result.ptr != argument.end()) {
    return -1;
  }
  return mode;
}

[[nodiscard]] auto precondition(int value) -> int pre(value > 0) {
  return value;
}

[[nodiscard]] auto postcondition(int value) -> int post(result : result > 0) {
  return value;
}

auto assertion(int value) -> void {
  contract_assert(value > 0);
}

auto main(int argc, char **argv) -> int {
  switch (parseMode(argc, argv)) {
    case 0: return precondition(0);
    case 1: return postcondition(0);
    case 2: assertion(0); return 0;
    default: return 2;
  }
}
