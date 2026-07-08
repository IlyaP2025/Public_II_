#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>

namespace s21 {
namespace mlp {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    // Общие методы для обновления интерфейса
    void setStatus(const QString &status);
    void appendLog(const QString &text);

    // Управление кнопками
    void setStartEnabled(bool enabled);
    void setPauseEnabled(bool enabled);
    void setStopEnabled(bool enabled);

    // Прогресс обучения
    void setProgress(int current, int total);
    void resetProgress();

signals:
    void startTraining(double learningRate, int epochs);
    void pauseTraining();
    void stopTraining();
    void applySettings(const std::vector<size_t>& layerSizes);

private:
    void setupUI();
    QTabWidget *tabWidget_;

    // Вкладка Training
    QDoubleSpinBox *learningRateSpin_;
    QSpinBox *epochsSpin_;
    QPushButton *startBtn_;
    QPushButton *pauseBtn_;
    QPushButton *stopBtn_;
    QPlainTextEdit *logEdit_;
    QLabel *statusLabel_;
    QProgressBar *progressBar_;
    QLabel *progressLabel_;

    // Вкладка Settings
    QComboBox *hiddenLayersCombo_;        // количество скрытых слоёв (2–5)
    QVector<QSpinBox*> neuronSpinBoxes_;  // поля для числа нейронов
    QCheckBox *usePercentageCheck_;       // использовать проценты?
    QSpinBox *percentageSpin_;            // процент сжатия
    QPushButton *applyBtn_;
};

} // namespace mlp
} // namespace s21
#endif // MAINWINDOW_H
