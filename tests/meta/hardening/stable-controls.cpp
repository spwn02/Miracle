import std;
import Miracle.Meta;
using namespace Miracle::meta;
struct Base {};
struct Record : Base {
  int field;
  static int shared;
  Record() = default;
  auto method() -> void;

private:
  int hidden;

public:
  static consteval auto inside() -> bool {
    return std::ranges::equal(
        fields<Record>(), std::meta::nonstatic_data_members_of(^^Record, Miracle::Access::current()));
  }
};
using Alias = Record;
enum class E { First, Second };
auto function(int, double) -> long;
template <class T>
struct Box {};
static_assert(Record::inside());
static_assert(fields<Alias>(Miracle::Access::unchecked()).size() == 2);
static_assert(fields<Record>(Miracle::Access::unprivileged()).size() == 1);
static_assert(staticFields<Record>(Miracle::Access::unchecked()).size() == 1);
static_assert(bases<Record>(Miracle::Access::unchecked()).size() == 1);
static_assert(subobjects<Record>(Miracle::Access::unchecked()).size() == 3);
static_assert(enumerators<E>().size() == 2);
static_assert(parameters(^^function).size() == 2);
static_assert(templateArguments(^^Box<int>).size() == 1);
static_assert(constructors<Record>(Miracle::Access::unchecked()).size() ==
              std::ranges::count_if(std::meta::members_of(^^Record, Miracle::Access::unchecked()),
                  std::meta::is_constructor));
constexpr auto viaRecord = Miracle::Access::unprivileged().via(^^Record);
static_assert(
    std::ranges::equal(fields<Record>(viaRecord), std::meta::nonstatic_data_members_of(^^Record, viaRecord)));
consteval auto invalid() -> bool {
  try {
    (void)fields(^^int);
  } catch (const exception &error) {
    return error.from() == ^^int;
  }
  return false;
}
static_assert(invalid());
auto main() -> int {
}
