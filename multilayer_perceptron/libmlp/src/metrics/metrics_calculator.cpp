#include "metrics/metrics_calculator.h"
#include <stdexcept>
#include <vector>

namespace s21 {
namespace mlp {

Metrics MetricsCalculator::Calculate(const std::vector<int>& predicted,
                                     const std::vector<int>& actual) {
    if (predicted.size() != actual.size() || predicted.empty()) {
        throw std::invalid_argument("Vectors must be non-empty and equal size");
    }

    size_t total = predicted.size();
    size_t correct = 0;
    int num_classes = 26;   // EMNIST letters
    std::vector<int> tp(num_classes, 0), fp(num_classes, 0), fn(num_classes, 0);

    for (size_t i = 0; i < total; ++i) {
        int pred = predicted[i];
        int act = actual[i];
        if (pred == act) {
            ++correct;
            ++tp[pred];
        } else {
            ++fp[pred];
            ++fn[act];
        }
    }

    double avg_prec = 0.0, avg_rec = 0.0, avg_f1 = 0.0;
    for (int c = 0; c < num_classes; ++c) {
        int tp_c = tp[c], fp_c = fp[c], fn_c = fn[c];
        double prec = (tp_c + fp_c) > 0 ? static_cast<double>(tp_c) / (tp_c + fp_c) : 0.0;
        double rec = (tp_c + fn_c) > 0 ? static_cast<double>(tp_c) / (tp_c + fn_c) : 0.0;
        double f1 = (prec + rec) > 0 ? 2.0 * prec * rec / (prec + rec) : 0.0;
        avg_prec += prec;
        avg_rec += rec;
        avg_f1 += f1;
    }
    avg_prec /= num_classes;
    avg_rec /= num_classes;
    avg_f1 /= num_classes;

    double accuracy = static_cast<double>(correct) / total;

    return { accuracy, avg_prec, avg_rec, avg_f1 };
}

} // namespace mlp
} // namespace s21
