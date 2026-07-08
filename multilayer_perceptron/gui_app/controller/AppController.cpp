#include "AppController.h"
#include <QApplication>
#include <iostream>
#include <iomanip>

namespace s21 {
namespace mlp {

AppController::AppController(QObject *parent) : QObject(parent) {
    window_ = new MainWindow();
    window_->show();

    connect(window_, &MainWindow::startTraining, this, &AppController::onStartTraining);
    connect(window_, &MainWindow::pauseTraining, this, &AppController::onPauseTraining);
    connect(window_, &MainWindow::stopTraining, this, &AppController::onStopTraining);

    perceptron_ = std::make_unique<MatrixPerceptron>(std::vector<size_t>{784, 128, 26});
}

AppController::~AppController() {
    stopRequested_ = true;
    pauseCV_.notify_all();
    if (trainingThread_.joinable()) trainingThread_.join();
}

void AppController::show() { window_->show(); }

void AppController::setState(TrainingState state) {
    state_ = state;
    switch (state) {
    case TrainingState::Idle:
        window_->setStatus("Idle");
        window_->setStartEnabled(true);
        window_->setPauseEnabled(false);
        window_->setStopEnabled(false);
        break;
    case TrainingState::Training:
        window_->setStatus("Training...");
        window_->setStartEnabled(false);
        window_->setPauseEnabled(true);
        window_->setStopEnabled(true);
        break;
    case TrainingState::Paused:
        window_->setStatus("Paused");
        window_->setStartEnabled(false);
        window_->setPauseEnabled(false);
        window_->setStopEnabled(true);
        break;
    }
}

void AppController::onStartTraining(double learningRate, int epochs) {
    if (state_ == TrainingState::Training) return;
    if (state_ == TrainingState::Paused) {
        pauseRequested_ = false;
        pauseCV_.notify_all();
        setState(TrainingState::Training);
        return;
    }
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
        pauseRequested_ = false;
        pauseCV_.notify_all();
        setState(TrainingState::Idle);
        if (trainingThread_.joinable()) trainingThread_.join();
    }
}

void AppController::onTrainingFinished() {
    setState(TrainingState::Idle);
    if (trainingThread_.joinable()) trainingThread_.join();
}

void AppController::runTraining(double learningRate, int epochs) {
    trainingThread_ = std::thread([this, learningRate, epochs]() {
        QMetaObject::invokeMethod(window_, [this]() {
            window_->appendLog("Loading EMNIST dataset...");
        }, Qt::QueuedConnection);

        auto [train, test] = loader_.Load("../../datasets/emnist-letters-train.csv", 0.2);

        QMetaObject::invokeMethod(window_, [this, train_size = train.size(), test_size = test.size()]() {
            window_->appendLog(QString("Train: %1, Test: %2").arg(train_size).arg(test_size));
        }, Qt::QueuedConnection);

        SimpleTrainer trainer(learningRate, epochs, true);

        auto onEpoch = [this](int epoch, double trainLoss, double validLoss) {
            if (stopRequested_) return;
            std::unique_lock<std::mutex> lock(pauseMutex_);
            pauseCV_.wait(lock, [this]() { return !pauseRequested_ || stopRequested_; });
            if (stopRequested_) return;
            QMetaObject::invokeMethod(window_, [this, epoch, trainLoss, validLoss]() {
                window_->appendLog(QString("Epoch %1: train=%2 valid=%3")
                    .arg(epoch).arg(trainLoss, 0, 'f', 6).arg(validLoss, 0, 'f', 6));
            }, Qt::QueuedConnection);
        };

        trainer.Train(*perceptron_, train, test, onEpoch);
        QMetaObject::invokeMethod(this, &AppController::onTrainingFinished, Qt::QueuedConnection);
    });
}

} // namespace mlp
} // namespace s21
