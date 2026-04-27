#ifndef S21_OPENGL_RENDERER_H
#define S21_OPENGL_RENDERER_H

#include <QImage>
#include <QOpenGLFunctions_3_3_Core>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

#include "gl_buffer.h"
#include "gl_vertex_array.h"
#include "scene/mesh.h"
#include "scene/scene.h"
#include "settings/settings.h"
#include "shader_program.h"

namespace s21 {

struct MeshBuffers {
  GLVertexArray vao;
  GLVertexArray vaoTriangles;
  GLBuffer vbo;
  GLBuffer normalVBO;
  GLBuffer texCoordVBO;
  GLBuffer ebo;
  size_t edgeCount = 0;
  size_t vertexCount = 0;
  bool transformChanged = true;
  GLsizeiptr normalBufferSize =
      0;  // Новое добавление для правильной работы слайсера сглаживания

  MeshBuffers(QOpenGLFunctions_3_3_Core* gl)
      : vao(gl),
        vaoTriangles(gl),
        vbo(gl, GL_ARRAY_BUFFER),
        normalVBO(gl, GL_ARRAY_BUFFER),
        texCoordVBO(gl, GL_ARRAY_BUFFER),
        ebo(gl, GL_ELEMENT_ARRAY_BUFFER) {}
};

class OpenGLRenderer {
 public:
  enum class ShadingType { Wireframe, Flat, Phong };
  explicit OpenGLRenderer(QOpenGLFunctions_3_3_Core* gl, Scene* scene);
  ~OpenGLRenderer();

  void Initialize();
  void Render(const std::vector<const Mesh*>& meshes,
              const S21Matrix& viewMatrix, const S21Matrix& projMatrix);
  void UpdateMesh(const Mesh& mesh);
  void RemoveMesh(const Mesh* mesh);
  void SetSettings(Settings* settings);
  void UpdateSettings();

  void SetFlipNormals(bool flip);
  bool GetFlipNormals() const { return flipNormals_; }
  void SetShadingType(ShadingType type);
  void SetLightPosition(float x, float y, float z) { lightPos_ = {x, y, z}; }
  void SetLightColor(const QColor& color) { lightColor_ = color; }
  void SetCameraPosition(const Point& pos) { cameraPosition_ = pos; }
  void SetObjectColor(const QColor& color) { objectColor_ = color; }
  void SetTexture(const QImage& image);
  void ClearTexture();

  void MarkMeshTransformChanged(const Mesh* mesh);

  const std::string& GetShaderError() const { return shaderError_; }

  void SetShadingModel(Settings::ShadingModel model);

  void UpdateMeshNormals(const Mesh& mesh);

 private:
  void RenderWireframe(const std::vector<const Mesh*>& meshes,
                       const S21Matrix& view, const S21Matrix& proj);
  void RenderSolid(const std::vector<const Mesh*>& meshes,
                   const S21Matrix& view, const S21Matrix& proj);
  void CreateBuffers(const Mesh& mesh, MeshBuffers& buffers);
  void UpdateNormalMatrixForMesh(MeshBuffers& buffers, const S21Matrix& model);

  Settings::ShadingModel shadingModel_ = Settings::ShadingModel::Phong;

  QOpenGLFunctions_3_3_Core* gl_;
  Scene* scene_;
  Settings* settings_ = nullptr;
  ShaderProgram wireframeShader_;
  ShaderProgram solidShader_;
  std::unordered_map<const Mesh*, std::unique_ptr<MeshBuffers>> bufferCache_;
  ShadingType shadingType_ = ShadingType::Wireframe;
  bool flipNormals_ = false;
  Point lightPos_{2.0f, 3.0f, 4.0f};
  QColor lightColor_{255, 255, 255};
  Point cameraPosition_{0, 0, 5};
  QColor objectColor_{200, 200, 200};
  GLuint textureID_ = 0;

  std::string shaderError_;
};

}  // namespace s21

#endif  // S21_OPENGL_RENDERER_H
