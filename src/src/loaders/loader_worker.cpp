#include "loader_worker.h"

#include <memory>

#include "iloader.h"
#include "loader_factory.h"

namespace s21 {

LoaderWorker::LoaderWorker(QObject* parent) : QObject(parent) {}

void LoaderWorker::doLoad(const QString& filename) {
  std::vector<std::unique_ptr<SceneObject>> objects;
  std::string error;
  auto loader = LoaderFactory::Instance().CreateLoader("obj");
  if (!loader) {
    error = "No loader for .obj files";
    auto objects_ptr =
        std::make_shared<std::vector<std::unique_ptr<SceneObject>>>();
    emit finished(objects_ptr, QString::fromStdString(error));
    return;
  }
  bool ok = loader->Load(filename.toStdString(), objects, error);
  auto objects_ptr =
      std::make_shared<std::vector<std::unique_ptr<SceneObject>>>(
          std::move(objects));
  if (!ok) {
    emit finished(objects_ptr, QString::fromStdString(error));
  } else {
    emit finished(objects_ptr, QString());
  }
}

}  // namespace s21
