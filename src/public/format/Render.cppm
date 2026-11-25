export module Miracle.Format:Render;

import std;
import Miracle.Numeric;

import :Types;
import :Style;
import :Emit;
import :Message;

namespace Miracle::format::detail {

/// Adapts an output iterator to the `TextSink` protocol. Iterator writes cannot report failure.
template <class OutputIterator>
class IteratorSink final {
public:
  constexpr explicit IteratorSink(OutputIterator out)
      : out_(std::move(out)) {}

  constexpr auto write(const std::string_view text) -> bool {
    out_ = std::ranges::copy(text, std::move(out_)).out;
    return true;
  }

  [[nodiscard]] constexpr auto release() -> OutputIterator {
    return std::move(out_);
  }

private:
  OutputIterator out_;
};

} // namespace Miracle::format::detail

export namespace Miracle::format {

/// Renders `message` into `out`. Plain presentation or `ColorMode::Never` writes exactly `message.view()`;
/// otherwise styled ranges are wrapped in SGR sequences, never leak into neighbours and end with a reset.
/// Not `noexcept`: the iterator may throw.
template <class MessageType, class OutputIterator>
  requires requires(const MessageType &message) {
    { message.view() } -> std::convertible_to<std::string_view>;
    { message.styles() } -> std::convertible_to<std::span<const StyleSpan>>;
  } and std::output_iterator<OutputIterator, char>
[[nodiscard]] auto renderTo(const MessageType &message, OutputIterator out, const RenderPolicy policy = {})
    -> OutputIterator {
  detail::IteratorSink<OutputIterator> sink{std::move(out)};
  RenderState state{.policy = policy};
  detail::AnsiWriter<detail::IteratorSink<OutputIterator>> writer{sink, state};

  const std::string_view text = message.view();
  usize position{};
  for (const StyleSpan &span : message.styles()) {
    auto _ = writer.put(text.substr(position, span.begin - position), Style{});
    _ = writer.put(text.substr(span.begin, span.end - span.begin), span.style);
    position = span.end;
  }
  auto _ = writer.put(text.substr(position), Style{});
  _ = writer.finish();
  return sink.release();
}

/// Renders `message` into a new string. May allocate and is therefore not `noexcept`.
template <class MessageType>
[[nodiscard]] auto render(const MessageType &message, const RenderPolicy policy = {}) -> std::string {
  std::string result;
  auto _ = renderTo(message, std::back_inserter(result), policy);
  return result;
}

} // namespace Miracle::format
