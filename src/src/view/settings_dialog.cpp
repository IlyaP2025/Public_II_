#include "settings_dialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace s21 {

SettingsDialog::SettingsDialog(QWidget* parent)
    : QDialog(parent), settings_(Settings::instance()) {
  setWindowTitle("Settings");

  auto* mainLayout = new QVBoxLayout(this);

  // Проекция
  auto* projGroup = new QGroupBox("Projection");
  auto* projLayout = new QHBoxLayout;
  projLayout->addWidget(new QLabel("Type:"));
  projectionCombo_ = new QComboBox;
  projectionCombo_->addItem("Orthographic");
  projectionCombo_->addItem("Perspective");
  projLayout->addWidget(projectionCombo_);
  projGroup->setLayout(projLayout);
  mainLayout->addWidget(projGroup);

  // Цвета + чекбокс "Flip normals"
  auto* colorGroup = new QGroupBox("Colors");
  auto* colorLayout = new QGridLayout;
  colorLayout->addWidget(new QLabel("Background:"), 0, 0);
  bgColorButton_ = new QPushButton;
  bgColorButton_->setFixedSize(50, 25);
  connect(bgColorButton_, &QPushButton::clicked, this,
          &SettingsDialog::onBackgroundColorClicked);
  colorLayout->addWidget(bgColorButton_, 0, 1);

  colorLayout->addWidget(new QLabel("Edges:"), 1, 0);
  edgeColorButton_ = new QPushButton;
  edgeColorButton_->setFixedSize(50, 25);
  connect(edgeColorButton_, &QPushButton::clicked, this,
          &SettingsDialog::onEdgeColorClicked);
  colorLayout->addWidget(edgeColorButton_, 1, 1);

  colorLayout->addWidget(new QLabel("Vertices:"), 2, 0);
  vertexColorButton_ = new QPushButton;
  vertexColorButton_->setFixedSize(50, 25);
  connect(vertexColorButton_, &QPushButton::clicked, this,
          &SettingsDialog::onVertexColorClicked);
  colorLayout->addWidget(vertexColorButton_, 2, 1);

  // Чекбокс на новой строке
  flipNormalsCheck_ = new QCheckBox("Flip normals");
  colorLayout->addWidget(flipNormalsCheck_, 3, 0, 1, 2);

  colorGroup->setLayout(colorLayout);
  mainLayout->addWidget(colorGroup);

  // Рёбра
  auto* edgeGroup = new QGroupBox("Edges");
  auto* edgeLayout = new QGridLayout;
  edgeLayout->addWidget(new QLabel("Thickness:"), 0, 0);
  edgeThicknessSpin_ = new QDoubleSpinBox;
  edgeThicknessSpin_->setRange(0.1, 10.0);
  edgeThicknessSpin_->setSingleStep(0.5);
  edgeThicknessSpin_->setDecimals(1);
  edgeLayout->addWidget(edgeThicknessSpin_, 0, 1);

  edgeLayout->addWidget(new QLabel("Type:"), 1, 0);
  edgeTypeCombo_ = new QComboBox;
  edgeTypeCombo_->addItem("Solid");
  edgeTypeCombo_->addItem("Dashed");
  edgeLayout->addWidget(edgeTypeCombo_, 1, 1);

  edgeLayout->addWidget(new QLabel("Dash spacing:"), 2, 0);
  dashFactorSpin_ = new QDoubleSpinBox;
  dashFactorSpin_->setRange(1.0, 20.0);
  dashFactorSpin_->setSingleStep(1.0);
  dashFactorSpin_->setDecimals(1);
  edgeLayout->addWidget(dashFactorSpin_, 2, 1);
  edgeGroup->setLayout(edgeLayout);
  mainLayout->addWidget(edgeGroup);

  // Вершины
  auto* vertexGroup = new QGroupBox("Vertices");
  auto* vertexLayout = new QGridLayout;
  vertexLayout->addWidget(new QLabel("Type:"), 0, 0);
  vertexTypeCombo_ = new QComboBox;
  vertexTypeCombo_->addItem("None");
  vertexTypeCombo_->addItem("Circle");
  vertexTypeCombo_->addItem("Square");
  vertexLayout->addWidget(vertexTypeCombo_, 0, 1);

  vertexLayout->addWidget(new QLabel("Size:"), 1, 0);
  vertexSizeSpin_ = new QDoubleSpinBox;
  vertexSizeSpin_->setRange(1.0, 20.0);
  vertexSizeSpin_->setSingleStep(1.0);
  vertexSizeSpin_->setDecimals(1);
  vertexLayout->addWidget(vertexSizeSpin_, 1, 1);
  vertexGroup->setLayout(vertexLayout);
  mainLayout->addWidget(vertexGroup);

  // Кнопки OK/Cancel
  auto* buttonBox =
      new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
  connect(buttonBox, &QDialogButtonBox::accepted, this,
          &SettingsDialog::onAccepted);
  connect(buttonBox, &QDialogButtonBox::rejected, this,
          &SettingsDialog::onRejected);
  mainLayout->addWidget(buttonBox);

  loadSettings();
}

