#ifndef RENDER_DIALOG_H
#define RENDER_DIALOG_H

#include <QDialog>
#include <QSpinBox>
#include <QComboBox>
#include <QPushButton>
#include <QLabel>
#include <QFutureWatcher>
#include <QImage>

namespace s21 {
class RayTracer;

class RenderDialog : public QDialog {
    Q_OBJECT
public:
    explicit RenderDialog(RayTracer* tracer, QWidget* parent = nullptr);

private slots:
    void onRender();
    void onRenderFinished();
    void setProgress(int percent);   // обновляет статус

private:
    RayTracer* tracer_;
    QSpinBox* widthSpin_;
    QSpinBox* heightSpin_;
    QComboBox* formatCombo_;
    QPushButton* renderBtn_;
    QLabel* previewLabel_;
    QLabel* statusLabel_;
    QFutureWatcher<QImage> watcher_;
};

} // namespace s21
#endif
