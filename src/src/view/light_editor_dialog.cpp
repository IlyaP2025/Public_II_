#include "light_editor_dialog.h"
#include <QVBoxLayout>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QColorDialog>
#include <QDialogButtonBox>

namespace s21 {

LightEditorDialog::LightEditorDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Light Properties");
    auto *mainLayout = new QVBoxLayout(this);

    auto *posGroup = new QGroupBox("Position");
    auto *posForm = new QFormLayout;
    posX_ = new QDoubleSpinBox; posX_->setRange(-100, 100); posX_->setDecimals(2);
    posY_ = new QDoubleSpinBox; posY_->setRange(-100, 100); posY_->setDecimals(2);
    posZ_ = new QDoubleSpinBox; posZ_->setRange(-100, 100); posZ_->setDecimals(2);
    posForm->addRow("X:", posX_);
    posForm->addRow("Y:", posY_);
    posForm->addRow("Z:", posZ_);
    posGroup->setLayout(posForm);
    mainLayout->addWidget(posGroup);

    auto *colorGroup = new QGroupBox("Color Components");
    auto *colorForm = new QFormLayout;

    ambientBtn_ = new QPushButton; ambientBtn_->setFixedSize(40, 20);
    connect(ambientBtn_, &QPushButton::clicked, this, &LightEditorDialog::onAmbientClicked);
    colorForm->addRow("Ambient:", ambientBtn_);

    diffuseBtn_ = new QPushButton; diffuseBtn_->setFixedSize(40, 20);
    connect(diffuseBtn_, &QPushButton::clicked, this, &LightEditorDialog::onDiffuseClicked);
    colorForm->addRow("Diffuse:", diffuseBtn_);

    specularBtn_ = new QPushButton; specularBtn_->setFixedSize(40, 20);
    connect(specularBtn_, &QPushButton::clicked, this, &LightEditorDialog::onSpecularClicked);
    colorForm->addRow("Specular:", specularBtn_);

    colorGroup->setLayout(colorForm);
    mainLayout->addWidget(colorGroup);

    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);

    updateButtonColor(ambientBtn_, ambientColor_);
    updateButtonColor(diffuseBtn_, diffuseColor_);
    updateButtonColor(specularBtn_, specularColor_);
}

void LightEditorDialog::updateButtonColor(QPushButton* btn, const QColor& color) {
    btn->setStyleSheet(QString("background-color: %1").arg(color.name()));
    btn->setProperty("color", color);
}

void LightEditorDialog::onAmbientClicked() {
    QColor c = QColorDialog::getColor(ambientColor_, this, "Ambient Color");
    if (c.isValid()) { ambientColor_ = c; updateButtonColor(ambientBtn_, c); }
}
void LightEditorDialog::onDiffuseClicked() {
    QColor c = QColorDialog::getColor(diffuseColor_, this, "Diffuse Color");
    if (c.isValid()) { diffuseColor_ = c; updateButtonColor(diffuseBtn_, c); }
}
void LightEditorDialog::onSpecularClicked() {
    QColor c = QColorDialog::getColor(specularColor_, this, "Specular Color");
    if (c.isValid()) { specularColor_ = c; updateButtonColor(specularBtn_, c); }
}

void LightEditorDialog::setLight(const LightSource& light) {
    posX_->setValue(light.position.x);
    posY_->setValue(light.position.y);
    posZ_->setValue(light.position.z);
    ambientColor_  = QColor::fromRgbF(light.ambient.r,  light.ambient.g,  light.ambient.b);
    diffuseColor_  = QColor::fromRgbF(light.diffuse.r,  light.diffuse.g,  light.diffuse.b);
    specularColor_ = QColor::fromRgbF(light.specular.r, light.specular.g, light.specular.b);
    updateButtonColor(ambientBtn_,  ambientColor_);
    updateButtonColor(diffuseBtn_,  diffuseColor_);
    updateButtonColor(specularBtn_, specularColor_);
}

LightSource LightEditorDialog::getLight() const {
    LightSource l;
    l.position = glm::vec3(posX_->value(), posY_->value(), posZ_->value());
    l.ambient  = glm::vec3(ambientColor_.redF(),  ambientColor_.greenF(),  ambientColor_.blueF());
    l.diffuse  = glm::vec3(diffuseColor_.redF(),  diffuseColor_.greenF(),  diffuseColor_.blueF());
    l.specular = glm::vec3(specularColor_.redF(), specularColor_.greenF(), specularColor_.blueF());
    return l;
}

} // namespace s21
