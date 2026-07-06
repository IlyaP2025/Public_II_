#include "AppController.h"
#include <QApplication>
#include <iostream>
#include <iomanip>

namespace s21 {
namespace mlp {

AppController::AppController(QObject *parent)
    : QObject(parent) {
    window_ = new MainWindow();
    window_->show();

    // Соединения сигналов окна с контроллером
    connect(window_, &MainWindow::startTraining, this, &AppController::onStartTraining);
    connect(window_, &MainWindow::pauseTraining, this, &AppController::onPauseTraining);
    connect(window_, &MainWindow::stopTraining, this, &AppController::onStopTraining);

    // Инициализация перцептрона (архитектура по умолчанию)
    perceptron_ = std::make_unique<MatrixPerceptron>(std::vector<size_t>{784, 128, 26});
}

AppController::~AppController() {
    stopRequested_ = true;
    if (trainingThread_.joinable()) {
        trainingThread_.join();
    }
}

void AppController::show() {
    window_->show();
}

void AppController::setState(TrainingState state) {
    state_ = state;
    switch (state) {
    case TrainingState::Idle:
        window_->setStatus("Idle");
        window_->setTrainingEnabled(true);
        break;
    case TrainingState::Training:
        window_->setStatus("Training...");
        window_->setTrainingEnabled(false);
        break;
    case TrainingState::Paused:
        window_->setStatus("Paused");
        window_->setTrainingEnabled(false);
        // В этом состоянии startBtn уже disabled, pauseBtn disabled, stopBtn active
        window_->pauseBtn_->setEnabled(false);   // пауза невозможна, пока на паузе
        window_->startBtn_->setEnabled(false);
        window_->stopBtn_->setEnabled(true);
        break;
    }
}

void AppController::onStartTraining(double learningRate, int epochs) {
    if (state_ == TrainingState::Training) return;  // уже идёт
    if (state_ == TrainingState::Paused) {
        // Возобновить
        pauseRequested_ = false;
        pauseCV_.notify_all();
        setState(TrainingState::Training);
        return;
    }
    // Запустить новое обучение
    setState(TrainingState::Training);
    runTraining(learningRate, epochs);
}

void AppController::onPauseTraining() {
    if (state_ == TrainingState::Training) {
        pauseRequested_ = true;
        setState(TrainingState::Paused);
    }
}

void AppController::onStopTraining() {
    if (state_ == TrainingState::Training || state_ == TrainingState::Paused) {
        stopRequested_ = true;
        pauseRequested_ = false;  // чтобы поток не завис на паузе
        pauseCV_.notify_all();
        setState(TrainingState::Idle);
        if (trainingThread_.joinable()) {
            trainingThread_.join();
        }
    }
}

void AppController::onTrainingFinished() {
    setState(TrainingState::Idle);
    if (trainingThread_.joinable()) {
        trainingThread_.join();
    }
}

void AppController::runTraining(double learningRate, int epochs) {
    // Запуск обучения в отдельном потоке
    trainingThread_ = std::thread([this, learningRate, epochs]() {
        // Загрузка данных (может занять время, можно вынести в отдельный шаг)
        window_->appendLog("Loading EMNIST dataset...");
        auto [train, test] = loader_.Load("../../datasets/emnist-letters-train.csv", 0.2);
        window_->appendLog(QString("Train size: %1, Test size: %2").arg(train.size()).arg(test.size()));

        // Создаём тренер
        SimpleTrainer trainer(learningRate, epochs, true);

        // Колбэк после каждой эпохи (вызывается в потоке обучения)
        auto onEpoch = [this](int epoch, double trainLoss, double validLoss) {
            if (stopRequested_) return;
            // Проверка паузы
            std::unique_lock<std::mutex> lock(pauseMutex_);
            pauseCV_.wait(lock, [this] { return !pauseRequested_ || stopRequested_; });
            if (stopRequested_) return;

            // Отправляем информацию в GUI
            QMetaObject::invokeMethod(window_, [this, epoch, trainLoss, validLoss]() {
                window_->appendLog(QString("Epoch %1: train_loss=%2 valid_loss=%3")
                                      .arg(epoch)
                                      .arg(trainLoss, 0, 'f', 6)
                                      .arg(validLoss, 0, 'f', 6));
            }, Qt::QueuedConnection);
        };

        // Запускаем обучение (блокирующий вызов)
        trainer.Train(*perceptron_, train, test, onEpoch);

        // По завершении (или остановке) сигналим контроллеру
        QMetaObject::invokeMethod(this, &AppController::onTrainingFinished, Qt::QueuedConnection);
    });
}

}  // namespace mlp
}  // namespace s21
