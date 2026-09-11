import std;
import Miracle;

using namespace Miracle;

enum class[[= diagnostics::prefix("P")]] PanicProbeCode : u8 {
  Boom = 9,
};

class ThrowOnceBuffer final : public std::streambuf {
public:
  explicit ThrowOnceBuffer(std::streambuf *next)
      : next_(next) {
  }

protected:
  auto xsputn(const char *text, std::streamsize count) -> std::streamsize override {
    if (throw_) {
      throw_ = false;
      throw std::ios_base::failure{"forced panic render failure"};
    }
    return next_->sputn(text, count);
  }

  auto overflow(int value) -> int override {
    if (throw_) {
      throw_ = false;
      throw std::ios_base::failure{"forced panic render failure"};
    }
    if (value == traits_type::eof()) {
      return traits_type::not_eof(value);
    }
    return next_->sputc(static_cast<char>(value));
  }

  auto sync() -> int override {
    return next_->pubsync();
  }

private:
  std::streambuf *next_{};
  bool throw_{true};
};

[[nodiscard]] auto parseMode(int argc, char **argv) -> int {
  if (argc <= 1) {
    return 0;
  }

  // The C main ABI exposes argv as a pointer array; this single indexed read is
  // the bounded access established by argc above.
  // NOLINTNEXTLINE(cppcoreguidelines-pro-bounds-pointer-arithmetic)
  const StringView argument{argv[1]};
  int mode{};
  const auto result = std::from_chars(argument.begin(), argument.end(), mode);
  if (std::make_error_code(result.ec) or result.ptr != argument.end()) {
    return -1;
  }
  return mode;
}

// This subprocess fixture deliberately invokes allocating diagnostic builders;
// allocation failure follows the process-level test harness policy.
// NOLINTNEXTLINE(bugprone-exception-escape)
auto main(int argc, char **argv) -> int {
  const int mode = parseMode(argc, argv);
  const PanicOptions options{
      .render =
          {
              .color = ColorMode::Never,
              .source = SourceMode::None,
              .detail = DetailMode::Compact,
              .presentation = Presentation::Plain,
          },
      .trace = TraceMode::None,
      .breakpoint = false,
  };

  if (mode == 0) {
    panic("string panic", options);
  }
  if (mode == 1) {
    Error error{Error::Message{"error panic"}};
    panic(error, options);
  }
  if (mode == 2) {
    panic(Diagnostic::error(PanicProbeCode::Boom).message("diagnostic panic"), options);
  }
  if (mode == 3) {
    auto traced = options;
    traced.render.detail = DetailMode::Normal;
    traced.trace = TraceMode::Current;
    panic("traced panic", traced);
  }

  ThrowOnceBuffer buffer{std::cerr.rdbuf()};
  std::cerr.rdbuf(&buffer);
  std::cerr.exceptions(std::ios::badbit);
  panic("fallback panic", options);
}
