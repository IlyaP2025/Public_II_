#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QSettings>

#include "settings/settings.h"

using namespace s21;

class SettingsTest : public ::testing::Test {
 protected:
  void SetUp() override {
    Settings& s = Settings::instance();
    // Сбрасываем все настройки на значения по умолчанию
    s.setProjectionType(Settings::ProjectionType::Orthographic);
    s.setBackgroundColor(Qt::black);
    s.setEdgeColor(Qt::white);
    s.setVertexColor(Qt::red);
    s.setEdgeThickness(1.0f);
    s.setEdgeType(Settings::EdgeType::Solid);
    s.setVertexType(Settings::VertexType::None);
    s.setVertexSize(5.0f);
    s.setSelectedEdgeColor(Qt::red);
    s.setSelectedVertexColor(Qt::red);
    s.setDashFactor(5.0f);
    s.setSmoothingFactor(1.0f);
    s.setShadingModel(Settings::ShadingModel::Phong);
    s.setFlipNormals(false);
    s.setTexturePath("");
    s.setObjectColor(QColor(200, 200, 200));

    // Очищаем QSettings, чтобы не мешали сохранённые данные
    QSettings testSettings("TestSchool21", "Test3DViewer");
    testSettings.clear();
    testSettings.sync();
  }
};

TEST_F(SettingsTest, DefaultValues) {
  Settings& s = Settings::instance();
  EXPECT_EQ(s.projectionType(), Settings::ProjectionType::Orthographic);
  EXPECT_EQ(s.backgroundColor(), QColor(Qt::black));
  EXPECT_EQ(s.edgeColor(), QColor(Qt::white));
  EXPECT_EQ(s.vertexColor(), QColor(Qt::red));
  EXPECT_FLOAT_EQ(s.edgeThickness(), 1.0f);
  EXPECT_EQ(s.edgeType(), Settings::EdgeType::Solid);
  EXPECT_EQ(s.vertexType(), Settings::VertexType::None);
  EXPECT_FLOAT_EQ(s.vertexSize(), 5.0f);
  EXPECT_FLOAT_EQ(s.dashFactor(), 5.0f);
  EXPECT_FLOAT_EQ(s.smoothingFactor(), 1.0f);
  EXPECT_EQ(s.shadingModel(), Settings::ShadingModel::Phong);
  EXPECT_FALSE(s.flipNormals());
  EXPECT_EQ(s.objectColor(), QColor(200, 200, 200));
  // texturePath не проверяем, так как он мог остаться от других тестов
}

TEST_F(SettingsTest, SettersAndGetters) {
  Settings& s = Settings::instance();

  s.setProjectionType(Settings::ProjectionType::Perspective);
  EXPECT_EQ(s.projectionType(), Settings::ProjectionType::Perspective);

  s.setBackgroundColor(Qt::red);
  EXPECT_EQ(s.backgroundColor(), QColor(Qt::red));

  s.setEdgeColor(Qt::blue);
  EXPECT_EQ(s.edgeColor(), QColor(Qt::blue));

  s.setVertexColor(Qt::green);
  EXPECT_EQ(s.vertexColor(), QColor(Qt::green));

  s.setEdgeThickness(2.5f);
  EXPECT_FLOAT_EQ(s.edgeThickness(), 2.5f);

  s.setEdgeType(Settings::EdgeType::Dashed);
  EXPECT_EQ(s.edgeType(), Settings::EdgeType::Dashed);

  s.setVertexType(Settings::VertexType::Circle);
  EXPECT_EQ(s.vertexType(), Settings::VertexType::Circle);

  s.setVertexSize(10.0f);
  EXPECT_FLOAT_EQ(s.vertexSize(), 10.0f);

  s.setSelectedEdgeColor(Qt::yellow);
  EXPECT_EQ(s.selectedEdgeColor(), QColor(Qt::yellow));

  s.setSelectedVertexColor(Qt::cyan);
  EXPECT_EQ(s.selectedVertexColor(), QColor(Qt::cyan));

  s.setDashFactor(3.0f);
  EXPECT_FLOAT_EQ(s.dashFactor(), 3.0f);

  s.setSmoothingFactor(0.5f);
  EXPECT_FLOAT_EQ(s.smoothingFactor(), 0.5f);

  s.setShadingModel(Settings::ShadingModel::Flat);
  EXPECT_EQ(s.shadingModel(), Settings::ShadingModel::Flat);

  s.setFlipNormals(true);
  EXPECT_TRUE(s.flipNormals());

  s.setTexturePath("/path/to/tex.bmp");
  EXPECT_EQ(s.texturePath(), QString("/path/to/tex.bmp"));

  s.setObjectColor(Qt::magenta);
  EXPECT_EQ(s.objectColor(), QColor(Qt::magenta));
}

TEST_F(SettingsTest, SaveAndLoad) {
  Settings& s = Settings::instance();
  s.setProjectionType(Settings::ProjectionType::Perspective);
  s.setBackgroundColor(Qt::gray);
  s.setSmoothingFactor(0.3f);
  s.setShadingModel(Settings::ShadingModel::Flat);
  s.save();

  // Загружаем настройки заново
  s.load();
  EXPECT_EQ(s.projectionType(), Settings::ProjectionType::Perspective);
  EXPECT_EQ(s.backgroundColor(), QColor(Qt::gray));
  EXPECT_FLOAT_EQ(s.smoothingFactor(), 0.3f);
  EXPECT_EQ(s.shadingModel(), Settings::ShadingModel::Flat);
}
