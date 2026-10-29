export module Miracle:Memory;

import std;
import :Types;

export namespace Miracle::memory {

/// Advisory process-wide resident-memory snapshot.
struct ProcessMemorySnapshot final {
  usize residentBytes{};
};

/// Returns a native process-memory snapshot when the current platform exposes one.
[[nodiscard]] auto processMemory() noexcept -> Option<ProcessMemorySnapshot>;

} // namespace Miracle::memory
