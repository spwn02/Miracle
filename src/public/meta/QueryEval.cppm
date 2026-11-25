export module Miracle.Meta:QueryEval;

import std;
import :Types;
import :Cache;
import :Reflect;
import :QueryIR;
import :QueryPlan;
import :QueryState;

namespace Miracle::meta {

template <class State, class Sink>
consteval auto evaluateState(State state, Sink sink) -> bool;

/// Emits a direct span-like source in order, stopping as soon as the downstream sink returns false.
template <class Source, class Sink>
consteval auto emitSource(Source source, Sink &sink) -> bool {
  for (auto &&item : source.values) {
    if (not sink(std::forward<decltype(item)>(item))) {
      return false;
    }
  }
  return true;
}

/// Emits a persistent source after restoring each stored representation to its exact logical category.
///
/// Static promoted backing is necessarily const. Reference-valued queries therefore unwrap their persisted
/// pointers directly, while owning queries reconstruct one fresh logical value for each visit and forward it
/// as an xvalue. This keeps a barrier from silently changing `T` into `const T&` merely because its backing
/// storage is immutable.
template <class Value, class Stored, class Sink>
consteval auto emitSource(StoredSource<Value, Stored> source, Sink &sink) -> bool {
  for (const auto &stored : source.values) {
    const bool keepGoing = withStoredLogical<Value>(
        stored, [&](auto &&value) consteval -> bool { return sink(std::forward<decltype(value)>(value)); });
    if (not keepGoing) {
      return false;
    }
  }
  return true;
}

/// Emits a chained source while distinguishing child-query termination from downstream cancellation.
///
/// `evaluateState()` returns false both when a stateful adaptor such as `take` naturally terminates its child
/// query and when the terminal sink requests cancellation. Only the latter may suppress the right-hand
/// source.
template <class LeftState, class RightState, class Sink>
consteval auto emitSource(ChainSource<LeftState, RightState> source, Sink &sink) -> bool {
  bool downstreamStopped{};
  auto trackedSink = [&](auto &&item) consteval -> bool {
    const bool keepGoing = sink(std::forward<decltype(item)>(item));
    downstreamStopped = not keepGoing;
    return keepGoing;
  };

  static_cast<void>(evaluateState(std::move(source.left), trackedSink));
  if (downstreamStopped) {
    return false;
  }
  return evaluateState(std::move(source.right), sink);
}

/// Emits `zip` using evaluation-local pairing buffers.
///
/// Both child queries are evaluated before pairs are emitted, and the shorter logical sequence sets the
/// result cardinality. The pairing buffers are evaluation-local because they only need to outlive this
/// synchronous loop; promoting them would add static storage without extending any observable lifetime.
template <class LeftState, class RightState, class Sink>
consteval auto emitSource(ZipSource<LeftState, RightState> source, Sink &sink) -> bool {
  using LeftValue = LeftState::value_type;
  using RightValue = RightState::value_type;
  std::vector<StoredT<LeftValue>> left;
  std::vector<StoredT<RightValue>> right;
  evaluateState(std::move(source.left), [&]<class Item>(Item &&item) consteval -> bool {
    left.push_back(storeValueAs<LeftValue>(std::forward<Item>(item)));
    return true;
  });
  evaluateState(std::move(source.right), [&]<class Item>(Item &&item) consteval -> bool {
    right.push_back(storeValueAs<RightValue>(std::forward<Item>(item)));
    return true;
  });

  const auto count = std::min(left.size(), right.size());
  for (std::size_t index{}; index < count; ++index) {
    bool keepGoing{};
    withStoredLogical<LeftValue>(std::move(left[index]), [&](auto &&leftValue) consteval -> void {
      withStoredLogical<RightValue>(std::move(right[index]), [&](auto &&rightValue) consteval -> void {
        auto item = std::pair<LeftValue, RightValue>{
            std::forward<decltype(leftValue)>(leftValue), std::forward<decltype(rightValue)>(rightValue)};
        keepGoing = sink(std::move(item));
      });
    });
    if (not keepGoing) {
      return false;
    }
  }
  return true;
}

/// Two-slot local storage for same-type value transformations inside one compiled segment.
///
/// A `map<T -> T>` must not become a fake typed boundary merely to keep a transformed prvalue alive. The
/// inactive slot is constructed directly from the map prvalue, so guaranteed copy elision preserves immovable
/// values and no additional Query-visible move/copy is introduced. Reference-valued segments do not
/// instantiate this storage at all. Both slots are tracked explicitly so exception unwinding destroys every
/// live object.
template <class T>
  requires(std::is_object_v<T> and not std::is_reference_v<T>)
struct SegmentValueStorage final {
  union Slot {
    /// Inactive union member used before `value` lifetime begins.
    unsigned char inactive;
    /// Storage for one live same-type map result.
    T value;

