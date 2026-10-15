export module Miracle.Meta:Query;

import std;
import :Types;
import :Cache;
import :Reflect;

export namespace Miracle::meta {

template <class State>
struct Query;

} // namespace Miracle::meta

namespace Miracle::meta {

/// Private bridge used by `flatten` to evaluate a nested Query without exposing its state representation.
template <class State, class Function>
consteval auto evaluateQuery(const Query<State> &query, Function &&function) -> bool;

/// Homogeneous value-level opcode used by the Query program IR.
///
/// Operation topology is data: adding another logical operation must not require another stage class-template
/// specialization. Typed/cardinality boundaries remain separate compiler concerns and are intentionally not
/// encoded here.
enum class QueryOp : unsigned char {
  /// Transform one logical value into another logical value.
  Map,
  /// Retain a logical value only when a predicate accepts it.
  Filter,
  /// Apply an Optional-like transform and retain only engaged results.
  FilterMap,
  /// Expand one nested Query/range value into the downstream logical stream.
  Flatten,
  /// Observe one logical value without changing it.
  Inspect,
  /// Retain at most the first `a` accepted logical values.
  Take,
  /// Discard the first `a` accepted logical values.
  Skip,
  /// Retain values while the predicate stored in `operand`/capture state remains true.
  TakeWhile,
  /// Discard values while the predicate stored in `operand`/capture state remains true.
  SkipWhile,
  /// Discard `a` values, then retain at most `b` values.
  Slice,
  /// Transform each logical value into `(index, value)` using an evaluation counter.
  Enumerate,
};

/// Compact structural instruction carried by the immutable Query program.
///
/// `operand` identifies structural callable/configuration state when C++26 can represent it directly. Scalar
/// fields carry compact operation parameters or future evaluation-state slot indices. Non-structural capture
/// state is deliberately not stored here.
struct Instruction final {
  /// Operation performed by this instruction.
  QueryOp op{};

  /// Structural callable/configuration reflection. An empty reflection means the callable lives in the flat
  /// non-structural capture environment instead.
  Info operand{};

  /// Primary compact operand. For captured callable operations this is the capture-environment index; for
  /// `Take`/`Skip` it is the count; for `Slice` it is the number of leading values to discard.
  std::size_t a{};

  /// Secondary compact operand. Currently used by `Slice` as the maximum number of values to retain.
  std::size_t b{};

  /// Compares the complete structural instruction identity used for reflected-program interning.
  constexpr auto operator==(const Instruction &) const -> bool = default;
};

/// Persistent reflected program node.
///
/// Each append reflects one structural `(previous, instruction)` value. Equivalent prefixes therefore inherit
/// the compiler's native structural identity instead of requiring a Miracle-owned global memo table.
struct ProgramNode final {
  /// Reflected identity of the preceding immutable program prefix.
  Info previous{};
  /// Instruction appended after `previous`.
  Instruction instruction{};

  /// Compares complete structural program-node identity for native compiler interning.
  constexpr auto operator==(const ProgramNode &) const -> bool = default;
};

/// Appends one instruction without normalization.
[[nodiscard]] consteval auto appendProgramRaw(Info previous, Instruction instruction) -> Info {
  return std::meta::reflect_constant(ProgramNode{.previous = previous, .instruction = instruction});
}

/// Appends one structural instruction and performs only locally provable, observation-preserving
/// normalization.
///
/// Rewrites are intentionally conservative. In particular, arithmetic that would overflow keeps the two
/// original instructions, and `Take(0)` is not allowed to erase an observable prefix: earlier
/// predicates/inspection may still be evaluated before the zero-take boundary is reached.
[[nodiscard]] consteval auto appendProgram(Info previous, Instruction instruction) -> Info {
  if (instruction.op == QueryOp::Skip and instruction.a == 0) {
    return previous;
  }

  if (previous == Info{}) {
    return appendProgramRaw(previous, instruction);
  }

  const auto tail = std::meta::extract<ProgramNode>(previous);
  const auto prior = tail.instruction;

  if (prior.op == QueryOp::Take and instruction.op == QueryOp::Take) {
    return appendProgramRaw(
        tail.previous, Instruction{.op = QueryOp::Take, .a = std::min(prior.a, instruction.a)});
  }

  if (prior.op == QueryOp::Skip and instruction.op == QueryOp::Skip) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(tail.previous, Instruction{.op = QueryOp::Skip, .a = prior.a + instruction.a});
    }
    return appendProgramRaw(previous, instruction);
  }

  if (prior.op == QueryOp::Skip and instruction.op == QueryOp::Take) {
    return appendProgramRaw(
        tail.previous, Instruction{.op = QueryOp::Slice, .a = prior.a, .b = instruction.a});
  }

  // Once a bounded Slice exists, a later Take can only tighten its output limit. Source traversal stops at
  // the same logical item in both formulations, including the zero-take case where Slice deliberately
  // preserves the established observable prefix traversal.
  if (prior.op == QueryOp::Slice and instruction.op == QueryOp::Take) {
    return appendProgramRaw(tail.previous,
        Instruction{.op = QueryOp::Slice, .a = prior.a, .b = std::min(prior.b, instruction.a)});
  }

  // Prefix skipping before a Slice is just a larger Slice offset when that offset is representable. Overflow
  // is intentionally left as two instructions so the original stepwise size_t semantics remain observable.
  if (prior.op == QueryOp::Skip and instruction.op == QueryOp::Slice) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(
          tail.previous, Instruction{.op = QueryOp::Slice, .a = prior.a + instruction.a, .b = instruction.b});
    }
    return appendProgramRaw(previous, instruction);
  }

  // Skipping inside an already-bounded Slice can be folded only while the second skip remains strictly inside
  // the first Slice's take window. At the exact/end boundary, replacing the pair with Slice(..., 0) would ask
  // the source for one additional item under the established Slice semantics, so that case stays separate.
  if (prior.op == QueryOp::Slice and instruction.op == QueryOp::Skip and instruction.a < prior.b) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(tail.previous,
          Instruction{.op = QueryOp::Slice, .a = prior.a + instruction.a, .b = prior.b - instruction.a});
    }
    return appendProgramRaw(previous, instruction);
  }

  // Two adjacent bounded slices compose exactly while the second Slice begins inside the first take window.
  // The strict inequality is intentional for the same source-observation reason as Slice + Skip above.
  if (prior.op == QueryOp::Slice and instruction.op == QueryOp::Slice and instruction.a < prior.b) {
    if (prior.a <= std::numeric_limits<std::size_t>::max() - instruction.a) {
      return appendProgramRaw(tail.previous,
          Instruction{.op = QueryOp::Slice,
              .a = prior.a + instruction.a,
              .b = std::min(prior.b - instruction.a, instruction.b)});
    }
    return appendProgramRaw(previous, instruction);
  }

  return appendProgramRaw(previous, instruction);
}

