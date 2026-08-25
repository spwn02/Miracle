module;

#include <unistd.h>

module Miracle;

import std;

namespace Miracle::memory {

auto processMemory() noexcept -> Option<ProcessMemorySnapshot> {
  std::ifstream statistics{"/proc/self/statm"};
  u64 totalPages{};
  u64 residentPages{};
  if (not(statistics >> totalPages >> residentPages))
    return None;
  static_cast<void>(totalPages);

  const isize pageSize = static_cast<isize>(::sysconf(_SC_PAGESIZE));
  if (pageSize <= 0)
    return None;

  return ProcessMemorySnapshot{
      .residentBytes = static_cast<usize>(residentPages) * static_cast<usize>(pageSize),
  };
}

} // namespace Miracle::memory
