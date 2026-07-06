#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLabel>

namespace s21 {
namespace mlp {

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

    // Методы для обновления интерфейса из контроллера
    void setStatus(const QString &status);
    void appendLog(const QString &text);
    void setTrainingEnabled(bool enabled);

signals:
    void startTraining(double learningRate, int epochs);
    void pauseTraining();
    void stopTraining();

private:
    void setupUI();

    // Элементы управления
    QDoubleSpinBox *learningRateSpin_;
    QSpinBox *epochsSpin_;
    QPushButton *startBtn_;
    QPushButton *pauseBtn_;
    QPushButton *stopBtn_;
    QPlainTextEdit *logEdit_;
    QLabel *statusLabel_;
};

}  // namespace mlp
}  // namespace s21

#endif  // MAINWINDOW_H