    /// Constructs an inactive slot without beginning the lifetime of `value`.
    constexpr Slot()
        : inactive{} {
    }

    Slot(const Slot &) = delete;
    auto operator=(const Slot &) -> Slot & = delete;
    Slot(Slot &&) = delete;
    auto operator=(Slot &&) -> Slot & = delete;

    /// Leaves destruction of the conditionally-live `value` member to `SegmentValueStorage`.
    constexpr ~Slot() {
    }
  };

  /// Alternating raw object slots used by same-type value maps.
  std::array<Slot, 2> slots{};
  /// Tracks which union slots currently own a live `T`.
  std::array<bool, 2> alive{};
  /// Active slot index, or `-1` before the first same-type map result.
  int active{-1};

  /// Constructs empty alternating storage with neither value slot active.
  SegmentValueStorage() = default;
  /// Copying raw conditional-lifetime storage is forbidden.
  SegmentValueStorage(const SegmentValueStorage &) = delete;
  /// Copy assignment is forbidden for raw conditional-lifetime storage.
  auto operator=(const SegmentValueStorage &) -> SegmentValueStorage & = delete;
  /// Moving raw conditional-lifetime storage is forbidden.
  SegmentValueStorage(SegmentValueStorage &&) = delete;
  /// Move assignment is forbidden for raw conditional-lifetime storage.
  auto operator=(SegmentValueStorage &&) -> SegmentValueStorage & = delete;

  /// Destroys whichever alternating slots currently own a live value.
  constexpr ~SegmentValueStorage() {
    if (alive.front()) {
      std::destroy_at(std::addressof(slots.front().value));
    }
    if (alive.back()) {
      std::destroy_at(std::addressof(slots.back().value));
    }
  }

  /// Returns the inactive alternating slot into which the next same-type map result must be constructed.
  [[nodiscard]] constexpr auto next() -> T * {
    return std::addressof(slots[active == 0 ? 1 : 0].value);
  }

  /// Returns writable placement-new storage for `value`, including a top-level-const logical type.
  ///
  /// Placement new requires `void*`; the slot is raw storage whose object lifetime has not started yet, so
  /// removing top-level cv here does not mutate an existing const object. This cast is confined to the
  /// lifetime boundary.
  [[nodiscard]] static constexpr auto writableAddress(T *value) -> void * {
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast) -- required only at the raw-storage lifetime
    // boundary.
    return const_cast<void *>( // NOLINT(cppcoreguidelines-pro-type-const-cast)
        static_cast<const volatile void *>(value));
  }

  /// Commits an already-constructed inactive slot and retires the previously owned slot, if any.
  constexpr auto commit(T *value) -> void {
    const auto nextIndex = value == std::addressof(slots[0].value) ? 0 : 1;
    alive[nextIndex] = true;

    const auto previous = active;
    active = nextIndex;
    if (previous >= 0) {
      const auto previousIndex = static_cast<std::size_t>(previous);
      alive.at(previousIndex) = false;
      std::destroy_at(std::addressof(slots.at(previousIndex).value));
    }
  }
};

