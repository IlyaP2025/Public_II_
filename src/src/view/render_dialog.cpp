#include "render_dialog.h"
#include "tracer/ray_tracer.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QDateTime>

namespace s21 {

RenderDialog::RenderDialog(RayTracer* tracer, QWidget* parent)
    : QDialog(parent), tracer_(tracer)
{
    setWindowTitle("Ray Tracing Render");
    setMinimumSize(400, 350);

    auto* mainLayout = new QVBoxLayout(this);

    // Разрешение
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

    // Формат
    auto* fmtLayout = new QHBoxLayout;
    fmtLayout->addWidget(new QLabel("Format:"));
    formatCombo_ = new QComboBox;
    formatCombo_->addItems({"BMP", "JPEG", "PNG"});
    fmtLayout->addWidget(formatCombo_);
    mainLayout->addLayout(fmtLayout);

    // Кнопка
    renderBtn_ = new QPushButton("Render");
    mainLayout->addWidget(renderBtn_);

    // Превью
    previewLabel_ = new QLabel;
    previewLabel_->setMinimumSize(320, 240);
    previewLabel_->setAlignment(Qt::AlignCenter);
    previewLabel_->setStyleSheet("border: 1px solid gray;");
    mainLayout->addWidget(previewLabel_);

    connect(renderBtn_, &QPushButton::clicked, this, &RenderDialog::onRender);
}

void RenderDialog::onRender() {
    int w = widthSpin_->value();
    int h = heightSpin_->value();
    QString format = formatCombo_->currentText().toLower();

    QImage image = tracer_->Render(w, h, 1);
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Rendering failed.");
        return;
    }

    // Сохраняем во временный файл (или запрашиваем путь)
    QString fileName = QString("raytraced_%1.%2")
                           .arg(QDateTime::currentSecsSinceEpoch())
                           .arg(format);
    if (!image.save(fileName, format.toStdString().c_str())) {
        QMessageBox::warning(this, "Error", "Failed to save image.");
        return;
    }

    // Показываем превью
    previewLabel_->setPixmap(QPixmap::fromImage(image).scaled(
        previewLabel_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    QMessageBox::information(this, "Success", "Image saved as " + fileName);
}

} // namespace s21
