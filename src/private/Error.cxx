module Miracle;

import std;
import :Types;
import :Error;

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

static constexpr auto generateStyle(bool locations, bool colours) -> decltype(auto) {
  return [locations, colours](const Pair<usize, Error::Message> &pair) constexpr -> String {
    const auto &[idx, msg] = pair;
    String res{};

    String message{};

    if (locations)
      message = embed(msg);
    else
      message = msg.message;

    if (idx == 0) {
      StringView header{colours ? "Error: {}\n\n\033[33mCaused by:\033[0m" : "Error: {}\n\nCaused by:"};
      res = std::format(std::runtime_format(header), message);
    } else {
      StringView footer{colours ? "  \033[33m{:d}:\033[0m {}" : "  {:d}: {}"};
      res = std::format(std::runtime_format(footer), idx - 1, message);
    }

    return res;
  };
}

auto Error::display(std::ostream &output, ErrorDisplayOptions options) const -> void {
  if (messages.size() == 1) {
    const Message &first = *messages.begin();
    output << std::format("Error: {}\n", options.locations ? embed(first) : first.message);
    return;
  }

  output << (messages | std::views::reverse | std::views::enumerate |
             std::views::transform(generateStyle(options.locations, options.colours)) |
             std::views::join_with('\n') | std::ranges::to<String>());
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

auto fatal(Error error, ErrorDisplayOptions options) -> void {
  error.display(std::cerr, options);
  std::terminate();
}
auto fatal(StringView message, ErrorDisplayOptions options, std::source_location location) -> void {
  fatal({Error::Message{String{message}, location}}, options);
}

} // namespace Miracle
