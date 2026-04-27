#ifndef S21_LIGHT_CONTROL_WIDGET_H
#define S21_LIGHT_CONTROL_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <vector>
#include "common/lighting.h"

namespace s21 {

class LightControlWidget : public QWidget {
    Q_OBJECT
public:
    explicit LightControlWidget(QWidget* parent = nullptr);
    void setLights(const std::vector<LightSource>& lights);
    std::vector<LightSource> getLights() const;

signals:
    void lightsChanged();
    void lightAdded(const LightSource& light);
    void lightRemoved(size_t index);
    void lightUpdated(size_t index, const LightSource& light);

private slots:
    void onAddLight();
    void onRemoveLight(size_t index);
    void onEditLight(size_t index);

private:
    void rebuildUI();
    QVBoxLayout* itemsLayout_;
    std::vector<LightSource> lights_;
};

} // namespace s21
#endif
