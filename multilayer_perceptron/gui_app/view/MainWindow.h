#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QPlainTextEdit>
#include <QTextEdit>
#include <QLabel>
#include <QProgressBar>
#include <QTabWidget>
#include <QComboBox>

namespace s21 {
namespace mlp {

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);

    void setStatus(const QString &status);
    void appendLog(const QString &text);
    void setStartEnabled(bool enabled);
    void setPauseEnabled(bool enabled);
    void setStopEnabled(bool enabled);
    void setProgress(int current, int total);
    void resetProgress();

    void displayPrediction(const std::vector<double>& pixels,
                           const std::vector<double>& probs, char letter);

signals:
    void startTraining(double learningRate, int epochs);
    void pauseTraining();
    void stopTraining();
    void applySettings(const std::vector<size_t>& layerSizes);
    void loadBmp();
    void saveWeights();
    void loadWeights();

private:
    void setupUI();
    QTabWidget *tabWidget_;

    // Training
    QDoubleSpinBox *learningRateSpin_;
    QSpinBox *epochsSpin_;
    QPushButton *startBtn_, *pauseBtn_, *stopBtn_;
    QPlainTextEdit *logEdit_;
    QLabel *statusLabel_;
    QProgressBar *progressBar_;
    QLabel *progressLabel_;

    // Settings
    QSpinBox *inputSizeSpin_;
    QSpinBox *outputSizeSpin_;
    QSpinBox *hiddenLayersSpin_;
    QComboBox *modeCombo_;
    QWidget *manualContainer_;
    QWidget *geometricContainer_;
    QSpinBox *percentageSpin_;  
    QSpinBox *minLayerSizeSpin_;   
    QVector<QSpinBox*> manualLayerSpins_;
    QPushButton *applyBtn_;
    QPushButton *saveBtn_;
    QPushButton *loadBtn_;

    // Experiment
    QPushButton *loadBmpBtn_;
    QLabel *imageLabel_;
    QTextEdit *predictionText_;

    void rebuildManualFields();
    void fillLinearDistribution();
};

} // namespace mlp
} // namespace s21
#endif // MAINWINDOW_H
