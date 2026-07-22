#include "serialization/simple_weights_serializer.h"
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace s21 {
namespace mlp {

void SimpleWeightsSerializer::Save(const std::string& filename,
                                   const std::vector<std::vector<double>>& weights) {
    std::ofstream file(filename);
    if (!file) throw std::runtime_error("Cannot open file for writing: " + filename);
    for (const auto& layer : weights) {
        for (size_t i = 0; i < layer.size(); ++i) {
            file << layer[i];
            if (i < layer.size() - 1) file << " ";
        }
        file << "\n";
    }
}

std::vector<std::vector<double>> SimpleWeightsSerializer::Load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) throw std::runtime_error("Cannot open file for reading: " + filename);
    std::vector<std::vector<double>> weights;
    std::string line;
    while (std::getline(file, line)) {
        std::stringstream ss(line);
        std::vector<double> layer;
        double val;
        while (ss >> val) layer.push_back(val);
        if (!layer.empty()) weights.push_back(std::move(layer));
    }
    return weights;
}

} // namespace mlp
} // namespace s21
