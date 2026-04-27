#include "opengl_renderer.h"

#include <QDebug>
#include <algorithm>
#include <iostream>

#include "common/debug.h"

namespace s21 {

OpenGLRenderer::OpenGLRenderer(QOpenGLFunctions_3_3_Core* gl, Scene* scene)
    : gl_(gl), scene_(scene), wireframeShader_(gl), solidShader_(gl) {
  if (!gl_) throw std::runtime_error("OpenGLFunctions is null");
}

OpenGLRenderer::~OpenGLRenderer() {}

void OpenGLRenderer::Initialize() {
  const char* wireframeVertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        uniform mat4 modelViewProjection;
        uniform mat4 model;
        uniform float pointSize;
        void main() {
            gl_Position = modelViewProjection * model * vec4(aPos, 1.0);
            gl_PointSize = pointSize;
        }
    )";
  const char* wireframeFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;
        uniform vec3 edgeColor;
        uniform vec3 vertexColor;
        uniform int vertexType;
        uniform int edgeType;
        uniform float stippleFactor;
        void main() {
            if (vertexType == 0) {
                if (edgeType == 1) {
                    int x = int(gl_FragCoord.x);
                    int y = int(gl_FragCoord.y);
                    int step = int(stippleFactor);
                    if (((x / step) + (y / step)) % 2 == 0) discard;
                }
                FragColor = vec4(edgeColor, 1.0);
            } else {
                if (vertexType == 1) {
                    float dist = length(gl_PointCoord - vec2(0.5));
                    if (dist > 0.5) discard;
                }
                FragColor = vec4(vertexColor, 1.0);
            }
        }
    )";

  const char* solidVertexSource = R"(
        #version 330 core
        layout(location = 0) in vec3 aPos;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;

        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        uniform mat3 normalMatrix;
        uniform int flipNormals;

        out vec3 FragPos;
        flat out vec3 FlatNormal;    // для Flat shading
        smooth out vec3 SmoothNormal; // для Phong shading
        out vec2 TexCoord;

        void main() {
            FragPos = vec3(model * vec4(aPos, 1.0));
            vec3 normal = normalize(normalMatrix * aNormal);
            if (flipNormals == 1) normal = -normal;
            FlatNormal = normal;
            SmoothNormal = normal;
            TexCoord = aTexCoord;
            gl_Position = projection * view * vec4(FragPos, 1.0);
        }
    )";

  const char* solidFragmentSource = R"(
        #version 330 core
        out vec4 FragColor;

        in vec3 FragPos;
        flat in vec3 FlatNormal;
        smooth in vec3 SmoothNormal;
        in vec2 TexCoord;

        uniform vec3 lightPos;
        uniform vec3 lightColor;
        uniform vec3 viewPos;
        uniform vec3 objectColor;
        uniform int shadingModel;    // 1 = Flat, 2 = Phong (0 = Wireframe обрабатывается отдельно)
        uniform sampler2D textureSampler;
        uniform int useTexture;

        void main() {
            vec3 color = objectColor;
            if (useTexture == 1) {
                color = texture(textureSampler, TexCoord).rgb;
            }

            vec3 normal;
            if (shadingModel == 1) {
                normal = normalize(FlatNormal);   // Flat
            } else {
                normal = normalize(SmoothNormal); // Phong
            }

            vec3 ambient = 0.2 * lightColor;
            vec3 lightDir = normalize(lightPos - FragPos);
            float diff = max(dot(normal, lightDir), 0.0);
            vec3 diffuse = diff * lightColor;
            vec3 viewDir = normalize(viewPos - FragPos);
            vec3 reflectDir = reflect(-lightDir, normal);
            float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
            vec3 specular = 0.5 * spec * lightColor;

            vec3 result = (ambient + diffuse + specular) * color;
            FragColor = vec4(result, 1.0); // FragColor = vec4(result, 1.0); vec4(normal * 0.5 + 0.5, 1.0);

        }
    )";

  try {
    wireframeShader_.attachShader(GL_VERTEX_SHADER, wireframeVertexSource);
    wireframeShader_.attachShader(GL_FRAGMENT_SHADER, wireframeFragmentSource);
    wireframeShader_.link();

    solidShader_.attachShader(GL_VERTEX_SHADER, solidVertexSource);
    solidShader_.attachShader(GL_FRAGMENT_SHADER, solidFragmentSource);
    solidShader_.link();
  } catch (const std::exception& e) {
    shaderError_ = e.what();
    throw;
  }

  solidShader_.setUniform("flipNormals", flipNormals_ ? 1 : 0);
}