void SettingsDialog::loadSettings() {
  projectionCombo_->setCurrentIndex(
      static_cast<int>(settings_.projectionType()));
  bgColor_ = settings_.backgroundColor();
  edgeColor_ = settings_.edgeColor();
  vertexColor_ = settings_.vertexColor();
  bgColorButton_->setStyleSheet(
      QString("background-color: %1").arg(bgColor_.name()));
  edgeColorButton_->setStyleSheet(
      QString("background-color: %1").arg(edgeColor_.name()));
  vertexColorButton_->setStyleSheet(
      QString("background-color: %1").arg(vertexColor_.name()));
  edgeThicknessSpin_->setValue(settings_.edgeThickness());
  edgeTypeCombo_->setCurrentIndex(static_cast<int>(settings_.edgeType()));
  vertexTypeCombo_->setCurrentIndex(static_cast<int>(settings_.vertexType()));
  vertexSizeSpin_->setValue(settings_.vertexSize());
  dashFactorSpin_->setValue(settings_.dashFactor());
  flipNormalsCheck_->setChecked(settings_.flipNormals());
}

void SettingsDialog::saveSettings() {
  settings_.setProjectionType(
      static_cast<Settings::ProjectionType>(projectionCombo_->currentIndex()));
  settings_.setBackgroundColor(bgColor_);
  settings_.setEdgeColor(edgeColor_);
  settings_.setVertexColor(vertexColor_);
  settings_.setEdgeThickness(static_cast<float>(edgeThicknessSpin_->value()));
  settings_.setEdgeType(
      static_cast<Settings::EdgeType>(edgeTypeCombo_->currentIndex()));
  settings_.setVertexType(
      static_cast<Settings::VertexType>(vertexTypeCombo_->currentIndex()));
  settings_.setVertexSize(static_cast<float>(vertexSizeSpin_->value()));
  settings_.setDashFactor(static_cast<float>(dashFactorSpin_->value()));
  settings_.setFlipNormals(flipNormalsCheck_->isChecked());
}

void SettingsDialog::onBackgroundColorClicked() {
  QColor color =
      QColorDialog::getColor(bgColor_, this, "Select Background Color");
  if (color.isValid()) {
    bgColor_ = color;
    bgColorButton_->setStyleSheet(
        QString("background-color: %1").arg(bgColor_.name()));
  }
}

void SettingsDialog::onEdgeColorClicked() {
  QColor color = QColorDialog::getColor(edgeColor_, this, "Select Edge Color");
  if (color.isValid()) {
    edgeColor_ = color;
    edgeColorButton_->setStyleSheet(
        QString("background-color: %1").arg(edgeColor_.name()));
  }
}

void SettingsDialog::onVertexColorClicked() {
  QColor color =
      QColorDialog::getColor(vertexColor_, this, "Select Vertex Color");
  if (color.isValid()) {
    vertexColor_ = color;
    vertexColorButton_->setStyleSheet(
        QString("background-color: %1").arg(vertexColor_.name()));
  }
}

void SettingsDialog::onAccepted() {
  saveSettings();
  accept();
}

void SettingsDialog::onRejected() { reject(); }

}  // namespace s21
