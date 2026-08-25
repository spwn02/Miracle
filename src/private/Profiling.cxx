module Miracle;

import std;

namespace Miracle::profiling {

auto ProfileSink::begin() noexcept -> usize {
  return activeScopes_++;
}

auto ProfileSink::record(String name,
    std::chrono::steady_clock::duration duration,
    std::source_location location,
    usize depth) -> void {
  duration_ += duration;
  events_.push_back(ProfileEvent{
      .name = std::move(name),
      .duration = duration,
      .location = location,
      .depth = depth,
      .kind = EventKind::Scope,
  });

  const auto found = aggregates_.find(events_.back().name);
  if (found == aggregates_.end()) {
    aggregates_.emplace(events_.back().name,
        ProfileAggregate{
            .count = 1,
            .total = duration,
            .minimum = duration,
            .maximum = duration,
        });
  } else {
    ProfileAggregate &aggregate = found->second;
    ++aggregate.count;
    aggregate.total += duration;
    aggregate.minimum = std::min(aggregate.minimum, duration);
    aggregate.maximum = std::max(aggregate.maximum, duration);
  }

  if (activeScopes_ != 0)
    --activeScopes_;
}

auto ProfileSink::snapshot() const -> ProfileSnapshot {
  return ProfileSnapshot{
      .duration = duration_,
      .events = events_,
      .aggregates = aggregates_,
  };
}

Scope::Scope(ProfileSink &sink, StringView name, std::source_location location)
    : sink_(std::addressof(sink))
    , name_(name)
    , location_(location)
    , started_(std::chrono::steady_clock::now())
    , depth_(sink_->begin()) {
}

Scope::~Scope() noexcept {
  try {
    sink_->record(std::move(name_), std::chrono::steady_clock::now() - started_, location_, depth_);
  } catch (...) { // NOLINT
    // Profiling is observational. A reporting allocation must never change program behavior.
  }
}

auto profileScope(ProfileSink &sink, StringView name, std::source_location location) -> Scope {
  return Scope{sink, name, location};
}

} // namespace Miracle::profiling
