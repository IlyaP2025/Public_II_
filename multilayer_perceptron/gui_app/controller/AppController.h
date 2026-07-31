#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <vector>
#include <cstddef>
#include <chrono>

#include "view/MainWindow.h"
#include "perceptron/matrix_perceptron.h"
#include "perceptron/graph_perceptron.h"
#include "data/emnist_loader.h"
#include "data/bmp_loader.h"
#include "serialization/simple_weights_serializer.h"
#include "metrics/metrics_calculator.h"

namespace s21 {
namespace mlp {

enum class TrainingState { Idle, Training, Paused };

class AppController : public QObject {
    Q_OBJECT
public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();
    void show();

private slots:
    void onStartTraining(double learningRate, int epochs);
    void onPauseTraining();
    void onStopTraining();
    void onTrainingFinished();
    void onApplySettings(const std::vector<size_t>& layers);
    void onLoadBmp();
    void onSaveWeights();
    void onLoadWeights();
    void onRunExperiment(double fraction);
    void onClassifyDrawn(const std::vector<double>& pixels);
    void onImplementationChanged(int index);

private:
    void setState(TrainingState state);
    void runTraining(double learningRate, int epochs);

    MainWindow *window_;
    TrainingState state_ = TrainingState::Idle;

    std::unique_ptr<MatrixPerceptron> perceptron_;
    EmnistLoader loader_;
    SimpleWeightsSerializer serializer_;

    std::thread trainingThread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> pauseRequested_{false};
    std::mutex pauseMutex_;
    std::condition_variable pauseCV_;
};

} // namespace mlp
} // namespace s21
#endif // APPCONTROLLER_H
