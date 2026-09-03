import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Switch;

namespace {

template <BasicStaticString Name>
struct Named final {
  static constexpr auto name = Name;
};

consteval auto compileTimeMessage() {
  return staticFormat<"member {} failed with code {}">(BasicStaticString{"position"}, -7);
}

static_assert(Named<"miracle">::name == BasicStaticString{"miracle"});
static_assert(compileTimeMessage() == BasicStaticString{"member position failed with code -7"});
static_assert(true, compileTimeMessage());

template <class T>
concept StaticFormatArg = requires(T value) { staticFormat<"{}">(value); };

template <class T>
concept RawPointerOrderable = requires(T value, const char *pointer) { value <=> pointer; };

static_assert(not StaticFormatArg<Vec<int>>);
static_assert(not RawPointerOrderable<StaticString<8>>);

} // namespace

namespace Tests::staticString {

[[ = test, = group("foundation"), = tag("static-string") ]] auto construction() -> void {
  constexpr auto value = BasicStaticString{"Miracle"};
  static_assert(value.size() == 7);
  static_assert(value.capacity() == 7);
  static_assert(not value.empty());
  static_assert(value.front() == 'M');
  static_assert(value.back() == 'e');
  static_assert(value[2] == 'r');
  static_assert(value.data()[value.size()] == '\0');

  constexpr StaticString<16> reserved{"Miracle"};
  static_assert(reserved.size() == 7);
  static_assert(reserved.capacity() == 16);
  static_assert(reserved == value);

  const StringView view = value;
  const char *text = value;
  const String owned = static_cast<String>(value);

  check(view == "Miracle"_exp);
  check(StringView{text} == "Miracle"_exp);
  check(owned == "Miracle"_exp);
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto iterationAndComparison() -> void {
  constexpr auto left = BasicStaticString{"abc"};
  constexpr StaticString<8> same{"abc"};
  constexpr auto greater = BasicStaticString{"abd"};

  static_assert(left == same);
  static_assert(left == "abc");
  static_assert("abc" == left);
  static_assert(left != "abd");
  static_assert(left != greater);
  static_assert((left <=> greater) < 0);
  static_assert((left <=> std::string_view{"abd"}) < 0);
  static_assert((std::string_view{"abb"} <=> left) < 0);
  static_assert(left.compare("abc") == 0);
  static_assert(left.compare("abb") > 0);
  static_assert(std::ranges::equal(left, std::string_view{"abc"}));
  static_assert(*left.rbegin() == 'c');
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto searching() -> void {
  constexpr auto value = BasicStaticString{"alpha beta alpha"};

  static_assert(value.startsWith("alpha"));
  static_assert(value.startsWith('a'));
  static_assert(value.endsWith("alpha"));
  static_assert(value.endsWith('a'));
  static_assert(value.contains("beta"));
  static_assert(value.contains('b'));
  static_assert(value.find("alpha") == 0);
  static_assert(value.find("alpha", 1) == 11);
  static_assert(value.rfind("alpha") == 11);
  static_assert(value.find("missing") == decltype(value)::npos);
  static_assert(value.rfind('z') == decltype(value)::npos);
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto substrings() -> void {
  constexpr auto value = BasicStaticString{"0123456789"};
  constexpr auto compileTime = value.substr<2, 4>();
  constexpr auto tail = value.substr<6>();

  static_assert(compileTime == BasicStaticString{"2345"});
  static_assert(tail == BasicStaticString{"6789"});
  static_assert(value.substr(3, 3) == "345");
  static_assert(value.substr(99).empty());
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto trimmingAndCase() -> void {
  constexpr auto value = BasicStaticString{" \tHello World\n "};

  static_assert(value.trim() == BasicStaticString{"Hello World"});
  static_assert(value.trimStart() == BasicStaticString{"Hello World\n "});
  static_assert(value.trimEnd() == BasicStaticString{" \tHello World"});
  static_assert(value.trim().lower() == BasicStaticString{"hello world"});
  static_assert(value.trim().upper() == BasicStaticString{"HELLO WORLD"});

  constexpr auto marked = BasicStaticString{"___value___"};
  constexpr auto underscore = [](char character) { return character == '_'; };
  static_assert(marked.trimBy(underscore) == BasicStaticString{"value"});
  static_assert(marked.trimStartBy(underscore) == BasicStaticString{"value___"});
  static_assert(marked.trimEndBy(underscore) == BasicStaticString{"___value"});
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto replacementAndRemoval() -> void {
  constexpr auto value = BasicStaticString{"alpha--beta--gamma"};

  static_assert(value.replace('a', 'A') == BasicStaticString{"AlphA--betA--gAmmA"});
  static_assert(value.replace(BasicStaticString{"--"}, BasicStaticString{" / "}) ==
                BasicStaticString{"alpha / beta / gamma"});
  static_assert(value.remove('-') == BasicStaticString{"alphabetagamma"});
  static_assert(value.remove(BasicStaticString{"--"}) == BasicStaticString{"alphabetagamma"});
  static_assert(value.replace(BasicStaticString{""}, BasicStaticString{"x"}) == value);
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto concatenateJoinSplit() -> void {
  constexpr auto concatenated = BasicStaticString{"Miracle"} + BasicStaticString{".Meta"};
  constexpr auto joined = join(BasicStaticString{"::"},
      BasicStaticString{"Miracle"},
      BasicStaticString{"Meta"},
      BasicStaticString{"Trait"});
  constexpr auto parts = BasicStaticString{"one,two,,four"}.split(',');
  constexpr auto words = BasicStaticString{"one--two--three"}.split(BasicStaticString{"--"});

  static_assert(concatenated == BasicStaticString{"Miracle.Meta"});
  static_assert(joined == BasicStaticString{"Miracle::Meta::Trait"});
  static_assert(parts.size() == 4);
  static_assert(parts[0] == BasicStaticString{"one"});
  static_assert(parts[1] == BasicStaticString{"two"});
  static_assert(parts[2].empty());
  static_assert(parts[3] == BasicStaticString{"four"});
  static_assert(words.size() == 3);
  static_assert(words[1] == BasicStaticString{"two"});
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto formatting() -> void {
  enum class Code : i32 { Failed = -12 };

  constexpr auto message =
      staticFormat<"{}: {} / {} / {{ok}}">(BasicStaticString{"result"}, 42, Code::Failed);
  constexpr auto literal = staticFormat<"{} {} {}">("text", true, 'x');
  constexpr auto limits =
      staticFormat<"{} {}">(std::numeric_limits<i64>::min(), std::numeric_limits<u64>::max());
  static_assert(message == BasicStaticString{"result: 42 / -12 / {ok}"});
  static_assert(literal == BasicStaticString{"text true x"});
  static_assert(limits == BasicStaticString{"-9223372036854775808 18446744073709551615"});

  check(std::format("{}", BasicStaticString{"Miracle"}) == "Miracle"_exp);
}

[[ = test, = group("foundation"), = tag("static-string") ]] auto hashing() -> void {
  constexpr auto first = BasicStaticString{"miracle"};
  constexpr StaticString<32> same{"miracle"};
  constexpr auto other = BasicStaticString{"Miracle"};

  static_assert(first.hash() == same.hash());
  static_assert(first.hash() != other.hash());
}

[[ = test, = group("foundation"), = tag("capabilities") ]] auto capabilityFacts() -> void {
  static_assert(capability::importStd);
  static_assert(capability::reflectionCore);
  static_assert(capability::reflectionQueries);
  static_assert(capability::reflectionAnnotations);
  static_assert(capability::reflectionStaticStorage);
  static_assert(capability::expansionStatements);
  static_assert(capability::stdVocabulary);
  static_assert(capability::stdHive);
}

} // namespace Tests::staticString

consteval {
  discover<^^Tests::staticString>();
}
