#include "main_window.h"
#include "common/debug.h"
#include "scene/model_builder.h"

#include <QApplication>
#include <QCloseEvent>
#include <QColorDialog>
#include <QComboBox>
#include <QDebug>
#include <QDir>
#include <QDockWidget>
#include <QDoubleSpinBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QPainter>
#include <QProgressDialog>
#include <QPushButton>
#include <QScrollArea>
#include <QScreen>
#include <QStatusBar>
#include <QStyle>
#include <QToolBar>
#include <QtConcurrent/QtConcurrentRun>
#include <QVBoxLayout>

#include "loaders/obj_loader.h"

namespace s21 {

// ============================================================================
// Конструктор / деструктор
// ============================================================================

MainWindow::MainWindow(std::shared_ptr<Facade> facade, GLWidget* glWidget,
                       QWidget* parent)
    : QMainWindow(parent), facade_(facade), glWidget_(glWidget) {
  setupUI();
  
  // Динамический размер окна: 70% от доступного пространства экрана
  QRect screenRect = QApplication::primaryScreen()->availableGeometry();
  resize(screenRect.width() * 0.7, screenRect.height() * 0.7);
  
  connectSignals();

  updateUIFromSelection();
  updateModelInfo();

  isLoading_ = false;
  isReplaceMode_ = false;

  if (auto scene = facade_->GetScene()) {
    scene->AddObserver(this);
  }

  connect(&modelLoaderWatcher_, &QFutureWatcher<RawModelData>::finished, this,
          &MainWindow::onModelLoaded);

  gifRecorder_ = std::make_unique<GifRecorder>(glWidget_, this);
  connect(gifRecorder_.get(), &GifRecorder::recordingFinished, this,
          &MainWindow::onGifFinished);

  facade_->AddStateObserver([this](AppState newState) {
    QMetaObject::invokeMethod(this, [this, newState]() {
      updateUiState(newState);
    });
  });

  loadInitialLights();

  updateUiState(AppState::Idle);
  qDebug() << "MainWindow constructor finished";
}

MainWindow::~MainWindow() {
  if (auto scene = facade_->GetScene()) {
    scene->RemoveObserver(this);
  }
}

// ============================================================================
// UI создание
// ============================================================================

void MainWindow::setupUI() {
  setCentralWidget(glWidget_);
  createToolBar();
  createRightPanel();

  infoLabel_ = new QLabel("No model selected");
  infoLabel_->setStyleSheet("font-size: 14pt; font-weight: bold;");
  statusBar()->addWidget(infoLabel_);

  connect(&Settings::instance(), &Settings::settingsChanged, this,
          &MainWindow::updateDisplayUI);
  updateDisplayUI();
}

void MainWindow::createToolBar() {
  auto* toolbar = addToolBar("File");
  toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

  QIcon newIcon = style()->standardIcon(QStyle::SP_FileIcon);
  QIcon openIcon = style()->standardIcon(QStyle::SP_DialogOpenButton);
  QIcon loadIcon = style()->standardIcon(QStyle::SP_FileDialogDetailedView);
  QIcon undoIcon = style()->standardIcon(QStyle::SP_ArrowBack);
  QIcon redoIcon = style()->standardIcon(QStyle::SP_ArrowForward);
  QIcon screenshotIcon = style()->standardIcon(QStyle::SP_ComputerIcon);
  QIcon recordIcon = style()->standardIcon(QStyle::SP_DriveCDIcon);
  QIcon fitIcon = style()->standardIcon(QStyle::SP_FileDialogDetailedView);

  newBtn_ = new QPushButton(newIcon, "New");
  openBtn_ = new QPushButton(openIcon, "Open");
  loadBtn_ = new QPushButton(loadIcon, "Load");
  undoBtn_ = new QPushButton(undoIcon, "Undo");
  redoBtn_ = new QPushButton(redoIcon, "Redo");
  screenshotBtn_ = new QPushButton(screenshotIcon, "Screenshot");
  recordBtn_ = new QPushButton(recordIcon, "Record GIF");
  fitBtn_ = new QPushButton(fitIcon, "Fit");

  loadTextureBtn_ = new QPushButton(openIcon, "Load Texture");
  clearTextureBtn_ = new QPushButton(openIcon, "Clear Texture");
  saveUVMapBtn_ = new QPushButton(openIcon, "Save UV Map");

  toolbar->addWidget(newBtn_);
  toolbar->addWidget(openBtn_);
  toolbar->addWidget(loadBtn_);
  toolbar->addSeparator();
  toolbar->addWidget(undoBtn_);
  toolbar->addWidget(redoBtn_);
  toolbar->addSeparator();
  toolbar->addWidget(screenshotBtn_);
  toolbar->addWidget(recordBtn_);
  toolbar->addWidget(fitBtn_);
  toolbar->addSeparator();
  toolbar->addWidget(loadTextureBtn_);
  toolbar->addWidget(clearTextureBtn_);
  toolbar->addWidget(saveUVMapBtn_);
}

void MainWindow::createRightPanel() {
  QDockWidget* dock = new QDockWidget("Controls", this);
  dock->setFeatures(QDockWidget::DockWidgetMovable |
                    QDockWidget::DockWidgetFloatable);
  dock->setAllowedAreas(Qt::RightDockWidgetArea);
  addDockWidget(Qt::RightDockWidgetArea, dock);

  QScrollArea* scrollArea = new QScrollArea(dock);
  scrollArea->setWidgetResizable(true);
  scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
  scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

  QTabWidget* tabWidget = new QTabWidget(scrollArea);
  scrollArea->setWidget(tabWidget);
  dock->setWidget(scrollArea);

  createTransformTab(tabWidget);
  createDisplayTab(tabWidget);
  createLightsTab(tabWidget);
}

void MainWindow::createTransformTab(QTabWidget* tabWidget) {
  QWidget* tab = new QWidget;
  QVBoxLayout* layout = new QVBoxLayout(tab);

  QGroupBox* moveGroup = new QGroupBox("Move");
  QFormLayout* moveForm = new QFormLayout;
  moveXSpin_ = new QDoubleSpinBox;
  moveXSpin_->setRange(-100, 100);
  moveXSpin_->setSingleStep(0.1);
  moveXSpin_->setDecimals(3);
  moveYSpin_ = new QDoubleSpinBox;
  moveYSpin_->setRange(-100, 100);
  moveYSpin_->setSingleStep(0.1);
  moveYSpin_->setDecimals(3);
  moveZSpin_ = new QDoubleSpinBox;
  moveZSpin_->setRange(-100, 100);
  moveZSpin_->setSingleStep(0.1);
  moveZSpin_->setDecimals(3);
  moveForm->addRow("X:", moveXSpin_);
  moveForm->addRow("Y:", moveYSpin_);
  moveForm->addRow("Z:", moveZSpin_);
  moveGroup->setLayout(moveForm);
  layout->addWidget(moveGroup);

  QGroupBox* rotateGroup = new QGroupBox("Rotate (degrees)");
  QFormLayout* rotateForm = new QFormLayout;
  rotateXSpin_ = new QDoubleSpinBox;
  rotateXSpin_->setRange(-360, 360);
  rotateXSpin_->setSingleStep(1);
  rotateXSpin_->setDecimals(1);
  rotateYSpin_ = new QDoubleSpinBox;
  rotateYSpin_->setRange(-360, 360);
  rotateYSpin_->setSingleStep(1);
  rotateYSpin_->setDecimals(1);
  rotateZSpin_ = new QDoubleSpinBox;
  rotateZSpin_->setRange(-360, 360);
  rotateZSpin_->setSingleStep(1);
  rotateZSpin_->setDecimals(1);
  rotateForm->addRow("X:", rotateXSpin_);
  rotateForm->addRow("Y:", rotateYSpin_);
  rotateForm->addRow("Z:", rotateZSpin_);
  rotateGroup->setLayout(rotateForm);
  layout->addWidget(rotateGroup);

  QGroupBox* scaleGroup = new QGroupBox("Scale");
  QFormLayout* scaleForm = new QFormLayout;
  scaleXSpin_ = new QDoubleSpinBox;
  scaleXSpin_->setRange(0.1, 10);
  scaleXSpin_->setSingleStep(0.1);
  scaleXSpin_->setDecimals(3);
  scaleYSpin_ = new QDoubleSpinBox;
  scaleYSpin_->setRange(0.1, 10);
  scaleYSpin_->setSingleStep(0.1);
  scaleYSpin_->setDecimals(3);
  scaleZSpin_ = new QDoubleSpinBox;
  scaleZSpin_->setRange(0.1, 10);
  scaleZSpin_->setSingleStep(0.1);
  scaleZSpin_->setDecimals(3);
  scaleForm->addRow("X:", scaleXSpin_);
  scaleForm->addRow("Y:", scaleYSpin_);
  scaleForm->addRow("Z:", scaleZSpin_);
  scaleGroup->setLayout(scaleForm);
  layout->addWidget(scaleGroup);

  layout->addStretch();
  tabWidget->addTab(tab, "Transform");
}

void MainWindow::createDisplayTab(QTabWidget* tabWidget) {
  QWidget* tab = new QWidget;
  QFormLayout* form = new QFormLayout(tab);

  projectionCombo_ = new QComboBox;
  projectionCombo_->addItem("Orthographic");
  projectionCombo_->addItem("Perspective");
  form->addRow("Projection:", projectionCombo_);

  bgColorBtn_ = new QPushButton;
  bgColorBtn_->setFixedSize(50, 25);
  form->addRow("Background:", bgColorBtn_);

  edgeColorBtn_ = new QPushButton;
  edgeColorBtn_->setFixedSize(50, 25);
  form->addRow("Edges:", edgeColorBtn_);

  edgeTypeCombo_ = new QComboBox;
  edgeTypeCombo_->addItem("Solid");
  edgeTypeCombo_->addItem("Dashed");
  form->addRow("Edge type:", edgeTypeCombo_);

  QLabel* dashLabel = new QLabel("Dash spacing:");
  dashFactorSpin_ = new QDoubleSpinBox;
  dashFactorSpin_->setRange(1.0, 20.0);
  dashFactorSpin_->setSingleStep(1.0);
  dashFactorSpin_->setDecimals(1);
  dashFactorSpin_->setToolTip("Spacing for dashed lines (pixels)");
  form->addRow(dashLabel, dashFactorSpin_);

  vertexColorBtn_ = new QPushButton;
  vertexColorBtn_->setFixedSize(50, 25);
  form->addRow("Vertices:", vertexColorBtn_);

  edgeThicknessSpin_ = new QDoubleSpinBox;
  edgeThicknessSpin_->setRange(0.1, 10.0);
  edgeThicknessSpin_->setSingleStep(0.5);
  edgeThicknessSpin_->setDecimals(1);
  form->addRow("Edge thickness:", edgeThicknessSpin_);

  vertexTypeCombo_ = new QComboBox;
  vertexTypeCombo_->addItem("None");
  vertexTypeCombo_->addItem("Circle");
  vertexTypeCombo_->addItem("Square");
  form->addRow("Vertex type:", vertexTypeCombo_);

  vertexSizeSpin_ = new QDoubleSpinBox;
  vertexSizeSpin_->setRange(1.0, 20.0);
  vertexSizeSpin_->setSingleStep(1.0);
  vertexSizeSpin_->setDecimals(1);
  form->addRow("Vertex size:", vertexSizeSpin_);

  selectedEdgeColorBtn_ = new QPushButton;
  selectedEdgeColorBtn_->setFixedSize(50, 25);
  form->addRow("Selected Edge:", selectedEdgeColorBtn_);

  selectedVertexColorBtn_ = new QPushButton;
  selectedVertexColorBtn_->setFixedSize(50, 25);
  form->addRow("Selected Vertex:", selectedVertexColorBtn_);

  objectColorBtn_ = new QPushButton;
  objectColorBtn_->setFixedSize(50, 25);
  form->addRow("Object color:", objectColorBtn_);
  connect(objectColorBtn_, &QPushButton::clicked, this, &MainWindow::onObjectColorClicked);

  flipNormalsCheckBox_ = new QCheckBox("Flip Normals");
  connect(flipNormalsCheckBox_, &QCheckBox::toggled, this, &MainWindow::onFlipNormalsChecked);
  form->addRow(flipNormalsCheckBox_);

  // Коэффициент сглаживания нормалей
  QLabel* smoothLabel = new QLabel("Smoothing:");
  smoothingSlider_ = new QSlider(Qt::Horizontal);
  smoothingSlider_->setRange(0, 100);
  smoothingSlider_->setValue(static_cast<int>(Settings::instance().smoothingFactor() * 100));
  connect(smoothingSlider_, &QSlider::valueChanged, this, [this](int value) {
      onSmoothingFactorChanged(value / 100.0);
  });
  form->addRow(smoothLabel, smoothingSlider_);

  shadingTypeCombo_ = new QComboBox;
  shadingTypeCombo_->addItem("Wireframe");
  shadingTypeCombo_->addItem("Flat");
  shadingTypeCombo_->addItem("Phong");
  form->addRow("Shading:", shadingTypeCombo_);

  tabWidget->addTab(tab, "Display");
}

void MainWindow::createLightsTab(QTabWidget* tabWidget) {
  lightControlWidget_ = new LightControlWidget(this);
  connect(lightControlWidget_, &LightControlWidget::lightAdded,
          this, &MainWindow::onLightAdded);
  connect(lightControlWidget_, &LightControlWidget::lightRemoved,
          this, &MainWindow::onLightRemoved);
  connect(lightControlWidget_, &LightControlWidget::lightUpdated,
          this, &MainWindow::onLightUpdated);
  // При любом изменении можно сразу обновлять рендерер
  connect(lightControlWidget_, &LightControlWidget::lightsChanged,
          this, &MainWindow::onLightsChanged);

  tabWidget->addTab(lightControlWidget_, "Lights");
}

// ============================================================================
// Слоты для освещения
// ============================================================================

void MainWindow::onLightAdded(const LightSource& light) {
  facade_->AddLight(light);
}

void MainWindow::onLightRemoved(size_t index) {
  facade_->RemoveLight(index);
}

void MainWindow::onLightUpdated(size_t index, const LightSource& light) {
  facade_->UpdateLight(index, light);
}

void MainWindow::onLightsChanged() {
  // После любого изменения отправляем новый набор источников в рендерер
  auto lights = facade_->GetLights();
  // Пока используем временный метод setLights (можно заменить на прямую передачу через Scene)
  if (glWidget_) {
    // В будущем заменить на glWidget_->setLights(lights);
    glWidget_->update();
  }
}

void MainWindow::loadInitialLights() {
  auto lights = Settings::instance().LoadLights();
  if (lights.empty()) {
    // Создаём один источник по умолчанию
    LightSource defaultLight;
    defaultLight.position = glm::vec3(2.0f, 3.0f, 4.0f);
    defaultLight.diffuse = glm::vec3(1.0f, 1.0f, 1.0f);
    defaultLight.specular = glm::vec3(1.0f, 1.0f, 1.0f);
    defaultLight.ambient = glm::vec3(0.1f, 0.1f, 0.1f);
    defaultLight.enabled = true;
    lights.push_back(defaultLight);
  }

  for (const auto& l : lights)
    facade_->AddLight(l);

  if (lightControlWidget_)
    lightControlWidget_->setLights(facade_->GetLights());
}

// ============================================================================
// Слоты файловых операций
// ============================================================================

void MainWindow::loadModelFromFile(const QString& fileName) {
    if (fileName.isEmpty()) return;
    openBtn_->setEnabled(false);
    loadBtn_->setEnabled(false);
    newBtn_->setEnabled(false);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    infoLabel_->setText("Loading model...");
    isLoading_ = true;
    facade_->NotifyLoadStarted();
    float smoothing = Settings::instance().smoothingFactor();
    QFuture<RawModelData> future = QtConcurrent::run(
    [fileName, smoothing]() { return ObjLoader::LoadData(fileName.toStdString(), smoothing); });
    modelLoaderWatcher_.setFuture(future);
}

void MainWindow::onNewModel() {
    DEBUG_PRINT_FUNC();
    AppState state = facade_->GetState();
    if (state == AppState::Loading || state == AppState::RecordingGif) {
        DEBUG_PRINT("Cannot clear model now, state=" << static_cast<int>(state));
        return;
    }
    facade_->GetScene()->Clear();
    infoLabel_->setText("No model selected");
    facade_->NotifyModelCleared();
}

void MainWindow::onOpenModel() {
    DEBUG_PRINT_FUNC();
    AppState state = facade_->GetState();
    if (state != AppState::Idle && state != AppState::Ready) {
        DEBUG_PRINT("Cannot load now, state=" << static_cast<int>(state));
        return;
    }
 
    QString fileName = QFileDialog::getOpenFileName(this, "Open OBJ File", "", "OBJ Files (*.obj)");

    if (fileName.isEmpty()) {
        return; // пользователь отменил выбор
    }  

    facade_->GetScene()->Clear();
    facade_->NotifyModelCleared();
    isReplaceMode_ = true;
    loadModelFromFile(fileName);
}

void MainWindow::onLoadModel() {
    DEBUG_PRINT_FUNC();
    AppState state = facade_->GetState();
    if (state != AppState::Idle && state != AppState::Ready) {
        DEBUG_PRINT("Cannot load now, state=" << static_cast<int>(state));
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, "Load Additional OBJ", "",
                                                  "OBJ Files (*.obj)");
    isReplaceMode_ = false;
    loadModelFromFile(fileName);
}

void MainWindow::onModelLoaded() {
  DEBUG_PRINT_FUNC();
  QApplication::restoreOverrideCursor();
  openBtn_->setEnabled(true);
  loadBtn_->setEnabled(true);
  newBtn_->setEnabled(true);

  RawModelData data = modelLoaderWatcher_.result();
  isLoading_ = false;

  if (!data.success) {
    facade_->NotifyLoadFinished(false, data.errorMsg);
    infoLabel_->setText("Load failed: " + QString::fromStdString(data.errorMsg));
    QMessageBox::warning(this, "Error", QString::fromStdString(data.errorMsg));
    return;
  }

  std::unique_ptr<Mesh> mesh;
  try {
    mesh = ModelBuilder::BuildFromRawData(data);
  } catch (const std::exception& e) {
    facade_->NotifyLoadFinished(false, e.what());
    infoLabel_->setText("Load failed: " + QString::fromStdString(e.what()));
    QMessageBox::warning(this, "Error", QString::fromStdString(e.what()));
    return;
  }

  SceneObject* obj = facade_->GetScene()->AddObject(std::move(mesh));
  facade_->GetScene()->SetSelected({obj});
  glWidget_->fitToScene();
  updateModelInfo();
  infoLabel_->setText("Model loaded successfully");
  facade_->NotifyLoadFinished(true);
}

// ============================================================================
// Слоты Undo/Redo, Screenshot, GIF
// ============================================================================

void MainWindow::onUndo() {
    if (!facade_->CanTransform()) return;
    facade_->Undo();
    updateUIFromSelection();
}

void MainWindow::onRedo() {
    if (!facade_->CanTransform()) return;
    facade_->Redo();
    updateUIFromSelection();
}

void MainWindow::onScreenshot() {
    if (!facade_->CanSaveScreenshot()) return;
    glWidget_->update();
    QApplication::processEvents();
    QImage image = glWidget_->grabFramebuffer();
    if (image.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to capture screenshot (null image)");
        return;
    }
    image = image.convertToFormat(QImage::Format_RGB32);
    QString selectedFilter;
    QString fileName = QFileDialog::getSaveFileName(
        this, "Save Screenshot", "", "BMP (*.bmp);;JPEG (*.jpg *.jpeg)",
        &selectedFilter);
    if (fileName.isEmpty()) return;
    if (selectedFilter.contains("*.bmp") && !fileName.endsWith(".bmp", Qt::CaseInsensitive))
        fileName += ".bmp";
    else if (selectedFilter.contains("*.jpg") && !fileName.endsWith(".jpg", Qt::CaseInsensitive) &&
             !fileName.endsWith(".jpeg", Qt::CaseInsensitive))
        fileName += ".jpg";
    QFileInfo fileInfo(fileName);
    QDir dir = fileInfo.absoluteDir();
    if (!dir.exists() && !dir.mkpath(".")) {
        QMessageBox::warning(this, "Error", "Cannot create directory for screenshot");
        return;
    }
    if (!image.save(fileName)) {
        qDebug() << "Failed to save screenshot to:" << fileName;
        QMessageBox::warning(this, "Error", "Failed to save screenshot");
    }
}

void MainWindow::startAutoRotation() {
    auto selected = facade_->GetSelected();
    if (selected.empty()) return;
    rotatingObject_ = selected.front();
    originalRotationY_ = rotatingObject_->GetTransform().GetRotation().y;
    currentRotationAngle_ = originalRotationY_;
    rotationTimer_ = new QTimer(this);
    connect(rotationTimer_, &QTimer::timeout, this, &MainWindow::rotateStep);
    rotationTimer_->start(1000 / 30);
}

void MainWindow::rotateStep() {
    if (!rotatingObject_) return;
    currentRotationAngle_ += 2.0f;
    if (currentRotationAngle_ >= 360.0f) currentRotationAngle_ -= 360.0f;
    Point newRot = rotatingObject_->GetTransform().GetRotation();
    newRot.y = currentRotationAngle_;
    rotatingObject_->GetTransform().SetRotation(newRot);
    facade_->GetScene()->NotifyTransformChanged(rotatingObject_);
}

void MainWindow::stopAutoRotation() {
    if (rotationTimer_) {
        rotationTimer_->stop();
        delete rotationTimer_;
        rotationTimer_ = nullptr;
    }
    if (rotatingObject_) {
        Point newRot = rotatingObject_->GetTransform().GetRotation();
        newRot.y = originalRotationY_;
        rotatingObject_->GetTransform().SetRotation(newRot);
        facade_->GetScene()->NotifyTransformChanged(rotatingObject_);
        rotatingObject_ = nullptr;
    }
}

void MainWindow::onRecordGif() {
    if (gifRecorder_->isRecording()) {
        if (!facade_->CanStopGifRecording()) return;
        gifRecorder_->stop();
        facade_->NotifyGifRecordingFinished();
        stopAutoRotation();
        return;
    } else {
        if (!facade_->CanStartGifRecording()) return;
    }

    QString fileName = QFileDialog::getSaveFileName(this, "Save GIF", "", "GIF (*.gif)");
    if (fileName.isEmpty()) return;
    if (!gifRecorder_->startRecording(fileName, 640, 480, 10, 5)) {
        QMessageBox::warning(this, "Error", "Failed to start recording");
        return;
    }
    facade_->NotifyGifRecordingStarted();
    recordBtn_->setText("Stop Recording");
    startAutoRotation();
}

void MainWindow::onGifFinished(bool success, const QString& error) {
    facade_->NotifyGifRecordingFinished();
    recordBtn_->setText("Record GIF");
    stopAutoRotation();
    if (!success && !error.isEmpty()) {
        QMessageBox::warning(this, "Error", error);
    }
}

void MainWindow::onFit() {
    if (!facade_->CanTransform()) return;
    auto selected = facade_->GetSelected();
    if (selected.empty()) return;
    Mesh* mesh = dynamic_cast<Mesh*>(selected.front());
    if (!mesh) return;
    auto bbox = mesh->GetBoundingBox();
    Point center = {(bbox.min.x + bbox.max.x) / 2,
                    (bbox.min.y + bbox.max.y) / 2,
                    (bbox.min.z + bbox.max.z) / 2};
    float size = std::max({bbox.max.x - bbox.min.x,
                           bbox.max.y - bbox.min.y,
                           bbox.max.z - bbox.min.z});
    if (size < 1e-6f) size = 1.0f;
    glWidget_->fitToBoundingBox(center, size);
}

// ============================================================================
// Вспомогательные методы для слотов трансформаций
// ============================================================================

void MainWindow::applyMove(int axis, double value) {
  auto selected = facade_->GetSelected();
  if (selected.empty()) return;
  Point delta{0, 0, 0};
  switch (axis) {
    case 0: delta.x = static_cast<float>(value) - selected.front()->GetTransform().GetPosition().x; break;
    case 1: delta.y = static_cast<float>(value) - selected.front()->GetTransform().GetPosition().y; break;
    case 2: delta.z = static_cast<float>(value) - selected.front()->GetTransform().GetPosition().z; break;
  }
  facade_->MoveSelected(delta);
}

void MainWindow::applyRotate(int axis, double value) {
  auto selected = facade_->GetSelected();
  if (selected.empty()) return;
  Point delta{0, 0, 0};
  switch (axis) {
    case 0: delta.x = static_cast<float>(value) - selected.front()->GetTransform().GetRotation().x; break;
    case 1: delta.y = static_cast<float>(value) - selected.front()->GetTransform().GetRotation().y; break;
    case 2: delta.z = static_cast<float>(value) - selected.front()->GetTransform().GetRotation().z; break;
  }
  facade_->RotateSelected(delta);
}

void MainWindow::applyScale(int axis, double value) {
  auto selected = facade_->GetSelected();
  if (selected.empty()) return;
  Point delta{0, 0, 0};
  switch (axis) {
    case 0: delta.x = static_cast<float>(value) - selected.front()->GetTransform().GetScale().x; break;
    case 1: delta.y = static_cast<float>(value) - selected.front()->GetTransform().GetScale().y; break;
    case 2: delta.z = static_cast<float>(value) - selected.front()->GetTransform().GetScale().z; break;
  }
  facade_->ScaleSelected(delta);
}

// ============================================================================
// Слоты трансформаций (используют apply*)
// ============================================================================

void MainWindow::onMoveXChanged(double value) { applyMove(0, value); }
void MainWindow::onMoveYChanged(double value) { applyMove(1, value); }
void MainWindow::onMoveZChanged(double value) { applyMove(2, value); }

void MainWindow::onRotateXChanged(double value) { applyRotate(0, value); }
void MainWindow::onRotateYChanged(double value) { applyRotate(1, value); }
void MainWindow::onRotateZChanged(double value) { applyRotate(2, value); }

void MainWindow::onScaleXChanged(double value) { applyScale(0, value); }
void MainWindow::onScaleYChanged(double value) { applyScale(1, value); }
void MainWindow::onScaleZChanged(double value) { applyScale(2, value); }

// ============================================================================
// Слоты настроек
// ============================================================================

void MainWindow::onProjectionChanged(int index) {
    Settings::instance().setProjectionType(static_cast<Settings::ProjectionType>(index));
}
void MainWindow::onBackgroundColorClicked() {
    QColor color = QColorDialog::getColor(Settings::instance().backgroundColor(), this);
    if (color.isValid()) {
        Settings::instance().setBackgroundColor(color);
        bgColorBtn_->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}
void MainWindow::onEdgeColorClicked() {
    QColor color = QColorDialog::getColor(Settings::instance().edgeColor(), this);
    if (color.isValid()) {
        Settings::instance().setEdgeColor(color);
        edgeColorBtn_->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}
void MainWindow::onVertexColorClicked() {
    QColor color = QColorDialog::getColor(Settings::instance().vertexColor(), this);
    if (color.isValid()) {
        Settings::instance().setVertexColor(color);
        vertexColorBtn_->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}
void MainWindow::onSelectedEdgeColorClicked() {
    QColor color = QColorDialog::getColor(Settings::instance().selectedEdgeColor(), this);
    if (color.isValid()) {
        Settings::instance().setSelectedEdgeColor(color);
        selectedEdgeColorBtn_->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}
void MainWindow::onSelectedVertexColorClicked() {
    QColor color = QColorDialog::getColor(Settings::instance().selectedVertexColor(), this);
    if (color.isValid()) {
        Settings::instance().setSelectedVertexColor(color);
        selectedVertexColorBtn_->setStyleSheet(QString("background-color: %1").arg(color.name()));
    }
}
void MainWindow::onEdgeThicknessChanged(double value) {
    Settings::instance().setEdgeThickness(static_cast<float>(value));
}
void MainWindow::onVertexTypeChanged(int index) {
    Settings::instance().setVertexType(static_cast<Settings::VertexType>(index));
}
void MainWindow::onVertexSizeChanged(double value) {
    Settings::instance().setVertexSize(static_cast<float>(value));
}
void MainWindow::onEdgeTypeChanged(int index) {
    Settings::instance().setEdgeType(static_cast<Settings::EdgeType>(index));
}
void MainWindow::onDashFactorChanged(double value) {
    Settings::instance().setDashFactor(static_cast<float>(value));
}

// ============================================================================
// Обновление UI
// ============================================================================

void MainWindow::updateUIFromSelection() {
    auto selected = facade_->GetSelected();
    if (selected.empty()) {
        moveXSpin_->blockSignals(true);
        moveYSpin_->blockSignals(true);
        moveZSpin_->blockSignals(true);
        rotateXSpin_->blockSignals(true);
        rotateYSpin_->blockSignals(true);
        rotateZSpin_->blockSignals(true);
        scaleXSpin_->blockSignals(true);
        scaleYSpin_->blockSignals(true);
        scaleZSpin_->blockSignals(true);
        moveXSpin_->setValue(0);
        moveYSpin_->setValue(0);
        moveZSpin_->setValue(0);
        rotateXSpin_->setValue(0);
        rotateYSpin_->setValue(0);
        rotateZSpin_->setValue(0);
        scaleXSpin_->setValue(1);
        scaleYSpin_->setValue(1);
        scaleZSpin_->setValue(1);
        moveXSpin_->blockSignals(false);
        moveYSpin_->blockSignals(false);
        moveZSpin_->blockSignals(false);
        rotateXSpin_->blockSignals(false);
        rotateYSpin_->blockSignals(false);
        rotateZSpin_->blockSignals(false);
        scaleXSpin_->blockSignals(false);
        scaleYSpin_->blockSignals(false);
        scaleZSpin_->blockSignals(false);
        return;
    }
    SceneObject* obj = selected.front();
    Point pos = obj->GetTransform().GetPosition();
    Point rot = obj->GetTransform().GetRotation();
    Point scale = obj->GetTransform().GetScale();
    moveXSpin_->blockSignals(true);
    moveYSpin_->blockSignals(true);
    moveZSpin_->blockSignals(true);
    rotateXSpin_->blockSignals(true);
    rotateYSpin_->blockSignals(true);
    rotateZSpin_->blockSignals(true);
    scaleXSpin_->blockSignals(true);
    scaleYSpin_->blockSignals(true);
    scaleZSpin_->blockSignals(true);
    moveXSpin_->setValue(pos.x);
    moveYSpin_->setValue(pos.y);
    moveZSpin_->setValue(pos.z);
    rotateXSpin_->setValue(rot.x);
    rotateYSpin_->setValue(rot.y);
    rotateZSpin_->setValue(rot.z);
    scaleXSpin_->setValue(scale.x);
    scaleYSpin_->setValue(scale.y);
    scaleZSpin_->setValue(scale.z);
    moveXSpin_->blockSignals(false);
    moveYSpin_->blockSignals(false);
    moveZSpin_->blockSignals(false);
    rotateXSpin_->blockSignals(false);
    rotateYSpin_->blockSignals(false);
    rotateZSpin_->blockSignals(false);
    scaleXSpin_->blockSignals(false);
    scaleYSpin_->blockSignals(false);
    scaleZSpin_->blockSignals(false);
}

void MainWindow::updateModelInfo() {
    auto selected = facade_->GetSelected();
    if (selected.empty()) {
        infoLabel_->setText("No model selected");
        return;
    }
    Mesh* mesh = dynamic_cast<Mesh*>(selected.front());
    if (!mesh) {
        infoLabel_->setText("Selected object is not a mesh");
        return;
    }
    QString fileName = QString::fromStdString(mesh->GetSourceFile());
    QFileInfo fileInfo(fileName);
    QString shortName = fileInfo.fileName();
    QString text = QString("File: %1 | Vertices: %2 | Edges: %3")
                       .arg(shortName)
                       .arg(mesh->VertexCount())
                       .arg(mesh->EdgeCount());
    infoLabel_->setText(text);
}

void MainWindow::updateDisplayUI() {
    auto& s = Settings::instance();
    bgColorBtn_->setStyleSheet(QString("background-color: %1").arg(s.backgroundColor().name()));
    edgeColorBtn_->setStyleSheet(QString("background-color: %1").arg(s.edgeColor().name()));
    vertexColorBtn_->setStyleSheet(QString("background-color: %1").arg(s.vertexColor().name()));
    selectedEdgeColorBtn_->setStyleSheet(QString("background-color: %1").arg(s.selectedEdgeColor().name()));
    selectedVertexColorBtn_->setStyleSheet(QString("background-color: %1").arg(s.selectedVertexColor().name()));
    projectionCombo_->setCurrentIndex(static_cast<int>(s.projectionType()));
    edgeThicknessSpin_->setValue(s.edgeThickness());
    vertexTypeCombo_->setCurrentIndex(static_cast<int>(s.vertexType()));
    vertexSizeSpin_->setValue(s.vertexSize());
    edgeTypeCombo_->setCurrentIndex(static_cast<int>(s.edgeType()));
    dashFactorSpin_->setValue(s.dashFactor());
    if (flipNormalsCheckBox_) flipNormalsCheckBox_->setChecked(s.flipNormals());
    if (objectColorBtn_) objectColorBtn_->setStyleSheet(QString("background-color: %1").arg(s.objectColor().name()));
}

void MainWindow::updateUiState(AppState newState) {
    bool canLoad = (newState == AppState::Idle || newState == AppState::Ready);
    bool canTransform = (newState == AppState::Ready);
    bool canStartGif = (newState == AppState::Ready);
    bool canStopGif = (newState == AppState::RecordingGif);
    bool canScreenshot = (newState == AppState::Ready);
    newBtn_->setEnabled(canLoad);
    openBtn_->setEnabled(canLoad);
    loadBtn_->setEnabled(canLoad);
    undoBtn_->setEnabled(canTransform);
    redoBtn_->setEnabled(canTransform);
    screenshotBtn_->setEnabled(canScreenshot && newState != AppState::RecordingGif);
    recordBtn_->setEnabled(canStartGif || canStopGif);
    fitBtn_->setEnabled(canTransform);
    if (newState == AppState::RecordingGif) recordBtn_->setText("Stop Recording");
    else recordBtn_->setText("Record GIF");
    if (newState == AppState::Loading) {
        setCursor(Qt::WaitCursor);
        statusBar()->showMessage(tr("Loading model..."));
    } else {
        setCursor(Qt::ArrowCursor);
        if (newState == AppState::Ready) statusBar()->showMessage(tr("Model loaded. Ready."), 2000);
        else if (newState == AppState::Idle) statusBar()->showMessage(tr("No model loaded."));
    }
    if (newState == AppState::Error) {
        QString errorMsg = QString::fromStdString(facade_->GetErrorMessage());
        QMessageBox::critical(this, tr("Error"), errorMsg);
        facade_->ResetError();
    }
}

// ============================================================================
// SceneObserver
// ============================================================================

void MainWindow::OnObjectAdded(SceneObject* object) { (void)object; updateModelInfo(); }
void MainWindow::OnObjectRemoved(SceneObject* object) { (void)object; updateModelInfo(); }
void MainWindow::OnSelectionChanged(const std::vector<SceneObject*>& selected) { (void)selected; updateUIFromSelection(); updateModelInfo(); }
void MainWindow::OnTransformChanged(SceneObject* object) { (void)object; updateUIFromSelection(); }
void MainWindow::closeEvent(QCloseEvent* event) { event->accept(); }

// ============================================================================
// Слоты освещения, затенения, флип нормалей
// ============================================================================

void MainWindow::onShadingTypeChanged(int index) {
  OpenGLRenderer::ShadingType type;
  switch (index) {
    case 0: type = OpenGLRenderer::ShadingType::Wireframe; break;
    case 1: type = OpenGLRenderer::ShadingType::Flat;       break;
    case 2: type = OpenGLRenderer::ShadingType::Phong;      break;
    default: type = OpenGLRenderer::ShadingType::Wireframe; break;
  }

  DEBUG_PRINT("=== Shading type changed to " << (index == 0 ? "Wireframe" : (index == 1 ? "Flat" : "Phong")) << " ===");
  glWidget_->setShadingType(type);
}

void MainWindow::onFlipNormalsChecked(bool checked) {
    glWidget_->setFlipNormals(checked);
    Settings::instance().setFlipNormals(checked);
}

// ============================================================================
// Слоты текстур и цвета
// ============================================================================

void MainWindow::onLoadTexture() {
    QString fileName = QFileDialog::getOpenFileName(this, "Load Texture", "", "BMP Files (*.bmp)");
    if (fileName.isEmpty()) return;
    QImage texture;
    if (!texture.load(fileName)) {
        QMessageBox::warning(this, "Error", "Failed to load texture");
        return;
    }
    if (texture.width() != texture.height() || texture.width() > 1024) {
        QMessageBox::warning(this, "Error", "Texture must be square and ≤1024×1024");
        return;
    }
    glWidget_->setTexture(texture);
    Settings::instance().setTexturePath(fileName);
}
void MainWindow::onClearTexture() {
    glWidget_->clearTexture();
    Settings::instance().setTexturePath("");
}
void MainWindow::onObjectColorClicked() {
    QColor color = QColorDialog::getColor(Settings::instance().objectColor(), this);
    if (color.isValid()) {
        Settings::instance().setObjectColor(color);
        objectColorBtn_->setStyleSheet(QString("background-color: %1").arg(color.name()));
        glWidget_->setObjectColor(color);
    }
}

void MainWindow::onSaveUVMap() {
    DEBUG_PRINT("=== onSaveUVMap called ===");
    
    auto selected = facade_->GetSelected();
    if (selected.empty()) {
        QMessageBox::warning(this, "Error", "No model selected");
        DEBUG_PRINT("No model selected");
        return;
    }
    
    Mesh* mesh = dynamic_cast<Mesh*>(selected.front());
    if (!mesh) {
        QMessageBox::warning(this, "Error", "Selected object is not a mesh");
        DEBUG_PRINT("Selected object is not a mesh");
        return;
    }
    
    const auto& uvs = mesh->GetUVs();
    const auto& edges = mesh->GetEdges();
    DEBUG_PRINT("UVs count: " << uvs.size() << ", Edges count: " << edges.size());
    
    if (uvs.empty() || edges.empty()) {
        QMessageBox::warning(this, "Error", "Model has no UV coordinates or edges");
        DEBUG_PRINT("UVs or edges empty");
        return;
    }
    
    QString texturePath = Settings::instance().texturePath();
    DEBUG_PRINT("Texture path from settings: " << texturePath);
    
    QImage texture;
    if (texturePath.isEmpty() || !texture.load(texturePath)) {
        QMessageBox::warning(this, "Error", "No texture loaded or failed to load texture file");
        DEBUG_PRINT("Failed to load texture from path: " << texturePath);
        return;
    }
    
    DEBUG_PRINT("Texture loaded: " << texture.width() << "x" << texture.height());
    
    QImage uvMap = texture.convertToFormat(QImage::Format_RGB32);
    if (uvMap.isNull()) {
        QMessageBox::warning(this, "Error", "Failed to convert texture to RGB32");
        DEBUG_PRINT("Conversion to RGB32 failed");
        return;
    }
    
    QPainter painter(&uvMap);
    if (!painter.isActive()) {
        QMessageBox::warning(this, "Error", "Failed to start painting on texture");
        DEBUG_PRINT("Painter not active");
        return;
    }
    
    painter.setPen(QPen(Qt::red, 2));
    int linesDrawn = 0;
    for (const auto& edge : edges) {
        if (edge.v1 < uvs.size() && edge.v2 < uvs.size()) {
            const Point2D& p1 = uvs[edge.v1];
            const Point2D& p2 = uvs[edge.v2];
            QPointF qp1(p1.u * uvMap.width(), (1.0f - p1.v) * uvMap.height());
            QPointF qp2(p2.u * uvMap.width(), (1.0f - p2.v) * uvMap.height());
            painter.drawLine(qp1, qp2);
            linesDrawn++;
        }
    }
    painter.end();
    DEBUG_PRINT("Drawn " << linesDrawn << " lines");
    
    QString fileName = QFileDialog::getSaveFileName(this, "Save UV Map", "", "BMP (*.bmp)");
    if (fileName.isEmpty()) {
        DEBUG_PRINT("Save dialog cancelled");
        return;
    }
    
    if (!fileName.endsWith(".bmp", Qt::CaseInsensitive)) {
        fileName += ".bmp";
    }
    
    if (uvMap.save(fileName)) {
        DEBUG_PRINT("UV map saved successfully to: " << fileName);
        QMessageBox::information(this, "Success", "UV map saved successfully");
    } else {
        DEBUG_PRINT("Failed to save UV map to: " << fileName);
        QMessageBox::warning(this, "Error", "Failed to save UV map");
    }
}

void MainWindow::onSmoothingFactorChanged(double value) {
  DEBUG_PRINT("=== onSmoothingFactorChanged: " << value << " ===");
  Settings::instance().setSmoothingFactor(static_cast<float>(value));
  auto selected = facade_->GetSelected();
  if (selected.empty()) {
    DEBUG_PRINT("No model selected, skipping");
    return;
  }
  Mesh* mesh = dynamic_cast<Mesh*>(selected.front());
  if (!mesh) {
    DEBUG_PRINT("Selected object is not a mesh");
    return;
  }
  
  DEBUG_PRINT("Updating mesh: " << mesh << " file: " << mesh->GetSourceFile().c_str());
  DEBUG_PRINT("  vertices: " << mesh->VertexCount() << ", normals: " << mesh->GetNormals().size());
  
  mesh->ComputeNormals(value);
  
  const auto& newNormals = mesh->GetNormals();
  if (!newNormals.empty()) {
    DEBUG_PRINT("  first normal after: (" << newNormals[0].x << ", " << newNormals[0].y << ", " << newNormals[0].z << ")");
  }
  
  glWidget_->UpdateMeshBuffers(mesh);
  glWidget_->update();
  DEBUG_PRINT("Mesh buffers updated and widget repaint requested");
}

void MainWindow::connectSignals() {
  // Трансформации
  connect(moveXSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onMoveXChanged);
  connect(moveYSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onMoveYChanged);
  connect(moveZSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onMoveZChanged);
  connect(rotateXSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onRotateXChanged);
  connect(rotateYSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onRotateYChanged);
  connect(rotateZSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onRotateZChanged);
  connect(scaleXSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onScaleXChanged);
  connect(scaleYSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onScaleYChanged);
  connect(scaleZSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onScaleZChanged);

  // Файловые операции
  connect(newBtn_, &QPushButton::clicked, this, &MainWindow::onNewModel);
  connect(openBtn_, &QPushButton::clicked, this, &MainWindow::onOpenModel);
  connect(loadBtn_, &QPushButton::clicked, this, &MainWindow::onLoadModel);
  connect(undoBtn_, &QPushButton::clicked, this, &MainWindow::onUndo);
  connect(redoBtn_, &QPushButton::clicked, this, &MainWindow::onRedo);

  // Скриншоты, GIF
  connect(screenshotBtn_, &QPushButton::clicked, this, &MainWindow::onScreenshot);
  connect(recordBtn_, &QPushButton::clicked, this, &MainWindow::onRecordGif);
  connect(fitBtn_, &QPushButton::clicked, this, &MainWindow::onFit);

  // Текстуры
  connect(loadTextureBtn_, &QPushButton::clicked, this, &MainWindow::onLoadTexture);
  connect(clearTextureBtn_, &QPushButton::clicked, this, &MainWindow::onClearTexture);
  connect(saveUVMapBtn_, &QPushButton::clicked, this, &MainWindow::onSaveUVMap);

  // Настройки отображения
  connect(projectionCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::onProjectionChanged);
  connect(bgColorBtn_, &QPushButton::clicked, this, &MainWindow::onBackgroundColorClicked);
  connect(edgeColorBtn_, &QPushButton::clicked, this, &MainWindow::onEdgeColorClicked);
  connect(vertexColorBtn_, &QPushButton::clicked, this, &MainWindow::onVertexColorClicked);
  connect(edgeThicknessSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onEdgeThicknessChanged);
  connect(vertexTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::onVertexTypeChanged);
  connect(vertexSizeSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onVertexSizeChanged);
  connect(selectedEdgeColorBtn_, &QPushButton::clicked, this, &MainWindow::onSelectedEdgeColorClicked);
  connect(selectedVertexColorBtn_, &QPushButton::clicked, this, &MainWindow::onSelectedVertexColorClicked);
  connect(edgeTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
          this, &MainWindow::onEdgeTypeChanged);
  connect(dashFactorSpin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
          this, &MainWindow::onDashFactorChanged);

  // Освещение и шейдинг
  connect(shadingTypeCombo_, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::onShadingTypeChanged);

  // Ошибки шейдеров
  connect(glWidget_, &GLWidget::shaderError, this, [this](const QString& msg) {
    QMessageBox::critical(this, tr("Shader Error"), msg);
  });
}

}  // namespace s21
