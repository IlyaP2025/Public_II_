#ifndef S21_MACRO_COMMAND_H
#define S21_MACRO_COMMAND_H

#include <memory>
#include <vector>

#include "command.h"

namespace s21 {

class MacroCommand : public Command {
 public:
  void addCommand(std::unique_ptr<Command> cmd) {
    if (cmd) commands_.push_back(std::move(cmd));
  }
  void execute() override {
    for (auto& cmd : commands_) cmd->execute();
  }
  void undo() override {
    for (auto it = commands_.rbegin(); it != commands_.rend(); ++it)
      (*it)->undo();
  }
  void redo() override {
    for (auto& cmd : commands_) cmd->redo();
  }
  bool empty() const { return commands_.empty(); }
  void clear() { commands_.clear(); }

 private:
  std::vector<std::unique_ptr<Command>> commands_;
};

}  // namespace s21

#endif  // S21_MACRO_COMMAND_H