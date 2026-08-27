#pragma once

#include <string_view>
#include <vector>

struct BenchmarkSamples {
    std::vector<double> reconcile;
    std::vector<double> style;
    std::vector<double> layout;
    std::vector<double> render_list;
    std::vector<double> ui_total;
    std::vector<double> submit;
    std::vector<double> frame;
    std::vector<double> resident_hide;
    std::vector<double> resident_show;
    double startup_ready_ms{};
};

void print_benchmark_report(const BenchmarkSamples& samples, std::string_view screen, int width,
                            int height);
