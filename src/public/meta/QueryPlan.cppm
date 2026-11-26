export module Miracle.Meta:QueryPlan;

import std;
import :Types;
import :Cache;
import :Reflect;
import :QueryIR;

namespace Miracle::meta {

/// One compiled same-type segment plus its optional typed continuation boundary.
///
/// The body is represented as the persistent half-open program range `(bodyBegin, bodyEnd]`. `boundary` is
/// empty for the terminal segment or names the single instruction that changes the logical item type before
/// evaluation continues in the next segment. Type reflections are kept as data rather than template
/// parameters.
struct SegmentNode final {
  /// Reflected identity of the previously closed segment in the persistent chain.
  Info previous{};
  /// Program prefix immediately before the first homogeneous body instruction.
  Info bodyBegin{};
  /// Program prefix ending with the last homogeneous body instruction.
  Info bodyEnd{};
  /// Optional typed-boundary instruction following the homogeneous body.
  Info boundary{};
  /// Exact logical type accepted by the homogeneous body.
  Info inputType{};
  /// Exact logical type produced after the optional typed boundary.
  Info outputType{};

  /// Compares the complete structural segment identity used for reflected-plan interning.
  constexpr auto operator==(const SegmentNode &) const -> bool = default;
};

/// Mutable-by-value planning cursor used only while compiling immutable Program IR into typed segments.
///
/// `segments` contains already-closed segments. The remaining fields describe the currently open same-type
/// body. Every update returns a new structural value; no persistent traversal state leaks into Query
/// evaluation.
struct SegmentPlan final {
  /// Persistent chain of segments already closed by typed boundaries.
  Info segments{};
  /// Program prefix immediately before the currently open homogeneous body.
  Info bodyBegin{};
  /// Program prefix ending at the currently open homogeneous body.
  Info bodyEnd{};
  /// Exact logical type flowing through the open body.
  Info valueType{};

  /// Compares all structural planning state used while extending an immutable Query plan.
  constexpr auto operator==(const SegmentPlan &) const -> bool = default;
};

/// Starts typed planning at one source logical type and one already-built program prefix.
[[nodiscard]] consteval auto beginSegmentPlan(Info program, Info valueType) -> SegmentPlan {
  return SegmentPlan{.bodyBegin = program, .bodyEnd = program, .valueType = valueType};
}

/// Appends one closed segment to the persistent segment chain.
[[nodiscard]] consteval auto appendSegment(Info previous,
    Info bodyBegin,
    Info bodyEnd,
    Info boundary,
    Info inputType,
    Info outputType) -> Info {
  return std::meta::reflect_constant(SegmentNode{.previous = previous,
      .bodyBegin = bodyBegin,
      .bodyEnd = bodyEnd,
      .boundary = boundary,
      .inputType = inputType,
      .outputType = outputType});
}

/// Returns whether one IR operation may legitimately change the logical item type.
[[nodiscard]] consteval auto permitsTypedBoundary(QueryOp operation) -> bool {
  return operation == QueryOp::Map or operation == QueryOp::FilterMap or operation == QueryOp::Flatten or
         operation == QueryOp::Enumerate;
}

/// Advances typed planning after one normalized IR append.
///
/// Same-type operations merely extend the open body. A genuine logical type transition closes that body with
/// the new instruction as its typed boundary, then starts a fresh body after the boundary. Comparing
/// reflected exact logical types means a `map<T -> T>` or `filterMap<T -> T>` remains inside the homogeneous
/// segment while reference/cv changes correctly form a boundary.
[[nodiscard]] consteval auto advanceSegmentPlan(SegmentPlan plan,
    Info programBefore,
    Info programAfter,
    Info outputType) -> SegmentPlan {
  if (plan.bodyEnd != programBefore) {
    throw std::meta::exception{
        "Miracle Meta Query typed-segment plan is not aligned with the normalized program", programBefore};
  }

  // A normalized no-op such as Skip(0) adds no instruction and therefore cannot create a segment boundary.
  if (programAfter == programBefore) {
    return plan;
  }

  if (plan.valueType == outputType) {
    plan.bodyEnd = programAfter;
    return plan;
  }

  const auto boundaryNode = std::meta::extract<ProgramNode>(programAfter);
  if (not permitsTypedBoundary(boundaryNode.instruction.op)) {
    throw std::meta::exception{
        "Miracle Meta Query non-transforming instruction changed the logical item type", programAfter};
  }

  plan.segments =
      appendSegment(plan.segments, plan.bodyBegin, programBefore, programAfter, plan.valueType, outputType);
  plan.bodyBegin = programAfter;
  plan.bodyEnd = programAfter;
  plan.valueType = outputType;
  return plan;
}

/// Finalizes the current homogeneous body, including an intentionally empty continuation body after a
/// terminal typed boundary. Keeping that final segment explicit gives the evaluator one uniform sink
/// continuation shape.
[[nodiscard]] consteval auto finishSegmentPlan(SegmentPlan plan) -> Info {
  return appendSegment(plan.segments, plan.bodyBegin, plan.bodyEnd, Info{}, plan.valueType, plan.valueType);
}

/// Counts persistent typed segments without constructing an auxiliary type pack.
template <Info Segments>
[[nodiscard]] consteval auto segmentCount() -> std::size_t {
  std::size_t count{};
  Info cursor = Segments;
  while (cursor != Info{}) {
    ++count;
    cursor = std::meta::extract<SegmentNode>(cursor).previous;
  }
  return count;
}

/// Linearizes persistent segment metadata in source/evaluation order.
template <Info Segments>
[[nodiscard]] consteval auto programSegments() {
  std::array<SegmentNode, segmentCount<Segments>()> result{};
  Info cursor = Segments;
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<SegmentNode>(cursor);
    result[--index] = node;
    cursor = node.previous;
  }
  return result;
}

