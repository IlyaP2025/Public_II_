#ifndef S21_SETTINGS_H
#define S21_SETTINGS_H

#include <QColor>
#include <QObject>
#include <QSettings>

#include "common/lighting.h"

namespace s21 {

class Settings : public QObject {
  Q_OBJECT

 public:
  static Settings& instance();

  // Тип проекции
  enum class ProjectionType { Orthographic, Perspective };
  ProjectionType projectionType() const;
  void setProjectionType(ProjectionType type);

  // Цвета
  QColor backgroundColor() const;
  QColor edgeColor() const;
  QColor vertexColor() const;
  QColor selectedEdgeColor() const;
  QColor selectedVertexColor() const;
  void setBackgroundColor(const QColor& color);
  void setEdgeColor(const QColor& color);
  void setVertexColor(const QColor& color);
  void setSelectedEdgeColor(const QColor& color);
  void setSelectedVertexColor(const QColor& color);

  // Толщина рёбер
  float edgeThickness() const;
  void setEdgeThickness(float thickness);

  // Тип рёбер
  enum class EdgeType { Solid, Dashed };
  EdgeType edgeType() const;
  void setEdgeType(EdgeType type);

  // Тип вершин
  enum class VertexType { None, Circle, Square };
  VertexType vertexType() const;
  void setVertexType(VertexType type);
  float vertexSize() const;
  void setVertexSize(float size);

  // Пунктирный шаг
  float dashFactor() const;
  void setDashFactor(float factor);

  // Нормали флип
  bool flipNormals() const { return flipNormals_; }
  void setFlipNormals(bool flip);

  // Сохранение/загрузка
  void load();
  void save();

  // Текстура
  QString texturePath() const;
  void setTexturePath(const QString& path);

  // Базовый цвет объекта
  QColor objectColor() const;
  void setObjectColor(const QColor& color);

  // Коэффициент сглаживания нормалей
  float smoothingFactor() const;
  void setSmoothingFactor(float factor);

  // Модель затенения
  enum class ShadingModel { Flat, Phong };
  ShadingModel shadingModel() const;
  void setShadingModel(ShadingModel model);

  // Освещение
  void SaveLights(const std::vector<LightSource>& lights);
  std::vector<LightSource> LoadLights() const;

 signals:
  void settingsChanged();

 private:
  explicit Settings(QObject* parent = nullptr);
  ~Settings() = default;

  ProjectionType projectionType_ = ProjectionType::Orthographic;
  QColor backgroundColor_ = Qt::black;
  QColor edgeColor_ = Qt::white;
  QColor vertexColor_ = Qt::red;
  QColor selectedEdgeColor_ = Qt::red;
  QColor selectedVertexColor_ = Qt::red;
  float edgeThickness_ = 1.0f;
  EdgeType edgeType_ = EdgeType::Solid;
  VertexType vertexType_ = VertexType::None;
  float vertexSize_ = 5.0f;
  float dashFactor_ = 5.0f;
  bool flipNormals_ = false;

  QSettings settings_;
  QString texturePath_;
  QColor objectColor_ = QColor(200, 200, 200);
  float smoothingFactor_ = 1.0f;
  ShadingModel shadingModel_ = ShadingModel::Phong;
};

}  // namespace s21

#endif  // S21_SETTINGS_H
