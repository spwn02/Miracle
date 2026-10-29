import std;
import Miracle.Meta;

auto unnamedParameter(int) -> void;

consteval {
  constexpr auto parameters = std::define_static_array(std::meta::parameters_of(^^unnamedParameter));
  (void)Miracle::meta::requireName(parameters.front());
}

auto main() -> int {
  return 0;
}
