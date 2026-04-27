#include "obj_loader.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <unordered_set>

#include "common/debug.h"

namespace s21 {

int ObjLoader::ResolveIndex(int index, size_t vertex_count) {
  if (index > 0) {
    int idx = index - 1;
    if (idx >= static_cast<int>(vertex_count)) return -1;
    return idx;
  } else if (index < 0) {
    int idx = static_cast<int>(vertex_count) + index;
    if (idx < 0 || idx >= static_cast<int>(vertex_count)) return -1;
    return idx;
  } else {
    return -1;
  }
}

void ObjLoader::AddExpandedTriangle(
    RawData& data, int v1, int v2, int v3, int uv1, int uv2, int uv3,
    std::unordered_set<std::pair<unsigned int, unsigned int>, PairHash>&
        edgeSet) {
  int idxV1 = ResolveIndex(v1, data.vertices.size());
  int idxV2 = ResolveIndex(v2, data.vertices.size());
  int idxV3 = ResolveIndex(v3, data.vertices.size());
  if (idxV1 < 0 || idxV2 < 0 || idxV3 < 0) {
    DEBUG_PRINT("Invalid vertex index in triangle: " << v1 << ", " << v2 << ", "
                                                     << v3);
    return;
  }

  const Point& p1 = data.vertices[idxV1];
  const Point& p2 = data.vertices[idxV2];
  const Point& p3 = data.vertices[idxV3];

  Point edge1 = {p2.x - p1.x, p2.y - p1.y, p2.z - p1.z};
  Point edge2 = {p3.x - p1.x, p3.y - p1.y, p3.z - p1.z};
  Point normal;
  normal.x = edge1.y * edge2.z - edge1.z * edge2.y;
  normal.y = edge1.z * edge2.x - edge1.x * edge2.z;
  normal.z = edge1.x * edge2.y - edge1.y * edge2.x;
  float len = std::sqrt(normal.x * normal.x + normal.y * normal.y +
                        normal.z * normal.z);
  if (len > 1e-6f) {
    normal.x /= len;
    normal.y /= len;
    normal.z /= len;
  }

  unsigned int baseIdx = data.expanded_vertices.size();
  data.expanded_original_indices.push_back(idxV1);
  data.expanded_original_indices.push_back(idxV2);
  data.expanded_original_indices.push_back(idxV3);

  data.expanded_vertices.push_back(p1);
  data.expanded_vertices.push_back(p2);
  data.expanded_vertices.push_back(p3);

  int idxUV1 = (uv1 != 0) ? ResolveIndex(uv1, data.uvs.size()) : -1;
  int idxUV2 = (uv2 != 0) ? ResolveIndex(uv2, data.uvs.size()) : -1;
  int idxUV3 = (uv3 != 0) ? ResolveIndex(uv3, data.uvs.size()) : -1;
  if (idxUV1 >= 0 && idxUV2 >= 0 && idxUV3 >= 0) {
    data.expanded_uvs.push_back(data.uvs[idxUV1]);
    data.expanded_uvs.push_back(data.uvs[idxUV2]);
    data.expanded_uvs.push_back(data.uvs[idxUV3]);
  } else {
    data.expanded_uvs.push_back(Point2D(0, 0));
    data.expanded_uvs.push_back(Point2D(0, 0));
    data.expanded_uvs.push_back(Point2D(0, 0));
  }

  data.expanded_normals.push_back(normal);
  data.expanded_normals.push_back(normal);
  data.expanded_normals.push_back(normal);
  // Сохраняем плоские нормали
  data.flat_normals.push_back(normal);
  data.flat_normals.push_back(normal);
  data.flat_normals.push_back(normal);

  auto addEdge = [&](unsigned int a, unsigned int b) {
    if (a > b) std::swap(a, b);
    edgeSet.emplace(a, b);
  };
  addEdge(baseIdx, baseIdx + 1);
  addEdge(baseIdx + 1, baseIdx + 2);
  addEdge(baseIdx + 2, baseIdx);

  data.expanded_triangles.push_back(baseIdx);
  data.expanded_triangles.push_back(baseIdx + 1);
  data.expanded_triangles.push_back(baseIdx + 2);
}

void ObjLoader::ComputeSmoothNormals(RawData& data, float smoothingFactor) {
  DEBUG_PRINT("ComputeSmoothNormals: smoothingFactor = " << smoothingFactor);
  if (data.expanded_vertices.empty() ||
      data.expanded_original_indices.empty()) {
    DEBUG_PRINT(
        "ComputeSmoothNormals: no expanded vertices or original indices");
    return;
  }

  // Вычисляем гладкие нормали (усреднение по исходным вершинам)
  std::vector<Point> smoothNormals(data.vertices.size(), Point(0, 0, 0));
  std::vector<int> normalCount(data.vertices.size(), 0);

  for (size_t i = 0; i < data.expanded_vertices.size(); ++i) {
    int origIdx = data.expanded_original_indices[i];
    if (origIdx >= 0 && origIdx < (int)data.vertices.size()) {
      smoothNormals[origIdx].x += data.flat_normals[i].x;
      smoothNormals[origIdx].y += data.flat_normals[i].y;
      smoothNormals[origIdx].z += data.flat_normals[i].z;
      normalCount[origIdx]++;
    }
  }

  for (size_t i = 0; i < data.vertices.size(); ++i) {
    if (normalCount[i] > 0) {
      smoothNormals[i].x /= normalCount[i];
      smoothNormals[i].y /= normalCount[i];
      smoothNormals[i].z /= normalCount[i];
      float len = std::sqrt(smoothNormals[i].x * smoothNormals[i].x +
                            smoothNormals[i].y * smoothNormals[i].y +
                            smoothNormals[i].z * smoothNormals[i].z);
      if (len > 1e-6f) {
        smoothNormals[i].x /= len;
        smoothNormals[i].y /= len;
        smoothNormals[i].z /= len;
      }
    }
  }

  // Сохраняем гладкие нормали для каждой развёрнутой вершины
  data.smooth_normals.resize(data.expanded_vertices.size());
  for (size_t i = 0; i < data.expanded_vertices.size(); ++i) {
    int origIdx = data.expanded_original_indices[i];
    if (origIdx >= 0 && origIdx < (int)data.vertices.size()) {
      data.smooth_normals[i] = smoothNormals[origIdx];
    } else {
      data.smooth_normals[i] = Point(0, 0, 0);
    }
  }

  // Интерполяция для начального фактора
  float t = std::clamp(smoothingFactor, 0.0f, 1.0f);
  data.expanded_normals.resize(data.expanded_vertices.size());
  for (size_t i = 0; i < data.expanded_vertices.size(); ++i) {
    const Point& flat = data.flat_normals[i];
    const Point& smooth = data.smooth_normals[i];
    Point blended;
    blended.x = flat.x * (1.0f - t) + smooth.x * t;
    blended.y = flat.y * (1.0f - t) + smooth.y * t;
    blended.z = flat.z * (1.0f - t) + smooth.z * t;
    float len = std::sqrt(blended.x * blended.x + blended.y * blended.y +
                          blended.z * blended.z);
    if (len > 1e-6f) {
      blended.x /= len;
      blended.y /= len;
      blended.z /= len;
    }
    data.expanded_normals[i] = blended;
  }
  DEBUG_PRINT("ComputeSmoothNormals: first blended normal = ("
              << data.expanded_normals[0].x << ", "
              << data.expanded_normals[0].y << ", "
              << data.expanded_normals[0].z << ")");
}

bool ObjLoader::Load(const std::string& filename,
                     std::vector<std::unique_ptr<SceneObject>>& out_objects,
                     std::string& error) {
  RawData raw;
  if (!ParseFile(filename, raw, error)) return false;
  if (raw.vertices.empty()) {
    error = "No vertices";
    return false;
  }
  if (raw.expanded_vertices.empty()) {
    error = "Triangulation failed";
    return false;
  }

  ComputeSmoothNormals(raw, 1.0f);

  auto mesh = std::make_unique<Mesh>();
  mesh->SetVertices(raw.expanded_vertices);
  mesh->SetNormals(raw.expanded_normals);
  mesh->SetFlatNormals(raw.flat_normals);
  mesh->SetSmoothNormals(raw.smooth_normals);
  mesh->SetUVs(raw.expanded_uvs);
  mesh->SetEdges(raw.expanded_edges);
  mesh->SetTriangles(raw.expanded_triangles);
  mesh->ComputeBoundingSphere();
  mesh->SetSourceFile(filename);
  out_objects.push_back(std::move(mesh));
  return true;
}

RawModelData ObjLoader::LoadData(const std::string& filename,
                                 float smoothingFactor) {
  DEBUG_PRINT("Loading OBJ: " << filename.c_str());
  RawModelData result;
  result.filename = filename;
  result.smoothingFactor = smoothingFactor;

  RawData raw;
  std::string error;
  if (!ParseFile(filename, raw, error)) {
    result.errorMsg = error;
    result.success = false;
    DEBUG_PRINT("ParseFile failed: " << error.c_str());
    return result;
  }
  DEBUG_PRINT("After ParseFile: vertices=" << raw.vertices.size()
                                           << ", expanded_vertices="
                                           << raw.expanded_vertices.size());

  if (raw.vertices.empty()) {
    result.errorMsg = "No vertices";
    result.success = false;
    DEBUG_PRINT(result.errorMsg.c_str());
    return result;
  }
  if (raw.expanded_vertices.empty()) {
    result.errorMsg = "Triangulation produced no vertices";
    result.success = false;
    DEBUG_PRINT(result.errorMsg.c_str());
    return result;
  }

  DEBUG_PRINT("Calling ComputeSmoothNormals...");
  ComputeSmoothNormals(raw, smoothingFactor);
  DEBUG_PRINT("ComputeSmoothNormals done");

  result.vertices = std::move(raw.expanded_vertices);
  result.normals = std::move(raw.expanded_normals);
  result.flat_normals = std::move(raw.flat_normals);
  result.smooth_normals = std::move(raw.smooth_normals);
  result.uvs = std::move(raw.expanded_uvs);
  result.edges = std::move(raw.expanded_edges);
  result.triangles = std::move(raw.expanded_triangles);
  result.success = true;

  DEBUG_PRINT("Loaded " << result.vertices.size() << " vertices, "
                        << result.edges.size() << " edges"
                        << ", normals: " << result.normals.size()
                        << ", uvs: " << result.uvs.size());
  return result;
}

bool ObjLoader::ParseFile(const std::string& filename, RawData& data,
                          std::string& error) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    error = "Cannot open file";
    return false;
  }

  std::unordered_set<std::pair<unsigned int, unsigned int>, PairHash> edgeSet;
  std::string line;

  while (std::getline(file, line)) {
    if (line.empty() || line[0] == '#') continue;

    std::istringstream iss(line);
    std::string token;
    iss >> token;

    if (token == "v") {
      float x, y, z;
      if (!(iss >> x >> y >> z)) {
        error = "Invalid vertex";
        return false;
      }
      data.vertices.push_back(Point(x, y, z));
    } else if (token == "vn") {
      float x, y, z;
      if (!(iss >> x >> y >> z)) {
        error = "Invalid normal";
        return false;
      }
    } else if (token == "vt") {
      float u, v;
      if (!(iss >> u >> v)) {
        error = "Invalid texture coordinate";
        return false;
      }
      data.uvs.push_back(Point2D(u, v));
    } else if (token == "f") {
      std::vector<int> faceVertices, faceUVs;
      std::string part;
      while (iss >> part) {
        size_t firstSlash = part.find('/');
        size_t secondSlash = part.find('/', firstSlash + 1);
        std::string vertStr = part.substr(0, firstSlash);
        if (vertStr.empty()) {
          error = "Missing vertex index";
          return false;
        }
        int vIdx = std::stoi(vertStr);
        faceVertices.push_back(vIdx);

        if (firstSlash != std::string::npos) {
          size_t texStart = firstSlash + 1;
          size_t texEnd =
              (secondSlash != std::string::npos) ? secondSlash : part.size();
          std::string texStr = part.substr(texStart, texEnd - texStart);
          faceUVs.push_back(texStr.empty() ? 0 : std::stoi(texStr));
        } else {
          faceUVs.push_back(0);
        }
      }

      if (faceVertices.size() < 3) {
        error = "Face must have at least 3 vertices";
        return false;
      }

      for (size_t i = 1; i + 1 < faceVertices.size(); ++i) {
        AddExpandedTriangle(data, faceVertices[0], faceVertices[i],
                            faceVertices[i + 1], faceUVs[0], faceUVs[i],
                            faceUVs[i + 1], edgeSet);
      }
    }
  }

  data.expanded_edges.reserve(edgeSet.size());
  for (const auto& p : edgeSet) {
    data.expanded_edges.emplace_back(p.first, p.second);
  }
  return true;
}

}  // namespace s21