/// Counts one reflected program at compile time without creating an intermediate container.
template <Info Program>
[[nodiscard]] consteval auto programInstructionCount() -> std::size_t {
  std::size_t count{};
  Info cursor = Program;
  while (cursor != Info{}) {
    ++count;
    cursor = std::meta::extract<ProgramNode>(cursor).previous;
  }
  return count;
}

/// Linearizes a persistent program into declaration order for homogeneous segment compilation.
///
/// The array extent is derived directly from the reflected program NTTP. This deliberately avoids temporary
/// `vector`/`define_static_array` machinery on the evaluator hot path: the structural probe showed
/// that the fixed template-library cost dwarfed the actual value-IR stage growth.
template <Info Program>
[[nodiscard]] consteval auto programInstructions() {
  std::array<Instruction, programInstructionCount<Program>()> result{};
  Info cursor = Program;
  for (std::size_t index = result.size(); index != 0;) {
    const auto node = std::meta::extract<ProgramNode>(cursor);
    result[--index] = node.instruction;
    cursor = node.previous;
  }
  return result;
}

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

/// One independently addressable slot in the flat non-structural callable environment.
///
/// `Index` deliberately participates in the slot type so two occurrences of the same closure type remain
/// distinct.
/// `[[no_unique_address]]` also lets empty captured-state wrappers avoid unnecessary storage when the
/// language permits.
template <std::size_t Index, class Capture>
struct CaptureSlot {
  /// Irreducible non-structural callable state for this slot index.
  [[no_unique_address]] Capture value;
};

/// Flat implementation selected by an index sequence so no capture points at a predecessor environment.
template <class Indices, class... Captures>
struct CaptureEnvironmentImpl;

template <std::size_t... Indices, class... Captures>
struct CaptureEnvironmentImpl<std::index_sequence<Indices...>, Captures...> final
    : CaptureSlot<Indices, Captures>... {
  /// Number of independently stored non-structural callable values in the flat environment.
  static constexpr std::size_t size_ = sizeof...(Captures);
};

/// Stores only irreducible non-structural Query callable values.
///
/// Captureless/structural callables belong directly in `Instruction::operand`; this environment therefore
/// grows with M non-structural captured stages rather than N total Query operations. The representation is a
/// single flat pack of indexed slots: there is no recursive `Environment<Previous, Capture>` chain and no
/// tuple-backed stage list.
///
/// A reflected `define_aggregate` representation was measured before selecting this form. Both
/// representations were semantically viable, but the synthesized aggregate imposed substantially higher fixed
/// and scaling frontend cost on the reference probes.
template <class... Captures>
using CaptureEnvironment = CaptureEnvironmentImpl<std::index_sequence_for<Captures...>, Captures...>;

/// Returns one captured callable by the compact slot index stored in the Query instruction.
template <std::size_t Index, class... Captures>
[[nodiscard]] consteval auto captureAt(CaptureEnvironment<Captures...> &environment) -> decltype(auto) {
  static_assert(Index < sizeof...(Captures), "Miracle Meta Query capture slot is out of range");
  using Capture = Captures...[Index];
  return (static_cast<CaptureSlot<Index, Capture> &>(environment).value);
}

/// Appends one non-structural callable while keeping the resulting environment flat.
template <class... Captures, class Capture>
[[nodiscard]] consteval auto appendCapture(CaptureEnvironment<Captures...> environment, Capture capture)
    -> CaptureEnvironment<Captures..., Capture> {
  return [&]<std::size_t... Indices>(std::index_sequence<Indices...>) consteval -> auto {
    return CaptureEnvironment<Captures..., Capture>{
        CaptureSlot<Indices, Captures>{std::move(captureAt<Indices>(environment))}...,
        CaptureSlot<sizeof...(Captures), Capture>{std::move(capture)}};
  }(std::index_sequence_for<Captures...>{});
}

/// The immutable instruction plus the capture environment resulting from binding one callable.
template <class Environment>
struct CallableBinding final {
  /// IR instruction containing either reflected callable identity or capture slot.
  Instruction instruction;
  /// Flat environment after binding any newly required non-structural callable.
  Environment environment;
};

/// Binds a callable without duplicating opcode semantics into static/stateful stage types.
///
/// Structural values are reflected directly into the instruction. A genuinely non-structural closure receives
/// the next compact environment slot instead; `Instruction::a` records that slot and `operand` remains empty.
template <class... Captures, class Function>
[[nodiscard]] consteval auto bindCallable(QueryOp operation,
    CaptureEnvironment<Captures...> environment,
    Function function) {
  if constexpr (std::meta::is_structural_type(^^std::remove_cvref_t<Function>)) {
    return CallableBinding<CaptureEnvironment<Captures...>>{
        Instruction{.op = operation, .operand = std::meta::reflect_constant(function)},
        std::move(environment)};
  } else {
    constexpr auto slot = sizeof...(Captures);
    auto nextEnvironment = appendCapture(std::move(environment), std::move(function));
    return CallableBinding<decltype(nextEnvironment)>{
        Instruction{.op = operation, .a = slot}, std::move(nextEnvironment)};
  }
}

/// Computes the logical element type produced by flattening either an input range or another Query.
///
/// Lvalue ranges preserve their reference type; rvalue ranges use their value type so no Query can retain a
/// reference into a temporary range after the flattening call completes.
template <class T, bool IsRange = std::ranges::input_range<std::remove_cvref_t<T>>>
struct FlattenValue;

template <class T>
struct FlattenValue<T, true> {
  /// Exact downstream logical type: lvalue ranges preserve reference identity, rvalue ranges own values.
  using type = std::conditional_t<std::is_lvalue_reference_v<T>,
      std::ranges::range_reference_t<T>,
      std::ranges::range_value_t<std::remove_cvref_t<T>>>;
};

