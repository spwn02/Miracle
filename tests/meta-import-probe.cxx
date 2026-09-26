import Miracle.Meta;

struct MetaImportProbe final {
  int value{};
};

static_assert(Miracle::meta::fields<MetaImportProbe>().size() == 1);
static_assert(Miracle::reflect<MetaImportProbe>().raw() == ^^MetaImportProbe);
static_assert(Miracle::meta::requireName(Miracle::meta::fields<MetaImportProbe>().front()).size() == 5);

auto main() -> int {
  return 0;
}
