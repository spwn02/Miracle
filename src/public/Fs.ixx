export module Miracle:Fs;

import std;
import :Types;
import :Error;

export namespace Miracle::fs {

class [[nodiscard]] File {
public:
  [[nodiscard]] explicit File(const Path &path, const std::ios::openmode &openmode)
      : file_(path, openmode)
      , path_(path) {
  }
  ~File() {
    close();
  }

  File(const File &) = delete ("Since this class manages unique file streams, copy is not "
                               "supported; use move instead");
  auto operator=(const File &)
      -> File & = delete ("Since this class manages unique file streams, copy is not "
                          "supported; use move instead");
  File(File &&) noexcept = default;
  auto operator=(File &&) noexcept -> File & = default;

  [[nodiscard]] auto read() const -> Result<String>;
  auto write(StringView buf) -> Result<void>;

  auto close() noexcept -> void;
  auto deleteFile() noexcept -> Result<void>;
  [[nodiscard]] auto ok() const noexcept -> bool;

  [[nodiscard]] explicit operator bool() const {
    return file_.is_open();
  }

private:
  std::fstream file_;
  Path path_;
};

struct OpenOptions {
  bool create{}, create_new{}, read{}, write{}, append{}, truncate{};

  [[nodiscard]]
  auto open(const Path &path) const -> Result<File>;
};

/// Creates a unique directory below the platform's system temporary directory.
[[nodiscard]] auto temporaryDirectory(StringView prefix = "miracle") -> Result<Path>;

/// Returns a unique not-yet-created path below parent for a file with suffix.
[[nodiscard]] auto temporaryPath(const Path &parent, StringView prefix = "file", StringView suffix = ".tmp")
    -> Result<Path>;

/// Creates one named child directory below parent.
[[nodiscard]] auto createDirectory(const Path &parent, StringView name) -> Result<Path>;

} // namespace Miracle::fs
