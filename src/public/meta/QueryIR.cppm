export module Miracle.Meta:QueryIR;

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

} // namespace Miracle::meta