template <class T>
  requires(not std::ranges::input_range<std::remove_cvref_t<T>>)
struct FlattenValue<T, false> {
  /// Nested Query logical value type used when the flattened item is itself a Query.
  using type = std::remove_cvref_t<T>::Value;
};

/// Data-oriented public Query state used by the Query facade.
///
/// Operation topology is not encoded in the C++ type. `program` and `segmentPlan` are immutable structural
/// values, while `environment` contains only irreducible non-structural callable state. Consequently the
/// state type changes only when the logical value/source type changes or the capture environment genuinely
/// grows.
template <class Source, class Value, class Environment = CaptureEnvironment<>>
struct QueryPlanState final {
  /// Concrete source representation consumed by the compiled evaluator.
  using source_type = Source;
  /// Exact logical item type visible at the current Query API boundary.
  using value_type = Value;
  /// Flat type containing only unavoidable non-structural captures.
  using environment_type = Environment;

  /// Source value/state from which terminal evaluation begins.
  Source source;
  /// Non-structural callable state, empty when all operands are structural.
  [[no_unique_address]] Environment environment;
  /// Reflected identity of the normalized persistent instruction program.
  Info program{};
  /// Incremental typed-segment plan aligned with `program`.
  SegmentPlan segmentPlan{};
};

/// Constructs an empty compiled Query state for one source/logical value type.
template <class Value, class Source>
[[nodiscard]] consteval auto makeInitialQueryPlan(Source source) {
  using State = QueryPlanState<Source, Value>;
  return State{std::move(source), CaptureEnvironment<>{}, Info{}, beginSegmentPlan(Info{}, ^^Value)};
}

/// Appends one already-bound IR instruction and updates typed-segment planning after normalization.
template <class Result, class Source, class Value, class Environment>
[[nodiscard]] consteval auto appendQueryInstruction(QueryPlanState<Source, Value, Environment> state,
    Instruction instruction) {
  const auto before = state.program;
  const auto after = appendProgram(before, instruction);
  const auto nextPlan = advanceSegmentPlan(state.segmentPlan, before, after, ^^Result);
  using NextState = QueryPlanState<Source, Result, Environment>;
  return NextState{std::move(state.source), std::move(state.environment), after, nextPlan};
}

/// Appends one callable-backed instruction. Structural callables stay in IR; only genuinely non-structural
/// callable values grow the flat environment type.
template <class Result, class Source, class Value, class Environment, class Function>
[[nodiscard]] consteval auto appendQueryCallable(QueryPlanState<Source, Value, Environment> state,
    QueryOp operation,
    Function function) {
  auto binding = bindCallable(operation, std::move(state.environment), std::move(function));
  const auto before = state.program;
  const auto after = appendProgram(before, binding.instruction);
  const auto nextPlan = advanceSegmentPlan(state.segmentPlan, before, after, ^^Result);
  using NextEnvironment = decltype(binding.environment);
  using NextState = QueryPlanState<Source, Result, NextEnvironment>;
  return NextState{std::move(state.source), std::move(binding.environment), after, nextPlan};
}

/// Borrowed static-span source. Query never extends the lifetime of `values`.
template <class Element>
struct SpanSource final {
  /// Borrowed source elements; Query does not extend their lifetime.
  std::span<Element> values;
  /// Exact lvalue logical type emitted for each borrowed element.
  using value_type = Element &;
};

/// Composite source that evaluates the right query only after the left query is exhausted.
template <class LeftState, class RightState>
struct ChainSource final {
  /// First Query state evaluated to exhaustion.
  LeftState left;
  /// Second Query state evaluated after `left`.
  RightState right;
  /// Shared logical value type required by chain composition.
  using value_type = LeftState::value_type;
};

/// Composite source pairing two independently evaluated query states until the shorter side ends.
template <class LeftState, class RightState>
struct ZipSource final {
  /// Left Query state evaluated into temporary persistence backing.
  LeftState left;
  /// Right Query state evaluated into temporary persistence backing.
  RightState right;
  /// Exact logical value category produced by the left source.
  using left_value_type = LeftState::value_type;
  /// Exact logical value category produced by the right source.
  using right_value_type = RightState::value_type;
  /// Paired logical output type.
  using value_type = std::pair<left_value_type, right_value_type>;
};

/// Static-storage source used after operations that must persist/reorder logical values.
///
/// Reference-valued queries store pointers and recover the original references through `unwrap`; owning
/// values are stored directly. The source therefore preserves Query's logical `Value` even across a
/// persistence barrier.
template <class Value, class Stored>
struct StoredSource final {
  /// Program-lifetime persisted representations backing this source.
  std::span<const Stored> values;
  /// Logical type reconstructed from each stored representation.
  using value_type = Value;
};

