#ifndef MLP_METRICS_CALCULATOR_H
#define MLP_METRICS_CALCULATOR_H

#include <vector>

namespace s21 {
namespace mlp {

struct Metrics {
    double accuracy;
    double precision;
    double recall;
    double f1;
};

class MetricsCalculator {
public:
    static Metrics Calculate(const std::vector<int>& predicted,
                             const std::vector<int>& actual);
};

} // namespace mlp
} // namespace s21
#endif
