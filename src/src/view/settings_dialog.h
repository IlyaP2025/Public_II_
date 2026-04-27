#ifndef S21_SETTINGS_DIALOG_H
#define S21_SETTINGS_DIALOG_H

#include <QCheckBox>
#include <QColor>
#include <QDialog>

#include "settings/settings.h"

QT_BEGIN_NAMESPACE
class QComboBox;
class QPushButton;
class QDoubleSpinBox;
class QLabel;
class QDialogButtonBox;
QT_END_NAMESPACE

namespace s21 {

class SettingsDialog : public QDialog {
  Q_OBJECT

 public:
  explicit SettingsDialog(QWidget* parent = nullptr);

 private slots:
  void onBackgroundColorClicked();
  void onEdgeColorClicked();
  void onVertexColorClicked();
  void onAccepted();
  void onRejected();

 private:
  void loadSettings();
  void saveSettings();

  Settings& settings_;

  QComboBox* projectionCombo_;
  QPushButton* bgColorButton_;
  QPushButton* edgeColorButton_;
  QPushButton* vertexColorButton_;
  QDoubleSpinBox* edgeThicknessSpin_;
  QComboBox* edgeTypeCombo_;
  QComboBox* vertexTypeCombo_;
  QDoubleSpinBox* vertexSizeSpin_;
  QDoubleSpinBox* dashFactorSpin_;
  QCheckBox* flipNormalsCheck_;

  QColor bgColor_, edgeColor_, vertexColor_;
};

}  // namespace s21

#endif  // S21_SETTINGS_DIALOG_H
