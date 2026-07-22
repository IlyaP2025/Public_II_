#include "data/bmp_loader.h"
#include <fstream>
#include <stdexcept>
#include <cstring>
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace s21 {
namespace mlp {

#pragma pack(push, 1)
struct BmpHeader {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
};
struct BmpInfoHeader {
    uint32_t size;
    int32_t width;
    int32_t height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t size_image;
    int32_t x_ppm;
    int32_t y_ppm;
    uint32_t clr_used;
    uint32_t clr_important;
};
#pragma pack(pop)

std::vector<double> BmpLoader::LoadImage(const std::string& path) {
    std::ifstream file(path, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Cannot open BMP file: " + path);
    }

    BmpHeader header;
    file.read(reinterpret_cast<char*>(&header), sizeof(header));
    if (header.type != 0x4D42) {   // "BM"
        throw std::runtime_error("Not a valid BMP file");
    }

    BmpInfoHeader info;
    file.read(reinterpret_cast<char*>(&info), sizeof(info));
    if (info.bit_count != 24) {
        throw std::runtime_error("Only 24-bit BMP is supported");
    }

    file.seekg(header.offset, std::ios::beg);

    int width = info.width;
    int height = info.height;
    int row_padded = (width * 3 + 3) & (~3);
    std::vector<uint8_t> row(row_padded);

    std::vector<uint8_t> pixels(width * height);
    for (int y = 0; y < height; ++y) {
        file.read(reinterpret_cast<char*>(row.data()), row_padded);
        uint8_t* src = row.data();
        uint8_t* dst = pixels.data() + (height - 1 - y) * width;
        for (int x = 0; x < width; ++x) {
            uint8_t b = src[0];
            uint8_t g = src[1];
            uint8_t r = src[2];
            *dst++ = static_cast<uint8_t>(0.299 * r + 0.587 * g + 0.114 * b);
            src += 3;
        }
    }

    // Масштабирование до 28x28 (без инверсии)
    std::vector<double> image(28 * 28);
    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            double src_x = (x + 0.5) * width / 28.0 - 0.5;
            double src_y = (y + 0.5) * height / 28.0 - 0.5;
            if (src_x < 0) src_x = 0;
            if (src_y < 0) src_y = 0;

            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = std::min(x0 + 1, width - 1);
            int y1 = std::min(y0 + 1, height - 1);

            double dx = src_x - x0;
            double dy = src_y - y0;

            double val = 
                pixels[y0 * width + x0] * (1 - dx) * (1 - dy) +
                pixels[y1 * width + x0] * dy * (1 - dx) +
                pixels[y0 * width + x1] * dx * (1 - dy) +
                pixels[y1 * width + x1] * dx * dy;

            image[y * 28 + x] = val / 255.0;   // простая нормализация
        }
    }

    return image;
}

}  // namespace mlp
}  // namespace s21
