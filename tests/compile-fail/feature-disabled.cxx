import Miracle;

using namespace Miracle;

inline constexpr auto tracing = feature::define<"tracing">();
inline constexpr auto features = feature::catalog<tracing>;
inline constexpr auto disabled = features.resolve<feature::buildSet<>>();

consteval {
  feature::requireEnabled<tracing, disabled>();
}
