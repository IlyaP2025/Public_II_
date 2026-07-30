#include "data/image_normalizer.h"
#include "DrawWidget.h"
#include <QPainter>
#include <QPen>
#include <QMouseEvent>

namespace s21 {
namespace mlp {

DrawWidget::DrawWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(280, 280);
    canvas_ = QImage(280, 280, QImage::Format_Grayscale8);
    canvas_.fill(Qt::white);
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
    QImage scaled = canvas_.scaled(28, 28, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    std::vector<double> pixels(28 * 28);
    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            int gray = qGray(scaled.pixel(x, y));
            // Инверсия: белый фон -> 0, чёрный рисунок -> 1
            pixels[y * 28 + x] = 1.0 - gray / 255.0;
        }
    }
    return pixels;
}

std::vector<double> DrawWidget::getProcessedImage() const {
    QImage scaled = canvas_.scaled(28, 28, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    std::vector<double> pixels(28 * 28);
    for (int y = 0; y < 28; ++y) {
        for (int x = 0; x < 28; ++x) {
            int gray = qGray(scaled.pixel(x, y));
            pixels[y * 28 + x] = gray / 255.0;   // чёрный фон = 0, белая буква = 1
        }
    }
    // Нормализация: центрирование, масштабирование, контраст
    return ImageNormalizer::Normalize(pixels);
}

} // namespace mlp
} // namespace s21
