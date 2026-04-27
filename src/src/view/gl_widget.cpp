#include "gl_widget.h"

#include <QDebug>
#include <cfloat>
#include <cmath>
#include <iostream>
#include <limits>

#include "common/debug.h"
#include "facade/facade.h"
#include "scene/picking_manager.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace {
const float ROTATION_SENSITIVITY = 0.5f;
const float PAN_SENSITIVITY = 0.01f;
const float ZOOM_SENSITIVITY = 1.0f;
}  // namespace

namespace s21 {

void GLWidget::fitToBoundingBox(const Point& center, float size) {
  DEBUG_PRINT_FUNC();
  DEBUG_PRINT("center=(" << center.x << "," << center.y << "," << center.z
                         << ") size=" << size);

  if (!camera_) return;

  camera_->SetTarget(center.x, center.y, center.z);
  camera_->SetUp(0, 1, 0);

  Settings::ProjectionType projType = Settings::instance().projectionType();

  if (projType == Settings::ProjectionType::Orthographic) {
    float targetSize = size * 1.2f;
    float neededZoom = targetSize / 10.0f;
    camera_->SetZoomFactor(neededZoom);
    camera_->SetFarPlane(size * 2.0f + 100.0f);
    camera_->SetPosition(center.x, center.y, center.z + 5.0f);
  } else {
    float fov = 45.0f * static_cast<float>(M_PI) / 180.0f;
    float tanHalfFov = std::tan(fov / 2.0f);
    if (tanHalfFov < 1e-6f) tanHalfFov = 1e-6f;
    float aspect = static_cast<float>(width()) / static_cast<float>(height());
    float requiredSize = size;
    if (aspect > 1.0f) {
      requiredSize = size * aspect;
    }
    float distance = (requiredSize / 2.0f) / tanHalfFov * 1.2f;
    camera_->SetPosition(center.x, center.y, center.z + distance);
    camera_->SetNearPlane(0.1f);
    camera_->SetFarPlane(distance + size * 2.0f);
  }

  DEBUG_PRINT("Camera after fit: pos=("
              << camera_->GetPosition().x << ", " << camera_->GetPosition().y
              << ", " << camera_->GetPosition().z << "), target=("
              << camera_->GetTargetX() << ", " << camera_->GetTargetY() << ", "
              << camera_->GetTargetZ() << ")");
  update();
}

void GLWidget::fitToScene() {
  if (!scene_ || !camera_) return;
  auto meshes = scene_->GetAllMeshes();
  if (meshes.empty()) return;

  Point min(FLT_MAX, FLT_MAX, FLT_MAX);
  Point max(-FLT_MAX, -FLT_MAX, -FLT_MAX);
  for (const Mesh* mesh : meshes) {
    auto bbox = mesh->GetBoundingBox();
    if (bbox.min.x < min.x) min.x = bbox.min.x;
    if (bbox.min.y < min.y) min.y = bbox.min.y;
    if (bbox.min.z < min.z) min.z = bbox.min.z;
    if (bbox.max.x > max.x) max.x = bbox.max.x;
    if (bbox.max.y > max.y) max.y = bbox.max.y;
    if (bbox.max.z > max.z) max.z = bbox.max.z;
  }

  Point center = {(min.x + max.x) / 2, (min.y + max.y) / 2,
                  (min.z + max.z) / 2};
  float size = std::max({max.x - min.x, max.y - min.y, max.z - min.z});
  if (size < 1e-6f) size = 1.0f;
  DEBUG_PRINT("fitToScene: center=(" << center.x << "," << center.y << ","
                                     << center.z << ") size=" << size);
  fitToBoundingBox(center, size);
}

GLWidget::GLWidget(QWidget* parent) : QOpenGLWidget(parent) {
  connect(&Settings::instance(), &Settings::settingsChanged, this,
          &GLWidget::onSettingsChanged);
}

GLWidget::~GLWidget() {
  if (scene_) {
    scene_->RemoveObserver(this);
  }
}

void GLWidget::SetScene(Scene* scene) {
  scene_ = scene;
  UpdateMeshes();
}

void GLWidget::UpdateMeshes() {
  if (!scene_) return;
  meshes_ = scene_->GetAllMeshes();
  DEBUG_PRINT("UpdateMeshes: meshes count = " << meshes_.size());
  for (size_t i = 0; i < meshes_.size(); ++i) {
    DEBUG_PRINT("  mesh[" << i << "] = " << meshes_[i]
                          << " file: " << meshes_[i]->GetSourceFile().c_str()
                          << " vertices = " << meshes_[i]->VertexCount());
  }
}

void GLWidget::initializeGL() {
  initializeOpenGLFunctions();
  glClearColor(0.1f, 0.1f, 0.1f, 1.0f);

  try {
    renderer_ = std::make_unique<OpenGLRenderer>(
        static_cast<QOpenGLFunctions_3_3_Core*>(this), scene_);
    renderer_->Initialize();
  } catch (const std::exception& e) {
    QString errorMsg = QString::fromStdString(renderer_->GetShaderError());
    if (errorMsg.isEmpty()) errorMsg = e.what();
    emit shaderError(errorMsg);
    renderer_.reset();
  }

  if (renderer_) {
    renderer_->SetSettings(&Settings::instance());
    renderer_->SetFlipNormals(Settings::instance().flipNormals());

    if (pendingLightPosition_) {
      renderer_->SetLightPosition(pendingLightX_, pendingLightY_,
                                  pendingLightZ_);
    }
    if (pendingLightColor_) {
      renderer_->SetLightColor(pendingLightColorValue_);
    }
  }

  camera_ = std::make_unique<Camera>();
  camera_->SetProjectionType(Settings::instance().projectionType());

  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LESS);
}

