import Miracle;

using namespace Miracle;

inline constexpr auto first = feature::define<"first">(feature::conflictsWithNamed<"second">);
inline constexpr auto second = feature::define<"second">();
inline constexpr auto features = feature::catalog<first, second>;

inline constexpr auto impossible = features.resolve<feature::buildSet<>, first, second>();
