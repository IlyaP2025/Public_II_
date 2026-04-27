#ifndef S21_GL_VERTEX_ARRAY_H
#define S21_GL_VERTEX_ARRAY_H

#include <QOpenGLFunctions_3_3_Core>

namespace s21 {

class GLVertexArray {
 public:
  explicit GLVertexArray(QOpenGLFunctions_3_3_Core* gl) : gl_(gl) {
    gl_->glGenVertexArrays(1, &id_);
  }

  ~GLVertexArray() {
    if (id_ != 0) gl_->glDeleteVertexArrays(1, &id_);
  }

  GLVertexArray(const GLVertexArray&) = delete;
  GLVertexArray& operator=(const GLVertexArray&) = delete;

  GLVertexArray(GLVertexArray&& other) noexcept
      : gl_(other.gl_), id_(other.id_) {
    other.id_ = 0;
  }

  GLVertexArray& operator=(GLVertexArray&& other) noexcept {
    if (this != &other) {
      if (id_ != 0) gl_->glDeleteVertexArrays(1, &id_);
      gl_ = other.gl_;
      id_ = other.id_;
      other.id_ = 0;
    }
    return *this;
  }

  void bind() const { gl_->glBindVertexArray(id_); }
  void unbind() const { gl_->glBindVertexArray(0); }
  GLuint id() const { return id_; }

 private:
  QOpenGLFunctions_3_3_Core* gl_;
  GLuint id_ = 0;
};

}  // namespace s21

#endif  // S21_GL_VERTEX_ARRAY_H