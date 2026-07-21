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

    // ----------------- Вкладка Training -----------------
    QWidget *trainingTab = new QWidget();
    QVBoxLayout *trainLayout = new QVBoxLayout(trainingTab);

    // Параметры обучения
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

    // Кнопки управления
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

    // Статус
    statusLabel_ = new QLabel("Idle");
    trainLayout->addWidget(statusLabel_);

    // Прогресс-бар и метка
    QHBoxLayout *progressLayout = new QHBoxLayout;
    progressBar_ = new QProgressBar();
    progressBar_->setRange(0, 100);
    progressBar_->setValue(0);
    progressBar_->setVisible(false);
    progressLabel_ = new QLabel("");
    progressLayout->addWidget(progressBar_);
    progressLayout->addWidget(progressLabel_);
    trainLayout->addLayout(progressLayout);

    // Лог
    logEdit_ = new QPlainTextEdit();
    logEdit_->setReadOnly(true);
    trainLayout->addWidget(logEdit_);

    tabWidget_->addTab(trainingTab, "Training");

    // Подключение сигналов кнопок
    connect(startBtn_, &QPushButton::clicked, this, [this]() {
        emit startTraining(learningRateSpin_->value(), epochsSpin_->value());
    });
    connect(pauseBtn_, &QPushButton::clicked, this, &MainWindow::pauseTraining);
    connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::stopTraining);

    // ----------------- Вкладка Experiment -----------------
    QWidget *experimentTab = new QWidget();
    QVBoxLayout *expLayout = new QVBoxLayout(experimentTab);
    expLayout->addWidget(new QLabel("Experiment tab – coming soon."));
    tabWidget_->addTab(experimentTab, "Experiment");

    // ----------------- Вкладка Settings -----------------
    QWidget *settingsTab = new QWidget();
    QVBoxLayout *settLayout = new QVBoxLayout(settingsTab);

    QGroupBox *archGroup = new QGroupBox("Network Architecture");
    QFormLayout *formLayout = new QFormLayout;

    hiddenLayersCombo_ = new QComboBox();
    for (int i = 2; i <= 5; ++i) {
        hiddenLayersCombo_->addItem(QString::number(i), i);
    }
    formLayout->addRow("Hidden layers:", hiddenLayersCombo_);

    // Контейнер для полей нейронов (будет обновляться при смене числа слоёв)
    QWidget *neuronsWidget = new QWidget();
    QVBoxLayout *neuronsLayout = new QVBoxLayout(neuronsWidget);
    neuronsLayout->setContentsMargins(0, 0, 0, 0);
    formLayout->addRow(neuronsWidget);

    usePercentageCheck_ = new QCheckBox("Use percentage compression");
    formLayout->addRow(usePercentageCheck_);

    percentageSpin_ = new QSpinBox();
    percentageSpin_->setRange(10, 90);
    percentageSpin_->setValue(50);
    percentageSpin_->setEnabled(false);

    formLayout->addRow("Min layer size:", new QLabel("(when using percentages)"));
    minLayerSizeSpin_ = new QSpinBox();
    minLayerSizeSpin_->setRange(2, 1024);
    minLayerSizeSpin_->setValue(26);
    minLayerSizeSpin_->setEnabled(false);   // активно только при включённых процентах
    formLayout->addRow("Min layer size:", minLayerSizeSpin_);
    connect(usePercentageCheck_, &QCheckBox::toggled, minLayerSizeSpin_, &QSpinBox::setEnabled);

    formLayout->addRow("Compression %:", percentageSpin_);
    connect(usePercentageCheck_, &QCheckBox::toggled, percentageSpin_, &QSpinBox::setEnabled);
    connect(hiddenLayersCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this, neuronsLayout]() {
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
            spin->setRange(10, 1024);
            spin->setValue(128);
            row->addWidget(spin);
            neuronsLayout->addLayout(row);
            neuronSpinBoxes_.append(spin);
        }
    });
    // Вызвать для инициализации
    hiddenLayersCombo_->currentIndexChanged(0);

    archGroup->setLayout(formLayout);
    settLayout->addWidget(archGroup);

    applyBtn_ = new QPushButton("Apply");
    settLayout->addWidget(applyBtn_);
    settLayout->addStretch();

    tabWidget_->addTab(settingsTab, "Settings");

    // Сигнал Apply
    connect(applyBtn_, &QPushButton::clicked, this, [this]() {
        std::vector<size_t> layers;
        layers.push_back(784); // входной
        if (usePercentageCheck_->isChecked()) {
            int prev = 784;
            int percent = percentageSpin_->value();
            int numHidden = hiddenLayersCombo_->currentData().toInt();
            for (int i = 0; i < numHidden; ++i) {
                int minSize = minLayerSizeSpin_->value();
                int neurons = std::max(minSize, prev * percent / 100);
                layers.push_back(neurons);
                prev = neurons;
            }
        } else {
            for (auto* spin : neuronSpinBoxes_) {
                layers.push_back(spin->value());
            }
        }
        layers.push_back(26); // выходной
        emit applySettings(layers);
    });
}

// Реализация публичных методов
void MainWindow::setStatus(const QString &status) {
    statusLabel_->setText(status);
}

void MainWindow::appendLog(const QString &text) {
    logEdit_->appendPlainText(text);
}

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
