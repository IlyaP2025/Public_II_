#ifndef S21_GL_WIDGET_H
#define S21_GL_WIDGET_H

#include <QMouseEvent>
#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <memory>
#include <optional>
#include <vector>

#include "renderer/opengl_renderer.h"
#include "scene/camera.h"
#include "scene/mesh.h"
#include "scene/scene.h"
#include "scene/scene_observer.h"
#include "settings/settings.h"

namespace s21 {

class Facade;

class GLWidget : public QOpenGLWidget,
                 protected QOpenGLFunctions_3_3_Core,
                 public SceneObserver {
  Q_OBJECT

 public:
  explicit GLWidget(QWidget* parent = nullptr);
  ~GLWidget() override;

  void SetScene(Scene* scene);
  void SetFacade(Facade* facade) { facade_ = facade; }

  void fitToBoundingBox(const Point& center, float size);
  void fitToScene();

  void OnObjectAdded(SceneObject* object) override;
  void OnObjectRemoved(SceneObject* object) override;
  void OnSelectionChanged(const std::vector<SceneObject*>& selected) override;
  void OnTransformChanged(SceneObject* object) override;

  void setLightPosition(float x, float y, float z);
  void setLightColor(const QColor& color);
  void setShadingType(OpenGLRenderer::ShadingType type);
  void setFlipNormals(bool flip);
  void toggleFlipNormals();
  bool getFlipNormals() const;

  void setTexture(const QImage& image);
  void clearTexture();
  void setObjectColor(const QColor& color);

  void UpdateMeshBuffers(Mesh* mesh);

  void setShadingModel(Settings::ShadingModel model);

 signals:
  void shaderError(const QString& message);

 protected:
  void initializeGL() override;
  void resizeGL(int w, int h) override;
  void paintGL() override;

  void mousePressEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

 public slots:
  void onSettingsChanged();

 private:
  void UpdateMeshes();

  std::unique_ptr<OpenGLRenderer> renderer_;
  std::vector<const Mesh*> meshes_;
  Scene* scene_ = nullptr;
  Facade* facade_ = nullptr;
  std::unique_ptr<Camera> camera_;

  QPoint lastMousePos_;
  bool isRotating_ = false;
  bool isPanning_ = false;
  bool isDraggingObject_ = false;
  QPoint dragStartMousePos_;

  bool pendingLightPosition_ = false;
  float pendingLightX_ = 2.0f, pendingLightY_ = 3.0f, pendingLightZ_ = 4.0f;
  bool pendingLightColor_ = false;
  QColor pendingLightColorValue_ = Qt::white;
};

}  // namespace s21

#endif  // S21_GL_WIDGET_H
