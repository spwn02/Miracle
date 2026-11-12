import std;
import Miracle;
import Switch;

using namespace Miracle;

constexpr auto testCode = format::code("ITR", 4);
constexpr auto accentStyle =
    format::Style{.foreground = format::TerminalColor::indexed(14), .bold = true, .italic = true};
constexpr auto testMessage =
    format::message(Severity::Warning, testCode, "callable cannot accept sequence element ")
        .context(format::fragment("Iter::map", accentStyle))
        .field("callable", "MoveOnlyCallable")
        .field("element", format::fragment("std::tuple<int, float>", accentStyle))
        .note("direct call, tuple expansion and reflected aggregate failed")
        .help("define a callable matching one supported invocation shape\n");
static_assert(testMessage.length + 1 == testMessage.capacity());
static_assert(testMessage.styleCount == testMessage.styleStorage.size());
// static_assert(false, testMessage);

constexpr auto msg = format::Message{"Hello "}.append(format::styled("world", accentStyle));
static_assert(msg.view() == "Hello world");
static_assert(msg.length + 1 == msg.capacity());
static_assert(msg.styleCount == msg.styleStorage.size());

constexpr auto msg2 = format::Message{Severity::Error, format::code<3>("STR", 1), "invalid character type"}
                          .context("BasicStaticString")
                          .field("actual", format::fragment("UnstableChar"))
                          .note("the character type is not standard-layout")
                          .help("use a trivially-copyable, standard-layout character type");
static_assert(msg2.length + 3 == msg2.capacity()); // +2 for enum
static_assert(msg2.styleCount == msg2.styleStorage.size());
// static_assert(false, msg2);

namespace Tests::format {
[[= Switch::test]] auto render() -> void {
  auto text =
      Miracle::format::message(Severity::Error, Miracle::format::code("STR", 1), "invalid character type")
          .context("BasicStaticString");
  Switch::check(
      Miracle::format::render(text) == "\nerror[STR001]: invalid character type\n  --> BasicStaticString");
}

} // namespace Tests::format

consteval {
  Switch::discover<^^Tests::format>();
}
