#ifndef S21_MAIN_WINDOW_H
#define S21_MAIN_WINDOW_H

#include <QCheckBox>
#include <QFutureWatcher>
#include <QMainWindow>
#include <QSlider>
#include <QTimer>
#include <memory>
#include <vector>

#include "common/model_data.h"
#include "facade/facade.h"
#include "gif_recorder.h"
#include "gl_widget.h"
#include "scene/scene_observer.h"
#include "settings/settings.h"

QT_BEGIN_NAMESPACE
class QDoubleSpinBox;
class QPushButton;
class QLabel;
class QTabWidget;
class QComboBox;
class QGroupBox;
QT_END_NAMESPACE

namespace s21 {

class MainWindow : public QMainWindow, public SceneObserver {
  Q_OBJECT

 public:
  MainWindow(std::shared_ptr<Facade> facade, GLWidget* glWidget,
             QWidget* parent = nullptr);
  ~MainWindow() override;

  void OnObjectAdded(SceneObject* object) override;
  void OnObjectRemoved(SceneObject* object) override;
  void OnSelectionChanged(const std::vector<SceneObject*>& selected) override;
  void OnTransformChanged(SceneObject* object) override;

 protected:
  void closeEvent(QCloseEvent* event) override;

 private slots:
  void onNewModel();
  void onOpenModel();
  void onLoadModel();
  void onUndo();
  void onRedo();
  void onScreenshot();
  void onRecordGif();
  void onGifFinished(bool success, const QString& error);
  void onFit();

  void onMoveXChanged(double value);
  void onMoveYChanged(double value);
  void onMoveZChanged(double value);
  void onRotateXChanged(double value);
  void onRotateYChanged(double value);
  void onRotateZChanged(double value);
  void onScaleXChanged(double value);
  void onScaleYChanged(double value);
  void onScaleZChanged(double value);

  void onProjectionChanged(int index);
  void onBackgroundColorClicked();
  void onEdgeColorClicked();
  void onVertexColorClicked();
  void onSelectedEdgeColorClicked();
  void onSelectedVertexColorClicked();
  void onEdgeThicknessChanged(double value);
  void onVertexTypeChanged(int index);
  void onVertexSizeChanged(double value);
  void onEdgeTypeChanged(int index);
  void onDashFactorChanged(double value);

  void onLightColorClicked();
  void onLightPosChanged();
  void onShadingTypeChanged(int index);
  void onFlipNormalsChecked(bool checked);
  void onModelLoaded();

  void onLoadTexture();
  void onClearTexture();
  void onSaveUVMap();
  void onObjectColorClicked();

  void startAutoRotation();
  void rotateStep();
  void stopAutoRotation();

  void onSmoothingFactorChanged(double value);

 private:
  void setupUI();
  void createToolBar();
  void createRightPanel();
  void createTransformTab(QTabWidget* tabWidget);
  void createDisplayTab(QTabWidget* tabWidget);
  void updateUIFromSelection();
  void updateModelInfo();
  void connectSignals();
  void updateDisplayUI();
  void updateUiState(AppState newState);
  void loadModelFromFile(const QString& fileName);

  void applyMove(int axis, double value);
  void applyRotate(int axis, double value);
  void applyScale(int axis, double value);

  QComboBox* projectionCombo_;
  QPushButton* bgColorBtn_;
  QPushButton* edgeColorBtn_;
  QPushButton* vertexColorBtn_;
  QDoubleSpinBox* edgeThicknessSpin_;
  QComboBox* vertexTypeCombo_;
  QDoubleSpinBox* vertexSizeSpin_;
  QPushButton* selectedEdgeColorBtn_;
  QPushButton* selectedVertexColorBtn_;
  QComboBox* edgeTypeCombo_;
  QDoubleSpinBox* dashFactorSpin_;
  QCheckBox* flipNormalsCheckBox_;

  QGroupBox* lightGroup_;
  QDoubleSpinBox* lightPosX_;
  QDoubleSpinBox* lightPosY_;
  QDoubleSpinBox* lightPosZ_;
  QPushButton* lightColorBtn_;
  QComboBox* shadingTypeCombo_;

  QPushButton* newBtn_;
  QPushButton* openBtn_;
  QPushButton* loadBtn_;
  QPushButton* undoBtn_;
  QPushButton* redoBtn_;
  QPushButton* screenshotBtn_;
  QPushButton* recordBtn_;
  QPushButton* fitBtn_;
  QPushButton* loadTextureBtn_;
  QPushButton* clearTextureBtn_;
  QPushButton* saveUVMapBtn_;
  QPushButton* objectColorBtn_;

  QDoubleSpinBox* moveXSpin_;
  QDoubleSpinBox* moveYSpin_;
  QDoubleSpinBox* moveZSpin_;
  QDoubleSpinBox* rotateXSpin_;
  QDoubleSpinBox* rotateYSpin_;
  QDoubleSpinBox* rotateZSpin_;
  QDoubleSpinBox* scaleXSpin_;
  QDoubleSpinBox* scaleYSpin_;
  QDoubleSpinBox* scaleZSpin_;

  QSlider* smoothingSlider_;

  QLabel* infoLabel_;

  QTimer* rotationTimer_ = nullptr;
  float originalRotationY_ = 0.0f;
  float currentRotationAngle_ = 0.0f;
  SceneObject* rotatingObject_ = nullptr;

  std::shared_ptr<Facade> facade_;
  GLWidget* glWidget_;

  bool isMoving_ = false;
  bool isRotating_ = false;
  bool isScaling_ = false;

  QFutureWatcher<RawModelData> modelLoaderWatcher_;
  bool isLoading_ = false;
  bool isReplaceMode_ = false;

  std::unique_ptr<GifRecorder> gifRecorder_;
};

}  // namespace s21

#endif  // S21_MAIN_WINDOW_H
