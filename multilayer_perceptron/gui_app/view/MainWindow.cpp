#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFormLayout>
#include <QMessageBox>

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

    // Входной и выходной размеры
    inputSizeSpin_ = new QSpinBox();
    inputSizeSpin_->setRange(1, 10000);
    inputSizeSpin_->setValue(784);
    formLayout->addRow("Input size:", inputSizeSpin_);

    outputSizeSpin_ = new QSpinBox();
    outputSizeSpin_->setRange(1, 1000);
    outputSizeSpin_->setValue(26);
    formLayout->addRow("Output size:", outputSizeSpin_);

    // Число скрытых слоёв
    hiddenLayersCombo_ = new QComboBox();
    for (int i = 2; i <= 5; ++i) {
        hiddenLayersCombo_->addItem(QString::number(i), i);
    }
    formLayout->addRow("Hidden layers:", hiddenLayersCombo_);

    // Контейнер для полей Layer 1..N (изначально пустые)
    QWidget *neuronsWidget = new QWidget();
    QVBoxLayout *neuronsLayout = new QVBoxLayout(neuronsWidget);
    neuronsLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->addRow(neuronsWidget);

    auto rebuildNeuronFields = [this, neuronsLayout]() {
        // Очищаем старые поля
        QLayoutItem *child;
        while ((child = neuronsLayout->takeAt(0)) != nullptr) {
            delete child->widget();
            delete child;
        }
        neuronSpinBoxes_.clear();
        int layers = hiddenLayersCombo_->currentData().toInt();
        for (int i = 1; i <= layers; ++i) {
            QHBoxLayout *row = new QHBoxLayout;
            row->addWidget(new QLabel(QString("Layer %1 neurons:").arg(i)));
            QSpinBox *spin = new QSpinBox();
            spin->setRange(1, 1024);
            spin->setValue(0);          // 0 означает "не задано"
            spin->setSpecialValueText(""); // пустой текст при 0
            row->addWidget(spin);
            neuronsLayout->addLayout(row);
            neuronSpinBoxes_.append(spin);
        }
    };

    connect(hiddenLayersCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, rebuildNeuronFields);
    rebuildNeuronFields(); // начальное заполнение

    archGroup->setLayout(formLayout);
    settLayout->addWidget(archGroup);

    applyBtn_ = new QPushButton("Apply");
    settLayout->addWidget(applyBtn_);
    settLayout->addStretch();
    tabWidget_->addTab(settingsTab, "Settings");

    // Обработчик кнопки Apply
    connect(applyBtn_, &QPushButton::clicked, this, [this]() {
        std::vector<size_t> layers;
        layers.push_back(inputSizeSpin_->value());

        bool allEmpty = true;
        for (auto* spin : neuronSpinBoxes_) {
            if (spin->value() > 0) {
                allEmpty = false;
                break;
            }
        }

        if (allEmpty) {
            // Автоматический линейный расчёт
            int input = inputSizeSpin_->value();
            int output = outputSizeSpin_->value();
            int num = neuronSpinBoxes_.size();
            if (num > 0) {
                double step = static_cast<double>(input - output) / (num + 1);
                for (int i = 0; i < num; ++i) {
                    int neurons = static_cast<int>(input - step * (i + 1) + 0.5);
                    neurons = std::max(1, neurons);
                    neuronSpinBoxes_[i]->setValue(neurons);
                }
            }
            // Повторно собираем значения из полей
            for (auto* spin : neuronSpinBoxes_) {
                layers.push_back(spin->value());
            }
        } else {
            // Используем значения, введённые пользователем
            for (auto* spin : neuronSpinBoxes_) {
                int val = spin->value();
                if (val <= 0) {
                    QMessageBox::warning(this, "Error",
                        "All layer sizes must be set. Fill them manually or press Apply with empty fields for automatic linear distribution.");
                    return;
                }
                layers.push_back(val);
            }
        }

        layers.push_back(outputSizeSpin_->value());
        emit applySettings(layers);
    });
}

// Публичные методы
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
