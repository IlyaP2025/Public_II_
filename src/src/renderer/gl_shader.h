#ifndef S21_GL_SHADER_H
#define S21_GL_SHADER_H

#include <QOpenGLFunctions_3_3_Core>
#include <stdexcept>
#include <string>

namespace s21 {

class GLShader {
 public:
  explicit GLShader(QOpenGLFunctions_3_3_Core* gl) : gl_(gl) {
    program_ = gl_->glCreateProgram();
  }

  ~GLShader() {
    if (program_ != 0) gl_->glDeleteProgram(program_);
  }

  GLShader(const GLShader&) = delete;
  GLShader& operator=(const GLShader&) = delete;

  GLShader(GLShader&& other) noexcept
      : gl_(other.gl_), program_(other.program_) {
    other.program_ = 0;
  }

  GLShader& operator=(GLShader&& other) noexcept {
    if (this != &other) {
      if (program_ != 0) gl_->glDeleteProgram(program_);
      gl_ = other.gl_;
      program_ = other.program_;
      other.program_ = 0;
    }
    return *this;
  }

  void attachShader(GLenum type, const std::string& source) {
    GLuint shader = gl_->glCreateShader(type);
    const char* src = source.c_str();
    gl_->glShaderSource(shader, 1, &src, nullptr);
    gl_->glCompileShader(shader);

    GLint success;
    gl_->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      char infoLog[512];
      gl_->glGetShaderInfoLog(shader, 512, nullptr, infoLog);
      gl_->glDeleteShader(shader);
      throw std::runtime_error("Shader compilation failed: " +
                               std::string(infoLog));
    }

    gl_->glAttachShader(program_, shader);
    gl_->glDeleteShader(shader);
  }

  void link() {
    gl_->glLinkProgram(program_);
    GLint success;
    gl_->glGetProgramiv(program_, GL_LINK_STATUS, &success);
    if (!success) {
      char infoLog[512];
      gl_->glGetProgramInfoLog(program_, 512, nullptr, infoLog);
      throw std::runtime_error("Shader linking failed: " +
                               std::string(infoLog));
    }
  }

  void use() const { gl_->glUseProgram(program_); }
  void unuse() const { gl_->glUseProgram(0); }

  GLuint id() const { return program_; }

 private:
  QOpenGLFunctions_3_3_Core* gl_;
  GLuint program_;
};

}  // namespace s21

#endif  // S21_GL_SHADER_H