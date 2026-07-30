#include "data/image_normalizer.h"
#include <algorithm>
#include <stdexcept>
#include <cmath>

namespace s21 {
namespace mlp {

namespace {

struct BBox {
    int x_min, y_min, x_max, y_max;
    int width() const { return x_max - x_min + 1; }
    int height() const { return y_max - y_min + 1; }
};

// Бинарный поиск bounding box по порогу
BBox findBBox(const std::vector<double>& img, double threshold) {
    BBox box{27, 27, 0, 0};
    bool found = false;
    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            if (img[y * 28 + x] > threshold) {
                found = true;
                if (x < box.x_min) box.x_min = x;
                if (y < box.y_min) box.y_min = y;
                if (x > box.x_max) box.x_max = x;
                if (y > box.y_max) box.y_max = y;
            }
        }
    }
    if (!found) {
        // Если нет значащих пикселей – вернуть нулевой box (вся область)
        return {0, 0, 27, 27};
    }
    return box;
}

// Сдвиг изображения (билинейная интерполяция)
std::vector<double> translate(const std::vector<double>& src, double dx, double dy) {
    std::vector<double> dst(28 * 28, 0.0);
    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            double src_x = x - dx;
            double src_y = y - dy;
            if (src_x < 0 || src_x >= 27 || src_y < 0 || src_y >= 27) continue;
            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = std::min(x0 + 1, 27);
            int y1 = std::min(y0 + 1, 27);
            double fx = src_x - x0;
            double fy = src_y - y0;
            dst[y * 28 + x] = 
                src[y0 * 28 + x0] * (1 - fx) * (1 - fy) +
                src[y1 * 28 + x0] * fy * (1 - fx) +
                src[y0 * 28 + x1] * fx * (1 - fy) +
                src[y1 * 28 + x1] * fx * fy;
        }
    }
    return dst;
}

// Масштабирование внутри холста 28x28 (только пиксели, попавшие в новый bounding box)
void scaleToBox(std::vector<double>& img, const BBox& srcBox, int targetSize) {
    std::vector<double> temp(28 * 28, 0.0);
    double scaleX = targetSize / static_cast<double>(srcBox.width());
    double scaleY = targetSize / static_cast<double>(srcBox.height());
    double scale = std::min(scaleX, scaleY);  // сохраняем пропорции

    for (int y = 0; y < targetSize; ++y) {
        for (int x = 0; x < targetSize; ++x) {
            double src_x = srcBox.x_min + x / scale;
            double src_y = srcBox.y_min + y / scale;
            if (src_x < 0 || src_x >= 27 || src_y < 0 || src_y >= 27) continue;
            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = std::min(x0 + 1, 27);
            int y1 = std::min(y0 + 1, 27);
            double fx = src_x - x0;
            double fy = src_y - y0;
            double val = 
                img[y0 * 28 + x0] * (1 - fx) * (1 - fy) +
                img[y1 * 28 + x0] * fy * (1 - fx) +
                img[y0 * 28 + x1] * fx * (1 - fy) +
                img[y1 * 28 + x1] * fx * fy;
            // Размещаем в центре холста
            int dst_x = (28 - targetSize) / 2 + x;
            int dst_y = (28 - targetSize) / 2 + y;
            temp[dst_y * 28 + dst_x] = val;
        }
    }
    img = std::move(temp);
}

// Min‑max нормализация
void minMaxNormalize(std::vector<double>& img) {
    double minVal = 1.0, maxVal = 0.0;
    for (double v : img) {
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
    }
    if (maxVal - minVal < 1e-6) return;   // все пиксели одинаковые
    for (double& v : img) {
        v = (v - minVal) / (maxVal - minVal);
    }
}

}  // anonymous namespace

std::vector<double> ImageNormalizer::Normalize(const std::vector<double>& input) {
    if (input.size() != 784) {
        throw std::invalid_argument("Input must be 784 elements");
    }

    // 1. Найти bounding box (порог 0.1)
    BBox box = findBBox(input, 0.1);
    if (box.width() <= 2 || box.height() <= 2) {
        // Буква слишком мала – возвращаем исходное изображение без изменений
        return input;
    }

    // 2. Сдвинуть центр масс bounding box в (14,14)
    double cx = (box.x_min + box.x_max) / 2.0;
    double cy = (box.y_min + box.y_max) / 2.0;
    double dx = cx - 14.0;
    double dy = cy - 14.0;
    std::vector<double> shifted = translate(input, dx, dy);

    // 3. Пересчитать bounding box после сдвига (может немного измениться)
    BBox shiftedBox = findBBox(shifted, 0.1);

    // 4. Масштабировать содержимое до 20x20 и вписать в центр
    const int targetSize = 20;
    scaleToBox(shifted, shiftedBox, targetSize);

    // 5. Min‑max нормализация
    minMaxNormalize(shifted);

    return shifted;
}

}  // namespace mlp
}  // namespace s21
