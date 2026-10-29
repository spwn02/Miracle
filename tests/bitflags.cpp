import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Switch;

enum class[[= bitflags]] BufferUsage : u8 {
  None = 0,
  Vertex = 1 << 0,
  Index = 1 << 1,
  Uniform = 1 << 6,
  Storage = 1 << 3,
};

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace Tests::bitflags {

[[ = test, = group("core"), = tag("bitflags") ]] auto bitflags() -> void {
  BufferUsage flags = BufferUsage::Vertex | BufferUsage::Index;
  check(bits(flags) == 0b11_exp);
  flags |= BufferUsage::Storage;
  check(bits(flags) == 0b1011_exp);
  check(has(flags, BufferUsage::Vertex));
}

[[ = test, = group("core"), = tag("bitflags") ]] auto bitflagsDisplay() -> void {
  BufferUsage flags = BufferUsage::Vertex | BufferUsage::Storage;
  check(std::format("{}", flags) == "9"_exp);
  check(std::format("{:b}", flags) == "00001001"_exp);
  check(std::format("{:x}", flags) == "9"_exp);
}

[[ = test, = group("core"), = tag("bitflags") ]] auto bitflagsDisplayWithPrefix() -> void {
  BufferUsage flags = BufferUsage::Vertex | BufferUsage::Storage;
  check(std::format("{:p}", flags) == "9"_exp);
  check(std::format("{:bp}", flags) == "0b00001001"_exp);
  check(std::format("{:xp}", flags) == "0x9"_exp);
}

[[ = test, = group("core"), = tag("bitflags") ]] auto bitflagsAll() -> Expression {
  return bits(all<BufferUsage>()) == 0b1001011_exp;
}

} // namespace Tests::bitflags
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  discover<^^Tests::bitflags>();
}
