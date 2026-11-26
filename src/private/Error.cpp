module Miracle.Error;

namespace Miracle {

Error::Message::Message(std::source_location location)
    : location(location) {
}

Error::Message::Message(const char *message, std::source_location location)
    : message(message)
    , location(location) {
}

Error::Message::Message(String message, std::source_location location)
    : message(std::move(message))
    , location(location) {
}

Error::Error() = default;
Error::Error(Message message)
    : messages{std::move(message)} {};

Error::Error(Error &&other) noexcept = default;
auto Error::operator=(Error &&other) noexcept -> Error & = default;

auto Error::with(Message other) -> Error & {
  messages.insert(std::move(other));
  return *this;
}

auto Error::release() noexcept -> Error {
  return std::exchange(*this, {});
}
Error::operator String() const {
  return display();
}
Error::operator bool() const {
  return not messages.empty();
}

auto Error::operator==(const Error &other) const -> bool {
  return display() == other.display();
}
auto Error::operator==(StringView other) const -> bool {
  return display() == other;
}
auto Error::operator<<(Message other) -> void {
  messages.insert(std::move(other));
}

static constexpr auto embed(const Error::Message &message) -> String {
  return std::format("{}[{}:{}]: {}: {}",
      message.location.file_name(),
      message.location.line(),
      message.location.column(),
      message.location.function_name(),
      message.message);
}

namespace {

/// Text sink appending to a `String`; the only output channel of `Error::display`.
struct StringSink final {
  String &bytes;

  auto write(const StringView text) -> bool {
    bytes.append(text);
    return true;
  }
};

/// Chain labels ("Caused by:", "0:") use the shared Format encoder; there is no local SGR table.
constexpr auto labelStyle = format::Style{.foreground = format::TerminalColor::indexed(3)};

} // namespace

auto Error::display(std::ostream &output, ErrorDisplayOptions options) const -> void {
  String text;
  StringSink sink{text};
  format::RenderState state{.policy = {options.colours ? Presentation::Terminal : Presentation::Plain,
                                options.colours ? ColorMode::Always : ColorMode::Never}};
  const auto plain = format::Style{};
  const auto put = [&](const StringView bytes, const format::Style &style) {
    return format::emit(sink, state, format::Event{.kind = format::EventKind::Text, .text = bytes, .style = style});
  };
  const auto render = [&](const Message &message) { return options.locations ? embed(message) : message.message; };

  if (messages.size() == 1) {
    auto _ = put(std::format("Error: {}\n", render(*messages.begin())), plain);
    output << text;
    return;
  }

  // Newest message first. Indexing a vector of references keeps the traversal independent of how the message
  // container's iterators compose with reverse/enumerate views.
  Vec<const Message *> newestFirst;
  newestFirst.reserve(messages.size());
  for (const Message &entry : messages)
    newestFirst.push_back(&entry);
  std::ranges::reverse(newestFirst);

  bool first = true;
  for (const auto [index, entry] : newestFirst | std::views::enumerate) {
    const Message &message = *entry;
    if (not first)
      auto _ = put("\n", plain);
    if (index == 0) {
      auto _ = put(std::format("Error: {}\n\n", render(message)), plain);
      _ = put("Caused by:", labelStyle);
    } else {
      auto _ = put("  ", plain);
      _ = put(std::format("{:d}:", index - 1), labelStyle);
      _ = put(std::format(" {}", render(message)), plain);
    }
    first = false;
  }
  auto _ = format::finish(sink, state);
  output << text;
}

auto Error::display(ErrorDisplayOptions options) const -> String {
  std::stringstream output;
  display(output, options);
  return output.str();
}

auto todo(std::source_location loc) -> bail {
  Error::Message message{loc};
  message.message = "TODO";
  return bail(message);
}

[[nodiscard]] auto diagnose(const Error &error) -> Diagnostic {
  if (error.messages.empty()) {
    return Diagnostic::error(ErrorDiagnosticCode::Message).message("empty error");
  }

  auto iterator = error.messages.begin();
  Diagnostic diagnostic =
      Diagnostic::error(ErrorDiagnosticCode::Message, iterator->location).message(iterator->message);
  ++iterator;
  for (; iterator != error.messages.end(); ++iterator) {
    diagnostic = Diagnostic::error(ErrorDiagnosticCode::Message, iterator->location)
                     .message(iterator->message)
                     .cause(std::move(diagnostic));
  }
  return diagnostic;
}

} // namespace Miracle
