#include "AppController.h"
#include <QApplication>
#include <QFileDialog>
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
    connect(window_, &MainWindow::loadBmp, this, &AppController::onLoadBmp);
    connect(window_, &MainWindow::saveWeights, this, &AppController::onSaveWeights);
    connect(window_, &MainWindow::loadWeights, this, &AppController::onLoadWeights);
    connect(window_, &MainWindow::runExperiment, this, &AppController::onRunExperiment);
    connect(window_, &MainWindow::classifyDrawn, this, &AppController::onClassifyDrawn);
    connect(window_, &MainWindow::implementationChanged, this, &AppController::onImplementationChanged);
    connect(window_, &MainWindow::crossValidate, this, &AppController::onCrossValidate);

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

void AppController::onLoadBmp() {
    if (state_ != TrainingState::Idle) {
        QMetaObject::invokeMethod(window_, [this]() {
            window_->appendLog("Cannot load BMP while training.");
        }, Qt::QueuedConnection);
        return;
    }

    QString fileName = QFileDialog::getOpenFileName(window_, "Open BMP", "", "BMP Files (*.bmp)");
    if (fileName.isEmpty()) return;

    try {
        std::vector<double> pixels = BmpLoader::LoadImage(fileName.toStdString());
        std::vector<double> output = perceptron_->Predict(pixels);

        int classIdx = std::max_element(output.begin(), output.end()) - output.begin();
        char letter = 'A' + classIdx;

        QMetaObject::invokeMethod(window_, [this, pixels, output, letter]() {
            window_->displayPrediction(pixels, output, letter);
        }, Qt::QueuedConnection);
    } catch (const std::exception& e) {
        QMetaObject::invokeMethod(window_, [this, msg = std::string(e.what())]() {
            window_->appendLog(QString("BMP load error: %1").arg(QString::fromStdString(msg)));
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

            std::random_device rd;
            std::mt19937 g(rd());
            const int reportInterval = 5000;

            for (int epoch = 0; epoch < epochs && !stopRequested_; ++epoch) {
                std::shuffle(train.begin(), train.end(), g);
                int total = static_cast<int>(train.size());

                for (int i = 0; i < total && !stopRequested_; ++i) {
                    std::unique_lock<std::mutex> lock(pauseMutex_);
                    pauseCV_.wait(lock, [this]() { return !pauseRequested_ || stopRequested_; });
                    if (stopRequested_) break;

                    const auto& sample = train[i];
                    perceptron_->Forward(sample.first);
                    perceptron_->Backward(sample.second);
                    perceptron_->UpdateWeights(learningRate);

                    if ((i + 1) % reportInterval == 0 || (i + 1) == total) {
                        int current = i + 1;
                        QMetaObject::invokeMethod(window_, [this, current, total]() {
                            window_->setProgress(current, total);
                        }, Qt::QueuedConnection);
                    }
                }

                if (stopRequested_) break;

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

void AppController::onSaveWeights() {
    if (state_ != TrainingState::Idle) {
        window_->appendLog("Cannot save while training.");
        return;
    }
    const auto& layers = perceptron_->LayerSizes();
    QString defaultName = "weights";
    for (size_t i = 0; i < layers.size(); ++i) {
        defaultName += "_" + QString::number(layers[i]);
    }
    defaultName += ".txt";

    QString filename = QFileDialog::getSaveFileName(window_, "Save Weights", defaultName, "Text Files (*.txt)");
    if (filename.isEmpty()) return;
    try {
        auto weights = perceptron_->GetWeights();
        serializer_.Save(filename.toStdString(), weights);
        window_->appendLog("Weights saved to " + filename);
    } catch (const std::exception& e) {
        window_->appendLog(QString("Save error: %1").arg(e.what()));
    }
}

void AppController::onLoadWeights() {
    if (state_ != TrainingState::Idle) {
        window_->appendLog("Cannot load while training.");
        return;
    }
    QString filename = QFileDialog::getOpenFileName(window_, "Load Weights", "weights.txt", "Text Files (*.txt)");
    if (filename.isEmpty()) return;
    try {
        auto weights = serializer_.Load(filename.toStdString());
        perceptron_->SetWeights(weights);
        window_->appendLog("Weights loaded from " + filename);
    } catch (const std::exception& e) {
        window_->appendLog(QString("Load error: %1").arg(e.what()));
    }
}

void AppController::onRunExperiment(double fraction) {
    if (state_ != TrainingState::Idle) {
        window_->appendLog("Cannot run experiment while training.");
        return;
    }

    window_->appendLog("Running experiment...");
    window_->setStatus("Experiment...");
    window_->setStartEnabled(false);   // блокируем кнопку Start и, возможно, саму кнопку Run Experiment

    std::thread([this, fraction]() {
        try {
            auto start = std::chrono::steady_clock::now();

            auto [train, test] = loader_.Load("datasets/emnist-letters-train.csv", 0.2);
            std::vector<DataSample> test_set = test;

            if (fraction < 1.0) {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(test_set.begin(), test_set.end(), g);
                size_t newSize = static_cast<size_t>(test_set.size() * fraction);
                if (newSize == 0) newSize = 1;
                test_set.resize(newSize);
            }

            std::vector<int> predicted, actual;
            for (const auto& sample : test_set) {
                auto out = perceptron_->Predict(sample.first);
                int pred = std::max_element(out.begin(), out.end()) - out.begin();
                int act = std::max_element(sample.second.begin(), sample.second.end()) - sample.second.begin();
                predicted.push_back(pred);
                actual.push_back(act);
            }

            Metrics m = MetricsCalculator::Calculate(predicted, actual);
            auto end = std::chrono::steady_clock::now();
            double elapsed = std::chrono::duration<double>(end - start).count();

            QMetaObject::invokeMethod(window_, [this, m, elapsed]() {
                window_->displayMetrics(m.accuracy, m.precision, m.recall, m.f1, elapsed);
                window_->appendLog(QString("Experiment finished in %1 sec").arg(elapsed, 0, 'f', 2));
                window_->setStatus("Idle");
                window_->setStartEnabled(true);
            }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            QMetaObject::invokeMethod(window_, [this, msg = std::string(e.what())]() {
                window_->appendLog(QString("Experiment error: %1").arg(QString::fromStdString(msg)));
                window_->setStatus("Idle");
                window_->setStartEnabled(true);
            }, Qt::QueuedConnection);
        }
    }).detach();
}

void AppController::onClassifyDrawn(const std::vector<double>& pixels) {
    if (state_ != TrainingState::Idle) {
        window_->appendLog("Cannot classify while training.");
        return;
    }
    try {
        std::vector<double> output = perceptron_->Predict(pixels);
        int classIdx = std::max_element(output.begin(), output.end()) - output.begin();
        char letter = 'A' + classIdx;
        QMetaObject::invokeMethod(window_, [this, pixels, output, letter]() {
            window_->displayPrediction(pixels, output, letter);
        }, Qt::QueuedConnection);
    } catch (const std::exception& e) {
        QMetaObject::invokeMethod(window_, [this, msg = std::string(e.what())]() {
            window_->appendLog(QString("Draw error: %1").arg(QString::fromStdString(msg)));
        }, Qt::QueuedConnection);
    }
}

void AppController::onImplementationChanged(int index) {
    if (state_ != TrainingState::Idle) {
        window_->appendLog("Cannot change implementation while training.");
        return;
    }
    // Получаем текущую архитектуру из существующего перцептрона
    std::vector<size_t> layers = perceptron_->LayerSizes();
    if (index == 0) {
        perceptron_ = std::make_unique<MatrixPerceptron>(layers);
        window_->appendLog("Switched to Matrix implementation.");
    } else {
        perceptron_ = std::make_unique<GraphPerceptron>(layers);
        window_->appendLog("Switched to Graph implementation.");
    }
}

void AppController::onCrossValidate(int k) {
    if (state_ != TrainingState::Idle) {
        window_->appendLog("Cannot run cross-validation while training.");
        return;
    }
    window_->appendLog(QString("Starting %1-fold cross-validation...").arg(k));
    window_->setStartEnabled(false);   // блокируем кнопки

    // Загружаем полный датасет (без разделения)
    std::thread([this, k]() {
        try {
            auto full = loader_.LoadFile("datasets/emnist-letters-train.csv");
            SimpleTrainer trainer(0.1, 2, true);   // можно взять текущие параметры из GUI, но упростим
            double avgAcc = trainer.CrossValidate(*perceptron_, full, k);

            QMetaObject::invokeMethod(window_, [this, avgAcc]() {
                window_->appendLog(QString("Cross-validation complete. Average accuracy: %1%")
                    .arg(avgAcc * 100, 0, 'f', 2));
                window_->setStartEnabled(true);
            }, Qt::QueuedConnection);
        } catch (const std::exception& e) {
            QMetaObject::invokeMethod(window_, [this, msg = std::string(e.what())]() {
                window_->appendLog(QString("Cross-validation error: %1").arg(QString::fromStdString(msg)));
                window_->setStartEnabled(true);
            }, Qt::QueuedConnection);
        }
    }).detach();
}

} // namespace mlp
} // namespace s21
