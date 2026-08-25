#include "MainWindow.h"
#include "DrawWidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
#include <QDialog>
#include <cmath>

namespace s21 {
namespace mlp {

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) {
    setWindowTitle("Multilayer Perceptron");
    setupUI();
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);
    tabWidget_ = new QTabWidget(this);
    mainLayout->addWidget(tabWidget_);

    // ================= Вкладка Training =================
    QWidget *trainingTab = new QWidget();
    QVBoxLayout *trainLayout = new QVBoxLayout(trainingTab);

    QGroupBox *paramsGroup = new QGroupBox("Training Parameters");
    QHBoxLayout *paramsLayout = new QHBoxLayout(paramsGroup);
    paramsLayout->addWidget(new QLabel("Learning rate:"));
    learningRateSpin_ = new QDoubleSpinBox();
    learningRateSpin_->setRange(0.001, 1.0);
    learningRateSpin_->setSingleStep(0.01);
    learningRateSpin_->setValue(0.1);
    paramsLayout->addWidget(learningRateSpin_);
    paramsLayout->addWidget(new QLabel("Epochs:"));
    epochsSpin_ = new QSpinBox();
    epochsSpin_->setRange(1, 1000);
    epochsSpin_->setValue(2);
    paramsLayout->addWidget(epochsSpin_);
    trainLayout->addWidget(paramsGroup);

    QHBoxLayout *btnLayout = new QHBoxLayout;
    startBtn_ = new QPushButton("Start");
    pauseBtn_ = new QPushButton("Pause");
    stopBtn_ = new QPushButton("Stop");
    pauseBtn_->setEnabled(false);
    stopBtn_->setEnabled(false);
    btnLayout->addWidget(startBtn_);
    btnLayout->addWidget(pauseBtn_);
    btnLayout->addWidget(stopBtn_);
    trainLayout->addLayout(btnLayout);

    statusLabel_ = new QLabel("Idle");
    trainLayout->addWidget(statusLabel_);

    QHBoxLayout *progressLayout = new QHBoxLayout;
    progressBar_ = new QProgressBar();
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setVisible(false);
    progressLabel_ = new QLabel("");
    progressLayout->addWidget(progressBar_);
    progressLayout->addWidget(progressLabel_);
    trainLayout->addLayout(progressLayout);

    logEdit_ = new QPlainTextEdit();
    logEdit_->setReadOnly(true);
    trainLayout->addWidget(logEdit_);

    errorGraph_ = new ErrorGraph();
    trainLayout->addWidget(errorGraph_);

    tabWidget_->addTab(trainingTab, "Training");