void GLWidget::resizeGL(int w, int h) { glViewport(0, 0, w, h); }

void GLWidget::paintGL() {
  DEBUG_PRINT("paintGL: meshes_.size() = " << meshes_.size());
  QColor bg = Settings::instance().backgroundColor();
  glClearColor(bg.redF(), bg.greenF(), bg.blueF(), 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  if (!renderer_ || !scene_ || !camera_) {
    DEBUG_PRINT("paintGL: renderer, scene or camera is null");
    return;
  }

  float aspect = static_cast<float>(width()) / static_cast<float>(height());
  S21Matrix proj = camera_->GetProjectionMatrix(aspect);
  S21Matrix view = camera_->GetViewMatrix();

  renderer_->Render(meshes_, view, proj);
}

void GLWidget::mousePressEvent(QMouseEvent* event) {
  if (event->button() == Qt::RightButton) {
    isRotating_ = true;
    lastMousePos_ = event->pos();
    event->accept();
    return;
  }
  if (event->button() == Qt::MiddleButton) {
    isPanning_ = true;
    lastMousePos_ = event->pos();
    event->accept();
    return;
  }

  if (event->button() == Qt::LeftButton) {
    if (!scene_ || !camera_) {
      QOpenGLWidget::mousePressEvent(event);
      return;
    }

    QPointF pos = event->position();
    int mx = static_cast<int>(pos.x());
    int my = static_cast<int>(pos.y());

    Mesh* selected_mesh = PickingManager::PickObject(mx, my, width(), height(),
                                                     *camera_, *scene_);

    if (selected_mesh) {
      scene_->SetSelected({selected_mesh});
      isDraggingObject_ = true;
      dragStartMousePos_ = event->pos();
    } else {
      scene_->SetSelected({});
    }
  }

  QOpenGLWidget::mousePressEvent(event);
}

void GLWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (event->button() == Qt::RightButton) {
    isRotating_ = false;
    event->accept();
    return;
  }
  if (event->button() == Qt::MiddleButton) {
    isPanning_ = false;
    event->accept();
    return;
  }
  if (event->button() == Qt::LeftButton) {
    isDraggingObject_ = false;
    event->accept();
    return;
  }
  QOpenGLWidget::mouseReleaseEvent(event);
}

void GLWidget::mouseMoveEvent(QMouseEvent* event) {
  if (facade_ && !facade_->CanTransform()) {
    QOpenGLWidget::mouseMoveEvent(event);
    return;
  }

  QPoint delta = event->pos() - lastMousePos_;
  if (isRotating_ && camera_) {
    camera_->Rotate(delta.x() * ROTATION_SENSITIVITY,
                    -delta.y() * ROTATION_SENSITIVITY);
    update();
  } else if (isPanning_ && camera_) {
    camera_->Pan(delta.x() * PAN_SENSITIVITY, -delta.y() * PAN_SENSITIVITY);
    update();
  } else if (isDraggingObject_ && facade_) {
    auto selected = scene_->GetSelected();
    if (!selected.empty()) {
      QPoint deltaPixels = event->pos() - dragStartMousePos_;
      if (deltaPixels.isNull()) return;

      const float speed = 0.005f;

      Point deltaWorld(-static_cast<float>(deltaPixels.x()) * speed,
                       -static_cast<float>(deltaPixels.y()) * speed, 0.0f);

      facade_->MoveSelected(deltaWorld);
      dragStartMousePos_ = event->pos();
      update();
    }
  }
  lastMousePos_ = event->pos();
  QOpenGLWidget::mouseMoveEvent(event);
}

