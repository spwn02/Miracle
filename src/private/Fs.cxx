module Miracle;

import std;
import :Fs;

namespace Miracle::fs {

auto File::read() const -> Result<String> {
  if (not ok())
    return bail({
        "Failed to open a file: {}",
        path_.string(),
    });

  std::stringstream buf;

  buf << file_.rdbuf();
  return buf.str();
}

auto File::write(StringView buf) -> Result<void> {
  if (not ok())
    return bail({
        "Failed to open a file: {}",
        path_.string(),
    });

  file_ << buf;

  return {};
}

auto File::close() noexcept -> void {
  if (ok())
    file_.close();
}

auto File::deleteFile() noexcept -> Result<void> {
  if (ok())
    file_.close();

  std::error_code error;
  std::filesystem::remove(path_, error);
  if (error)
    return bail({
        "Failed to delete file: {}: {}",
        path_.string(),
        error.message(),
    });

  return {};
}

auto File::ok() const noexcept -> bool {
  return file_.is_open();
}

auto OpenOptions::open(const Path &path) const -> Result<File> {
  if (std::filesystem::is_directory(path)) {
    return bail({
        "Provided file is a directory: {}",
        path.string(),
    });
  }

  using ios = std::ios;
  ios::openmode mode{};

  auto createCheck = [&] -> Result<void> {
    if (not(mode & ios::out))
      return bail({
          "Attempted to create a file without write/append mode: {}",
          path.string(),
      });

    std::error_code err;
    std::filesystem::create_directories(path.parent_path(), err);
    if (err) {
      String msg = err.message();
      return bail({
          "Failed to create parent directory: {}",
          msg,
      });
    }

    return {};
  };

  if (read)
    mode |= ios::in;
  if (write)
    mode |= ios::out;
  if (append)
    mode |= ios::app | ios::out;
  if (truncate) {
    if ((mode & ios::out) == 0)
      return bail({
          "Attempted to truncate file without write mode: {}",
          path.string(),
      });
    mode |= ios::trunc;
  }
  if (create) {
    Result<void> res = createCheck();
    if (not res)
      return bail(std::move(res.error()));
  } else if (create_new) {
    if (std::filesystem::exists(path))
      return bail({"File already exists: {}", path.string()});

    Result<void> res = createCheck();
    if (not res)
      return bail(std::move(res.error()));

    mode |= ios::trunc;
  }

  if ((mode & (ios::in | ios::out)) == 0)
    return bail({
        "Attempted to open a file without any of read/write/append modes: {}",
        path.string(),
    });

  if (not(create or create_new) and not std::filesystem::exists(path))
    return bail({
        "Attempted to access a non-existent file without create/create_new "
        "mode: {}",
        path.string(),
    });

  File file{path, mode};
  if (!file)
    return bail({"Failed to open a file: {}", path.string()});

  return file;
}

auto temporaryDirectory(StringView prefix) -> Result<Path> {
  std::error_code error{};
  const Path temporary = std::filesystem::temp_directory_path(error);
  if (error)
    return bail({"Failed to locate the system temporary directory: {}", error.message()});

  const u64 stamp = static_cast<u64>(std::chrono::steady_clock::now().time_since_epoch().count());
  const u64 thread = static_cast<u64>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
  const auto candidates = std::views::indices(128) | std::views::transform([&](usize index) -> Path {
    return temporary / std::format("{}-{}-{}-{}", prefix, thread, stamp, index);
  });
  const auto found = std::ranges::find_if(candidates, [](const Path &candidate) -> bool {
    std::error_code createError{};
    return std::filesystem::create_directories(candidate, createError) and not createError;
  });
  if (found == candidates.end())
    return bail({"Failed to create a unique temporary directory with prefix: {}", prefix});

  return *found;
}

auto temporaryPath(const Path &parent, StringView prefix, StringView suffix) -> Result<Path> {
  const u64 stamp = static_cast<u64>(std::chrono::steady_clock::now().time_since_epoch().count());
  const u64 thread = static_cast<u64>(std::hash<std::thread::id>{}(std::this_thread::get_id()));
  const auto candidates = std::views::indices(128) | std::views::transform([&](usize index) -> Path {
    return parent / std::format("{}-{}-{}-{}{}", prefix, thread, stamp, index, suffix);
  });
  const auto found = std::ranges::find_if(candidates, [](const Path &candidate) -> bool {
    std::error_code existsError{};
    return not std::filesystem::exists(candidate, existsError) and not existsError;
  });
  if (found == candidates.end())
    return bail({"Failed to create a unique temporary path with prefix: {}", prefix});

  return *found;
}

auto createDirectory(const Path &parent, StringView name) -> Result<Path> {
  Path path = parent / String{name};
  std::error_code error{};
  if (not std::filesystem::create_directory(path, error) or error)
    return bail({"Failed to create directory {}: {}", path.string(), error.message()});

  return path;
}

} // namespace Miracle::fs
