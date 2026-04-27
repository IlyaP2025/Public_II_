#ifndef S21_LIGHT_EDITOR_DIALOG_H
#define S21_LIGHT_EDITOR_DIALOG_H

#include <QDialog>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QColor>
#include "common/lighting.h"

namespace s21 {

class LightEditorDialog : public QDialog {
    Q_OBJECT
public:
    explicit LightEditorDialog(QWidget *parent = nullptr);

    void setLight(const LightSource& light);
    LightSource getLight() const;

private slots:
    void onAmbientClicked();
    void onDiffuseClicked();
    void onSpecularClicked();

private:
    void updateButtonColor(QPushButton* btn, const QColor& color);

    QDoubleSpinBox *posX_, *posY_, *posZ_;
    QPushButton *ambientBtn_, *diffuseBtn_, *specularBtn_;
    QColor ambientColor_{255,255,255};
    QColor diffuseColor_{255,255,255};
    QColor specularColor_{255,255,255};
};

} // namespace s21
#endif