/// Forwards one current segment object using the segment's exact logical value category.
template <class Logical>
[[nodiscard]] consteval auto forwardSegmentValue(std::remove_reference_t<Logical> &value) -> decltype(auto) {
  if constexpr (std::is_lvalue_reference_v<Logical>) {
    return static_cast<Logical>(value);
  } else {
    return static_cast<std::remove_reference_t<Logical> &&>(value);
  }
}

/// Emits one value through one compiled homogeneous segment.
///
/// Most operations execute directly in one C++26 expansion statement. Same-type owning `map` results use
/// `SegmentValueStorage`, avoiding per-map continuation templates while preserving immovable values. A
/// same-type `filterMap` re-enters this same function specialization with an ordinary suffix index so its
/// Optional-like wrapper stays alive; the suffix is ordinary evaluation state rather than an NTTP, so it does
/// not create per-operation template recursion.
template <Info Program,
    Info Segments,
    std::size_t SegmentIndex,
    class Environment,
    class EvaluationState,
    class Item,
    class Sink>
// Keeping opcode dispatch in this one expansion-statement lowering function is intentional: splitting each
// opcode into helper templates recreates per-operation template topology and regresses the frontend-cost
// invariant measured for the value IR. The body is therefore exempt from generic readability size/complexity
// thresholds only. NOLINTNEXTLINE(readability-function-size,readability-function-cognitive-complexity)
consteval auto emitCompiledSegment(Environment &environment,
    EvaluationState &evaluation,
    Item &&item,
    Sink &sink,
    std::size_t startInstruction = 0,
    bool stopAfterCurrent = false) -> bool {
  constexpr auto segments = programSegments<Segments>();
  static_assert(SegmentIndex < segments.size(), "Miracle Meta Query compiled segment index is out of range");
  constexpr auto segment = segments[SegmentIndex];
  constexpr auto statePlan = compileEvaluationStatePlan<Program>();
  using Logical = [:segment.inputType:];
  using Object = std::remove_reference_t<Logical>;

  using ExpectedItem =
      std::conditional_t<std::is_lvalue_reference_v<Logical>, Logical, std::remove_reference_t<Logical> &&>;
  static_assert(std::same_as<decltype(std::forward<Item>(item)), ExpectedItem>,
      "Miracle Meta Query compiled segment received an object of the wrong exact logical type");

  Object *current = std::addressof(item);
  using Storage =
      std::conditional_t<std::is_reference_v<Logical>, std::monostate, SegmentValueStorage<Object>>;
  Storage storage{};
  std::size_t ordinal{};

  template for (constexpr auto indexed :
      programIndexedInstructionsBetween<segment.bodyBegin, segment.bodyEnd>()) {
    constexpr auto instruction = indexed.instruction;
    constexpr auto stateSlot = statePlan.slots[indexed.ordinal].index;
    const auto currentOrdinal = ordinal++;
    if (currentOrdinal >= startInstruction) {
      if constexpr (instruction.op == QueryOp::Map) {
        if constexpr (std::is_reference_v<Logical>) {
          if constexpr (instruction.operand != Info{}) {
            auto &&mapped = adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
            static_assert(std::same_as<decltype(mapped), Logical>,
                "Miracle Meta Query same-type map changed its exact logical reference type");
            current = std::addressof(mapped);
          } else {
            auto &&mapped =
                adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
            static_assert(std::same_as<decltype(mapped), Logical>,
                "Miracle Meta Query same-type captured map changed its exact logical reference type");
            current = std::addressof(mapped);
          }
        } else {
          auto *target = storage.next();
          if constexpr (instruction.operand != Info{}) {
            using Result =
                decltype(adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
            static_assert(std::same_as<Result, Logical>,
                "Miracle Meta Query same-type map changed its exact logical value type");
            ::new (Storage::writableAddress(target))
                Logical(adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
          } else {
            using Result = decltype(adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
            static_assert(std::same_as<Result, Logical>,
                "Miracle Meta Query same-type captured map changed its exact logical value type");
            ::new (Storage::writableAddress(target)) Logical(adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
          }
          storage.commit(target);
          current = target;
        }
      } else if constexpr (instruction.op == QueryOp::FilterMap) {
        auto result = [&]() consteval -> auto {
          if constexpr (instruction.operand != Info{}) {
            return adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
          } else {
            return adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
          }
        }();
        static_assert(OptionalLike<decltype(result)>, "Query::filterMap() requires an Optional-like result");
        using ResultValue = OptionalResultT<decltype(result)>;
        static_assert(std::same_as<ResultValue, Logical>,
            "Miracle Meta Query same-type filterMap changed its exact logical value type");
        if (not result.has_value()) {
          return not stopAfterCurrent;
        }
        return withOptionalLogical<ResultValue>(result, [&](auto &&engaged) consteval -> bool {
          return emitCompiledSegment<Program, Segments, SegmentIndex>(environment,
              evaluation,
              std::forward<decltype(engaged)>(engaged),
              sink,
              currentOrdinal + 1,
              stopAfterCurrent);
        });
      } else if constexpr (instruction.op == QueryOp::Filter) {
        bool accepted{};
        if constexpr (instruction.operand != Info{}) {
          accepted = static_cast<bool>(
              adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
        } else {
          accepted = static_cast<bool>(
              adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
        }
        if (not accepted) {
          return not stopAfterCurrent;
        }
      } else if constexpr (instruction.op == QueryOp::Inspect) {
        if constexpr (instruction.operand != Info{}) {
          adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
        } else {
          adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
        }
      } else if constexpr (instruction.op == QueryOp::Take) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query Take instruction has no counter-state slot");
        auto &remaining = evaluation.counters[stateSlot];
        if (remaining == 0) {
          return false;
        }
        --remaining;
        stopAfterCurrent = stopAfterCurrent or remaining == 0;
      } else if constexpr (instruction.op == QueryOp::Skip) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query Skip instruction has no counter-state slot");
        auto &remaining = evaluation.counters[stateSlot];
        if (remaining != 0) {
          --remaining;
          return not stopAfterCurrent;
        }
      } else if constexpr (instruction.op == QueryOp::TakeWhile) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query TakeWhile instruction has no flag-state slot");
        auto &active = evaluation.flags[stateSlot];
        if (not active) {
          return false;
        }
        bool accepted{};
        if constexpr (instruction.operand != Info{}) {
          accepted = static_cast<bool>(
              adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
        } else {
          accepted = static_cast<bool>(
              adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
        }
        if (not accepted) {
          active = false;
          return false;
        }
      } else if constexpr (instruction.op == QueryOp::SkipWhile) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query SkipWhile instruction has no flag-state slot");
        auto &skipping = evaluation.flags[stateSlot];
        if (skipping) {
          bool rejected{};
          if constexpr (instruction.operand != Info{}) {
            rejected = not static_cast<bool>(
                adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current)));
          } else {
            rejected = not static_cast<bool>(adaptiveInvoke(
                captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current)));
          }
          if (not rejected) {
            return not stopAfterCurrent;
          }
          skipping = false;
        }
      } else if constexpr (instruction.op == QueryOp::Slice) {
        static_assert(stateSlot != noEvaluationStateSlot,
            "Miracle Meta Query Slice instruction has no counter-state slot");
        auto &skip = evaluation.counters[stateSlot];
        auto &take = evaluation.counters[stateSlot + 1];
        if (skip != 0) {
          --skip;
          return not stopAfterCurrent;
        }
        if (take == 0) {
          return false;
        }
        --take;
        stopAfterCurrent = stopAfterCurrent or take == 0;
      } else {
        static_assert(instruction.op != QueryOp::Flatten and instruction.op != QueryOp::Enumerate,
            "Miracle Meta Query typed-boundary instruction escaped into a homogeneous segment body");
      }
    }
  }

  if constexpr (segment.boundary == Info{}) {
    const bool keepGoing = static_cast<bool>(sink(forwardSegmentValue<Logical>(*current)));
    return keepGoing and not stopAfterCurrent;
  } else {
    constexpr auto boundaryNode = std::meta::extract<ProgramNode>(segment.boundary);
    constexpr auto instruction = boundaryNode.instruction;
    constexpr auto boundaryOrdinal = programInstructionCount<segment.boundary>() - 1;
    constexpr auto stateSlot = statePlan.slots[boundaryOrdinal].index;
    static_assert(SegmentIndex + 1 < segments.size(),
        "Miracle Meta Query boundary must be followed by a typed continuation segment");

    if constexpr (instruction.op == QueryOp::Map) {
      bool keepGoing{};
      if constexpr (instruction.operand != Info{}) {
        auto &&mapped = adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
        keepGoing = emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(mapped)>(mapped), sink);
      } else {
        auto &&mapped =
            adaptiveInvoke(captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
        keepGoing = emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(mapped)>(mapped), sink);
      }
      return keepGoing and not stopAfterCurrent;
    } else if constexpr (instruction.op == QueryOp::FilterMap) {
      auto result = [&]() consteval -> auto {
        if constexpr (instruction.operand != Info{}) {
          return adaptiveInvoke([:instruction.operand:], forwardSegmentValue<Logical>(*current));
        } else {
          return adaptiveInvoke(
              captureAt<instruction.a>(environment), forwardSegmentValue<Logical>(*current));
        }
      }();
      static_assert(OptionalLike<decltype(result)>, "Query::filterMap() requires an Optional-like result");
      if (not result.has_value()) {
        return not stopAfterCurrent;
      }
      using ResultValue = OptionalResultT<decltype(result)>;
      const bool keepGoing = withOptionalLogical<ResultValue>(result, [&](auto &&engaged) consteval -> bool {
        return emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(engaged)>(engaged), sink);
      });
      return keepGoing and not stopAfterCurrent;
    } else if constexpr (instruction.op == QueryOp::Flatten) {
      auto forwardNested = [&](auto &&nested) consteval -> bool {
        return emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
            environment, evaluation, std::forward<decltype(nested)>(nested), sink);
      };

      if constexpr (requires { evaluateQuery(forwardSegmentValue<Logical>(*current), forwardNested); }) {
        bool downstreamStopped{};
        auto trackedForward = [&](auto &&nested) consteval -> bool {
          const bool keepGoing = forwardNested(std::forward<decltype(nested)>(nested));
          downstreamStopped = not keepGoing;
          return keepGoing;
        };
        static_cast<void>(evaluateQuery(forwardSegmentValue<Logical>(*current), trackedForward));
        return not downstreamStopped and not stopAfterCurrent;
      } else if constexpr (std::ranges::input_range<std::remove_cvref_t<Logical>>) {
        using NextLogical = [:segments[SegmentIndex + 1].inputType:];
        for (auto &&nested : forwardSegmentValue<Logical>(*current)) {
          bool keepGoing{};
          if constexpr (std::is_reference_v<NextLogical>) {
            static_assert(std::same_as<decltype(nested), NextLogical>,
                "Miracle Meta Query flatten changed the exact nested reference type");
            keepGoing = forwardNested(nested);
          } else {
            using NextObject = std::remove_reference_t<NextLogical>;
            if constexpr (std::same_as<decltype(std::move(nested)), NextObject &&>) {
              // The owning range itself stays alive for this continuation, so forwarding its element as an
              // xvalue preserves value semantics without manufacturing an intermediate object.
              keepGoing = forwardNested(std::move(nested));
            } else {
              // A const/proxy range element cannot be forwarded as the exact owning logical type. Materialize
              // the one element locally; it lives until the synchronous downstream continuation returns.
              NextObject ownedNested(std::forward<decltype(nested)>(nested));
              keepGoing = forwardNested(std::move(ownedNested));
            }
          }
          if (not keepGoing) {
            return false;
          }
        }
        return not stopAfterCurrent;
      } else {
        static_assert(std::ranges::input_range<std::remove_cvref_t<Logical>>,
            "Query::flatten() requires a Query or an input range value");
      }
    } else if constexpr (instruction.op == QueryOp::Enumerate) {
      static_assert(stateSlot != noEvaluationStateSlot,
          "Miracle Meta Query Enumerate instruction has no counter-state slot");
      using Enumerated = std::pair<std::size_t, Logical>;
      using NextLogical = [:segments[SegmentIndex + 1].inputType:];
      static_assert(std::same_as<Enumerated, NextLogical>,
          "Miracle Meta Query enumerate boundary has the wrong continuation type");
      Enumerated pair{evaluation.counters[stateSlot]++, forwardSegmentValue<Logical>(*current)};
      const bool keepGoing = emitCompiledSegment<Program, Segments, SegmentIndex + 1>(
          environment, evaluation, std::move(pair), sink);
      return keepGoing and not stopAfterCurrent;
    } else {
      static_assert(instruction.op == QueryOp::Map or instruction.op == QueryOp::FilterMap or
                        instruction.op == QueryOp::Flatten or instruction.op == QueryOp::Enumerate,
          "Miracle Meta Query unsupported typed segment boundary");
      return false;
    }
  }
}