/// Invokes a Query callable using the richest shape the callable explicitly accepts.
///
/// Dispatch is ordered deliberately: an exact logical-value overload wins first, tuple protocol expansion is
/// second, and reflected aggregate-field expansion is the final structural fallback. This prevents a callable
/// that already accepts the object itself from being unexpectedly decomposed and keeps the adaptation local
/// to Meta Query.
template <class Function, class Item>
consteval auto adaptiveInvoke(Function &&function, Item &&item) -> decltype(auto) {
  if constexpr (std::invocable<Function, Item>) {
    // Preserve the logical value/category when the callable already understands it; decomposition would only
    // add compiler work and could select a different overload set.
    return std::invoke(std::forward<Function>(function), std::forward<Item>(item));
  } else if constexpr (requires { typename std::tuple_size<std::remove_cvref_t<Item>>::type; }) {
    // Tuple-like values use their established tuple protocol before reflection. This covers pair/tuple/custom
    // tuple-like values without requiring them to be aggregates or exposing implementation members.
    constexpr auto count = std::tuple_size_v<std::remove_cvref_t<Item>>;
    return [&]<std::size_t... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      static_assert(std::invocable<Function, decltype(std::get<Index>(std::declval<Item>()))...>,
          "Miracle Meta Query invocation requires a callable accepting the item or its tuple elements");
      return std::invoke(std::forward<Function>(function), std::get<Index>(std::forward<Item>(item))...);
    }(std::make_index_sequence<count>{});
  } else if constexpr (std::is_aggregate_v<std::remove_cvref_t<Item>>) {
    // Reuse Miracle's reflection façade instead of issuing a parallel raw <meta> query. `fields()` is the
    // exact semantic match for aggregate decomposition: only non-static data members participate, in
    // declaration order.
    constexpr auto fields = reflect<std::remove_cvref_t<Item>>().fields();
    return [&]<std::size_t... Index>(std::index_sequence<Index...>) -> decltype(auto) {
      // Validate the fully expanded call before splicing. The generated indices are bounded by fields.size(),
      // so the reflected member accesses below cannot escape the cached field universe.
      static_assert(
          requires(Function &&function, Item &&value) {
            std::invoke(std::forward<Function>(function),
                std::forward<Item>(value).[:fields[Index
            ]:]...); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
                     // - Index is generated by make_index_sequence.
          },
          "Miracle Meta Query invocation requires a callable accepting the item, its tuple elements, or its "
          "reflected members");
      return std::invoke(std::forward<Function>(function),
          std::forward<Item>(item)
              .[:fields[Index]:]...); // NOLINT(cppcoreguidelines-pro-bounds-avoid-unchecked-container-access)
    }(std::make_index_sequence<fields.size()>{});
  } else {
    // Reaching this branch means no supported invocation shape exists. Keep the diagnostic at the adaptation
    // boundary instead of allowing deeper std::invoke substitution failure to obscure the Query operation
    // that caused it.
    static_assert(std::invocable<Function, Item>,
        "Miracle Meta Query invocation requires a callable accepting the item, its tuple elements, or its "
        "reflected members");
  }
}

/// Invokes a binary Query callable while preserving both logical value categories.
///
/// Binary operations currently have no tuple/aggregate decomposition semantics; keeping that boundary
/// explicit avoids surprising comparator/reducer behavior and leaves room for a separately designed binary
/// adaptation policy later.
template <class Function, class Left, class Right>
consteval auto adaptiveInvokeBinary(Function &&function, Left &&left, Right &&right) -> decltype(auto) {
  if constexpr (std::invocable<Function, Left, Right>) {
    // Forward both values exactly so reference-sensitive comparators and reducers observe the Query's logical
    // types.
    return std::invoke(
        std::forward<Function>(function), std::forward<Left>(left), std::forward<Right>(right));
  } else {
    // Diagnose at the Query boundary rather than surfacing an implementation-level std::invoke substitution
    // trace.
    static_assert(std::invocable<Function, Left, Right>,
        "Miracle Meta Query binary invocation requires a callable accepting both logical values");
  }
}

/// Minimal optional protocol required by `filterMap`; deliberately avoids coupling the Query engine to one
/// wrapper type.
template <class T>
concept OptionalLike = requires(T value) {
  typename std::remove_cvref_t<T>::value_type;
  { value.has_value() } -> std::convertible_to<bool>;
  *value;
};

/// Extracts the logical engaged value from an Optional-like result.
template <class T>
struct OptionalResult final {
  /// Owning logical type reported by an ordinary Optional-like result.
  using type = std::remove_cvref_t<T>::value_type;
};

/// C++26 `std::optional<T&>` exposes `value_type` as `T`, so the reference must be restored explicitly for
/// Query's logical value type. Ordinary `std::optional<T>` retains its owning `T` result type.
template <class T>
struct OptionalResult<std::optional<T &>> final {
  /// Reference logical type restored from C++26 `std::optional<T&>`.
  using type = T &;
};

template <class T>
using OptionalResultT = OptionalResult<std::remove_cvref_t<T>>::type;

/// Presents one engaged Optional-like result to its continuation using the declared logical result type.
///
/// Standard optional values already expose `T&&` when the wrapper is moved. More general Optional-like
/// wrappers may expose proxy dereference values instead; those are materialized once into the declared
/// `value_type` rather than requiring std::optional-specific dereference semantics. Reference results are
/// stricter because manufacturing a temporary would destroy identity.
template <class ResultValue, class Optional, class Function>
consteval auto withOptionalLogical(Optional &result, Function &&function) -> decltype(auto) {
  if constexpr (std::is_reference_v<ResultValue>) {
    static_assert(std::same_as<decltype(*result), ResultValue>,
        "Query::filterMap() reference Optional-like results must dereference to the exact reference type");
    return std::forward<Function>(function)(static_cast<ResultValue>(*result));
  } else if constexpr (std::same_as<decltype(std::move(*result)), ResultValue &&>) {
    return std::forward<Function>(function)(std::move(*result));
  } else {
    ResultValue value(std::move(*result));
    return std::forward<Function>(function)(std::move(value));
  }
}

/// Chooses the static-storage representation for one logical Query value.
///
/// References become pointers so persistence never copies or silently owns the referent; object values remain
/// values.
template <class T>
struct Stored final {
  /// Referent/object type before persistence representation selection.
  using Raw = std::remove_reference_t<T>;
  /// Pointer for references, owning object otherwise.
  using type = std::conditional_t<std::is_reference_v<T>, Raw *, std::remove_cvref_t<T>>;
};

template <class T>
using StoredT = Stored<T>::type;

/// Converts one logical value to its persistence representation while preserving reference identity.
template <class Value, class Item>
constexpr auto storeValueAs(Item &&value) -> StoredT<Value> {
  if constexpr (std::is_reference_v<Value>) {
    // Persist only the address; copying the referent here would violate Query's exact-reference contract.
    return std::addressof(value);
  } else {
    // Owning logical values can be transferred directly into the static-storage representation.
    return std::forward<Item>(value);
  }
}

/// Presents an ordering operand according to its logical type, preserving owning pointer values.
template <class Value, class Stored>
constexpr auto orderingOperand(Stored &value)
    -> std::conditional_t<std::is_reference_v<Value>, Value, Stored &> {
  if constexpr (std::is_reference_v<Value>) {
    return static_cast<Value>(*value);
  } else {
    // Owning storage already is the logical value.
    return value;
  }
}

