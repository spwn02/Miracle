import Miracle;

using namespace Miracle;

// TEST-FMT-STATIC-003: message storage that dies with the constant evaluation cannot escape it. The compiler
// rejects the escaping pointer; Message never extends lifetime or copies into hidden static storage.
consteval auto danglingData() -> const char * {
  const format::Message message{Severity::Error, format::code("X", 1), "short-lived"};
  return message.data();
}
constexpr const char *escaped = danglingData();

auto main() -> int {
  return 0;
}
