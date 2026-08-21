import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Switch;

namespace Tests::error {

[[ = test, = group("Core"), = tag("error"), = shouldPanic("Failed") ]] auto fail() -> Result<usize> {
  return bail({"Failed"});
}

[[ = test, = group("Core"), = tag("error") ]] auto errorStoresMessage() -> void {
  Error err{"invalid query"};
  check(err.display() == "Error: invalid query\n"_exp);
}

[[ = test, = group("Core"), = tag("error") ]] auto errorFunction() -> void {
  check(not fail());
}

[[ = test, = group("Core"), = tag("error"), = shouldPanic() ]] auto empty() -> Result<void> {
  return bail({});
}

auto readFile(const Path &path) -> Result<void> {
  return bail({"Failed to open a file: {}", path.string()});
}

auto compileShaders(const String &shader) -> Result<void> {
  return readFile((std::filesystem::current_path() / (shader + ".txt")))
      .transform_error([&](Error err) constexpr noexcept -> Error {
        err << std::format("Failed to compile shader: {}", shader);
        return err;
      });
}

auto initGraphics() -> Result<void> {
  return compileShaders("foo").transform_error([](Error err) constexpr noexcept -> Error {
    err << "Failed to initialize graphics";
    return err;
  });
}

auto start() -> Result<void> {
  return initGraphics().transform_error([](Error err) constexpr noexcept -> Error {
    err << "Failed to start the application";
    return err;
  });
}

[[ = test, = group("core"), = tag("debug") ]] auto stackTrace() -> void {
  Result<void> res = start();
  require(not res);
  const String expected = std::format(R"(Error: Failed to start the application

Caused by:
  0: Failed to initialize graphics
  1: Failed to compile shader: foo
  2: Failed to open a file: {})",
      (std::filesystem::current_path() / "foo.txt").string());
  check(res.error().display() == expected);
}

[[ = test, = group("core"), = tag("debug") ]] auto errorLocation() -> void {
  const auto location = std::source_location::current();
  Error::Message msg{"Message", location};
  check(msg.location.file_name() == location.file_name());
  check(msg.location.line() == location.line());
  check(msg.location.column() == location.column());
}

[[ = test, = trace, = group("core"), = tag("debug") ]] auto releaseError() -> void {
  Error err1{"This value is about to be moved"};
  traceEvent("instantiated the first error");

  Error err2 = err1.release();
  traceEvent("transfered ownership of the first error to the second");

  require(not err1);
  require(err2);

  err1 << "This is a new value!";
  traceEvent("appended first error");
  require(err1);

  require(err1.display() == "Error: This is a new value!\n"_exp);
  require(err2.display() == "Error: This value is about to be moved\n"_exp);
}

} // namespace Tests::error

consteval {
  discover<^^Tests::error>();
}