void OpenGLRenderer::Render(const std::vector<const Mesh*>& meshes,
                            const S21Matrix& viewMatrix,
                            const S21Matrix& projMatrix) {
  if (!gl_ || !settings_) return;

  if (shadingType_ == ShadingType::Wireframe) {
    RenderWireframe(meshes, viewMatrix, projMatrix);
  } else {
    // Flat или Phong
    RenderSolid(meshes, viewMatrix, projMatrix);
  }

  gl_->glBindVertexArray(0);
  gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGLRenderer::RenderWireframe(const std::vector<const Mesh*>& meshes,
                                     const S21Matrix& viewMatrix,
                                     const S21Matrix& projMatrix) {
  wireframeShader_.use();

  GLint edgeColorLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "edgeColor");
  GLint vertexColorLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "vertexColor");
  GLint vertexTypeLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "vertexType");
  GLint pointSizeLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "pointSize");
  GLint mvpLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "modelViewProjection");
  GLint modelLoc = gl_->glGetUniformLocation(wireframeShader_.id(), "model");
  GLint edgeTypeLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "edgeType");
  GLint stippleFactorLoc =
      gl_->glGetUniformLocation(wireframeShader_.id(), "stippleFactor");

  if (pointSizeLoc != -1)
    gl_->glUniform1f(pointSizeLoc, settings_->vertexSize());
  if (stippleFactorLoc != -1)
    gl_->glUniform1f(stippleFactorLoc, settings_->dashFactor());

  std::vector<SceneObject*> selected;
  if (scene_) selected = scene_->GetSelected();

  if (vertexTypeLoc != -1) gl_->glUniform1i(vertexTypeLoc, 0);
  if (edgeTypeLoc != -1)
    gl_->glUniform1i(edgeTypeLoc, static_cast<int>(settings_->edgeType()));
  gl_->glLineWidth(settings_->edgeThickness());

  for (const Mesh* mesh : meshes) {
    auto it = bufferCache_.find(mesh);
    if (it == bufferCache_.end()) {
      UpdateMesh(*mesh);
      it = bufferCache_.find(mesh);
      if (it == bufferCache_.end()) continue;
    }
    const auto& buffers = it->second;
    if (buffers->edgeCount == 0) continue;

    bool isSelected =
        std::find(selected.begin(), selected.end(), mesh) != selected.end();
    if (edgeColorLoc != -1) {
      QColor c =
          isSelected ? settings_->selectedEdgeColor() : settings_->edgeColor();
      gl_->glUniform3f(edgeColorLoc, c.redF(), c.greenF(), c.blueF());
    }

    S21Matrix modelMatrix = mesh->GetTransform().GetModelMatrix();
    S21Matrix mvp = projMatrix * viewMatrix * modelMatrix;

    float modelArray[16];
    float mvpArray[16];
    for (int col = 0; col < 4; ++col) {
      for (int row = 0; row < 4; ++row) {
        modelArray[col * 4 + row] = static_cast<float>(modelMatrix(row, col));
        mvpArray[col * 4 + row] = static_cast<float>(mvp(row, col));
      }
    }

    if (mvpLoc != -1) gl_->glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpArray);
    if (modelLoc != -1)
      gl_->glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelArray);

    buffers->vao.bind();
    gl_->glDrawElements(GL_LINES, buffers->edgeCount, GL_UNSIGNED_INT, nullptr);
  }

  // Рисование вершин
  if (settings_->vertexType() != Settings::VertexType::None) {
    int vt = static_cast<int>(settings_->vertexType());
    if (vertexTypeLoc != -1) gl_->glUniform1i(vertexTypeLoc, vt);
    gl_->glEnable(GL_PROGRAM_POINT_SIZE);
    gl_->glEnable(GL_POINT_SPRITE);

    for (const Mesh* mesh : meshes) {
      auto it = bufferCache_.find(mesh);
      if (it == bufferCache_.end()) continue;
      const auto& buffers = it->second;
      if (buffers->vertexCount == 0) continue;

      bool isSelected =
          std::find(selected.begin(), selected.end(), mesh) != selected.end();
      if (vertexColorLoc != -1) {
        QColor c = isSelected ? settings_->selectedVertexColor()
                              : settings_->vertexColor();
        gl_->glUniform3f(vertexColorLoc, c.redF(), c.greenF(), c.blueF());
      }

      S21Matrix modelMatrix = mesh->GetTransform().GetModelMatrix();
      S21Matrix mvp = projMatrix * viewMatrix * modelMatrix;

      float modelArray[16];
      float mvpArray[16];
      for (int col = 0; col < 4; ++col) {
        for (int row = 0; row < 4; ++row) {
          modelArray[col * 4 + row] = static_cast<float>(modelMatrix(row, col));
          mvpArray[col * 4 + row] = static_cast<float>(mvp(row, col));
        }
      }

      if (mvpLoc != -1) gl_->glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, mvpArray);
      if (modelLoc != -1)
        gl_->glUniformMatrix4fv(modelLoc, 1, GL_FALSE, modelArray);

      buffers->vao.bind();
      gl_->glDrawArrays(GL_POINTS, 0, buffers->vertexCount);
    }

    gl_->glDisable(GL_POINT_SPRITE);
    gl_->glDisable(GL_PROGRAM_POINT_SIZE);
  }

  wireframeShader_.unuse();
}