/// Presents one persisted representation to a synchronous continuation using the Query's exact logical
/// category.
///
/// References are recovered by identity. Owning values are reconstructed locally and forwarded as xvalues so
/// immutable/static backing never leaks `const T&` into a Query whose logical value is `T`. The continuation
/// must not retain the owning value beyond the call.
template <class Value, class Stored, class Function>
consteval auto withStoredLogical(Stored &&stored, Function &&function) -> decltype(auto) {
  if constexpr (std::is_reference_v<Value>) {
    static_assert(std::is_pointer_v<std::remove_cvref_t<Stored>>,
        "Miracle Meta Query reference persistence must store an address");
    return std::forward<Function>(function)(static_cast<Value>(*stored));
  } else {
    using Object = std::remove_reference_t<Value>;
    Object value(std::forward<Stored>(stored));
    return std::forward<Function>(function)(std::move(value));
  }
}

/// Enforces the exact conditions under which Query may expose a persistent standard span.
///
/// References are intentionally rejected even though the engine can internally persist them as pointers:
/// public `materialize()` must not change `T&` into `T*` or copy the referent behind the caller's back.
template <class T>
constexpr auto requireMaterializable() -> void {
  if constexpr (std::is_reference_v<T> or not std::is_object_v<std::remove_cvref_t<T>> or
                not std::copy_constructible<std::remove_cvref_t<T>> or
                not std::meta::is_structural_type(^^std::remove_cvref_t<T>)) {
    throw std::meta::exception{"Query::materialize() requires a copy-constructible object value", {}};
  }
}

/// Returns whether one reorder/grouping barrier can create reusable static backing for its logical value.
///
/// This predicate is used in `if constexpr` at the barrier boundary. A consteval throw by itself is
/// insufficient: code after that throw would still be instantiated and could leak `define_static_array` hard
/// diagnostics before Miracle's intended semantic exception is observed.
template <class Value, class Stored = StoredT<Value>>
inline constexpr bool persistentQueryBacking =
    (std::is_reference_v<Value> or std::copy_constructible<std::remove_cvref_t<Value>>) and
    std::meta::is_structural_type(^^Stored);

template <class Value, class Stored = StoredT<Value>>
consteval auto rejectPersistentQueryBacking(Info source = {}) -> void {
  if constexpr (not std::is_reference_v<Value> and not std::copy_constructible<std::remove_cvref_t<Value>>) {
    throw std::meta::exception{
        "Query persistence barrier requires a copy-constructible owning logical value", source};
  } else {
    static_assert(not std::meta::is_structural_type(^^Stored));
    throw std::meta::exception{
        "Query persistence barrier requires a structurally promotable stored element", source};
  }
}

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

/// Declared before the public Query definition because ordering/set operations call it from exported template
/// bodies.
template <class Value, class Stored = StoredT<Value>>
consteval auto makeStoredQuery(const std::vector<Stored> &values);

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Finite, compile-time-only metadata query algebra.
///
/// `State` contains one source value, one flat non-structural capture environment, and immutable structural
/// Program/SegmentPlan values. The query is not a runtime range and deliberately does not inherit from a
/// range/view hierarchy. Logical reference types are preserved through the compiled lazy evaluator.
template <class State>
struct Query final {
  /// Exact logical element type of the current query; reference qualifiers are intentionally preserved.
  using Value = State::value_type;

  /// Constructs a query from its internal state. The state type is implementation vocabulary; callers
  /// normally use `query(source)` and never spell it directly.
  explicit consteval Query(State state)
      : state_(std::move(state)) {
  }

