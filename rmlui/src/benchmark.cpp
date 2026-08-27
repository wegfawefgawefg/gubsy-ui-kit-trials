#include "benchmark.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <numeric>
#include <string>

void BenchmarkSamples::Add(const Timings &value) {
  // collect one frame sample
  update.push_back(value.update_ms);
  render_record.push_back(value.render_record_ms);
  submit.push_back(value.submit_ms);
  gpu_complete.push_back(value.gpu_complete_ms);
  frame.push_back(value.frame_ms);
}

double milliseconds(TrialClock::time_point start, TrialClock::time_point end) {
  return std::chrono::duration<double, std::milli>(end - start).count();
}

long read_rss_kib() {
  // read resident process memory
  std::ifstream status("/proc/self/status");
  std::string key;
  while (status >> key) {
    if (key == "VmRSS:") {
      long value = 0;
      status >> value;
      return value;
    }
    std::string ignored;
    std::getline(status, ignored);
  }
  return -1;
}

void print_metric(const char *name, std::vector<double> values) {
  // summarize sorted frame samples
  if (values.empty())
    return;
  std::sort(values.begin(), values.end());
  const double mean = std::accumulate(values.begin(), values.end(), 0.0) /
                      static_cast<double>(values.size());
  const size_t p50 = static_cast<size_t>((values.size() - 1) * 0.50);
  const size_t p95 = static_cast<size_t>((values.size() - 1) * 0.95);
  const size_t p99 = static_cast<size_t>((values.size() - 1) * 0.99);
  std::printf("  \"%s\": {\"mean_ms\": %.4f, \"p50_ms\": %.4f, "
              "\"p95_ms\": %.4f, \"p99_ms\": %.4f, \"max_ms\": %.4f}",
              name, mean, values[p50], values[p95], values[p99], values.back());
}
