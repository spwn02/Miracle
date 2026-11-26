import std;
import Miracle;
import Switch;

using namespace Switch;

inline constexpr RunOptions testOptions{
    .executionMode = ExecutionMode::Diagnostic,
    .threads = 1,
    .retention = RetentionPolicy::Failures,
    .timeMode = TimeMode::Real,
    .traceMode = TraceMode::Annotations,
    .captureTiming = CapturePolicy::PerAttempt,
    .order = ExecutionOrder::Declaration,
    .isolation = CrashIsolation::InProcess,
};

auto main() -> int {
  Reporter reporter{
      ReporterOptions{
          .renderer =
              {
                  .color = ColorMode::Automatic,
                  .terminal = true,
                  .showSource = true,
                  .details = DetailMode::Trace,
              },
          .showPassedTests = false,
          .showAttempts = false,
          .showSummary = true,
      },
  };

  const RunReport report = runAll(reporter, std::cout, TestSelection{}, testOptions);
  return report.passed() ? 0 : 1;
}
