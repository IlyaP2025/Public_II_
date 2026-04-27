#ifndef S21_LOADER_WORKER_H
#define S21_LOADER_WORKER_H

#include <QObject>
#include <QString>
#include <memory>
#include <vector>

#include "scene/scene_object.h"

namespace s21 {

class LoaderWorker : public QObject {
  Q_OBJECT
 public:
  explicit LoaderWorker(QObject* parent = nullptr);

 public slots:
  void doLoad(const QString& filename);

 signals:
  void finished(
      std::shared_ptr<std::vector<std::unique_ptr<SceneObject>>> objects,
      const QString& error);
};

}  // namespace s21

#endif  // S21_LOADER_WORKER_H