void OpenGLRenderer::RenderSolid(const std::vector<const Mesh*>& meshes,
                                 const S21Matrix& viewMatrix,
                                 const S21Matrix& projMatrix) {
  solidShader_.use();
  solidShader_.setUniform("flipNormals", flipNormals_ ? 1 : 0);

  float viewArray[16], projArray[16];
  for (int col = 0; col < 4; ++col) {
    for (int row = 0; row < 4; ++row) {
      viewArray[col * 4 + row] = static_cast<float>(viewMatrix(row, col));
      projArray[col * 4 + row] = static_cast<float>(projMatrix(row, col));
    }
  }
  solidShader_.setUniform("view", viewArray);
  solidShader_.setUniform("projection", projArray);
  solidShader_.setUniform("lightPos", lightPos_.x, lightPos_.y, lightPos_.z);
  solidShader_.setUniform("lightColor", lightColor_.redF(),
                          lightColor_.greenF(), lightColor_.blueF());
  solidShader_.setUniform("viewPos", cameraPosition_.x, cameraPosition_.y,
                          cameraPosition_.z);
  solidShader_.setUniform("objectColor", objectColor_.redF(),
                          objectColor_.greenF(), objectColor_.blueF());
  solidShader_.setUniform("shadingType", static_cast<int>(shadingType_));
  solidShader_.setUniform("useTexture", (textureID_ != 0) ? 1 : 0);

  DEBUG_PRINT("RenderSolid: lightPos uniform = ("
              << lightPos_.x << ", " << lightPos_.y << ", " << lightPos_.z
              << ")");
  DEBUG_PRINT("RenderSolid: viewPos uniform = (" << cameraPosition_.x << ", "
                                                 << cameraPosition_.y << ", "
                                                 << cameraPosition_.z << ")");
  DEBUG_PRINT("RenderSolid: objectColor uniform = ("
              << objectColor_.redF() << ", " << objectColor_.greenF() << ", "
              << objectColor_.blueF() << ")");

  if (textureID_ != 0) {
    gl_->glActiveTexture(GL_TEXTURE0);
    gl_->glBindTexture(GL_TEXTURE_2D, textureID_);
    solidShader_.setUniform("textureSampler", 0);
  }

  int shadingModel = (shadingType_ == ShadingType::Flat) ? 1 : 2;
  solidShader_.setUniform("shadingModel", shadingModel);

  DEBUG_PRINT("RenderSolid: rendering " << meshes.size() << " meshes");
  for (const Mesh* mesh : meshes) {
    auto it = bufferCache_.find(mesh);
    if (it == bufferCache_.end()) {
      DEBUG_PRINT("  mesh " << mesh << " NOT in cache! file: "
                            << mesh->GetSourceFile().c_str());
      UpdateMesh(*mesh);
      it = bufferCache_.find(mesh);
      if (it == bufferCache_.end()) continue;
    }
    auto& buffers = *it->second;
    DEBUG_PRINT("  mesh " << mesh << " (" << mesh->GetSourceFile().c_str()
                          << "), VAO = " << buffers.vaoTriangles.id()
                          << ", vertices = " << buffers.vertexCount);

    if (buffers.vertexCount == 0) continue;

    S21Matrix modelMatrix =
        mesh->GetTransform()
            .GetModelMatrix();  // исправлено: имя переменной modelMatrix

    float modelArray[16];
    for (int col = 0; col < 4; ++col)
      for (int row = 0; row < 4; ++row)
        modelArray[col * 4 + row] = static_cast<float>(modelMatrix(row, col));
    solidShader_.setUniform("model", modelArray);

    DEBUG_PRINT("    model matrix: ["
                << modelMatrix(0, 0) << ", " << modelMatrix(0, 1) << ", "
                << modelMatrix(0, 2) << ", " << modelMatrix(0, 3) << "]");
    DEBUG_PRINT("                  ["
                << modelMatrix(1, 0) << ", " << modelMatrix(1, 1) << ", "
                << modelMatrix(1, 2) << ", " << modelMatrix(1, 3) << "]");
    DEBUG_PRINT("                  ["
                << modelMatrix(2, 0) << ", " << modelMatrix(2, 1) << ", "
                << modelMatrix(2, 2) << ", " << modelMatrix(2, 3) << "]");
    DEBUG_PRINT("                  ["
                << modelMatrix(3, 0) << ", " << modelMatrix(3, 1) << ", "
                << modelMatrix(3, 2) << ", " << modelMatrix(3, 3) << "]");

    if (buffers.transformChanged) {
      UpdateNormalMatrixForMesh(buffers, modelMatrix);
      buffers.transformChanged = false;
    }

    buffers.vaoTriangles.bind();

    if (!gl_->glIsVertexArray(buffers.vaoTriangles.id())) {
      DEBUG_PRINT("ERROR: VAO " << buffers.vaoTriangles.id()
                                << " is not valid!");
    }

    gl_->glDrawArrays(GL_TRIANGLES, 0, buffers.vertexCount);
  }

  solidShader_.unuse();
}

