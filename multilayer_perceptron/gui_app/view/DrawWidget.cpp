#include "DrawWidget.h"
#include <QPainter>
#include <QPen>
#include <QMouseEvent>
#include <vector>
#include <algorithm>
#include <cmath>

namespace s21 {
namespace mlp {

DrawWidget::DrawWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(280, 280);
    canvas_ = QImage(280, 280, QImage::Format_Grayscale8);
    canvas_.fill(Qt::white);   // белый фон, чёрная кисть
    drawing_ = false;
}

void DrawWidget::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.drawImage(0, 0, canvas_);
    painter.setPen(Qt::black);
    painter.drawRect(0, 0, width()-1, height()-1);
}

void DrawWidget::mousePressEvent(QMouseEvent *event) {
    if (event->button() == Qt::LeftButton) {
        drawing_ = true;
        lastPoint_ = event->pos();
        drawLineTo(lastPoint_);
    }
}

void DrawWidget::mouseMoveEvent(QMouseEvent *event) {
    if (drawing_) {
        drawLineTo(event->pos());
    }
}

void DrawWidget::mouseReleaseEvent(QMouseEvent *) {
    drawing_ = false;
}

void DrawWidget::drawLineTo(const QPoint &end) {
    QPainter painter(&canvas_);
    // Уменьшили толщину до 12 для более тонких линий
    painter.setPen(QPen(Qt::black, 12, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(lastPoint_, end);
    lastPoint_ = end;
    update();
}

std::vector<double> DrawWidget::getProcessedImage() const {
    const int srcSize = 280;
    const int dstSize = 28;
    const int padding = 30;   // отступ вокруг буквы

    // 1. Копируем холст в матрицу 0..255
    std::vector<uint8_t> src(srcSize * srcSize);
    for (int y = 0; y < srcSize; ++y) {
        const uchar* line = canvas_.scanLine(y);
        for (int x = 0; x < srcSize; ++x) {
            src[y * srcSize + x] = line[x];
        }
    }

    // 2. Инверсия, если фон светлый (средняя > 128)
    double avg = 0.0;
    for (int v : src) avg += v;
    avg /= src.size();
    if (avg > 128.0) {
        for (uint8_t& v : src) {
            v = 255 - v;
        }
    }

    // 3. Находим bounding box белых пикселей (>128)
    int x_min = srcSize, y_min = srcSize, x_max = -1, y_max = -1;
    for (int y = 0; y < srcSize; ++y) {
        for (int x = 0; x < srcSize; ++x) {
            if (src[y * srcSize + x] > 128) {
                x_min = std::min(x_min, x);
                y_min = std::min(y_min, y);
                x_max = std::max(x_max, x);
                y_max = std::max(y_max, y);
            }
        }
    }

    // Если буква не найдена – чёрный холст
    if (x_max < 0) {
        return std::vector<double>(dstSize * dstSize, 0.0);
    }

    // 4. Вырезаем прямоугольник с отступами
    x_min = std::max(0, x_min - padding);
    y_min = std::max(0, y_min - padding);
    x_max = std::min(srcSize - 1, x_max + padding);
    y_max = std::min(srcSize - 1, y_max + padding);

    int crop_w = x_max - x_min + 1;
    int crop_h = y_max - y_min + 1;

    std::vector<uint8_t> crop(crop_w * crop_h);
    for (int y = 0; y < crop_h; ++y) {
        for (int x = 0; x < crop_w; ++x) {
            crop[y * crop_w + x] = src[(y_min + y) * srcSize + (x_min + x)];
        }
    }

    // 5. Добавляем поля до квадрата
    int max_dim = std::max(crop_w, crop_h);
    std::vector<uint8_t> square(max_dim * max_dim, 0);
    int paste_x = (max_dim - crop_w) / 2;
    int paste_y = (max_dim - crop_h) / 2;
    for (int y = 0; y < crop_h; ++y) {
        for (int x = 0; x < crop_w; ++x) {
            square[(paste_y + y) * max_dim + (paste_x + x)] = crop[y * crop_w + x];
        }
    }

    // 6. Масштабируем квадрат до 28×28 (билинейная интерполяция)
    std::vector<double> scaled(dstSize * dstSize, 0.0);
    double scale = static_cast<double>(dstSize) / max_dim;
    for (int y = 0; y < dstSize; ++y) {
        for (int x = 0; x < dstSize; ++x) {
            double src_x = x / scale;
            double src_y = y / scale;
            if (src_x < 0 || src_y < 0 || src_x >= max_dim - 1 || src_y >= max_dim - 1)
                continue;
            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = std::min(x0 + 1, max_dim - 1);
            int y1 = std::min(y0 + 1, max_dim - 1);
            double fx = src_x - x0;
            double fy = src_y - y0;
            double val =
                square[y0 * max_dim + x0] * (1 - fx) * (1 - fy) +
                square[y1 * max_dim + x0] * fy * (1 - fx) +
                square[y0 * max_dim + x1] * fx * (1 - fy) +
                square[y1 * max_dim + x1] * fx * fy;
            scaled[y * dstSize + x] = val / 255.0;
        }
    }

    // 7. Лёгкое размытие (2×2) для смягчения краёв
    std::vector<double> blurred = scaled;
    for (int y = 0; y < dstSize - 1; ++y) {
        for (int x = 0; x < dstSize - 1; ++x) {
            double sum = scaled[y * dstSize + x] + scaled[(y+1) * dstSize + x] +
                         scaled[y * dstSize + (x+1)] + scaled[(y+1) * dstSize + (x+1)];
            blurred[y * dstSize + x] = sum / 4.0;
        }
    }

    return blurred;
}

} // namespace mlp
} // namespace s21