    connect(startBtn_, &QPushButton::clicked, this, [this]() {
        emit startTraining(learningRateSpin_->value(), epochsSpin_->value());
    });
    connect(pauseBtn_, &QPushButton::clicked, this, &MainWindow::pauseTraining);
    connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::stopTraining);

    // ================= Вкладка Experiment =================
    QWidget *experimentTab = new QWidget();
    QVBoxLayout *expLayout = new QVBoxLayout(experimentTab);

    loadBmpBtn_ = new QPushButton("Load BMP");

    QPushButton *drawBtn = new QPushButton("Draw Letter");
    expLayout->addWidget(drawBtn);
    connect(drawBtn, &QPushButton::clicked, this, [this]() {
        QDialog *dlg = new QDialog(this);
        dlg->setWindowTitle("Draw a letter");
        QVBoxLayout *dlgLayout = new QVBoxLayout(dlg);
        DrawWidget *draw = new DrawWidget(dlg);
        dlgLayout->addWidget(draw);
        QPushButton *classifyBtn = new QPushButton("Classify");
        dlgLayout->addWidget(classifyBtn);
        connect(classifyBtn, &QPushButton::clicked, this, [this, draw, dlg]() {
            std::vector<double> pixels = draw->getProcessedImage();
            emit classifyDrawn(pixels);
            dlg->accept();
        });
        dlg->exec();
        delete dlg;
    });

    expLayout->addWidget(loadBmpBtn_);

    imageLabel_ = new QLabel();
    imageLabel_->setFixedSize(140, 140);
    imageLabel_->setStyleSheet("border: 1px solid black;");
    expLayout->addWidget(imageLabel_);

    predictionText_ = new QTextEdit();
    predictionText_->setReadOnly(true);
    predictionText_->setMaximumHeight(150);
    expLayout->addWidget(predictionText_);

    // --- Experiment controls ---
    QHBoxLayout *expCtrlLayout = new QHBoxLayout;
    expCtrlLayout->addWidget(new QLabel("Test fraction:"));
    testFractionSpin_ = new QDoubleSpinBox();
    testFractionSpin_->setRange(0.0, 1.0);
    testFractionSpin_->setSingleStep(0.1);
    testFractionSpin_->setValue(0.2);
    expCtrlLayout->addWidget(testFractionSpin_);
    experimentBtn_ = new QPushButton("Run Experiment");
    expCtrlLayout->addWidget(experimentBtn_);
    expLayout->addLayout(expCtrlLayout);

    metricsText_ = new QTextEdit();
    metricsText_->setReadOnly(true);
    metricsText_->setMaximumHeight(100);
    expLayout->addWidget(metricsText_);

    expLayout->addStretch();
    tabWidget_->addTab(experimentTab, "Experiment");

    connect(loadBmpBtn_, &QPushButton::clicked, this, &MainWindow::loadBmp);
    connect(experimentBtn_, &QPushButton::clicked, this, [this]() {
        emit runExperiment(testFractionSpin_->value());
    });

    // ================= Вкладка Settings =================
    QWidget *settingsTab = new QWidget();
    QVBoxLayout *settLayout = new QVBoxLayout(settingsTab);

    QGroupBox *archGroup = new QGroupBox("Network Architecture");
    QFormLayout *formLayout = new QFormLayout;

    inputSizeSpin_ = new QSpinBox();
    inputSizeSpin_->setRange(1, 10000);
    inputSizeSpin_->setValue(784);
    formLayout->addRow("Input size:", inputSizeSpin_);

    outputSizeSpin_ = new QSpinBox();
    outputSizeSpin_->setRange(1, 1000);
    outputSizeSpin_->setValue(26);
    formLayout->addRow("Output size:", outputSizeSpin_);

    hiddenLayersSpin_ = new QSpinBox();
    hiddenLayersSpin_->setRange(2, 10);
    hiddenLayersSpin_->setValue(2);
    formLayout->addRow("Hidden layers:", hiddenLayersSpin_);

    modeCombo_ = new QComboBox();
    modeCombo_->addItem("Linear", 0);
    modeCombo_->addItem("Manual", 1);
    modeCombo_->addItem("Geometric", 2);
    formLayout->addRow("Mode:", modeCombo_);

    implCombo_ = new QComboBox();
    implCombo_->addItem("Matrix", 0);
    implCombo_->addItem("Graph", 1);
    formLayout->addRow("Implementation:", implCombo_);
    connect(implCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::implementationChanged);

    // Кросс-валидация
    QHBoxLayout *cvLayout = new QHBoxLayout;
    cvLayout->addWidget(new QLabel("K folds:"));
    kSpin_ = new QSpinBox();
    kSpin_->setRange(2, 10);
    kSpin_->setValue(5);
    cvLayout->addWidget(kSpin_);
    crossValBtn_ = new QPushButton("Cross-Validate");
    cvLayout->addWidget(crossValBtn_);
    settLayout->addLayout(cvLayout);
    connect(crossValBtn_, &QPushButton::clicked, this, [this]() {
        emit crossValidate(kSpin_->value());
    });

    archGroup->setLayout(formLayout);
    settLayout->addWidget(archGroup);

    applyBtn_ = new QPushButton("Apply");
    settLayout->addWidget(applyBtn_);

    // Кнопки сохранения / загрузки
    saveBtn_ = new QPushButton("Save Weights");
    loadBtn_ = new QPushButton("Load Weights");
    settLayout->addWidget(saveBtn_);
    settLayout->addWidget(loadBtn_);

    connect(saveBtn_, &QPushButton::clicked, this, &MainWindow::saveWeights);
    connect(loadBtn_, &QPushButton::clicked, this, &MainWindow::loadWeights);

    // --- Контейнер для ручного ввода слоёв (Manual) ---
    manualContainer_ = new QWidget();
    QVBoxLayout *manualLayout = new QVBoxLayout(manualContainer_);
    manualLayout->setContentsMargins(0, 0, 0, 0);

    // --- Контейнер для Geometric ---
    geometricContainer_ = new QWidget();
    QHBoxLayout *geoLayout = new QHBoxLayout(geometricContainer_);
    geoLayout->setContentsMargins(0, 0, 0, 0);
    percentageSpin_ = new QSpinBox();
    percentageSpin_->setRange(10, 90);
    percentageSpin_->setValue(50);
    minLayerSizeSpin_ = new QSpinBox();
    minLayerSizeSpin_->setRange(2, 1000);
    minLayerSizeSpin_->setValue(26);
    geoLayout->addWidget(new QLabel("Compress %:"));
    geoLayout->addWidget(percentageSpin_);
    geoLayout->addWidget(new QLabel("Min size:"));
    geoLayout->addWidget(minLayerSizeSpin_);
    settLayout->addWidget(geometricContainer_);
    geometricContainer_->hide();

    settLayout->addStretch();
    tabWidget_->addTab(settingsTab, "Settings");

    // Переключение режимов
    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, settLayout, settingsTab](int index) {
        // Скрываем все динамические контейнеры
        if (manualContainer_->isVisible()) {
            settLayout->removeWidget(manualContainer_);
            manualContainer_->hide();
        }
        geometricContainer_->hide();

        if (index == 1) { // Manual
            rebuildManualFields();
            if (settLayout->indexOf(manualContainer_) == -1) {
                settLayout->insertWidget(1, manualContainer_);
            }
            manualContainer_->show();
        } else if (index == 2) { // Geometric
            geometricContainer_->show();
        }
        // Linear (0) – ничего не показываем

        settLayout->invalidate();
        settLayout->activate();
        settingsTab->adjustSize();
    });

    // Обновление полей при изменении параметров (только для Manual)
    connect(hiddenLayersSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
        if (modeCombo_->currentIndex() == 1 && manualContainer_->isVisible())
            rebuildManualFields();
    });
    connect(inputSizeSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
        if (modeCombo_->currentIndex() == 1 && manualContainer_->isVisible())
            fillLinearDistribution();
    });
    connect(outputSizeSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
        if (modeCombo_->currentIndex() == 1 && manualContainer_->isVisible())
            fillLinearDistribution();
    });

    modeCombo_->setCurrentIndex(0);

    // Обработчик Apply
    connect(applyBtn_, &QPushButton::clicked, this, [this]() {
        std::vector<size_t> layers;
        layers.push_back(inputSizeSpin_->value());

        if (modeCombo_->currentIndex() == 0) { // Linear
            int input = inputSizeSpin_->value();
            int output = outputSizeSpin_->value();
            int num = hiddenLayersSpin_->value();
            double step = static_cast<double>(input - output) / (num + 1);
            for (int i = 1; i <= num; ++i) {
                int neurons = static_cast<int>(input - step * i + 0.5);
                neurons = std::max(1, neurons);
                layers.push_back(neurons);
            }
        } else if (modeCombo_->currentIndex() == 1) { // Manual
            if (static_cast<int>(manualLayerSpins_.size()) != hiddenLayersSpin_->value()) {
                QMessageBox::warning(this, "Error", "Layer count mismatch.");
                return;
            }
            for (auto* spin : manualLayerSpins_) {
                if (spin->value() <= 0) {
                    QMessageBox::warning(this, "Error", "All layer sizes must be positive.");
                    return;
                }
                layers.push_back(spin->value());
            }
        } else if (modeCombo_->currentIndex() == 2) { // Geometric
            int input = inputSizeSpin_->value();
            int output = outputSizeSpin_->value();
            int percent = percentageSpin_->value();
            int minSize = minLayerSizeSpin_->value();
            int hidden = hiddenLayersSpin_->value();
            int prev = input;
            for (int i = 0; i < hidden; ++i) {
                int neurons = std::max(minSize, prev * percent / 100);
                // Защита: если слой меньше выходного, приравниваем к выходному
                if (neurons < output) {
                    neurons = output;
                    // Заполняем оставшиеся скрытые слои значением output
                    while (i < hidden) {
                        layers.push_back(output);
                        ++i;
                    }
                    break;
                }
                layers.push_back(neurons);
                prev = neurons;
            }
        }

        layers.push_back(outputSizeSpin_->value());
        emit applySettings(layers);
    });
}

