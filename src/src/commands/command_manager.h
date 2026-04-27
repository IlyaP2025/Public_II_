#ifndef S21_COMMAND_MANAGER_H
#define S21_COMMAND_MANAGER_H

#include <memory>
#include <stack>
#include <stdexcept>

#include "command.h"

namespace s21 {

class CommandManager {
 public:
  void executeCommand(std::unique_ptr<Command> cmd) {
    if (!cmd) throw std::invalid_argument("CommandManager: null command");
    cmd->execute();
    undoStack_.push(std::move(cmd));
    while (!redoStack_.empty()) redoStack_.pop();
  }

  void undo() {
    if (undoStack_.empty()) return;
    auto cmd = std::move(undoStack_.top());
    undoStack_.pop();
    cmd->undo();
    redoStack_.push(std::move(cmd));
  }

  void redo() {
    if (redoStack_.empty()) return;
    auto cmd = std::move(redoStack_.top());
    redoStack_.pop();
    cmd->redo();
    undoStack_.push(std::move(cmd));
  }

  bool canUndo() const { return !undoStack_.empty(); }
  bool canRedo() const { return !redoStack_.empty(); }
  void clear() {
    while (!undoStack_.empty()) undoStack_.pop();
    while (!redoStack_.empty()) redoStack_.pop();
  }

 private:
  std::stack<std::unique_ptr<Command>> undoStack_;
  std::stack<std::unique_ptr<Command>> redoStack_;
};

}  // namespace s21

#endif  // S21_COMMAND_MANAGER_H