void OpenGLRenderer::UpdateNormalMatrixForMesh(MeshBuffers& /*buffers*/,
                                               const S21Matrix& model) {
  float normalMat[9];
  S21Matrix model3x3(3, 3);
  for (int i = 0; i < 3; ++i)
    for (int j = 0; j < 3; ++j) model3x3(i, j) = model(i, j);
  try {
    S21Matrix invModel = model3x3.InverseMatrix();
    S21Matrix normalMatMatrix = invModel.Transpose();
    for (int i = 0; i < 3; ++i)
      for (int j = 0; j < 3; ++j)
        normalMat[i * 3 + j] = static_cast<float>(normalMatMatrix(i, j));
  } catch (const std::exception&) {
    for (int i = 0; i < 9; ++i) normalMat[i] = 0.0f;
    normalMat[0] = normalMat[4] = normalMat[8] = 1.0f;
  }
  solidShader_.setUniformMat3("normalMatrix", normalMat);
}

void OpenGLRenderer::MarkMeshTransformChanged(const Mesh* mesh) {
  auto it = bufferCache_.find(mesh);
  if (it != bufferCache_.end()) {
    it->second->transformChanged = true;
  }
}

void OpenGLRenderer::CreateBuffers(const Mesh& mesh, MeshBuffers& buffers) {
  DEBUG_PRINT("CreateBuffers: mesh = " << &mesh << " file: "
                                       << mesh.GetSourceFile().c_str());
  const auto& vertices = mesh.GetVertices();
  const auto& normals = mesh.GetNormals();

  if (!normals.empty()) {
    buffers.normalVBO.bind();
    buffers.normalVBO.setData(normals.size() * sizeof(Point), normals.data(),
                              GL_STATIC_DRAW);
    buffers.normalBufferSize = normals.size() * sizeof(Point);
  }

  const auto& texCoords = mesh.GetUVs();
  const auto& edges = mesh.GetEdges();

  buffers.vbo.bind();
  buffers.vbo.setData(vertices.size() * sizeof(Point), vertices.data(),
                      GL_STATIC_DRAW);
  buffers.vertexCount = static_cast<GLsizei>(vertices.size());

  DEBUG_PRINT("  vertices: " << vertices.size()
                             << ", normals: " << normals.size());
  if (!normals.empty()) {
    DEBUG_PRINT("  first normal: (" << normals[0].x << ", " << normals[0].y
                                    << ", " << normals[0].z << ")");
    buffers.normalVBO.bind();
    buffers.normalVBO.setData(normals.size() * sizeof(Point), normals.data(),
                              GL_STATIC_DRAW);
  }
  if (!texCoords.empty()) {
    buffers.texCoordVBO.bind();
    buffers.texCoordVBO.setData(texCoords.size() * sizeof(Point2D),
                                texCoords.data(), GL_STATIC_DRAW);
  }

  std::vector<GLuint> edgeIndices;
  edgeIndices.reserve(edges.size() * 2);
  for (const auto& edge : edges) {
    edgeIndices.push_back(static_cast<GLuint>(edge.v1));
    edgeIndices.push_back(static_cast<GLuint>(edge.v2));
  }
  buffers.ebo.bind();
  buffers.ebo.setData(edgeIndices.size() * sizeof(GLuint), edgeIndices.data(),
                      GL_STATIC_DRAW);
  buffers.edgeCount = static_cast<GLsizei>(edgeIndices.size());

  buffers.vao.bind();
  buffers.vbo.bind();
  gl_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
  gl_->glEnableVertexAttribArray(0);
  if (!normals.empty()) {
    buffers.normalVBO.bind();
    gl_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point),
                               (void*)0);
    gl_->glEnableVertexAttribArray(1);
  }
  if (!texCoords.empty()) {
    buffers.texCoordVBO.bind();
    gl_->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Point2D),
                               (void*)0);
    gl_->glEnableVertexAttribArray(2);
  }
  buffers.ebo.bind();
  buffers.vao.unbind();

  buffers.vaoTriangles.bind();
  buffers.vbo.bind();
  gl_->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Point), (void*)0);
  gl_->glEnableVertexAttribArray(0);
  if (!normals.empty()) {
    buffers.normalVBO.bind();
    gl_->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Point),
                               (void*)0);
    gl_->glEnableVertexAttribArray(1);
  }
  if (!texCoords.empty()) {
    buffers.texCoordVBO.bind();
    gl_->glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Point2D),
                               (void*)0);
    gl_->glEnableVertexAttribArray(2);
  }
  buffers.vaoTriangles.unbind();

  gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
  buffers.transformChanged = true;
  DEBUG_PRINT("CreateBuffers finished, VAO = " << buffers.vaoTriangles.id());
}