// --------------- Вспомогательные методы ---------------
void MainWindow::rebuildManualFields() {
    QLayout *layout = manualContainer_->layout();
    if (layout) {
        QLayoutItem *child;
        while ((child = layout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
    }
    manualLayerSpins_.clear();

    int layers = hiddenLayersSpin_->value();
    for (int i = 1; i <= layers; ++i) {
        QWidget *rowWidget = new QWidget(manualContainer_);
        QHBoxLayout *rowLayout = new QHBoxLayout(rowWidget);
        rowLayout->setContentsMargins(0, 0, 0, 0);

        QLabel *label = new QLabel(QString("Layer %1:").arg(i));
        rowLayout->addWidget(label);

        QSpinBox *spin = new QSpinBox();
        spin->setRange(1, 1024);
        rowLayout->addWidget(spin);

        manualContainer_->layout()->addWidget(rowWidget);
        manualLayerSpins_.append(spin);
    }
    fillLinearDistribution();

    layout->invalidate();
    layout->activate();
    manualContainer_->updateGeometry();
}

void MainWindow::fillLinearDistribution() {
    if (manualLayerSpins_.isEmpty()) return;
    int input = inputSizeSpin_->value();
    int output = outputSizeSpin_->value();
    int num = manualLayerSpins_.size();
    double step = static_cast<double>(input - output) / (num + 1);
    for (int i = 0; i < num; ++i) {
        int neurons = static_cast<int>(input - step * (i + 1) + 0.5);
        neurons = std::max(1, neurons);
        manualLayerSpins_[i]->setValue(neurons);
    }
}

void MainWindow::displayPrediction(const std::vector<double>& pixels,
                                   const std::vector<double>& probs, char letter) {
    QImage img(28, 28, QImage::Format_Grayscale8);
    for (int y = 0; y < 28; ++y) {
        uchar* line = img.scanLine(y);
        for (int x = 0; x < 28; ++x) {
            int val = static_cast<int>(pixels[y * 28 + x] * 255);
            if (val < 0) val = 0;
            if (val > 255) val = 255;
            line[x] = static_cast<uchar>(val);
        }
    }
    QPixmap pixmap = QPixmap::fromImage(img).scaled(140, 140, Qt::KeepAspectRatio, Qt::FastTransformation);
    imageLabel_->setPixmap(pixmap);

    QString probStr = QString("Predicted: %1\n").arg(letter);
    for (int i = 0; i < 26; ++i) {
        probStr += QString("%1: %2%  ").arg(QChar('A' + i)).arg(probs[i] * 100, 0, 'f', 1);
        if ((i + 1) % 5 == 0) probStr += "\n";
    }
    predictionText_->setPlainText(probStr);
}

// Общие публичные методы
void MainWindow::setStatus(const QString &status) { statusLabel_->setText(status); }
void MainWindow::appendLog(const QString &text) { logEdit_->appendPlainText(text); }
void MainWindow::setStartEnabled(bool enabled) { startBtn_->setEnabled(enabled); }
void MainWindow::setPauseEnabled(bool enabled) { pauseBtn_->setEnabled(enabled); }
void MainWindow::setStopEnabled(bool enabled) { stopBtn_->setEnabled(enabled); }

void MainWindow::setProgress(int current, int total) {
    progressBar_->setVisible(true);
    progressLabel_->setVisible(true);
    int percent = total > 0 ? (current * 100 / total) : 0;
    progressBar_->setValue(percent);
    progressLabel_->setText(QString("%1 / %2").arg(current).arg(total));
}

void MainWindow::resetProgress() {
    progressBar_->setVisible(false);
    progressLabel_->setVisible(false);
    progressBar_->setValue(0);
    progressLabel_->clear();
}

void MainWindow::displayMetrics(double accuracy, double precision, double recall, double f1, double timeSec) {
    QString text = QString("Accuracy: %1%\nPrecision: %2%\nRecall: %3%\nF1: %4%\nTime: %5 sec")
        .arg(accuracy * 100, 0, 'f', 2)
        .arg(precision * 100, 0, 'f', 2)
        .arg(recall * 100, 0, 'f', 2)
        .arg(f1 * 100, 0, 'f', 2)
        .arg(timeSec, 0, 'f', 2);
    metricsText_->setPlainText(text);
}

void MainWindow::updateErrorGraph(double train, double valid) {
    if (errorGraph_) {
        errorGraph_->addData(train, valid);
    }
}

} // namespace mlp
} // namespace s21
