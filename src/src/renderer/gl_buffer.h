#ifndef S21_GL_BUFFER_H
#define S21_GL_BUFFER_H

#include <QOpenGLFunctions_3_3_Core>

namespace s21 {

class GLBuffer {
 public:
  explicit GLBuffer(QOpenGLFunctions_3_3_Core* gl, GLenum target)
      : gl_(gl), target_(target) {
    gl_->glGenBuffers(1, &id_);
  }

  ~GLBuffer() {
    if (id_ != 0) gl_->glDeleteBuffers(1, &id_);
  }

  GLBuffer(const GLBuffer&) = delete;
  GLBuffer& operator=(const GLBuffer&) = delete;

  GLBuffer(GLBuffer&& other) noexcept
      : gl_(other.gl_), id_(other.id_), target_(other.target_) {
    other.id_ = 0;
  }

  GLBuffer& operator=(GLBuffer&& other) noexcept {
    if (this != &other) {
      if (id_ != 0) gl_->glDeleteBuffers(1, &id_);
      gl_ = other.gl_;
      id_ = other.id_;
      target_ = other.target_;
      other.id_ = 0;
    }
    return *this;
  }

  void bind() const { gl_->glBindBuffer(target_, id_); }
  void unbind() const { gl_->glBindBuffer(target_, 0); }

  void setData(GLsizeiptr size, const void* data, GLenum usage) {
    bind();
    gl_->glBufferData(target_, size, data, usage);
  }

  GLuint id() const { return id_; }
  GLenum target() const { return target_; }

 private:
  QOpenGLFunctions_3_3_Core* gl_;
  GLuint id_ = 0;
  GLenum target_;
};

}  // namespace s21

#endif  // S21_GL_BUFFER_H