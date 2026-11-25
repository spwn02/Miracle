export module Miracle.Iter:SourceFactories;

import std;
import Miracle.Numeric;
import Miracle.Types;
import Miracle.Range;
import :Detail;
import :SourceViews;
import :Core;

export namespace Miracle {

/// Creates an Iter from any viewable range. Lvalues remain borrowed where the standard view does; owned
/// non-borrowed rvalues are consumed through `as_rvalue`.
template <std::ranges::viewable_range RangeType>
  requires(not OptionalLike<std::remove_cvref_t<RangeType>>)
[[nodiscard]] constexpr auto iter(RangeType &&range) {
  auto view = viewForRange(std::forward<RangeType>(range));
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates the half-open integer pipeline `[0, stop)`.
template <std::integral T>
  requires(not std::same_as<std::remove_cv_t<T>, bool>)
[[nodiscard]] constexpr auto iter(T stop) {
  return iter(Range<T>{stop});
}

/// Creates the half-open integer pipeline `[start, stop)` using Range's lossless endpoint rules.
template <std::integral Left, std::integral Right>
  requires CompatibleRangeEndpoints<Left, Right>
[[nodiscard]] constexpr auto iter(Left start, Right stop) {
  return iter(Range{start, stop});
}

/// Creates a positively-strided half-open integer pipeline. The step participates in common-type deduction
/// and must have compatible signedness with the endpoints.
template <std::integral Left, std::integral Right, std::integral Step>
  requires CompatibleRangeEndpoints<Left, Right> and (not std::same_as<std::remove_cv_t<Step>, bool>) and
           (std::is_signed_v<Left> == std::is_signed_v<Step>)
[[nodiscard]] constexpr auto iter(Left start, Right stop, Step step) pre(step > 0) {
  if (step <= 0) {
    std::terminate();
  }
  using Common = std::common_type_t<Left, Right, Step>;
  auto base = Range<Common>{Common{start}, Common{stop}} | std::views::stride(Common{step});
  return Iter<decltype(base)>{std::move(base)};
}

/// Views an lvalue Optional/Expected-like object as zero or one borrowed item.
template <OptionalLike Maybe>
[[nodiscard]] constexpr auto iter(Maybe &maybe) {
  using View = MaybeRefView<Maybe>;
  return Iter<View>{View{maybe}};
}

/// Owns a rvalue Optional/Expected-like object as zero or one consumable item.
template <OptionalLike Maybe>
  requires(not std::is_lvalue_reference_v<Maybe>)
[[nodiscard]] constexpr auto iter(Maybe &&maybe) {
  using Stored = std::remove_cvref_t<Maybe>;
  auto view = MaybeValueView<Stored>{std::forward<Maybe>(maybe)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates an empty random-access, sized, borrowed Iter source of `T`.
template <class T>
[[nodiscard]] constexpr auto empty() {
  return Iter{std::views::empty<T>};
}

/// Creates a one-element source. The stored value is exposed as an rvalue so move-only values can be
/// materialized without a copy.
template <class T>
[[nodiscard]] constexpr auto once(T value) {
  auto view = std::views::single(std::move(value)) | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Lazily invokes `function` once when traversal begins and yields its cached result exactly once.
template <class Function>
[[nodiscard]] constexpr auto onceWith(Function function) {
  using Stored = std::decay_t<Function>;
  auto view = OnceWith<Stored>{std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates an endless standard repeat view. Each position references the one stored value, following C++
/// `repeat_view` semantics rather than cloning a new value on every dereference.
template <class T>
[[nodiscard]] constexpr auto repeat(T value) {
  auto view = std::views::repeat(std::move(value));
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates a bounded standard repeat view with exactly `count` positions.
template <class T>
[[nodiscard]] constexpr auto repeatN(T value, usize count) {
  auto view = std::views::repeat(std::move(value), count);
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates an endless callable source that invokes `function` once per increment and caches the result for
/// repeated dereference of that logical item.
template <class Function>
[[nodiscard]] constexpr auto repeatWith(Function function) {
  using Stored = std::decay_t<Function>;
  auto view = RepeatWith<Stored>{std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates a single-pass source from `function() -> Option<T>`. The first empty result terminates traversal.
template <class Function>
[[nodiscard]] constexpr auto fromFn(Function function) {
  using Stored = std::decay_t<Function>;
  auto view = FromFn<Stored>{std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Creates a single-pass sequence beginning with `seed` and repeatedly invoking `function(const T&) ->
/// Option<T>`. Each successor is computed before its predecessor becomes consumable.
template <class T, class Function>
  requires(not OptionalLike<std::remove_cvref_t<T>>)
[[nodiscard]] constexpr auto successors(T seed, Function function) {
  using Value = std::decay_t<T>;
  using Stored = std::decay_t<Function>;
  auto view = Successors<Value, Stored>{std::move(seed), std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Rust-parity successor source whose optional first value may make the sequence initially exhausted.
template <OptionalLike Maybe, class Function>
[[nodiscard]] constexpr auto successors(Maybe first, Function function) {
  using Value = std::remove_cvref_t<typename std::remove_cvref_t<Maybe>::value_type>;
  using Stored = std::decay_t<Function>;
  Option<Value> seed{};
  if (first.has_value()) {
    seed.emplace(std::move(*first));
  }
  auto view = Successors<Value, Stored>{std::move(seed), std::move(function)} | std::views::as_rvalue;
  return Iter<decltype(view)>{std::move(view)};
}

/// Free-function spelling equivalent to `iter(first).chain(second)`.
template <std::ranges::viewable_range First, std::ranges::viewable_range Second>
[[nodiscard]] constexpr auto chain(First &&first, Second &&second) {
  return iter(std::forward<First>(first)).chain(std::forward<Second>(second));
}

/// Free-function spelling equivalent to `iter(first).zip(second)`.
template <std::ranges::viewable_range First, std::ranges::viewable_range Second>
[[nodiscard]] constexpr auto zip(First &&first, Second &&second) {
  return iter(std::forward<First>(first)).zip(std::forward<Second>(second));
}

} // namespace Miracle
