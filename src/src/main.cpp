#include <QApplication>
#include <QDebug>  // <-- добавить
#include <QMessageBox>
#include <memory>

#include "facade/facade.h"
#include "loaders/loader_factory.h"
#include "loaders/obj_loader.h"
#include "scene/scene.h"
#include "view/gl_widget.h"
#include "view/main_window.h"

int main(int argc, char* argv[]) {
  QApplication app(argc, argv);

  // Регистрация загрузчика
  s21::LoaderFactory::Instance().Register(
      "obj", []() { return std::make_unique<s21::ObjLoader>(); });

  auto scene = std::make_shared<s21::Scene>();
  auto* glWidget = new s21::GLWidget;
  glWidget->SetScene(scene.get());
  scene->AddObserver(glWidget);

  auto facade = std::make_shared<s21::Facade>(scene);
  glWidget->SetFacade(facade.get());

  try {
    qDebug() << "Creating MainWindow...";
    s21::MainWindow mainWindow(facade, glWidget);
    qDebug() << "MainWindow created, calling show()";
    mainWindow.show();
    qDebug() << "show() returned, entering event loop";
    return app.exec();
  } catch (const std::exception& e) {
    QMessageBox::critical(nullptr, "Initialization Error",
                          QString("Failed to start application:\n") + e.what());
    return 1;
  }
}