  /// Maps every logical value using the Meta-specific adaptive invocation rules.
  template <class Self, class Function>
  [[nodiscard]] consteval auto map(this Self &&self, Function function) {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(
        not std::is_void_v<Result>, "Query::map() cannot return void; use forEach() for terminal work");
    auto next =
        appendQueryCallable<Result>(std::forward<Self>(self).state_, QueryOp::Map, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Keeps values accepted by a consteval predicate while preserving the exact logical value type.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto filter(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(std::convertible_to<Result, bool>, "Query::filter() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::Filter, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Maps values to Optional-like results and emits only engaged results.
  /// C++26 `std::optional<T&>` results preserve `T&` as the next logical value; ordinary owning optionals
  /// produce their `value_type`.
  template <class Self, class Function>
  [[nodiscard]] consteval auto filterMap(this Self &&self, Function function) {
    using Maybe = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(OptionalLike<Maybe>, "Query::filterMap() requires an Optional-like result");
    using Result = OptionalResultT<Maybe>;
    auto next =
        appendQueryCallable<Result>(std::forward<Self>(self).state_, QueryOp::FilterMap, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Flattens a map result that is itself a finite range or another Query.
  template <class Self, class Function>
  [[nodiscard]] consteval auto flatMap(this Self &&self, Function function) {
    return std::forward<Self>(self).map(std::move(function)).flatten();
  }

  /// Flattens one level of Query or input-range values without materializing an intermediate sequence.
  template <class Self>
  [[nodiscard]] consteval auto flatten(this Self &&self) {
    static_assert(requires(Value value) { value; }, "Query::flatten() requires a logical value type");
    using Result = FlattenValue<Value>::type;
    auto next =
        appendQueryInstruction<Result>(std::forward<Self>(self).state_, Instruction{.op = QueryOp::Flatten});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Observes each logical value without changing the sequence.
  template <class Self, class Function>
  [[nodiscard]] consteval auto inspect(this Self &&self, Function function) {
    using Result = decltype(adaptiveInvoke(std::declval<Function &>(), std::declval<Value>()));
    static_assert(std::same_as<Result, void>, "Query::inspect() callback must return void");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::Inspect, std::move(function));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Stops after `count` logical values.
  template <class Self>
  [[nodiscard]] consteval auto take(this Self &&self, std::size_t count) {
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Take, .a = count});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Skips the first `count` logical values.
  template <class Self>
  [[nodiscard]] consteval auto skip(this Self &&self, std::size_t count) {
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Skip, .a = count});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Takes values while the predicate remains true; the first failure terminates traversal.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto takeWhile(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::takeWhile() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::TakeWhile, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Skips values while the predicate remains true, then passes every later value.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto skipWhile(this Self &&self, Predicate predicate) {
    using Result = decltype(adaptiveInvoke(std::declval<Predicate &>(), std::declval<Value>()));
    static_assert(
        std::convertible_to<Result, bool>, "Query::skipWhile() predicate must be boolean-convertible");
    auto next =
        appendQueryCallable<Value>(std::forward<Self>(self).state_, QueryOp::SkipWhile, std::move(predicate));
    return Query<decltype(next)>{std::move(next)};
  }

  /// Selects a half-open positional slice `[start, stop)` using the same semantics as Miracle Range.
  template <class Self>
  [[nodiscard]] consteval auto slice(this Self &&self, std::size_t start, std::size_t stop) {
    const auto take = stop > start ? stop - start : std::size_t{};
    auto next = appendQueryInstruction<Value>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Slice, .a = start, .b = take});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Accepts any finite integral endpoint range, including Miracle's `Range`, as `[start, stop)`.
  template <class Self, std::ranges::input_range Range>
    requires std::integral<std::ranges::range_value_t<Range>>
  [[nodiscard]] consteval auto slice(this Self &&self, Range range) {
    // Miracle::Range is itself an input range, but iterating it yields the values *inside* [start, stop), not
    // the two endpoints. Prefer an exposed integral start/stop pair when the range supplies one; generic
    // endpoint carriers such as std::array{start, stop} retain the established first-two-values behavior.
    if constexpr (requires {
                    range.start;
                    range.stop;
                  }) {
      using Start = std::remove_cvref_t<decltype(range.start)>;
      using Stop = std::remove_cvref_t<decltype(range.stop)>;
      if constexpr (std::integral<Start> and std::integral<Stop>) {
        return std::forward<Self>(self).slice(
            static_cast<std::size_t>(range.start), static_cast<std::size_t>(range.stop));
      }
    }

    auto iterator = std::ranges::begin(range);
    const auto last = std::ranges::end(range);
    if (iterator == last) {
      return std::forward<Self>(self).slice(0, 0);
    }
    const auto start = static_cast<std::size_t>(*iterator);
    ++iterator;
    if (iterator == last) {
      return std::forward<Self>(self).slice(start, start);
    }
    return std::forward<Self>(self).slice(start, static_cast<std::size_t>(*iterator));
  }

  /// Enumerates logical values as `(index, value)` pairs without materializing the source.
  template <class Self>
  [[nodiscard]] consteval auto enumerate(this Self &&self) {
    using Result = std::pair<std::size_t, Value>;
    auto next = appendQueryInstruction<Result>(
        std::forward<Self>(self).state_, Instruction{.op = QueryOp::Enumerate});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Concatenates two queries with identical logical value types.
  ///
  /// Child-query termination is local to that child: `left.take(n)` ending naturally still advances into the
  /// right source. Only downstream terminal cancellation stops the whole chain. No persistence barrier is
  /// used.
  template <class Self, class OtherState>
  [[nodiscard]] consteval auto chain(this Self &&self, Query<OtherState> other)
    requires std::same_as<Value, typename Query<OtherState>::Value>
  {
    using Source = ChainSource<State, OtherState>;
    auto next = makeInitialQueryPlan<Value>(Source{std::forward<Self>(self).state_, std::move(other.state_)});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Zips two queries until either child query is exhausted.
  ///
  /// Miracle preserves the established evaluation order in which both child queries are evaluated before
  /// pairing. Pairing storage is evaluation-local only; no child result is promoted into hidden static
  /// backing.
  template <class Self, class OtherState>
  [[nodiscard]] consteval auto zip(this Self &&self, Query<OtherState> other) {
    using Result = std::pair<Value, typename Query<OtherState>::Value>;
    using Source = ZipSource<State, OtherState>;
    auto next =
        makeInitialQueryPlan<Result>(Source{std::forward<Self>(self).state_, std::move(other.state_)});
    return Query<decltype(next)>{std::move(next)};
  }

  /// Returns the first logical value, preserving references through C++26 `optional<T&>`.
  template <class Self>
  [[nodiscard]] consteval auto first(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return false;
    });
    return result;
  }

  /// Returns the last logical value, preserving references where the logical value is a reference.
  template <class Self>
  [[nodiscard]] consteval auto last(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      result.emplace(std::forward<decltype(item)>(item));
      return true;
    });
    return result;
  }

  /// Returns the zero-based `index`th logical value, preserving the exact logical value type.
  template <class Self>
  [[nodiscard]] consteval auto nth(this Self &&self, std::size_t index) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::size_t current{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (current++ == index) {
        result.emplace(std::forward<decltype(item)>(item));
        return false;
      }
      return true;
    });
    return result;
  }

  /// Finds the first value accepted by `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto find(this Self &&self, Predicate predicate) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).filter(std::move(predicate)).first();
  }

  /// Returns the first value produced by an Optional-like mapper.
  template <class Self, class Function>
  [[nodiscard]] consteval auto findMap(this Self &&self, Function function) {
    return std::forward<Self>(self).filterMap(std::move(function)).first();
  }

