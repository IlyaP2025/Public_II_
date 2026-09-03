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

#include "ErrorGraph.h"

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
    void displayMetrics(double accuracy, double precision, double recall, double f1, double timeSec);

    void updateErrorGraph(double train, double valid);

signals:
    void startTraining(double learningRate, int epochs);
    void pauseTraining();
    void stopTraining();
    void applySettings(const std::vector<size_t>& layerSizes);
    void loadBmp();
    void saveWeights();
    void loadWeights();
    void runExperiment(double fraction);
    void classifyDrawn(const std::vector<double>& pixels);
    void implementationChanged(int index);
    void crossValidate(int k);
    void benchmarkRequested(int repetitions);

private:
    void setupUI();
    QTabWidget *tabWidget_;

    // Training
    QDoubleSpinBox *learningRateSpin_;
    QSpinBox *epochsSpin_;
    QPushButton *startBtn_, *pauseBtn_, *stopBtn_;
    QPlainTextEdit *logEdit_;
    ErrorGraph *errorGraph_;
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
    QComboBox *implCombo_;
    QPushButton *crossValBtn_;
    QSpinBox *kSpin_;

    // Experiment
    QPushButton *loadBmpBtn_;
    QLabel *imageLabel_;
    QTextEdit *predictionText_;
    QDoubleSpinBox *testFractionSpin_;
    QPushButton *experimentBtn_;
    QTextEdit *metricsText_;

    void rebuildManualFields();
    void fillLinearDistribution();

    QPushButton *benchmarkBtn_;
    QSpinBox *benchmarkRepsSpin_;    

};

} // namespace mlp
} // namespace s21
#endif // MAINWINDOW_H