void GLWidget::wheelEvent(QWheelEvent* event) {
  if (facade_ && !facade_->CanTransform()) {
    event->accept();
    return;
  }
  if (camera_) {
    float delta = event->angleDelta().y() / 120.0f * ZOOM_SENSITIVITY;
    camera_->Zoom(delta);
    update();
  }
  event->accept();
}

void GLWidget::onSettingsChanged() {
  if (camera_) {
    camera_->SetProjectionType(Settings::instance().projectionType());
  }
  if (renderer_) {
    renderer_->SetFlipNormals(Settings::instance().flipNormals());
  }
  update();
}

void GLWidget::OnObjectAdded(SceneObject* object) {
  (void)object;
  UpdateMeshes();
  update();
}

void GLWidget::OnObjectRemoved(SceneObject* object) {
  qDebug() << "GLWidget::OnObjectRemoved: object =" << object;
  Mesh* mesh = dynamic_cast<Mesh*>(object);
  DEBUG_PRINT("GLWidget::OnObjectRemoved: mesh = " << mesh);
  if (mesh && renderer_) {
    renderer_->RemoveMesh(mesh);
    DEBUG_PRINT("Removed mesh from renderer cache");
  }
  UpdateMeshes();
  update();
}

void GLWidget::OnSelectionChanged(const std::vector<SceneObject*>& selected) {
  (void)selected;
  DEBUG_PRINT(
      "GLWidget::OnSelectionChanged: selected count = " << selected.size());
  update();
}

void GLWidget::OnTransformChanged(SceneObject* object) {
  if (renderer_ && object) {
    Mesh* mesh = dynamic_cast<Mesh*>(object);
    if (mesh) {
      renderer_->MarkMeshTransformChanged(mesh);
    }
  }
  update();
}

void GLWidget::setLightPosition(float x, float y, float z) {
  if (renderer_) {
    renderer_->SetLightPosition(x, y, z);
  } else {
    pendingLightPosition_ = true;
    pendingLightX_ = x;
    pendingLightY_ = y;
    pendingLightZ_ = z;
  }
  update();
}

void GLWidget::setLightColor(const QColor& color) {
  if (renderer_) {
    renderer_->SetLightColor(color);
  } else {
    pendingLightColor_ = true;
    pendingLightColorValue_ = color;
  }
  update();
}

void GLWidget::setShadingType(OpenGLRenderer::ShadingType type) {
  if (renderer_) renderer_->SetShadingType(type);
  update();
}

void GLWidget::setFlipNormals(bool flip) {
  if (renderer_) renderer_->SetFlipNormals(flip);
  update();
}

void GLWidget::toggleFlipNormals() {
  if (renderer_) {
    renderer_->SetFlipNormals(!renderer_->GetFlipNormals());
    update();
  }
}

bool GLWidget::getFlipNormals() const {
  return renderer_ ? renderer_->GetFlipNormals() : false;
}

void GLWidget::setTexture(const QImage& image) {
  if (renderer_) renderer_->SetTexture(image);
  update();
}

void GLWidget::clearTexture() {
  if (renderer_) renderer_->ClearTexture();
  update();
}

void GLWidget::setObjectColor(const QColor& color) {
  if (renderer_) renderer_->SetObjectColor(color);
  update();
}

void GLWidget::UpdateMeshBuffers(Mesh* mesh) {
  DEBUG_PRINT("GLWidget::UpdateMeshBuffers: mesh = "
              << mesh
              << " file: " << (mesh ? mesh->GetSourceFile().c_str() : "null"));
  if (renderer_ && mesh) {
    renderer_->UpdateMeshNormals(*mesh);
    update();  // гварантируем перерисовку
    DEBUG_PRINT("Renderer UpdateMesh called");
  } else {
    DEBUG_PRINT("Renderer or mesh is null");
  }
}

void GLWidget::setShadingModel(Settings::ShadingModel model) {
  if (renderer_) renderer_->SetShadingModel(model);
  update();
}

}  // namespace s21
