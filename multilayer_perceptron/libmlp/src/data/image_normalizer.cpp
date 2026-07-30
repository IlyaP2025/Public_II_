#include "data/image_normalizer.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace s21 {
namespace mlp {

std::vector<double> ImageNormalizer::Normalize(const std::vector<double>& input) {
    if (input.size() != 784) {
        throw std::invalid_argument("Input must be 784 elements");
    }

    const int size = 28;
    const double threshold = 0.1;

    // 1. Найти bounding box всех пикселей, яркость которых выше порога
    int x_min = size, y_min = size, x_max = -1, y_max = -1;
    bool found = false;
    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            if (input[y * size + x] > threshold) {
                found = true;
                x_min = std::min(x_min, x);
                y_min = std::min(y_min, y);
                x_max = std::max(x_max, x);
                y_max = std::max(y_max, y);
            }
        }
    }

    // Если буква не найдена – вернуть исходное изображение
    if (!found) return input;

    // Добавляем по 2 пикселя с каждой стороны (если возможно)
    x_min = std::max(0, x_min - 2);
    y_min = std::max(0, y_min - 2);
    x_max = std::min(size - 1, x_max + 2);
    y_max = std::min(size - 1, y_max + 2);

    int box_w = x_max - x_min + 1;
    int box_h = y_max - y_min + 1;

    // 2. Вырезать содержимое bounding box с билинейной интерполяцией в новый квадрат 20x20
    const int target_size = 20;
    std::vector<double> scaled(target_size * target_size, 0.0);

    double scale_x = static_cast<double>(target_size) / box_w;
    double scale_y = static_cast<double>(target_size) / box_h;
    double scale = std::min(scale_x, scale_y);   // сохраняем пропорции

    int new_w = static_cast<int>(box_w * scale);
    int new_h = static_cast<int>(box_h * scale);
    // Центрируем в квадрате 20x20
    int offset_x = (target_size - new_w) / 2;
    int offset_y = (target_size - new_h) / 2;

    for (int y = 0; y < new_h; ++y) {
        for (int x = 0; x < new_w; ++x) {
            double src_x = x_min + x / scale;
            double src_y = y_min + y / scale;
            // Билинейная интерполяция
            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = std::min(x0 + 1, size - 1);
            int y1 = std::min(y0 + 1, size - 1);
            double fx = src_x - x0;
            double fy = src_y - y0;
            double val =
                input[y0 * size + x0] * (1 - fx) * (1 - fy) +
                input[y1 * size + x0] * fy * (1 - fx) +
                input[y0 * size + x1] * fx * (1 - fy) +
                input[y1 * size + x1] * fx * fy;
            scaled[(offset_y + y) * target_size + (offset_x + x)] = val;
        }
    }

    // 3. Вписать результат в центр холста 28x28 (чёрный фон)
    std::vector<double> result(size * size, 0.0);
    int paste_x = (size - target_size) / 2;
    int paste_y = (size - target_size) / 2;
    for (int y = 0; y < target_size; ++y) {
        for (int x = 0; x < target_size; ++x) {
            result[(paste_y + y) * size + (paste_x + x)] = scaled[y * target_size + x];
        }
    }

    // 4. Min‑max нормализация
    double min_val = 1.0, max_val = 0.0;
    for (double v : result) {
        if (v < min_val) min_val = v;
        if (v > max_val) max_val = v;
    }
    if (max_val > min_val) {
        for (double& v : result) {
            v = (v - min_val) / (max_val - min_val);
        }
    }

    return result;
}

}  // namespace mlp
}  // namespace s21
