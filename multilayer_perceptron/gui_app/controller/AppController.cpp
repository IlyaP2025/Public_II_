#include "AppController.h"
#include <QApplication>
#include <iostream>
#include <iomanip>
#include <random>
#include <algorithm>

namespace s21 {
namespace mlp {

AppController::AppController(QObject *parent) : QObject(parent) {
    window_ = new MainWindow();
    window_->show();

    connect(window_, &MainWindow::startTraining, this, &AppController::onStartTraining);
    connect(window_, &MainWindow::pauseTraining, this, &AppController::onPauseTraining);
    connect(window_, &MainWindow::stopTraining, this, &AppController::onStopTraining);
    connect(window_, &MainWindow::applySettings, this, &AppController::onApplySettings);

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
        window_->resetProgress();
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

void AppController::onApplySettings(const std::vector<size_t>& layers) {
    if (state_ != TrainingState::Idle) {
        QMetaObject::invokeMethod(window_, [this]() {
            window_->appendLog("Cannot change architecture while training.");
        }, Qt::QueuedConnection);
        return;
    }
    // Пересоздаём перцептрон
    try {
        perceptron_ = std::make_unique<MatrixPerceptron>(layers);
        QMetaObject::invokeMethod(window_, [this, layers]() {
            QString msg = "Architecture updated: ";
            for (size_t i = 0; i < layers.size(); ++i) {
                msg += QString::number(layers[i]);
                if (i < layers.size() - 1) msg += " -> ";
            }
            window_->appendLog(msg);
        }, Qt::QueuedConnection);
    } catch (const std::exception& e) {
        QMetaObject::invokeMethod(window_, [this, msg = std::string(e.what())]() {
            window_->appendLog(QString("Error: %1").arg(QString::fromStdString(msg)));
        }, Qt::QueuedConnection);
    }
}

void AppController::runTraining(double learningRate, int epochs) {
    trainingThread_ = std::thread([this, learningRate, epochs]() {
        try {
            QMetaObject::invokeMethod(window_, [this]() {
                window_->appendLog("Loading EMNIST dataset...");
            }, Qt::QueuedConnection);

            auto [train, test] = loader_.Load("datasets/emnist-letters-train.csv", 0.2);

            QMetaObject::invokeMethod(window_, [this, train_size = train.size(), test_size = test.size()]() {
                window_->appendLog(QString("Train: %1, Test: %2").arg(train_size).arg(test_size));
            }, Qt::QueuedConnection);

            // Ручной цикл обучения с прогрессом
            std::random_device rd;
            std::mt19937 g(rd());
            const int reportInterval = 5000; // обновление прогресса каждые 5000 примеров

            for (int epoch = 0; epoch < epochs && !stopRequested_; ++epoch) {
                std::shuffle(train.begin(), train.end(), g);
                int total = static_cast<int>(train.size());

                for (int i = 0; i < total && !stopRequested_; ++i) {
                    // Обработка паузы
                    std::unique_lock<std::mutex> lock(pauseMutex_);
                    pauseCV_.wait(lock, [this]() { return !pauseRequested_ || stopRequested_; });
                    if (stopRequested_) break;

                    const auto& sample = train[i];
                    perceptron_->Forward(sample.first);
                    perceptron_->Backward(sample.second);
                    perceptron_->UpdateWeights(learningRate);

                    // Обновление прогресса
                    if ((i + 1) % reportInterval == 0 || (i + 1) == total) {
                        int current = i + 1;
                        QMetaObject::invokeMethod(window_, [this, current, total]() {
                            window_->setProgress(current, total);
                        }, Qt::QueuedConnection);
                    }
                }

                if (stopRequested_) break;

                // Расчёт ошибки после эпохи
                double trainLoss = 0.0;
                for (const auto& s : train) {
                    auto out = perceptron_->Predict(s.first);
                    for (size_t k = 0; k < out.size(); ++k) {
                        double err = out[k] - s.second[k];
                        trainLoss += err * err;
                    }
                }
                trainLoss /= train.size();

                double validLoss = 0.0;
                for (const auto& s : test) {
                    auto out = perceptron_->Predict(s.first);
                    for (size_t k = 0; k < out.size(); ++k) {
                        double err = out[k] - s.second[k];
                        validLoss += err * err;
                    }
                }
                validLoss /= test.size();

                QMetaObject::invokeMethod(window_, [this, epoch, trainLoss, validLoss]() {
                    window_->appendLog(QString("Epoch %1: train=%2 valid=%3")
                        .arg(epoch).arg(trainLoss, 0, 'f', 6).arg(validLoss, 0, 'f', 6));
                }, Qt::QueuedConnection);
            }

            QMetaObject::invokeMethod(this, &AppController::onTrainingFinished, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            QMetaObject::invokeMethod(window_, [this, msg = std::string(e.what())]() {
                window_->appendLog(QString("Error: %1").arg(QString::fromStdString(msg)));
                setState(TrainingState::Idle);
            }, Qt::QueuedConnection);
        }
    });
}

} // namespace mlp
} // namespace s21