  /// Returns whether any logical value compares equal to `value`.
  template <class Self, class T>
  [[nodiscard]] consteval auto contains(this Self &&self, const T &value) -> bool {
    bool found{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (std::forward<decltype(item)>(item) == value) {
        found = true;
        return false;
      }
      return true;
    });
    return found;
  }

  /// Returns the first logical index accepted by `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto position(this Self &&self, Predicate predicate) -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = index;
        return false;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns the greatest logical index accepted by `predicate`, or no value when none matches.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto rposition(this Self &&self, Predicate predicate)
      -> std::optional<std::size_t> {
    std::optional<std::size_t> result{};
    std::size_t index{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = index;
      }
      ++index;
      return true;
    });
    return result;
  }

  /// Returns whether every logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto all(this Self &&self, Predicate predicate) -> bool {
    bool result{true};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = false;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether at least one logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto any(this Self &&self, Predicate predicate) -> bool {
    bool result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        result = true;
        return false;
      }
      return true;
    });
    return result;
  }

  /// Returns whether no logical value satisfies `predicate`.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto none(this Self &&self, Predicate predicate) -> bool {
    return not std::forward<Self>(self).any(std::move(predicate));
  }

  /// Counts logical values by evaluating the fused pipeline.
  template <class Self>
  [[nodiscard]] consteval auto count(this Self &&self) -> std::size_t {
    std::size_t result{};
    std::forward<Self>(self).visit([&](auto &&) consteval -> bool {
      ++result;
      return true;
    });
    return result;
  }

  /// Counts logical values accepted by `predicate`.
  ///
  /// The predicate uses the same adaptive invocation rules as `filter`, `all`, `any`, and `find`, so raw
  /// `std::meta` predicates, Miracle wrappers, lambdas, and user callables compose identically.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto count(this Self &&self, Predicate predicate) -> std::size_t {
    std::size_t result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (static_cast<bool>(adaptiveInvoke(predicate, std::forward<decltype(item)>(item)))) {
        ++result;
      }
      return true;
    });
    return result;
  }

  /// Returns whether the logical sequence contains no values.
  template <class Self>
  [[nodiscard]] consteval auto isEmpty(this Self &&self) -> bool {
    bool empty{true};
    std::forward<Self>(self).visit([&](auto &&) consteval -> bool {
      empty = false;
      return false;
    });
    return empty;
  }

  /// Returns the direct source cardinality before any lazy IR has been appended.
  ///
  /// Query operation topology is now value IR rather than part of the Query C++ type, so member availability
  /// can no longer encode “empty program”. Calling `size()` after an adaptor is therefore a semantic error;
  /// use `count()`.
  [[nodiscard]] consteval auto size(this const Query &self)
    requires requires(const State::source_type &source) { source.values.size(); }
  {
    if (self.state_.program != Info{}) {
      throw std::meta::exception{
          "Query::size() is available only for an unmodified direct source; use count()",
          self.state_.program};
    }
    return self.state_.source.values.size();
  }

  /// Reverses the logical sequence while preserving references. This is an eager barrier only at the
  /// operation itself; later stages are still fused into the resulting Query state.
  template <class Self>
  [[nodiscard]] consteval auto reverse(this Self &&self) {
    if constexpr (not persistentQueryBacking<Value>) {
      rejectPersistentQueryBacking<Value>(self.state_.program);
    } else {
      std::vector<StoredT<Value>> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        return true;
      });
      std::ranges::reverse(values);
      return makeStoredQuery<Value>(values);
    }
  }

  /// Sorts logical values using ordinary ordering. Reference values are sorted by referent without changing
  /// their identity.
  template <class Self>
  [[nodiscard]] consteval auto sort(this Self &&self) {
    return std::forward<Self>(self).sortBy(std::less<>{});
  }

  /// Sorts logical values with a binary comparator. The comparator receives the exact logical value types.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto sortBy(this Self &&self, Compare compare) {
    constexpr bool byteBackedBool =
        std::same_as<std::remove_cvref_t<Value>, bool> and not std::is_reference_v<Value>;
    using Stored = std::conditional_t<byteBackedBool, std::uint8_t, StoredT<Value>>;
    if constexpr (not persistentQueryBacking<Value, Stored>) {
      rejectPersistentQueryBacking<Value, Stored>(self.state_.program);
    } else {
      std::vector<Stored> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        if constexpr (byteBackedBool) {
          values.push_back(static_cast<std::uint8_t>(static_cast<bool>(item)));
        } else {
          values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        }
        return true;
      });
      for (std::size_t index{1}; index < values.size(); ++index) {
        auto current = std::move(values[index]);
        std::size_t position = index;
        while (position != 0) {
          auto &previous = values[position - 1];
          bool precedes{};
          if constexpr (byteBackedBool) {
            // `uint8_t` only avoids owning `vector<bool>` storage. The comparator must still observe
            // Query's logical `bool` values rather than that storage representation.
            bool currentValue = current != 0;
            bool previousValue = previous != 0;
            precedes = static_cast<bool>(adaptiveInvokeBinary(compare, currentValue, previousValue));
          } else {
            precedes = static_cast<bool>(adaptiveInvokeBinary(
                compare, orderingOperand<Value>(current), orderingOperand<Value>(previous)));
          }
          if (not precedes) {
            break;
          }
          values[position] = std::move(previous);
          --position;
        }
        values[position] = std::move(current);
      }
      return makeStoredQuery<Value>(values);
    }
  }

  /// Sorts logical values by an adaptive key projection while retaining the original logical elements.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto sortByKey(this Self &&self, Projection projection) {
    auto comparator = [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
      return adaptiveInvoke(projection, std::forward<decltype(left)>(left)) <
             adaptiveInvoke(projection, std::forward<decltype(right)>(right));
    };
    return std::forward<Self>(self).sortBy(std::move(comparator));
  }

  /// Returns the minimum logical value under ordinary ordering.
  template <class Self>
  [[nodiscard]] consteval auto min(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).minBy(std::less<>{});
  }

  /// Returns the maximum logical value under ordinary ordering.
  template <class Self>
  [[nodiscard]] consteval auto max(this Self &&self) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).maxBy(std::less<>{});
  }

  /// Returns the logical minimum according to a binary ordering predicate.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto minBy(this Self &&self, Compare compare) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, item, *result))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical maximum according to a binary ordering predicate.
  template <class Self, class Compare>
  [[nodiscard]] consteval auto maxBy(this Self &&self, Compare compare) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    std::optional<Value> result{};
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      if (not result.has_value() or static_cast<bool>(adaptiveInvokeBinary(compare, *result, item))) {
        result.emplace(std::forward<decltype(item)>(item));
      }
      return true;
    });
    return result;
  }

  /// Returns the logical minimum according to an adaptive key projection.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto minByKey(this Self &&self, Projection projection) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).minBy(
        [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
          return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
        });
  }

  /// Returns the logical maximum according to an adaptive key projection.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto maxByKey(this Self &&self, Projection projection) -> std::optional<Value>
    requires(not std::is_rvalue_reference_v<Value>)
  {
    return std::forward<Self>(self).maxBy(
        [projection = std::move(projection)](auto &&left, auto &&right) consteval -> bool {
          return adaptiveInvoke(projection, left) < adaptiveInvoke(projection, right);
        });
  }

  /// Removes consecutive duplicate logical values. The operation is fused by keeping only the previous value.
  template <class Self>
  [[nodiscard]] consteval auto unique(this Self &&self) {
    return std::forward<Self>(self).uniqueBy(std::identity{});
  }

  /// Removes every later logical element whose projected key has already occurred, preserving encounter
  /// order.
  template <class Self, class Projection>
  [[nodiscard]] consteval auto uniqueBy(this Self &&self, Projection projection) {
    if constexpr (not persistentQueryBacking<Value>) {
      rejectPersistentQueryBacking<Value>(self.state_.program);
    } else {
      // `unique` is implemented as a stage-like eager normalization so arbitrary reference values remain
      // references.
      using Key = std::remove_cvref_t<decltype(adaptiveInvoke(projection, std::declval<Value>()))>;
      static_assert(std::copy_constructible<Key>, "Query::uniqueBy() requires a copy-constructible key");
      std::vector<Key> keys;
      std::vector<StoredT<Value>> values;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        auto key = adaptiveInvoke(projection, item);
        bool duplicate{};
        for (const Key &existing : keys) {
          if (existing == key) {
            duplicate = true;
            break;
          }
        }
        if (duplicate) {
          return true;
        }
        keys.push_back(key);
        values.push_back(storeValueAs<Value>(std::forward<decltype(item)>(item)));
        return true;
      });
      return makeStoredQuery<Value>(values);
    }
  }

  /// Splits logical values into two independently persistent queries while preserving encounter order.
  template <class Self, class Predicate>
  [[nodiscard]] consteval auto partition(this Self &&self, Predicate predicate) {
    using Stored = StoredT<Value>;
    if constexpr (not persistentQueryBacking<Value, Stored>) {
      rejectPersistentQueryBacking<Value, Stored>(self.state_.program);
    } else {
      std::vector<Stored> yes;
      std::vector<Stored> rejected;
      std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
        const bool accepted = static_cast<bool>(adaptiveInvoke(predicate, item));
        auto stored = storeValueAs<Value>(std::forward<decltype(item)>(item));
        if (accepted) {
          yes.push_back(std::move(stored));
        } else {
          rejected.push_back(std::move(stored));
        }
        return true;
      });
      return std::pair{makeStoredQuery<Value>(yes), makeStoredQuery<Value>(rejected)};
    }
  }

  /// Executes a compile-time callback for every logical value and returns no runtime result.
  template <class Self, class Function>
  consteval auto forEach(this Self &&self, Function function) -> void {
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      adaptiveInvoke(function, std::forward<decltype(item)>(item));
      return true;
    });
  }

  /// Promotes a value query into static standard storage. Reference-valued queries cannot be silently
  /// converted into a different element representation, so they fail instead of manufacturing a wrapper type.
  template <class Self>
  [[nodiscard]] consteval auto materialize(this Self &&self) {
    requireMaterializable<Value>();
    using Element = std::remove_cvref_t<Value>;
    std::vector<Element> values;
    std::forward<Self>(self).visit([&](auto &&item) consteval -> bool {
      values.emplace_back(std::forward<decltype(item)>(item));
      return true;
    });
    return promoteVector(values);
  }

