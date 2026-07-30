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
    const int target_size = 20;          // размер квадрата для масштабированной буквы
    const int margin = 2;                // поля внутри квадрата 20x20

    // 1. Найти bounding box всех значимых пикселей
    int x_min = size, y_min = size, x_max = -1, y_max = -1;
    double total_mass = 0.0;
    double sum_x = 0.0, sum_y = 0.0;
    bool found = false;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            double val = input[y * size + x];
            if (val > threshold) {
                found = true;
                x_min = std::min(x_min, x);
                y_min = std::min(y_min, y);
                x_max = std::max(x_max, x);
                y_max = std::max(y_max, y);
                total_mass += val;
                sum_x += x * val;
                sum_y += y * val;
            }
        }
    }

    if (!found) return input;   // пустое изображение

    // Добавляем по 2 пикселя с каждой стороны (но не выходя за границы)
    x_min = std::max(0, x_min - 2);
    y_min = std::max(0, y_min - 2);
    x_max = std::min(size - 1, x_max + 2);
    y_max = std::min(size - 1, y_max + 2);

    // Центр масс
    double cx = sum_x / total_mass;
    double cy = sum_y / total_mass;

    // Размеры расширенного бокса
    int box_w = x_max - x_min + 1;
    int box_h = y_max - y_min + 1;
    int max_dim = std::max(box_w, box_h);

    // Масштаб: уместить max_dim в target_size - 2*margin
    double scale = static_cast<double>(target_size - 2 * margin) / max_dim;

    // Создаём пустой квадрат target_size x target_size
    std::vector<double> scaled(target_size * target_size, 0.0);

    // Целевой центр в этом квадрате
    double dst_cx = (target_size - 1) / 2.0;
    double dst_cy = (target_size - 1) / 2.0;

    for (int y = 0; y < target_size; ++y) {
        for (int x = 0; x < target_size; ++x) {
            // Отображаем координаты из dst в src
            double src_x = (x - dst_cx) / scale + cx;
            double src_y = (y - dst_cy) / scale + cy;

            if (src_x < 0 || src_x > size - 1 || src_y < 0 || src_y > size - 1)
                continue;

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

            scaled[y * target_size + x] = val;
        }
    }

    // 5. Вставляем квадрат 20x20 в центр холста 28x28
    std::vector<double> result(size * size, 0.0);
    int paste_x = (size - target_size) / 2;
    int paste_y = (size - target_size) / 2;
    for (int y = 0; y < target_size; ++y) {
        for (int x = 0; x < target_size; ++x) {
            result[(paste_y + y) * size + (paste_x + x)] = scaled[y * target_size + x];
        }
    }

    // 6. Min‑max нормализация
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
