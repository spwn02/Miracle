import std;
import Miracle;
import Switch;

using namespace Miracle;

namespace Tests::memoryProfiling {

// TEST-REST-005: the supported-platform snapshot is advisory but present and sane; the optional shape is the
// contract on unsupported platforms.
[[= Switch::test]] auto processMemorySnapshot() -> void {
  const Option<memory::ProcessMemorySnapshot> first = memory::processMemory();
  static_assert(std::same_as<std::remove_cvref_t<decltype(first)>, std::optional<memory::ProcessMemorySnapshot>>);
  Switch::check(first.has_value());
  if (not first)
    return;
  Switch::check(first->residentBytes > 0);

  // A live allocation is visible in the resident set (advisory: only monotone growth of a touched block).
  std::vector<unsigned char> block(32U * 1024U * 1024U, 1);
  const auto second = memory::processMemory();
  Switch::check(second.has_value());
  Switch::check(block.back() == 1);
  if (second)
    Switch::check(second->residentBytes >= first->residentBytes);
}

// TEST-REST-006: scopes record in completion order with deterministic depths and name-keyed aggregates.
[[= Switch::test]] auto profileScopesNestAndAggregate() -> void {
  profiling::ProfileSink sink;
  {
    auto outer = profiling::profileScope(sink, "outer");
    for (int repetition = 0; repetition < 3; ++repetition) {
      auto inner = profiling::profileScope(sink, "inner");
    }
  }
  const profiling::ProfileSnapshot snapshot = sink.snapshot();

  Switch::check(snapshot.events.size() == 4);
  if (snapshot.events.size() != 4)
    return;
  for (usize index{}; index < 3; ++index) {
    Switch::check(snapshot.events[index].name == "inner");
    Switch::check(snapshot.events[index].depth == 1);
    Switch::check(snapshot.events[index].kind == profiling::EventKind::Scope);
  }
  Switch::check(snapshot.events[3].name == "outer");
  Switch::check(snapshot.events[3].depth == 0);

  Switch::check(snapshot.aggregates.size() == 2);
  const auto inner = snapshot.aggregates.find("inner");
  const auto outer = snapshot.aggregates.find("outer");
  Switch::check(inner != snapshot.aggregates.end() and outer != snapshot.aggregates.end());
  if (inner == snapshot.aggregates.end() or outer == snapshot.aggregates.end())
    return;
  Switch::check(inner->second.count == 3);
  Switch::check(outer->second.count == 1);
  Switch::check(inner->second.minimum <= inner->second.maximum);
  Switch::check(inner->second.total >= inner->second.maximum);

  std::chrono::steady_clock::duration sum{};
  for (const auto &event : snapshot.events)
    sum += event.duration;
  Switch::check(snapshot.duration == sum);

  // The snapshot is a stable copy: later scopes do not change it.
  { auto late = profiling::profileScope(sink, "late"); }
  Switch::check(snapshot.events.size() == 4);
  Switch::check(sink.snapshot().events.size() == 5);
}

// A scope left through an early exit still records exactly once.
[[= Switch::test]] auto profileScopeEarlyExit() -> void {
  profiling::ProfileSink sink;
  const auto run = [&sink](const bool exitEarly) -> int {
    auto scope = profiling::profileScope(sink, "work");
    if (exitEarly)
      return 1;
    return 2;
  };
  Switch::check(run(true) == 1);
  Switch::check(run(false) == 2);
  const auto snapshot = sink.snapshot();
  Switch::check(snapshot.events.size() == 2);
  Switch::check(snapshot.events[0].depth == 0 and snapshot.events[1].depth == 0);
}

} // namespace Tests::memoryProfiling

consteval {
  Switch::discover<^^Tests::memoryProfiling>();
}
