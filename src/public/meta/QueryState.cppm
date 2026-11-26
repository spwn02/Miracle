export module Miracle.Meta:QueryState;

import std;
import :Types;
import :Cache;
import :Reflect;
import :QueryIR;
import :QueryPlan;

namespace Miracle::meta {

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

} // namespace Miracle::meta
