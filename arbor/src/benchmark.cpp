#include "benchmark.hpp"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <numeric>
#include <string>

namespace {

long read_rss_kib() {
    std::ifstream status{"/proc/self/status"};
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

void print_metric(std::string_view name, std::vector<double> values) {
    if (values.empty())
        return;
    std::sort(values.begin(), values.end());
    const double mean =
        std::accumulate(values.begin(), values.end(), 0.0) / static_cast<double>(values.size());
    const auto at = [&](double fraction) {
        return values[static_cast<std::size_t>((values.size() - 1) * fraction)];
    };
    std::printf("  \"%.*s\": {\"mean_ms\": %.4f, \"p50_ms\": %.4f, "
                "\"p95_ms\": %.4f, \"p99_ms\": %.4f, \"max_ms\": %.4f},\n",
                static_cast<int>(name.size()), name.data(), mean, at(0.50), at(0.95), at(0.99),
                values.back());
}

} // namespace

void print_benchmark_report(const BenchmarkSamples& samples, std::string_view screen, int width,
                            int height) {
    // Report Arbor phases, lifecycle latency, and the whole-process envelope.
    std::printf("{\n  \"backend\": \"Arbor + SDL_GPU\",\n");
    std::printf("  \"screen\": \"%.*s\",\n", static_cast<int>(screen.size()), screen.data());
    std::printf("  \"viewport\": \"%dx%d\",\n", width, height);
    std::printf("  \"frames\": %zu,\n", samples.frame.size());
    std::printf("  \"workload\": \"one visible binding mutation per frame\",\n");
    std::printf("  \"startup_ready_ms\": %.4f,\n", samples.startup_ready_ms);
    print_metric("reconcile", samples.reconcile);
    print_metric("style", samples.style);
    print_metric("layout", samples.layout);
    print_metric("render_list", samples.render_list);
    print_metric("ui_total", samples.ui_total);
    print_metric("submit", samples.submit);
    print_metric("frame", samples.frame);
    print_metric("resident_hide", samples.resident_hide);
    print_metric("resident_show", samples.resident_show);
    std::printf("  \"rss_kib\": %ld\n}\n", read_rss_kib());
}
