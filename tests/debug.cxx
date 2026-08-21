import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Switch;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)
namespace Tests::debug {

struct[[= debug::derive]] Vec2 {
  f32 x{}, y{};
};

struct[[= debug::derive]] A {
  f32 x{};
};

struct[[= debug::derive]] Player {
  Vec2 pos{}, vel{};
  A a{};
  f32 accelaration{};
};

struct[[= debug::derive]] Group {
  Player p1{}, p2{};
};

static constexpr f32 posX = 10;
static constexpr f32 posY = 20;
static constexpr f32 acceleration = 45;

[[ = test, = trace, = group("core"), = tag("debug") ]] auto debug() -> void {
  traceEvent("formatting Vec2 ...");

  Vec2 vec2{
      .x = posX,
      .y = posY,
  };
  check(std::format("{}", vec2) == "Vec2 { x: 10, y: 20 }"_exp);
  check(std::format("{:#}", vec2) == R"(Vec2 {
  x: 10,
  y: 20,
})"_exp);

  traceEvent("formatting Group ...");

  Group group{};
  group.p1.accelaration = acceleration;
  group.p2.pos = {.x = posX * 2, .y = posY * 2};
  check(std::format("{}", group) == "Group { p1: Player { pos: Vec2 { x: 0, y: 0 }, vel: Vec2 { x: "
                                    "0, y: 0 }, a: A { x: 0 }, accelaration: 45 }, p2: Player { "
                                    "pos: Vec2 { x: 20, y: 40 }, vel: Vec2 { x: 0, y: 0 }, a: A { "
                                    "x: 0 }, accelaration: 0 } }"_exp);
  check(std::format("{:#}", group) == R"(Group {
  p1: Player {
    pos: Vec2 {
      x: 0,
      y: 0,
    },
    vel: Vec2 {
      x: 0,
      y: 0,
    },
    a: A {
      x: 0,
    },
    accelaration: 45,
  },
  p2: Player {
    pos: Vec2 {
      x: 20,
      y: 40,
    },
    vel: Vec2 {
      x: 0,
      y: 0,
    },
    a: A {
      x: 0,
    },
    accelaration: 0,
  },
})"_exp);
}

struct[[= debug::derive]] Vec3 {
  f32 x{}, y{}, z{};
  [[= debug::hide]] f32 acc{};
};

[[ = test, = group("core"), = tag("debug") ]] auto structsWithHiddenMembers() -> void {
  check(std::format("{}", Vec3{}) == R"(Vec3 { x: 0, y: 0, z: 0 })"_exp);
}

struct[[= debug::derive]] DataChunk {
  u32 fourcc{};
  u32 version{};
};

constexpr inline u32 signature = 0x56494430; // VID0

struct[[= debug::derive]] VideoChunk : DataChunk {
  [[= debug::rename("signature")]] u32 fourcc{signature};
  [[= debug::hide]] u32 version{1};
};

[[ = test, = group("core"), = tag("debug") ]] auto structWithRenamedMembers() -> void {
  check(std::format("{}", VideoChunk{}) == "VideoChunk { signature: 1447642160 }"_exp);
}

struct[[= debug::derive]] Empty {};

[[ = test, = group("core"), = tag("debug") ]] auto emptyStruct() -> void {
  check(std::format("{}", Empty{}) == "Empty"_exp);
}

struct[[= debug::derive]] EmptyNested {
  u32 a{}, b{}, c{};
  Empty empty;
};

[[ = test, = group("core"), = tag("debug") ]] auto emptyNestedStruct() -> void {
  check(std::format("{}", EmptyNested{}) == "EmptyNested { a: 0, b: 0, c: 0, empty: Empty }"_exp);
}

struct[[= debug::derive]] AllHidden {
  [[= debug::hide]] u32 a{}, b{};
  [[= debug::hide]] bool c{};
};

[[ = test, = group("core"), = tag("debug") ]] auto allHiddenStructs() -> void {
  /// Same as Empty
  check(std::format("{}", AllHidden{}) == "AllHidden"_exp);
}

enum class[[= debug::derive]] Color : u8 {
  Red = 1,
  Green = 2,
  Blue = 3,
};

enum class[[= debug::derive]] AliasedColor : u8 {
  Red = 1,
  Crimson = 1,
};

[[ = test, = group("core"), = tag("debug") ]] auto enumDisplay() -> void {
  check(debug::enumName(Color::Red) == "Red"_exp);
  check(debug::enumName(Color::Green) == "Green"_exp);
  check(debug::enumName(Color::Blue) == "Blue"_exp);
  check(debug::enumName(static_cast<Color>(69)) == "<unnamed>"_exp);
  check(std::format("{}", Color::Red) == "Red"_exp);
  check(std::format("{}", Color::Green) == "Green"_exp);
  check(std::format("{}", Color::Blue) == "Blue"_exp);
  check(std::format("{}", static_cast<Color>(69)) == "<unnamed>"_exp);
}

[[ = test, = group("core"), = tag("debug") ]] auto enumAliasesUseFirstVisibleName() -> void {
  check(debug::enumName(AliasedColor::Red) == "Red"_exp);
  check(debug::enumName(AliasedColor::Crimson) == "Red"_exp);
  check(std::format("{}", AliasedColor::Crimson) == "Red"_exp);
}

[[ = test, = group("core"), = tag("debug") ]] auto renamedEnum() -> void {
  enum class[[= debug::derive]] Status : u8 {
    Failed[[= debug::rename("failed")]] = 1,
    Skipped[[= debug::rename("skipped")]],
    Success[[= debug::rename("success")]],
  };

  check(std::format("{}", Status::Failed) == "failed"_exp);
  check(std::format("{}", Status::Skipped) == "skipped"_exp);
  check(std::format("{}", Status::Success) == "success"_exp);
  check(debug::enumName(Status::Failed) == "failed"_exp);
  check(debug::enumName(Status::Skipped) == "skipped"_exp);
  check(debug::enumName(Status::Success) == "success"_exp);
}

[[ = test, = group("core"), = tag("debug") ]] auto hiddenEnum() -> void {
  enum class[[= debug::derive]] HiddenEnum : u8 {
    Default = 1,
    Hidden[[= debug::hide]],
    Work,
  };

  check(std::format("{}", HiddenEnum::Hidden) == "<unnamed>"_exp);
}

[[ = test, = group("core"), = tag("debug") ]] auto hiddenEnumWithDefaultEnumerator() -> void {
  enum class[[= debug::derive]] HiddenEnum : u8 {
    Default[[ = debug::prefer, = debug::rename("default") ]] = 1,
    Hidden[[= debug::hide]],
    Work,
  };

  check(std::format("{}", HiddenEnum::Hidden) == "default"_exp);
  check(debug::enumName(HiddenEnum::Hidden) == "default"_exp);
}

} // namespace Tests::debug
// NOLINTEND(cppcoreguidelines-avoid-magic-numbers, readability-magic-numbers)

consteval {
  discover<^^Tests::debug>();
}
