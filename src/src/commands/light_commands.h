#ifndef S21_COMMANDS_LIGHT_COMMANDS_H_
#define S21_COMMANDS_LIGHT_COMMANDS_H_

#include "src/commands/command.h"
#include "src/common/lighting.h"

namespace s21 {
// Объявления классов AddLightCommand, RemoveLightCommand, UpdateLightCommand
class AddLightCommand : public Command {
public:
    AddLightCommand(Scene* scene, const LightSource& light);
    void execute() override;
    void undo() override;
    // ... реализация
};
// ... другие команды
}  // namespace s21
#endif  // S21_COMMANDS_LIGHT_COMMANDS_H_
