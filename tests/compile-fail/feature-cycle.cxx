import Miracle;

using namespace Miracle;

inline constexpr auto cycleA = feature::define<"cycle-a">(feature::dependsOnNamed<"cycle-b">);
inline constexpr auto cycleB = feature::define<"cycle-b">(feature::dependsOnNamed<"cycle-c">);
inline constexpr auto cycleC = feature::define<"cycle-c">(feature::dependsOnNamed<"cycle-a">);
inline constexpr auto features = feature::catalog<cycleA, cycleB, cycleC>;

inline constexpr auto impossible = features.resolve<feature::buildSet<>, cycleA>();
