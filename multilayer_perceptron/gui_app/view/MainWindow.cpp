#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>
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

    tabWidget_->addTab(trainingTab, "Training");

    connect(startBtn_, &QPushButton::clicked, this, [this]() {
        emit startTraining(learningRateSpin_->value(), epochsSpin_->value());
    });
    connect(pauseBtn_, &QPushButton::clicked, this, &MainWindow::pauseTraining);
    connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::stopTraining);

    // ================= Вкладка Experiment =================
    QWidget *experimentTab = new QWidget();
    QVBoxLayout *expLayout = new QVBoxLayout(experimentTab);
    expLayout->addWidget(new QLabel("Experiment tab – coming soon."));
    tabWidget_->addTab(experimentTab, "Experiment");

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
    formLayout->addRow("Mode:", modeCombo_);

    archGroup->setLayout(formLayout);
    settLayout->addWidget(archGroup);          // группа архитектуры

    // Кнопка Apply
    applyBtn_ = new QPushButton("Apply");
    settLayout->addWidget(applyBtn_);
    settLayout->addStretch();
    tabWidget_->addTab(settingsTab, "Settings");

    // --- Контейнер для ручного ввода слоёв (создаём, но НЕ добавляем в лейаут) ---
    manualContainer_ = new QWidget();
    QVBoxLayout *manualLayout = new QVBoxLayout(manualContainer_);
    manualLayout->setContentsMargins(0, 0, 0, 0);

    // Переключение режима: вставляем/удаляем контейнер в settLayout
    connect(modeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this, settLayout, settingsTab](int index) {
        if (index == 1) {   // Manual
            rebuildManualFields();                  // создаём поля
            // Вставляем контейнер сразу после archGroup (индекс 1 в settLayout)
            if (settLayout->indexOf(manualContainer_) == -1) {
                settLayout->insertWidget(1, manualContainer_);
            }
            manualContainer_->show();
        } else {            // Linear
            settLayout->removeWidget(manualContainer_);
            manualContainer_->hide();
        }
        settingsTab->adjustSize();
    });

    // Обновление полей при изменении параметров (только если контейнер видим)
    connect(hiddenLayersSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
        if (manualContainer_->isVisible()) rebuildManualFields();
    });
    connect(inputSizeSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
        if (manualContainer_->isVisible()) fillLinearDistribution();
    });
    connect(outputSizeSpin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]() {
        if (manualContainer_->isVisible()) fillLinearDistribution();
    });

    // Инициализация – Linear (контейнер скрыт)
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
        } else { // Manual
            if (static_cast<int>(manualLayerSpins_.size()) != hiddenLayersSpin_->value()) {
                QMessageBox::warning(this, "Error", "Layer count mismatch. Please re-apply.");
                return;
            }
            for (auto* spin : manualLayerSpins_) {
                if (spin->value() <= 0) {
                    QMessageBox::warning(this, "Error", "All layer sizes must be positive.");
                    return;
                }
                layers.push_back(spin->value());
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
        QHBoxLayout *row = new QHBoxLayout;
        row->addWidget(new QLabel(QString("Layer %1:").arg(i)));
        QSpinBox *spin = new QSpinBox();
        spin->setRange(1, 1024);
        row->addWidget(spin);
        manualContainer_->layout()->addItem(row);
        manualLayerSpins_.append(spin);
    }
    fillLinearDistribution();
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

// Публичные методы для контроллера
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

} // namespace mlp
} // namespace s21
