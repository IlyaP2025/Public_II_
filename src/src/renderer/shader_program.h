#ifndef S21_SHADER_PROGRAM_H
#define S21_SHADER_PROGRAM_H

#include <QOpenGLFunctions_3_3_Core>
#include <stdexcept>
#include <string>

namespace s21 {

class ShaderProgram {
 public:
  explicit ShaderProgram(QOpenGLFunctions_3_3_Core* gl);
  ~ShaderProgram();

  ShaderProgram(const ShaderProgram&) = delete;
  ShaderProgram& operator=(const ShaderProgram&) = delete;
  ShaderProgram(ShaderProgram&& other) noexcept;
  ShaderProgram& operator=(ShaderProgram&& other) noexcept;

  void attachShader(GLenum type, const std::string& source);
  void link();
  void use() const;
  void unuse() const;

  GLuint id() const { return program_; }

  // Методы установки uniform
  void setUniform(const char* name, int value) const;
  void setUniform(const char* name, float value) const;
  void setUniform(const char* name,
                  const float* matrix4x4) const;  // для матриц 4x4
  void setUniformMat3(const char* name,
                      const float* matrix3x3) const;  // для матриц 3x3
  void setUniform(const char* name, float x, float y, float z) const;
  void setUniform(const char* name, const QColor& color) const;

 private:
  QOpenGLFunctions_3_3_Core* gl_;
  GLuint program_;
};

}  // namespace s21

#endif  // S21_SHADER_PROGRAM_H