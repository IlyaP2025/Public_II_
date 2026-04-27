
#include <cmath>
#include <QSettings>

#include "settings.h"
#include "common/lighting.h"

namespace s21 {

Settings::Settings(QObject* parent)
    : QObject(parent), settings_("School21", "3DViewer") {
  load();
}

Settings& Settings::instance() {
  static Settings instance;
  return instance;
}

// Геттеры
Settings::ProjectionType Settings::projectionType() const {
  return projectionType_;
}
QColor Settings::backgroundColor() const { return backgroundColor_; }
QColor Settings::edgeColor() const { return edgeColor_; }
QColor Settings::vertexColor() const { return vertexColor_; }
float Settings::edgeThickness() const { return edgeThickness_; }
Settings::EdgeType Settings::edgeType() const { return edgeType_; }
Settings::VertexType Settings::vertexType() const { return vertexType_; }
float Settings::vertexSize() const { return vertexSize_; }
QColor Settings::selectedEdgeColor() const { return selectedEdgeColor_; }
QColor Settings::selectedVertexColor() const { return selectedVertexColor_; }
float Settings::dashFactor() const { return dashFactor_; }
float Settings::smoothingFactor() const { return smoothingFactor_; }

// Сеттеры
void Settings::setSelectedVertexColor(const QColor& color) {
  if (selectedVertexColor_ != color) {
    selectedVertexColor_ = color;
    save();
    emit settingsChanged();
  }
}

void Settings::setSelectedEdgeColor(const QColor& color) {
  if (selectedEdgeColor_ != color) {
    selectedEdgeColor_ = color;
    save();
    emit settingsChanged();
  }
}

void Settings::setProjectionType(ProjectionType type) {
  if (projectionType_ != type) {
    projectionType_ = type;
    save();
    emit settingsChanged();
  }
}

void Settings::setBackgroundColor(const QColor& color) {
  if (backgroundColor_ != color) {
    backgroundColor_ = color;
    save();
    emit settingsChanged();
  }
}

void Settings::setEdgeColor(const QColor& color) {
  if (edgeColor_ != color) {
    edgeColor_ = color;
    save();
    emit settingsChanged();
  }
}

void Settings::setVertexColor(const QColor& color) {
  if (vertexColor_ != color) {
    vertexColor_ = color;
    save();
    emit settingsChanged();
  }
}

void Settings::setEdgeThickness(float thickness) {
  if (std::abs(edgeThickness_ - thickness) > 1e-6f) {
    edgeThickness_ = thickness;
    save();
    emit settingsChanged();
  }
}

void Settings::setEdgeType(EdgeType type) {
  if (edgeType_ != type) {
    edgeType_ = type;
    save();
    emit settingsChanged();
  }
}

void Settings::setVertexType(VertexType type) {
  if (vertexType_ != type) {
    vertexType_ = type;
    save();
    emit settingsChanged();
  }
}

void Settings::setVertexSize(float size) {
  if (std::abs(vertexSize_ - size) > 1e-6f) {
    vertexSize_ = size;
    save();
    emit settingsChanged();
  }
}

void Settings::setDashFactor(float factor) {
  if (std::abs(dashFactor_ - factor) > 1e-6f) {
    dashFactor_ = factor;
    save();
    emit settingsChanged();
  }
}

void Settings::setSmoothingFactor(float factor) {
  factor = std::clamp(factor, 0.0f, 1.0f);
  if (std::abs(smoothingFactor_ - factor) > 1e-6f) {
    smoothingFactor_ = factor;
    save();
    emit settingsChanged();
  }
}

Settings::ShadingModel Settings::shadingModel() const { return shadingModel_; }
void Settings::setShadingModel(ShadingModel model) {
  if (shadingModel_ != model) {
    shadingModel_ = model;
    save();
    emit settingsChanged();
  }
}

void Settings::load() {
  projectionType_ = static_cast<ProjectionType>(
      settings_
          .value("projectionType",
                 static_cast<int>(ProjectionType::Orthographic))
          .toInt());
  flipNormals_ = settings_.value("flipNormals", false).toBool();

  backgroundColor_ =
      settings_.value("backgroundColor", QColor(Qt::black)).value<QColor>();
  edgeColor_ = settings_.value("edgeColor", QColor(Qt::white)).value<QColor>();
  vertexColor_ =
      settings_.value("vertexColor", QColor(Qt::red)).value<QColor>();
  edgeThickness_ = settings_.value("edgeThickness", 1.0f).toFloat();
  edgeType_ = static_cast<EdgeType>(
      settings_.value("edgeType", static_cast<int>(EdgeType::Solid)).toInt());
  vertexType_ = static_cast<VertexType>(
      settings_.value("vertexType", static_cast<int>(VertexType::None))
          .toInt());
  vertexSize_ = settings_.value("vertexSize", 5.0f).toFloat();
  selectedEdgeColor_ =
      settings_.value("selectedEdgeColor", QColor(Qt::red)).value<QColor>();
  selectedVertexColor_ =
      settings_.value("selectedVertexColor", QColor(Qt::red)).value<QColor>();
  dashFactor_ = settings_.value("dashFactor", 5.0f).toFloat();
  texturePath_ = settings_.value("texturePath", "").toString();
  objectColor_ =
      settings_.value("objectColor", QColor(200, 200, 200)).value<QColor>();
  smoothingFactor_ = settings_.value("smoothingFactor", 1.0f).toFloat();
  shadingModel_ = static_cast<ShadingModel>(
      settings_.value("shadingModel", static_cast<int>(ShadingModel::Phong))
          .toInt());
}

void Settings::setFlipNormals(bool flip) {
  if (flipNormals_ != flip) {
    flipNormals_ = flip;
    save();
    emit settingsChanged();
  }
}

void Settings::save() {
  settings_.setValue("projectionType", static_cast<int>(projectionType_));
  settings_.setValue("backgroundColor", backgroundColor_);
  settings_.setValue("edgeColor", edgeColor_);
  settings_.setValue("vertexColor", vertexColor_);
  settings_.setValue("edgeThickness", edgeThickness_);
  settings_.setValue("edgeType", static_cast<int>(edgeType_));
  settings_.setValue("vertexType", static_cast<int>(vertexType_));
  settings_.setValue("vertexSize", vertexSize_);
  settings_.setValue("selectedEdgeColor", selectedEdgeColor_);
  settings_.setValue("selectedVertexColor", selectedVertexColor_);
  settings_.setValue("dashFactor", dashFactor_);
  settings_.setValue("flipNormals", flipNormals_);
  settings_.setValue("texturePath", texturePath_);
  settings_.setValue("objectColor", objectColor_);
  settings_.setValue("smoothingFactor", smoothingFactor_);
  settings_.setValue("shadingModel", static_cast<int>(shadingModel_));
}

QString Settings::texturePath() const { return texturePath_; }

void Settings::setTexturePath(const QString& path) {
  if (texturePath_ != path) {
    texturePath_ = path;
    save();
    emit settingsChanged();
  }
}

QColor Settings::objectColor() const { return objectColor_; }

void Settings::setObjectColor(const QColor& color) {
  if (objectColor_ != color) {
    objectColor_ = color;
    save();
    emit settingsChanged();
  }
}

void Settings::SaveLights(const std::vector<LightSource>& lights) {
  QSettings s;
  s.beginGroup("lights");
  s.setValue("count", static_cast<int>(lights.size()));
  for (size_t i = 0; i < lights.size(); ++i) {
    const auto& l = lights[i];
    s.beginGroup(QString::number(i));
    s.setValue("enabled", l.enabled);
    s.setValue("pos_x", static_cast<double>(l.position.x));
    s.setValue("pos_y", static_cast<double>(l.position.y));
    s.setValue("pos_z", static_cast<double>(l.position.z));
    s.setValue("ambient_r", static_cast<double>(l.ambient.r));
    s.setValue("ambient_g", static_cast<double>(l.ambient.g));
    s.setValue("ambient_b", static_cast<double>(l.ambient.b));
    s.setValue("diffuse_r", static_cast<double>(l.diffuse.r));
    s.setValue("diffuse_g", static_cast<double>(l.diffuse.g));
    s.setValue("diffuse_b", static_cast<double>(l.diffuse.b));
    s.setValue("specular_r", static_cast<double>(l.specular.r));
    s.setValue("specular_g", static_cast<double>(l.specular.g));
    s.setValue("specular_b", static_cast<double>(l.specular.b));
    s.endGroup();
  }
  s.endGroup();
}

std::vector<LightSource> Settings::LoadLights() const {
  QSettings s;
  std::vector<LightSource> lights;
  s.beginGroup("lights");
  int count = s.value("count", 0).toInt();
  for (int i = 0; i < count; ++i) {
    s.beginGroup(QString::number(i));
    LightSource l;
    l.enabled = s.value("enabled", true).toBool();
    l.position.x = s.value("pos_x", 0.0).toFloat();
    l.position.y = s.value("pos_y", 0.0).toFloat();
    l.position.z = s.value("pos_z", 0.0).toFloat();
    l.ambient.r  = s.value("ambient_r", 0.1f).toFloat();
    l.ambient.g  = s.value("ambient_g", 0.1f).toFloat();
    l.ambient.b  = s.value("ambient_b", 0.1f).toFloat();
    l.diffuse.r  = s.value("diffuse_r", 1.0f).toFloat();
    l.diffuse.g  = s.value("diffuse_g", 1.0f).toFloat();
    l.diffuse.b  = s.value("diffuse_b", 1.0f).toFloat();
    l.specular.r = s.value("specular_r", 1.0f).toFloat();
    l.specular.g = s.value("specular_g", 1.0f).toFloat();
    l.specular.b = s.value("specular_b", 1.0f).toFloat();
    lights.push_back(l);
    s.endGroup();
  }
  s.endGroup();
  return lights;
}

}  // namespace s21