/// Counts instructions in the persistent half-open program range `(Begin, End]`.
template <Info Begin, Info End>
[[nodiscard]] consteval auto programInstructionCountBetween() -> std::size_t {
  std::size_t count{};
  Info cursor = End;
  while (cursor != Begin) {
    if (cursor == Info{}) {
      throw std::meta::exception{
          "Miracle Meta Query segment does not belong to its declared program prefix", End};
    }
    ++count;
    cursor = std::meta::extract<ProgramNode>(cursor).previous;
  }
  return count;
}

/// One instruction paired with its ordinal in the normalized immutable program.
///
/// Evaluation-state slots are assigned from this normalized order rather than while fluent adaptors are being
/// appended. That is important: local normalization may remove or replace stateful instructions, so
/// allocating a counter before normalization would leave holes or stale slots.
struct IndexedInstruction final {
  /// Normalized immutable instruction value.
  Instruction instruction{};
  /// Zero-based position in the complete normalized program.
  std::size_t ordinal{};

  /// Compares instruction identity and global normalized-program position.
  constexpr auto operator==(const IndexedInstruction &) const -> bool = default;
};

/// Linearizes a homogeneous body together with each instruction's global normalized-program ordinal.
template <Info Begin, Info End>
[[nodiscard]] consteval auto programIndexedInstructionsBetween() {
  std::array<IndexedInstruction, programInstructionCountBetween<Begin, End>()> result{};
  Info cursor = End;
  const auto firstOrdinal = programInstructionCount<Begin>();
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<ProgramNode>(cursor);
    --index;
    result[index] = IndexedInstruction{node.instruction, firstOrdinal + index};
    cursor = node.previous;
  }
  return result;
}

/// Sentinel stored for instructions that require no mutable evaluation-state slot.
inline constexpr std::size_t noEvaluationStateSlot = std::numeric_limits<std::size_t>::max();

/// Compact slot assigned to one normalized instruction. The opcode determines whether the index addresses the
/// counter pool or flag pool; stateless instructions retain `noEvaluationStateSlot`.
struct EvaluationStateSlot final {
  /// Dense counter/flag-pool index, or the stateless sentinel.
  std::size_t index{noEvaluationStateSlot};