void OpenGLRenderer::UpdateMesh(const Mesh& mesh) {
  DEBUG_PRINT("OpenGLRenderer::UpdateMesh: mesh = "
              << &mesh << " file: " << mesh.GetSourceFile().c_str());

  // Сбрасываем все текущие привязки VAO и буферов
  gl_->glBindVertexArray(0);
  gl_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

  auto it = bufferCache_.find(&mesh);
  if (it != bufferCache_.end()) {
    DEBUG_PRINT("  erasing old buffers for mesh " << &mesh);
    bufferCache_.erase(it);
  }
  auto buffers = std::make_unique<MeshBuffers>(gl_);
  CreateBuffers(mesh, *buffers);
  bufferCache_[&mesh] = std::move(buffers);

  gl_->glFlush();  // гарантируем завершение всех команд OpenGL

  DEBUG_PRINT("  new VAO = " << bufferCache_[&mesh]->vaoTriangles.id());
}

/*
void OpenGLRenderer::UpdateMesh(const Mesh& mesh) {
  DEBUG_PRINT("OpenGLRenderer::UpdateMesh: mesh = " << &mesh << " file: " <<
mesh.GetSourceFile().c_str()); auto it = bufferCache_.find(&mesh); if (it !=
bufferCache_.end()) { DEBUG_PRINT("  erasing old buffers for mesh " << &mesh);
    bufferCache_.erase(it);
  }
  auto buffers = std::make_unique<MeshBuffers>(gl_);
  CreateBuffers(mesh, *buffers);
  bufferCache_[&mesh] = std::move(buffers);
  DEBUG_PRINT("  new VAO = " << bufferCache_[&mesh]->vaoTriangles.id());
}
*/

