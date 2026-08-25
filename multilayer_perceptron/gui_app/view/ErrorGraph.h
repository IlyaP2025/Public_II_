#ifndef ERRORGRAPH_H
#define ERRORGRAPH_H

#include <QWidget>
#include <QVector>
#include <QPointF>
#include <QColor>

namespace s21 {
namespace mlp {

class ErrorGraph : public QWidget {
    Q_OBJECT
public:
    explicit ErrorGraph(QWidget *parent = nullptr);

    // Добавить новую точку (train_error, valid_error)
    void addData(double trainError, double validError);

    // Очистить график
    void clear();

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QVector<double> trainErrors_;
    QVector<double> validErrors_;
    double maxTrain_;
    double maxValid_;

    void recalcBounds();
};

} // namespace mlp
} // namespace s21
#endif // ERRORGRAPH_H