  /// Compares the compiled primitive-state slot identity.
  constexpr auto operator==(const EvaluationStateSlot &) const -> bool = default;
};

/// Immutable evaluation-state layout compiled from normalized value IR.
///
/// This is data, not one C++ type per stateful stage. `N` contributes only an array extent; the mutable
/// evaluator itself is parameterized by the two primitive pool sizes.
template <std::size_t N>
struct EvaluationStatePlan final {
  /// Per-instruction slot mapping in normalized program order.
  std::array<EvaluationStateSlot, N> slots{};
  /// Number of mutable size/count slots required by evaluation.
  std::size_t counterCount{};
  /// Number of mutable Boolean slots required by evaluation.
  std::size_t flagCount{};

  /// Compares the complete immutable primitive-state layout.
  constexpr auto operator==(const EvaluationStatePlan &) const -> bool = default;
};

/// Returns the number of contiguous counter slots required by one opcode.
[[nodiscard]] consteval auto counterStateWidth(QueryOp operation) -> std::size_t {
  if (operation == QueryOp::Take or operation == QueryOp::Skip or operation == QueryOp::Enumerate) {
    return 1;
  }
  if (operation == QueryOp::Slice) {
    return 2;
  }
  return 0;
}

/// Returns the number of contiguous Boolean flag slots required by one opcode.
[[nodiscard]] consteval auto flagStateWidth(QueryOp operation) -> std::size_t {
  return operation == QueryOp::TakeWhile or operation == QueryOp::SkipWhile ? 1 : 0;
}

/// Assigns dense primitive-state slots after all local IR normalization has completed.
template <Info Program>
[[nodiscard]] consteval auto compileEvaluationStatePlan() {
  constexpr auto instructions = programInstructions<Program>();
  EvaluationStatePlan<instructions.size()> result{};

  for (std::size_t ordinal{}; ordinal < instructions.size(); ++ordinal) {
    const auto counterWidth = counterStateWidth(instructions[ordinal].op);
    const auto flagWidth = flagStateWidth(instructions[ordinal].op);
    if (counterWidth != 0 and flagWidth != 0) {
      throw std::meta::exception{"Miracle Meta Query instruction requested incompatible state pools", Info{}};
    }
    if (counterWidth != 0) {
      result.slots[ordinal].index = result.counterCount;
      result.counterCount += counterWidth;
    } else if (flagWidth != 0) {
      result.slots[ordinal].index = result.flagCount;
      result.flagCount += flagWidth;
    }
  }
  return result;
}

/// Ephemeral mutable traversal state. Query and Program stay immutable; only these primitive pools change
/// while a terminal walks one source.
template <std::size_t CounterCount, std::size_t FlagCount>
struct Evaluation final {
  /// Mutable skip/take/slice/enumerate counters.
  std::array<std::size_t, CounterCount> counters{};
  /// Mutable takeWhile/skipWhile continuation flags.
  std::array<bool, FlagCount> flags{};
};

/// Initializes primitive traversal state directly from normalized instruction data.
template <Info Program>
[[nodiscard]] consteval auto makeEvaluation() {
  constexpr auto instructions = programInstructions<Program>();
  constexpr auto plan = compileEvaluationStatePlan<Program>();
  Evaluation<plan.counterCount, plan.flagCount> result{};

  for (std::size_t ordinal{}; ordinal < instructions.size(); ++ordinal) {
    const auto instruction = instructions[ordinal];
    const auto slot = plan.slots[ordinal].index;
    switch (instruction.op) {
      case QueryOp::Take:
      case QueryOp::Skip: result.counters[slot] = instruction.a; break;
      case QueryOp::Slice:
        result.counters[slot] = instruction.a;
        result.counters[slot + 1] = instruction.b;
        break;
      case QueryOp::Enumerate: result.counters[slot] = 0; break;
      case QueryOp::TakeWhile:
      case QueryOp::SkipWhile: result.flags[slot] = true; break;
      default: break;
    }
  }
  return result;
}

} // namespace Miracle::meta
