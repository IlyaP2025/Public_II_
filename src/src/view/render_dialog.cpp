#include "render_dialog.h"
#include "tracer/ray_tracer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>
#include <QtConcurrent/QtConcurrentRun>

namespace s21 {

RenderDialog::RenderDialog(RayTracer* tracer, QWidget* parent)
    : QDialog(parent), tracer_(tracer)
{
    setWindowTitle("Ray Tracing Render");
    setMinimumSize(400, 400);

    auto* mainLayout = new QVBoxLayout(this);

    auto* resLayout = new QHBoxLayout;
    resLayout->addWidget(new QLabel("Width:"));
    widthSpin_ = new QSpinBox;
    widthSpin_->setRange(1, 1920);
    widthSpin_->setValue(800);
    resLayout->addWidget(widthSpin_);

    resLayout->addWidget(new QLabel("Height:"));
    heightSpin_ = new QSpinBox;
    heightSpin_->setRange(1, 1440);
    heightSpin_->setValue(600);
    resLayout->addWidget(heightSpin_);
    mainLayout->addLayout(resLayout);

    auto* fmtLayout = new QHBoxLayout;
    fmtLayout->addWidget(new QLabel("Format:"));
    formatCombo_ = new QComboBox;
    formatCombo_->addItems({"BMP", "JPEG", "PNG"});
    fmtLayout->addWidget(formatCombo_);
    mainLayout->addLayout(fmtLayout);

    renderBtn_ = new QPushButton("Render");
    mainLayout->addWidget(renderBtn_);

    statusLabel_ = new QLabel;
    statusLabel_->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(statusLabel_);

    previewLabel_ = new QLabel;
    previewLabel_->setMinimumSize(320, 240);
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet("border: 1px solid gray;");
    mainLayout->addWidget(previewLabel_);

    connect(renderBtn_, &QPushButton::clicked, this, &RenderDialog::onRender);
    connect(&watcher_, &QFutureWatcher<QImage>::finished,
            this, &RenderDialog::onRenderFinished);
}

void RenderDialog::onRender() {
    int w = widthSpin_->value();
    int h = heightSpin_->value();

    renderBtn_->setEnabled(false);
    statusLabel_->setText("Rendering... 0%");
    statusLabel_->repaint();

    QFuture<QImage> future = QtConcurrent::run([this, w, h]() -> QImage {
        return tracer_->RenderWithProgress(w, h, 1,
            [this](int percent) {
                QMetaObject::invokeMethod(this, "setProgress", Qt::QueuedConnection,
                                         Q_ARG(int, percent));
            });
    });
    watcher_.setFuture(future);
}

void RenderDialog::onRenderFinished() {
    QImage image = watcher_.result();

    renderBtn_->setEnabled(true);

    if (image.isNull()) {
        statusLabel_->setText("Rendering failed.");
        QMessageBox::warning(this, "Error", "Rendering failed.");
        return;
    }

    QString format = formatCombo_->currentText().toLower();
    QString fileName = QString("raytraced_%1.%2")
                           .arg(QDateTime::currentSecsSinceEpoch())
                           .arg(format);
    if (!image.save(fileName, format.toStdString().c_str())) {
        statusLabel_->setText("Failed to save.");
        QMessageBox::warning(this, "Error", "Failed to save image.");
        return;
    }

    previewLabel_->setPixmap(QPixmap::fromImage(image).scaled(
        previewLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    statusLabel_->setText("Saved as " + fileName);
}

void RenderDialog::setProgress(int percent) {
    statusLabel_->setText(QString("Rendering... %1%").arg(percent));
    statusLabel_->repaint();
}

} // namespace s21
