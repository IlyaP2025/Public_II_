#ifndef DRAWWIDGET_H
#define DRAWWIDGET_H

#include <QWidget>
#include <QImage>
#include <QPoint>

namespace s21 {
namespace mlp {

class DrawWidget : public QWidget {
    Q_OBJECT
public:
    explicit DrawWidget(QWidget *parent = nullptr);
    std::vector<double> getProcessedImage() const;

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    QImage canvas_;
    bool drawing_;
    QPoint lastPoint_;
    void drawLineTo(const QPoint &end);
};

} // namespace mlp
} // namespace s21
#endif // DRAWWIDGET_H
