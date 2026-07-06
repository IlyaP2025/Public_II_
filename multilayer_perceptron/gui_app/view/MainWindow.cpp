#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>

namespace s21 {
namespace mlp {

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setWindowTitle("Multilayer Perceptron");
    setupUI();
}

void MainWindow::setupUI() {
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    QVBoxLayout *mainLayout = new QVBoxLayout(central);

    // Группа параметров обучения
    QGroupBox *paramsGroup = new QGroupBox("Training Parameters", this);
    QHBoxLayout *paramsLayout = new QHBoxLayout(paramsGroup);
    paramsLayout->addWidget(new QLabel("Learning rate:"));
    learningRateSpin_ = new QDoubleSpinBox(this);
    learningRateSpin_->setRange(0.001, 1.0);
    learningRateSpin_->setSingleStep(0.01);
    learningRateSpin_->setValue(0.1);
    paramsLayout->addWidget(learningRateSpin_);
    paramsLayout->addWidget(new QLabel("Epochs:"));
    epochsSpin_ = new QSpinBox(this);
    epochsSpin_->setRange(1, 1000);
    epochsSpin_->setValue(2);
    paramsLayout->addWidget(epochsSpin_);
    mainLayout->addWidget(paramsGroup);

    // Кнопки управления
    QHBoxLayout *btnLayout = new QHBoxLayout();
    startBtn_ = new QPushButton("Start", this);
    pauseBtn_ = new QPushButton("Pause", this);
    stopBtn_ = new QPushButton("Stop", this);
    pauseBtn_->setEnabled(false);
    stopBtn_->setEnabled(false);
    btnLayout->addWidget(startBtn_);
    btnLayout->addWidget(pauseBtn_);
    btnLayout->addWidget(stopBtn_);
    mainLayout->addLayout(btnLayout);

    // Статус
    statusLabel_ = new QLabel("Idle", this);
    mainLayout->addWidget(statusLabel_);

    // Лог
    logEdit_ = new QPlainTextEdit(this);
    logEdit_->setReadOnly(true);
    mainLayout->addWidget(logEdit_);

    // Подключение сигналов кнопок (передаются наружу через сигналы окна)
    connect(startBtn_, &QPushButton::clicked, this, [this]() {
        emit startTraining(learningRateSpin_->value(), epochsSpin_->value());
    });
    connect(pauseBtn_, &QPushButton::clicked, this, &MainWindow::pauseTraining);
    connect(stopBtn_, &QPushButton::clicked, this, &MainWindow::stopTraining);
}

void MainWindow::setStatus(const QString &status) {
    statusLabel_->setText(status);
}

void MainWindow::appendLog(const QString &text) {
    logEdit_->appendPlainText(text);
}

void MainWindow::setTrainingEnabled(bool enabled) {
    startBtn_->setEnabled(enabled);
    pauseBtn_->setEnabled(!enabled);   // pause активна, когда идёт обучение
    stopBtn_->setEnabled(!enabled);
}

}  // namespace mlp
}  // namespace s21
