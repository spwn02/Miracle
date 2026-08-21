import std;
import Miracle;
import Switch;

using namespace Miracle;
using namespace Miracle::fs;
using namespace Switch;

namespace Tests::fs {

[[ = test, = group("core"), = tag("fs") ]] auto fileIsDirectory() -> void {
  const Path path = std::filesystem::current_path();
  constexpr OpenOptions options{
      .create = true,
  };
  const Result<File> res = options.open(path);
  require(not res);
  const Error &err = res.error();
  require(err.display().starts_with("Error: Provided file is a directory"));
}

[[ = test, = group("core"), = tag("fs") ]] auto createFileWithoutWriteMode() -> void {
  const Path path = std::filesystem::current_path() / "tests" / "foo.txt";
  constexpr OpenOptions options{
      .create = true,
  };
  const Result<File> res = options.open(path);
  require(not res);
  const Error &err = res.error();
  require(err.display().starts_with("Error: Attempted to create a file without write/append mode"));
}

[[ = test, = group("core"), = tag("fs") ]] auto fileDoesNotExist() -> void {
  const Path path = std::filesystem::current_path() / "tests" / "bar.txt";
  constexpr OpenOptions options{
      .read = true,
  };
  const Result<File> res = options.open(path);
  require(not res);
  const Error &err = res.error();
  require(err.display().starts_with("Error: Attempted to access a non-existent file "
                                    "without create/create_new mode"));
}

[[ = test, = group("core"), = tag("fs") ]] auto readWriteNotChosen() -> void {
  const Path path = std::filesystem::current_path() / "tests" / "foo.txt";
  constexpr OpenOptions options{};
  const Result<File> res = options.open(path);
  require(not res);
  const Error &err = res.error();
  require(err.display().starts_with("Error: Attempted to open a file without "
                                    "any of read/write/append modes"));
}

[[ = test, = group("core"), = tag("fs") ]] auto truncateFileWithoutWriteMode() -> void {
  const Path path = std::filesystem::current_path() / "tests" / "foo.txt";
  constexpr OpenOptions options{.truncate = true};
  const Result<File> res = options.open(path);
  require(not res);
  const Error &err = res.error();
  require(err.display().starts_with("Error: Attempted to truncate file without write mode"));
}

[[ = test, = trace, = group("core"), = tag("fs") ]] auto fileOperations() -> void {
  const Path path = std::filesystem::current_path() / "tests" / "foo.txt";
  OpenOptions options{
      .create = true,
      .write = true,
  };

  /// Write file
  {
    Result<void> res =
        options.open(path).and_then([](File file) -> Result<void> { return file.write("Hello, World!\n"); });
    require(res);
    traceEvent(std::format("created file: {}", path.string()));
  }

  /// File already exists
  {
    options = {
        .create_new = true,
        .write = true,
    };
    const Result<File> res = options.open(path);
    require(not res);
    const Error &err = res.error();
    require(err.display().starts_with("Error: File already exists"));
  }

  /// Append file
  {
    options = {
        .write = true,
        .append = true,
    };
    Result<void> res = options.open(path).and_then([](File file) -> Result<void> {
      require(file.write("Appended 1!\n"));
      traceEvent("appended 1 ... ");
      require(file.write("Appended 2!\n"));
      traceEvent("appended 2 ... ");
      return {};
    });
    require(res);
    traceEvent(std::format("appended file: {}", path.string()));
  }

  /// Read file
  {
    options = {
        .read = true,
    };
    const Result<String> res =
        options.open(path).and_then([](File file) -> Result<String> { return file.read(); });
    require(res);
    require(*res == R"(Hello, World!
Appended 1!
Appended 2!
)"_exp);
    traceEvent(std::format("read file: {}", path.string()));
  }

  /// Truncate file
  {
    options = {
        .read = true,
        .write = true,
        .truncate = true,
    };
    const Result<String> res =
        options.open(path).and_then([](File file) -> Result<String> { return file.read(); });
    require(res);
    require(res->empty());
    traceEvent(std::format("truncated file: {}", path.string()));
  }

  /// Delete file
  {
    options = {
        .write = true,
    };
    Result<void> res =
        options.open(path).and_then([](File file) -> Result<void> { return file.deleteFile(); });
    require(res);
    traceEvent(std::format("deleted file: {}", path.string()));
  }
}

} // namespace Tests::fs

consteval {
  discover<^^Tests::fs>();
}
