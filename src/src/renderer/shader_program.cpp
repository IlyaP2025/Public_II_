#include "shader_program.h"

#include <QColor>

namespace s21 {

ShaderProgram::ShaderProgram(QOpenGLFunctions_3_3_Core* gl) : gl_(gl) {
  program_ = gl_->glCreateProgram();
}

ShaderProgram::~ShaderProgram() {
  if (program_ != 0) gl_->glDeleteProgram(program_);
}

ShaderProgram::ShaderProgram(ShaderProgram&& other) noexcept
    : gl_(other.gl_), program_(other.program_) {
  other.program_ = 0;
}

ShaderProgram& ShaderProgram::operator=(ShaderProgram&& other) noexcept {
  if (this != &other) {
    if (program_ != 0) gl_->glDeleteProgram(program_);
    gl_ = other.gl_;
    program_ = other.program_;
    other.program_ = 0;
  }
  return *this;
}

void ShaderProgram::attachShader(GLenum type, const std::string& source) {
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
    throw std::runtime_error(std::string("Shader compilation failed: ") +
                             infoLog);
  }

  gl_->glAttachShader(program_, shader);
  gl_->glDeleteShader(shader);
}

void ShaderProgram::link() {
  gl_->glLinkProgram(program_);
  GLint success;
  gl_->glGetProgramiv(program_, GL_LINK_STATUS, &success);
  if (!success) {
    char infoLog[512];
    gl_->glGetProgramInfoLog(program_, 512, nullptr, infoLog);
    throw std::runtime_error(std::string("Shader linking failed: ") + infoLog);
  }
}

void ShaderProgram::use() const { gl_->glUseProgram(program_); }

void ShaderProgram::unuse() const { gl_->glUseProgram(0); }

void ShaderProgram::setUniform(const char* name, int value) const {
  GLint loc = gl_->glGetUniformLocation(program_, name);
  if (loc != -1) gl_->glUniform1i(loc, value);
}

void ShaderProgram::setUniform(const char* name, float value) const {
  GLint loc = gl_->glGetUniformLocation(program_, name);
  if (loc != -1) gl_->glUniform1f(loc, value);
}

void ShaderProgram::setUniform(const char* name, const float* matrix4x4) const {
  GLint loc = gl_->glGetUniformLocation(program_, name);
  if (loc != -1) gl_->glUniformMatrix4fv(loc, 1, GL_FALSE, matrix4x4);
}

void ShaderProgram::setUniformMat3(const char* name,
                                   const float* matrix3x3) const {
  GLint loc = gl_->glGetUniformLocation(program_, name);
  if (loc != -1) gl_->glUniformMatrix3fv(loc, 1, GL_FALSE, matrix3x3);
}

void ShaderProgram::setUniform(const char* name, float x, float y,
                               float z) const {
  GLint loc = gl_->glGetUniformLocation(program_, name);
  if (loc != -1) gl_->glUniform3f(loc, x, y, z);
}

void ShaderProgram::setUniform(const char* name, const QColor& color) const {
  GLint loc = gl_->glGetUniformLocation(program_, name);
  if (loc != -1)
    gl_->glUniform3f(loc, color.redF(), color.greenF(), color.blueF());
}

}  // namespace s21