/// Static compiled-program entry point used by the value-IR execution bridge.
///
/// `Program` and `Segments` are the only reflected value identities promoted to NTTPs. Source values,
/// non-structural captures, and terminal state remain ordinary function parameters.
template <Info Program, Info Segments, class Source, class Environment, class Sink>
consteval auto evaluateCompiledProgramStatic(Source source, Environment environment, Sink &sink) -> bool {
  static_assert(
      segmentCount<Segments>() != 0, "Miracle Meta Query compiled plan must contain a final segment");
  constexpr auto segments = programSegments<Segments>();
  static_assert(segments.back().bodyEnd == Program,
      "Miracle Meta Query compiled segment plan does not terminate at the supplied program");

  auto evaluation = makeEvaluation<Program>();
  auto emitSink = [&](auto &&item) consteval -> bool {
    return emitCompiledSegment<Program, Segments, 0>(
        environment, evaluation, std::forward<decltype(item)>(item), sink);
  };
  return emitSource(std::move(source), emitSink);
}

/// Reifies one structural program/segment pair once, extracts the specialized evaluator, then crosses back
/// into ordinary C++ values for source, captures, and sink state. Non-structural state remains ordinary C++
/// data and is never routed through `reflect_invoke`.
template <class Source, class Environment, class Sink>
consteval auto evaluateCompiledProgram(Info program,
    Info segments,
    Source source,
    Environment environment,
    Sink &sink) -> bool {
  using SinkType = Sink;
  const Info specialization = std::meta::substitute(^^evaluateCompiledProgramStatic,
      {
          std::meta::reflect_constant(program),
          std::meta::reflect_constant(segments),
          ^^Source,
          ^^Environment,
          ^^SinkType});
  using Evaluator = bool (*)(Source, Environment, SinkType &);
  const auto evaluator = std::meta::extract<Evaluator>(specialization);
  return evaluator(std::move(source), std::move(environment), sink);
}

/// Evaluates one public Query plan by finalizing the currently open segment and invoking the compiled
/// evaluator.
///
/// Evaluation receives state by value: reusable lvalue Queries are copied, while rvalue Queries move their
/// state forward. Every terminal therefore gets fresh evaluation-local primitive state without preventing
/// move-only capture environments from flowing through a consumed Query pipeline.
template <class State, class Sink>
consteval auto evaluateState(State state, Sink sink) -> bool {
  const auto segments = finishSegmentPlan(state.segmentPlan);
  return evaluateCompiledProgram(
      state.program, segments, std::move(state.source), std::move(state.environment), sink);
}

} // namespace Miracle::meta
