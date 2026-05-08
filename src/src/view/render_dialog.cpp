#include "render_dialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QFileDialog>
#include <QMessageBox>
#include <QtConcurrent/QtConcurrentRun>
#include <QFutureWatcher>

namespace s21 {

RenderDialog::RenderDialog(RayTracer* tracer, QWidget* parent)
    : QDialog(parent), tracer_(tracer) {
    setWindowTitle("Render Settings");
    setMinimumSize(400, 300);

    auto* mainLayout = new QVBoxLayout(this);

    // Настройки
    auto* form = new QFormLayout;
    resolutionCombo_ = new QComboBox;
    resolutionCombo_->addItem("640x480", QSize(640, 480));
    resolutionCombo_->addItem("800x600", QSize(800, 600));
    resolutionCombo_->addItem("1024x768", QSize(1024, 768));
    resolutionCombo_->addItem("1920x1080", QSize(1920, 1080));
    resolutionCombo_->addItem("1920x1440", QSize(1920, 1440));
    form->addRow("Resolution:", resolutionCombo_);

    samplesSpin_ = new QSpinBox;
    samplesSpin_->setRange(1, 16);
    samplesSpin_->setValue(4);
    samplesSpin_->setToolTip("Number of samples per pixel (1 = no anti-aliasing)");
    form->addRow("Samples:", samplesSpin_);
    mainLayout->addLayout(form);

    // Кнопка Render
    auto* btnLayout = new QHBoxLayout;
    auto* renderBtn = new QPushButton("Render");
    connect(renderBtn, &QPushButton::clicked, this, &RenderDialog::onRender);
    btnLayout->addStretch();
    btnLayout->addWidget(renderBtn);
    mainLayout->addLayout(btnLayout);

    // Прогресс-бар
    progressBar_ = new QProgressBar;
    progressBar_->setVisible(false);
    mainLayout->addWidget(progressBar_);

    // Превью
    imageLabel_ = new QLabel;
    imageLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(imageLabel_, 1);

    // Кнопка Save (изначально скрыта)
    saveButton_ = new QPushButton("Save Image");
    saveButton_->setVisible(false);
    connect(saveButton_, &QPushButton::clicked, this, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, "Save Ray Traced Image", "",
                                                        "BMP (*.bmp);;JPEG (*.jpg);;PNG (*.png)");
        if (!fileName.isEmpty()) {
            renderedImage_.save(fileName);
            QMessageBox::information(this, "Saved", "Image saved successfully.");
        }
    });
    mainLayout->addWidget(saveButton_);
}

void RenderDialog::onRender() {
    QSize res = resolutionCombo_->currentData().toSize();
    int samples = samplesSpin_->value();

    progressBar_->setVisible(true);
    progressBar_->setRange(0, 0); // indeterminate
    saveButton_->setVisible(false);
    imageLabel_->clear();

    // Запуск в отдельном потоке
    auto* watcher = new QFutureWatcher<QImage>(this);
    connect(watcher, &QFutureWatcher<QImage>::finished, this, [this, watcher]() {
        renderedImage_ = watcher->result();
        imageLabel_->setPixmap(QPixmap::fromImage(renderedImage_).scaled(
            imageLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        progressBar_->setVisible(false);
        saveButton_->setVisible(true);
        watcher->deleteLater();
    });

    QFuture<QImage> future = QtConcurrent::run([this, res, samples]() {
        return tracer_->Render(res.width(), res.height(), samples);
    });
    watcher->setFuture(future);
}

} // namespace s21
