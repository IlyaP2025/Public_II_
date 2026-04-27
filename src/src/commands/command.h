#ifndef S21_COMMAND_H
#define S21_COMMAND_H

namespace s21 {

class Command {
 public:
  virtual ~Command() = default;
  virtual void execute() = 0;
  virtual void undo() = 0;
  virtual void redo() { execute(); }
};

}  // namespace s21

#endif  // S21_COMMAND_H
