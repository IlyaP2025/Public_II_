#include "DrawWidget.h"
#include <QPainter>
#include <QPen>
#include <QMouseEvent>

namespace s21 {
namespace mlp {

// Автоматическое определение фона и инверсия при необходимости
static void NormalizeToEmnistFormat(std::vector<double>& image) {
    double sum = 0.0;
    for (double v : image) sum += v;
    double avg = sum / image.size();
    if (avg > 0.5) {   // фон светлый → нужна инверсия
        for (double& v : image) v = 1.0 - v;
    }
}

DrawWidget::DrawWidget(QWidget *parent) : QWidget(parent) {
    setFixedSize(280, 280);
    canvas_ = QImage(280, 280, QImage::Format_Grayscale8);
    canvas_.fill(Qt::white);   // белый фон (как обычная бумага)
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
    // Увеличили толщину кисти до 20 для лучшего совпадения с EMNIST
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
            pixels[y * 28 + x] = gray / 255.0;
        }
    }
    NormalizeToEmnistFormat(pixels);
    return pixels;
}

} // namespace mlp
} // namespace s21
