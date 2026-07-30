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
    painter.setPen(QPen(Qt::black, 20, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.drawLine(lastPoint_, end);
    lastPoint_ = end;
    update();
}

std::vector<double> DrawWidget::getProcessedImage() const {
    const int srcSize = 280;
    const int dstSize = 28;
    const int targetSize = 20;          // размер квадрата для масштабированной буквы
    const int margin = 2;               // отступ внутри квадрата 20x20

    // 1. Переводим холст в одномерный массив 0..255 (белый=255, чёрный=0)
    std::vector<uint8_t> srcPixels(srcSize * srcSize);
    for (int y = 0; y < srcSize; ++y) {
        const uchar* line = canvas_.scanLine(y);
        for (int x = 0; x < srcSize; ++x) {
            srcPixels[y * srcSize + x] = line[x];
        }
    }

    // 2. Ищем bounding box буквы (пиксели темнее 250 → не фон)
    int x_min = srcSize, y_min = srcSize, x_max = -1, y_max = -1;
    for (int y = 0; y < srcSize; ++y) {
        for (int x = 0; x < srcSize; ++x) {
            if (srcPixels[y * srcSize + x] < 250) {   // не белый
                x_min = std::min(x_min, x);
                y_min = std::min(y_min, y);
                x_max = std::max(x_max, x);
                y_max = std::max(y_max, y);
            }
        }
    }

    // Если ничего не нарисовано, возвращаем чёрный холст
    if (x_max < 0) {
        return std::vector<double>(dstSize * dstSize, 0.0);
    }

    // Добавляем небольшой отступ (4 пикселя), но не выходим за границы
    const int padding = 4;
    x_min = std::max(0, x_min - padding);
    y_min = std::max(0, y_min - padding);
    x_max = std::min(srcSize - 1, x_max + padding);
    y_max = std::min(srcSize - 1, y_max + padding);

    int box_w = x_max - x_min + 1;
    int box_h = y_max - y_min + 1;

    // 3. Масштабируем содержимое box в квадрат 20x20, сохраняя пропорции
    std::vector<double> scaled(targetSize * targetSize, 0.0);
    double scale = std::min(static_cast<double>(targetSize - 2*margin) / box_w,
                            static_cast<double>(targetSize - 2*margin) / box_h);
    int new_w = static_cast<int>(box_w * scale);
    int new_h = static_cast<int>(box_h * scale);
    int offset_x = (targetSize - new_w) / 2;
    int offset_y = (targetSize - new_h) / 2;

    for (int y = 0; y < new_h; ++y) {
        for (int x = 0; x < new_w; ++x) {
            double src_x = x_min + x / scale;
            double src_y = y_min + y / scale;
            // Билинейная интерполяция по исходному холсту 280x280
            int x0 = static_cast<int>(src_x);
            int y0 = static_cast<int>(src_y);
            int x1 = std::min(x0 + 1, srcSize - 1);
            int y1 = std::min(y0 + 1, srcSize - 1);
            double fx = src_x - x0;
            double fy = src_y - y0;
            double val =
                srcPixels[y0 * srcSize + x0] * (1 - fx) * (1 - fy) +
                srcPixels[y1 * srcSize + x0] * fy * (1 - fx) +
                srcPixels[y0 * srcSize + x1] * fx * (1 - fy) +
                srcPixels[y1 * srcSize + x1] * fx * fy;
            scaled[(offset_y + y) * targetSize + (offset_x + x)] = val / 255.0;
        }
    }

    // 4. Вставляем квадрат 20x20 в центр холста 28x28 (чёрный фон)
    std::vector<double> result(dstSize * dstSize, 0.0);
    int paste_x = (dstSize - targetSize) / 2;
    int paste_y = (dstSize - targetSize) / 2;
    for (int y = 0; y < targetSize; ++y) {
        for (int x = 0; x < targetSize; ++x) {
            result[(paste_y + y) * dstSize + (paste_x + x)] = scaled[y * targetSize + x];
        }
    }

    // 5. Авто‑инверсия: если фон светлый, делаем инверсию (чтобы фон стал 0)
    double sum = 0;
    for (double v : result) sum += v;
    if (sum / result.size() > 0.5) {
        for (double& v : result) v = 1.0 - v;
    }

    return result;
}

} // namespace mlp
} // namespace s21
