#ifndef S21_RENDER_DIALOG_H
#define S21_RENDER_DIALOG_H

#include <QDialog>
#include <QComboBox>
#include <QSpinBox>
#include <QProgressBar>
#include <QLabel>
#include <QPushButton>
#include "tracer/ray_tracer.h"

namespace s21 {

class RenderDialog : public QDialog {
    Q_OBJECT
public:
    explicit RenderDialog(RayTracer* tracer, QWidget* parent = nullptr);

private slots:
    void onRender();

private:
    RayTracer* tracer_;
    QComboBox* resolutionCombo_;
    QSpinBox* samplesSpin_;
    QProgressBar* progressBar_;
    QLabel* imageLabel_;
    QPushButton* saveButton_;
    QImage renderedImage_;
};

} // namespace s21

#endif
