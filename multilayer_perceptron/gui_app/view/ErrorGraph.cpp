#include "ErrorGraph.h"
#include <QPainter>
#include <QPen>
#include <algorithm>
#include <cmath>

namespace s21 {
namespace mlp {

ErrorGraph::ErrorGraph(QWidget *parent)
    : QWidget(parent), maxTrain_(0.0), maxValid_(0.0) {
    setMinimumSize(400, 200);
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
}

void ErrorGraph::addData(double trainError, double validError) {
    trainErrors_.push_back(trainError);
    validErrors_.push_back(validError);
    if (trainError > maxTrain_) maxTrain_ = trainError;
    if (validError > maxValid_) maxValid_ = validError;
    update();   // перерисовать
}

void ErrorGraph::clear() {
    trainErrors_.clear();
    validErrors_.clear();
    maxTrain_ = maxValid_ = 0.0;
    update();
}

void ErrorGraph::paintEvent(QPaintEvent *) {
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), Qt::white);

    if (trainErrors_.empty()) return;

    double globalMax = std::max(maxTrain_, maxValid_);
    double yScale = height() / (globalMax * 1.1);   // 10% запас сверху
    double xScale = width() / (double)(trainErrors_.size() - 1);

    // Рисуем сетку
    painter.setPen(QPen(Qt::lightGray, 1, Qt::DotLine));
    int numXLines = 5;
    for (int i = 0; i <= numXLines; ++i) {
        int y = height() - (i * height() / numXLines);
        painter.drawLine(0, y, width(), y);
    }

    // Рисуем кривую train (синий)
    painter.setPen(QPen(Qt::blue, 2));
    for (int i = 1; i < trainErrors_.size(); ++i) {
        QPointF p1((i-1) * xScale, height() - trainErrors_[i-1] * yScale);
        QPointF p2(i * xScale, height() - trainErrors_[i] * yScale);
        painter.drawLine(p1, p2);
    }

    // Рисуем кривую valid (красный)
    painter.setPen(QPen(Qt::red, 2));
    for (int i = 1; i < validErrors_.size(); ++i) {
        QPointF p1((i-1) * xScale, height() - validErrors_[i-1] * yScale);
        QPointF p2(i * xScale, height() - validErrors_[i] * yScale);
        painter.drawLine(p1, p2);
    }

    // Легенда
    painter.setPen(Qt::blue);
    painter.drawText(10, 20, "Train");
    painter.setPen(Qt::red);
    painter.drawText(80, 20, "Valid");
}

} // namespace mlp
} // namespace s21
