#ifndef APPCONTROLLER_H
#define APPCONTROLLER_H

#include <QObject>
#include <memory>
#include <thread>
#include <atomic>

#include "view/MainWindow.h"
#include "perceptron/matrix_perceptron.h"
#include "data/emnist_loader.h"
#include "trainer/simple_trainer.h"

namespace s21 {
namespace mlp {

// Состояния конечного автомата обучения
enum class TrainingState {
    Idle,
    Training,
    Paused
};

class AppController : public QObject {
    Q_OBJECT

public:
    explicit AppController(QObject *parent = nullptr);
    ~AppController();

    void show();   // показывает главное окно

private slots:
    void onStartTraining(double learningRate, int epochs);
    void onPauseTraining();
    void onStopTraining();
    void onTrainingFinished();

private:
    void setState(TrainingState state);
    void runTraining(double learningRate, int epochs);

    MainWindow *window_;
    TrainingState state_ = TrainingState::Idle;

    // Модельные объекты
    std::unique_ptr<MatrixPerceptron> perceptron_;
    EmnistLoader loader_;
    SimpleTrainer *trainer_;  // будет создаваться при старте

    // Поток для обучения
    std::thread trainingThread_;
    std::atomic<bool> stopRequested_{false};
    std::atomic<bool> pauseRequested_{false};
    std::mutex pauseMutex_;
    std::condition_variable pauseCV_;
};

}  // namespace mlp
}  // namespace s21

#endif  // APPCONTROLLER_H