private:
  template <class StateType>
  friend struct Query;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(const Query<StateType> &, Function &&) -> bool
    requires std::copy_constructible<StateType>;

  template <class StateType, class Function>
  friend consteval auto evaluateQuery(Query<StateType> &&, Function &&) -> bool;

  template <class Self, class Function>
  consteval auto visit(this Self &&self, Function function) -> bool {
    return evaluateState(std::forward<Self>(self).state_, std::move(function));
  }

  State state_;
};

} // namespace Miracle::meta

namespace Miracle::meta {

template <class State, class Function>
consteval auto evaluateQuery(const Query<State> &query, Function &&function) -> bool
  requires std::copy_constructible<State>
{
  return evaluateState(query.state_, std::forward<Function>(function));
}

template <class State, class Function>
consteval auto evaluateQuery(Query<State> &&query, Function &&function) -> bool {
  return evaluateState(std::move(query).state_, std::forward<Function>(function));
}

/// Promotes a reordered/persisted logical sequence and returns a new Query sourcing that static backing.
///
/// Internal reference storage is allowed through pointers, but the stored representation itself must still be
/// structural so `std::define_static_array` can legally provide program-lifetime backing.
template <class Value, class Stored>
consteval auto makeStoredQuery(const std::vector<Stored> &values) {
  if constexpr (not persistentQueryBacking<Value, Stored>) {
    rejectPersistentQueryBacking<Value, Stored>();
  } else {
    auto span = promoteVector(values);
    using Source = StoredSource<Value, Stored>;
    auto state = makeInitialQueryPlan<Value>(Source{span});
    return Query<decltype(state)>{std::move(state)};
  }
}

} // namespace Miracle::meta

export namespace Miracle::meta {

/// Creates a persistent Meta Query from a standard reflection vector.
///
/// Raw `<meta>` range producers return temporary `std::vector<Info>` values. Query promotes their reflection
/// identities into static backing during this call, so the source vector needs to remain valid only for the
/// call itself. Temporary standard results and existing vectors use the same promotion path;
/// their elements are copied into static backing rather than retaining a view of the input vector.
[[nodiscard]] consteval auto query(const std::vector<Info> &source) {
  return makeStoredQuery<Info>(source);
}

/// Creates a Meta Query over a finite static span without copying its source.
/// The span follows ordinary `std::span` lifetime rules: its referenced storage must remain valid for every
/// Query evaluation that uses it. Query does not take ownership of external span storage.
template <class T, std::size_t Extent>
[[nodiscard]] consteval auto query(std::span<T, Extent> source) {
  using Source = SpanSource<T>;
  auto state = makeInitialQueryPlan<T &>(Source{std::span<T>{source}});
  return Query<decltype(state)>{std::move(state)};
}

/// Creates a Meta Query over a mutable lvalue array without copying its source or discarding mutability.
template <class T, std::size_t N>
[[nodiscard]] consteval auto query(std::array<T, N> &source) {
  return query(std::span<T>{source});
}

/// Creates a Meta Query over a const lvalue array without copying its source.
template <class T, std::size_t N>
[[nodiscard]] consteval auto query(const std::array<T, N> &source) {
  return query(std::span<const T>{source});
}

template <class T, std::size_t N>
consteval auto query(std::array<T, N> &&) = delete (
    "Query cannot borrow a temporary array; keep the source alive or materialize it first");

template <class T, std::size_t N>
consteval auto query(const std::array<T, N> &&) = delete (
    "Query cannot borrow a temporary array; keep the source alive or materialize it first");

} // namespace Miracle::meta
