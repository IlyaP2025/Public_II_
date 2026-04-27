#include "light_control_widget.h"
#include "light_editor_dialog.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>

namespace s21 {

LightControlWidget::LightControlWidget(QWidget* parent) : QWidget(parent) {
    auto* mainLayout = new QVBoxLayout(this);

    auto* headerLayout = new QHBoxLayout;
    headerLayout->addWidget(new QLabel("<b>Light Sources</b>"));
    headerLayout->addStretch();
    auto* addBtn = new QPushButton("+");
    addBtn->setFixedSize(30, 30);
    connect(addBtn, &QPushButton::clicked, this, &LightControlWidget::onAddLight);
    headerLayout->addWidget(addBtn);
    mainLayout->addLayout(headerLayout);

    itemsLayout_ = new QVBoxLayout;
    mainLayout->addLayout(itemsLayout_);
    mainLayout->addStretch();
}

void LightControlWidget::onAddLight() {
    if (lights_.size() >= 5) {
        QMessageBox::warning(this, "Limit", "Maximum 5 lights allowed.");
        return;
    }
    LightEditorDialog dlg(this);
    if (dlg.exec() == QDialog::Accepted) {
        LightSource l = dlg.getLight();
        l.enabled = true;
        lights_.push_back(l);
        rebuildUI();
        emit lightAdded(l);
        emit lightsChanged();
    }
}

void LightControlWidget::onRemoveLight(size_t index) {
    if (index >= lights_.size()) return;
    lights_.erase(lights_.begin() + static_cast<long>(index));
    rebuildUI();
    emit lightRemoved(index);
    emit lightsChanged();
}

void LightControlWidget::onEditLight(size_t index) {
    if (index >= lights_.size()) return;
    LightEditorDialog dlg(this);
    dlg.setLight(lights_[index]);
    if (dlg.exec() == QDialog::Accepted) {
        LightSource upd = dlg.getLight();
        upd.enabled = lights_[index].enabled;
        lights_[index] = upd;
        rebuildUI();
        emit lightUpdated(index, upd);
        emit lightsChanged();
    }
}

void LightControlWidget::setLights(const std::vector<LightSource>& lights) {
    lights_ = lights;
    rebuildUI();
}

std::vector<LightSource> LightControlWidget::getLights() const {
    return lights_;
}

void LightControlWidget::rebuildUI() {
    QLayoutItem* child;
    while ((child = itemsLayout_->takeAt(0)) != nullptr) {
        if (child->widget()) delete child->widget();
        delete child;
    }

    for (size_t i = 0; i < lights_.size(); ++i) {
        auto* row = new QWidget;
        auto* rowLayout = new QHBoxLayout(row);
        rowLayout->setContentsMargins(2, 2, 2, 2);

        auto* cb = new QCheckBox;
        cb->setChecked(lights_[i].enabled);
        connect(cb, &QCheckBox::toggled, this, [this, i](bool val) {
            if (i < lights_.size()) {
                lights_[i].enabled = val;
                emit lightUpdated(i, lights_[i]);
                emit lightsChanged();
            }
        });
        rowLayout->addWidget(cb);

        rowLayout->addWidget(new QLabel(QString("Light %1").arg(i+1)));

        auto* editBtn = new QPushButton("Edit");
        connect(editBtn, &QPushButton::clicked, this, [this, i]() { onEditLight(i); });
        rowLayout->addWidget(editBtn);

        auto* delBtn = new QPushButton("X");
        delBtn->setFixedSize(24, 24);
        connect(delBtn, &QPushButton::clicked, this, [this, i]() { onRemoveLight(i); });
        rowLayout->addWidget(delBtn);

        itemsLayout_->addWidget(row);
    }
}

} // namespace s21