void OpenGLRenderer::RemoveMesh(const Mesh* mesh) {
  DEBUG_PRINT("OpenGLRenderer::RemoveMesh: mesh = "
              << mesh << ", cache size before = " << bufferCache_.size());
  bufferCache_.erase(mesh);
  DEBUG_PRINT(
      "OpenGLRenderer::RemoveMesh: cache size after = " << bufferCache_.size());
}

void OpenGLRenderer::SetSettings(Settings* settings) { settings_ = settings; }

void OpenGLRenderer::UpdateSettings() {}

void OpenGLRenderer::SetFlipNormals(bool flip) {
  flipNormals_ = flip;
  solidShader_.setUniform("flipNormals", flipNormals_ ? 1 : 0);
}

void OpenGLRenderer::SetShadingType(ShadingType type) {
  DEBUG_PRINT("OpenGLRenderer::SetShadingType: " << static_cast<int>(type)
                                                 << ", clearing cache");
  shadingType_ = type;
  gl_->glBindVertexArray(0);
  gl_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  gl_->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void OpenGLRenderer::SetTexture(const QImage& image) {
  if (textureID_ != 0) gl_->glDeleteTextures(1, &textureID_);
  gl_->glGenTextures(1, &textureID_);
  gl_->glBindTexture(GL_TEXTURE_2D, textureID_);
  gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  gl_->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  QImage rgba = image.convertToFormat(QImage::Format_RGBA8888);
  gl_->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, rgba.width(), rgba.height(), 0,
                    GL_RGBA, GL_UNSIGNED_BYTE, rgba.constBits());
  gl_->glBindTexture(GL_TEXTURE_2D, 0);
}

void OpenGLRenderer::ClearTexture() {
  if (textureID_ != 0) gl_->glDeleteTextures(1, &textureID_);
  textureID_ = 0;
}

void OpenGLRenderer::SetShadingModel(Settings::ShadingModel model) {
  shadingModel_ = model;
}

void OpenGLRenderer::UpdateMeshNormals(const Mesh& mesh) {
  auto it = bufferCache_.find(&mesh);
  if (it == bufferCache_.end()) {
    // Если меша нет в кэше, создаём полностью
    UpdateMesh(mesh);
    return;
  }
  auto& buffers = *it->second;
  const auto& normals = mesh.GetNormals();
  if (normals.empty()) return;

  // Привязываем VAO, чтобы убедиться, что все состояния корректны
  buffers.vaoTriangles.bind();

  // Обновляем буфер нормалей
  buffers.normalVBO.bind();
  GLsizeiptr size = normals.size() * sizeof(Point);
  if (size <= buffers.normalBufferSize) {
    gl_->glBufferSubData(GL_ARRAY_BUFFER, 0, size, normals.data());
    DEBUG_PRINT("Updated normals via glBufferSubData, size=" << size);
  } else {
    gl_->glBufferData(GL_ARRAY_BUFFER, size, normals.data(), GL_STATIC_DRAW);
    buffers.normalBufferSize = size;
    DEBUG_PRINT("Recreated normal VBO, new size=" << size);
  }

  // Снимаем привязки
  gl_->glBindBuffer(GL_ARRAY_BUFFER, 0);
  gl_->glBindVertexArray(0);
}

}  // namespace s21
