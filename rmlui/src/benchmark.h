#pragma once

#include <chrono>
#include <vector>

using TrialClock = std::chrono::steady_clock;

struct Timings {
  double update_ms = 0.0;
  double render_record_ms = 0.0;
  double submit_ms = 0.0;
  double gpu_complete_ms = 0.0;
  double frame_ms = 0.0;
};

struct BenchmarkSamples {
  std::vector<double> update;
  std::vector<double> render_record;
  std::vector<double> submit;
  std::vector<double> gpu_complete;
  std::vector<double> frame;

  void Add(const Timings &value);
};

double milliseconds(TrialClock::time_point start, TrialClock::time_point end);
long read_rss_kib();
void print_metric(const char *name, std::vector<double> values);
