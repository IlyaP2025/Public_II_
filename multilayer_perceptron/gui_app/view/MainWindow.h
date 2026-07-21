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

// Режим расчёта размеров скрытых слоёв
enum class LayerSizeMode { Percent, Linear };

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    // Обновление интерфейса из контроллера
    void setStatus(const QString &status);
    void appendLog(const QString &text);

    // Управление кнопками обучения
    void setStartEnabled(bool enabled);
    void setPauseEnabled(bool enabled);
    void setStopEnabled(bool enabled);

    // Индикация прогресса
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

    // --- Вкладка Training ---
    QDoubleSpinBox *learningRateSpin_;
    QSpinBox *epochsSpin_;
    QPushButton *startBtn_;
    QPushButton *pauseBtn_;
    QPushButton *stopBtn_;
    QPlainTextEdit *logEdit_;
    QLabel *statusLabel_;
    QProgressBar *progressBar_;
    QLabel *progressLabel_;

    // --- Вкладка Settings ---
    QSpinBox *inputSizeSpin_;
    QSpinBox *outputSizeSpin_;
    QComboBox *hiddenLayersCombo_;
    QVector<QSpinBox*> neuronSpinBoxes_;

    // Режим расчёта и его параметры
    QComboBox *sizeModeCombo_;
    QSpinBox *percentageSpin_;
    QSpinBox *minLayerSizeSpin_;

    QPushButton *applyBtn_;
};

} // namespace mlp
} // namespace s21
#endif // MAINWINDOW_H
