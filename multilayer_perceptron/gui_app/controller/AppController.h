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

#include "view/MainWindow.h"
#include "perceptron/matrix_perceptron.h"
#include "data/emnist_loader.h"
#include "data/bmp_loader.h"

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
    void onLoadBmp();   // обработка запроса загрузки BMP

private:
    void setState(TrainingState state);
    void runTraining(double learningRate, int epochs);

    MainWindow *window_;
    TrainingState state_ = TrainingState::Idle;

    std::unique_ptr<MatrixPerceptron> perceptron_;
    EmnistLoader loader_;

    std::thread trainingThread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> pauseRequested_{false};
    std::mutex pauseMutex_;
    std::condition_variable pauseCV_;
};

} // namespace mlp
} // namespace s21
#endif // APPCONTROLLER_H
