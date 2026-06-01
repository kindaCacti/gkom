
#ifndef MESH_LOADER_H
#define MESH_LOADER_H

#include <glad/glad.h>

#include <glm/glm.hpp>

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <optional>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "mesh.h"

namespace mesh_loader {

namespace detail {

inline bool startsWith(std::string_view s, std::string_view prefix) {
    return s.size() >= prefix.size() && s.substr(0, prefix.size()) == prefix;
}

inline std::string_view ltrim(std::string_view s) {
    while (!s.empty() &&
           (s.front() == ' ' || s.front() == '\t' || s.front() == '\r')) {
        s.remove_prefix(1);
    }
    return s;
}

inline bool parseInt(std::string_view s, int &out) {
    s = ltrim(s);
    if (s.empty()) {
        return false;
    }
    const char *begin = s.data();
    const char *end = s.data() + s.size();
    auto result = std::from_chars(begin, end, out);
    return result.ec == std::errc{};
}

struct FaceVertexIndex {
    int v = 0;
    int vt = 0;
    int vn = 0;
};

inline FaceVertexIndex parseFaceVertex(std::string_view token) {
    // token forms: v, v/vt, v//vn, v/vt/vn
    FaceVertexIndex out;

    auto nextPart = [&](std::string_view &t) -> std::string_view {
        auto slash = t.find('/');
        if (slash == std::string_view::npos) {
            auto part = t;
            t = {};
            return part;
        }
        auto part = t.substr(0, slash);
        t.remove_prefix(slash + 1);
        return part;
    };

    std::string_view t = token;
    std::string_view p1 = nextPart(t);
    std::string_view p2 = nextPart(t);
    std::string_view p3 = t;

    (void)parseInt(p1, out.v);
    if (!p2.empty()) {
        (void)parseInt(p2, out.vt);
    }
    if (!p3.empty()) {
        (void)parseInt(p3, out.vn);
    }
    return out;
}

inline int resolveObjIndex(int idx, std::size_t count) {
    // OBJ indices are 1-based. Negative indices are relative to the end.
    if (idx > 0) {
        return idx - 1;
    }
    if (idx < 0) {
        return static_cast<int>(count) + idx;
    }
    return -1;
}

struct VertexKey {
    int v;
    int vt;
    int vn;
    int mat;

    bool operator==(const VertexKey &other) const {
        return v == other.v && vt == other.vt && vn == other.vn &&
               mat == other.mat;
    }
};

struct VertexKeyHasher {
    std::size_t operator()(const VertexKey &k) const noexcept {
        // A simple mix for 4 ints.
        std::size_t h1 = std::hash<int>{}(k.v);
        std::size_t h2 = std::hash<int>{}(k.vt);
        std::size_t h3 = std::hash<int>{}(k.vn);
        std::size_t h4 = std::hash<int>{}(k.mat);
        std::size_t h = h1;
        h ^= h2 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        h ^= h3 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        h ^= h4 + 0x9e3779b97f4a7c15ULL + (h << 6) + (h >> 2);
        return h;
    }
};

inline void parse_mtl_diffuse_colors(
    const std::filesystem::path &mtlPath,
    std::unordered_map<std::string, glm::vec3> &outDiffuse) {
    std::ifstream mtlFile(mtlPath);
    if (!mtlFile.is_open()) {
        return;
    }

    std::string current;
    std::string line;
    while (std::getline(mtlFile, line)) {
        std::string_view sv(line);
        sv = ltrim(sv);
        if (sv.empty() || sv.front() == '#') {
            continue;
        }

        if (startsWith(sv, "newmtl ")) {
            sv.remove_prefix(7);
            sv = ltrim(sv);
            current = std::string(sv);
            continue;
        }

        if (current.empty()) {
            continue;
        }

        if (startsWith(sv, "Kd ")) {
            sv.remove_prefix(3);
            sv = ltrim(sv);
            float r = 1.f, g = 1.f, b = 1.f;
            std::istringstream iss{std::string(sv)};
            iss >> r >> g >> b;
            if (!iss.fail()) {
                outDiffuse[current] = glm::vec3(r, g, b);
            }
            continue;
        }
    }
}

} // namespace detail

// Loads a Wavefront OBJ file and uploads it as a GL mesh.
// Supported statements: v, vt, vn, f.
// Also supported (colors only): mtllib/usemtl with MTL Kd (diffuse color).
// Vertex layout:
// - location 0: position (vec3)
// - location 1: normal (vec3) if any vn exists
// - location 2: color (vec3) if color is provided or MTL Kd is available
// - location 3: texcoord (vec2) if any vt exists
// NOTE: Requires a valid OpenGL context to be current when called.
inline std::optional<Mesh>
load_obj(const std::string &path,
         std::optional<glm::vec3> color = std::nullopt) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::nullopt;
    }

    std::vector<std::string> lines;
    lines.reserve(4096);
    {
        std::string line;
        while (std::getline(file, line)) {
            lines.push_back(std::move(line));
        }
    }

    struct Vec2 {
        float x = 0.f;
        float y = 0.f;
    };
    struct Vec3 {
        float x = 0.f;
        float y = 0.f;
        float z = 0.f;
    };

    std::vector<Vec3> positions;
    positions.reserve(1024);

    // vt/vn are optional.
    std::vector<Vec2> texcoords;
    std::vector<Vec3> normals;
    texcoords.reserve(1024);
    normals.reserve(1024);

    // Material diffuse colors (Kd) parsed from referenced .mtl files.
    std::unordered_map<std::string, glm::vec3> materialDiffuse;
    materialDiffuse.reserve(64);

    const std::filesystem::path objPath = std::filesystem::path(path);
    const std::filesystem::path objDir = objPath.has_parent_path()
                                             ? objPath.parent_path()
                                             : std::filesystem::path(".");
    for (const auto &raw : lines) {
        std::string_view sv(raw);
        sv = detail::ltrim(sv);
        if (sv.empty() || sv.front() == '#') {
            continue;
        }
        if (!detail::startsWith(sv, "mtllib ")) {
            continue;
        }
        sv.remove_prefix(7);
        sv = detail::ltrim(sv);

        // mtllib can list multiple files on the same line.
        std::istringstream iss{std::string(sv)};
        std::string mtlName;
        while (iss >> mtlName) {
            const auto mtlPath = (objDir / mtlName).lexically_normal();
            detail::parse_mtl_diffuse_colors(mtlPath, materialDiffuse);
        }
    }

    const bool hasColors = color.has_value() || !materialDiffuse.empty();
    bool hasTexcoords = false;
    bool hasNormals = false;
    for (const auto &raw : lines) {
        std::string_view sv(raw);
        sv = detail::ltrim(sv);
        if (sv.empty() || sv.front() == '#') {
            continue;
        }
        if (detail::startsWith(sv, "vt ")) {
            hasTexcoords = true;
        }
        if (detail::startsWith(sv, "vn ")) {
            hasNormals = true;
        }
    }

    const int strideFloats =
        3 + (hasNormals ? 3 : 0) + (hasColors ? 3 : 0) + (hasTexcoords ? 2 : 0);

    // packed: position, normal?, color?, texCoord?
    std::vector<float> vertices;
    vertices.reserve(4096 * strideFloats);
    std::vector<unsigned int> indices; // triangles
    std::unordered_map<detail::VertexKey, unsigned int, detail::VertexKeyHasher>
        vertexMap;
    vertexMap.reserve(2048);

    float minX = 0.f, maxX = 0.f, minY = 0.f, maxY = 0.f, minZ = 0.f,
          maxZ = 0.f;
    glm::vec3 currentMtlColor(1.f, 1.f, 1.f);
    int currentMtlId = -1;
    std::unordered_map<std::string, int> materialId;
    materialId.reserve(materialDiffuse.size());
    std::vector<glm::vec3> materialColors;
    materialColors.reserve(materialDiffuse.size());
    if (!materialDiffuse.empty()) {
        for (const auto &kv : materialDiffuse) {
            const int id = static_cast<int>(materialColors.size());
            materialId.emplace(kv.first, id);
            materialColors.push_back(kv.second);
        }
    }

    for (const auto &raw : lines) {
        std::string_view sv(raw);
        sv = detail::ltrim(sv);
        if (sv.empty() || sv.front() == '#') {
            continue;
        }

        if (!color.has_value() && !materialDiffuse.empty() &&
            detail::startsWith(sv, "usemtl ")) {
            sv.remove_prefix(7);
            sv = detail::ltrim(sv);
            const std::string name(sv);

            auto it = materialId.find(name);
            if (it != materialId.end()) {
                currentMtlId = it->second;
                if (currentMtlId >= 0 &&
                    static_cast<std::size_t>(currentMtlId) <
                        materialColors.size()) {
                    currentMtlColor =
                        materialColors[static_cast<std::size_t>(currentMtlId)];
                } else {
                    currentMtlColor = glm::vec3(1.f, 1.f, 1.f);
                }
            } else {
                currentMtlId = -1;
                currentMtlColor = glm::vec3(1.f, 1.f, 1.f);
            }
            continue;
        }

        if (detail::startsWith(sv, "v ")) {
            // v x y z
            sv.remove_prefix(2);
            sv = detail::ltrim(sv);

            float x = 0.f, y = 0.f, z = 0.f;
            std::istringstream iss{std::string(sv)};
            iss >> x >> y >> z;
            if (!iss.fail()) {
                positions.push_back(Vec3{x, y, z});
                minX = std::min(minX, x);
                maxX = std::max(maxX, x);
                minY = std::min(minY, y);
                maxY = std::max(maxY, y);
                minZ = std::min(minZ, z);
                maxZ = std::max(maxZ, z);
            }
            continue;
        }

        if (detail::startsWith(sv, "vt ")) {
            // vt u v
            sv.remove_prefix(3);
            sv = detail::ltrim(sv);
            float u = 0.f, v = 0.f;
            std::istringstream iss{std::string(sv)};
            iss >> u >> v;
            if (!iss.fail()) {
                texcoords.push_back(Vec2{u, v});
            }
            continue;
        }

        if (detail::startsWith(sv, "vn ")) {
            // vn x y z
            sv.remove_prefix(3);
            sv = detail::ltrim(sv);
            float x = 0.f, y = 0.f, z = 0.f;
            std::istringstream iss{std::string(sv)};
            iss >> x >> y >> z;
            if (!iss.fail()) {
                normals.push_back(Vec3{x, y, z});
            }
            continue;
        }

        if (detail::startsWith(sv, "f ")) {
            // f a b c [d ...]
            sv.remove_prefix(2);
            sv = detail::ltrim(sv);

            std::vector<detail::FaceVertexIndex> face;
            face.reserve(8);

            std::istringstream iss{std::string(sv)};
            std::string tok;
            while (iss >> tok) {
                face.push_back(detail::parseFaceVertex(tok));
            }
            if (face.size() < 3) {
                continue;
            }

            auto emitVertexIndex =
                [&](const detail::FaceVertexIndex &fvi) -> unsigned int {
                const int posIndex =
                    detail::resolveObjIndex(fvi.v, positions.size());
                if (posIndex < 0 ||
                    static_cast<std::size_t>(posIndex) >= positions.size()) {
                    return 0;
                }

                int vtIndex = -1;
                int vnIndex = -1;
                if (hasTexcoords && fvi.vt != 0) {
                    vtIndex = detail::resolveObjIndex(fvi.vt, texcoords.size());
                }
                if (hasNormals && fvi.vn != 0) {
                    vnIndex = detail::resolveObjIndex(fvi.vn, normals.size());
                }

                const int matKey =
                    (hasColors && !color.has_value()) ? currentMtlId : 0;
                detail::VertexKey key{posIndex, vtIndex, vnIndex, matKey};
                auto it = vertexMap.find(key);
                if (it != vertexMap.end()) {
                    return it->second;
                }

                const Vec3 &p = positions[static_cast<std::size_t>(posIndex)];
                const unsigned int newIndex =
                    static_cast<unsigned int>(vertices.size() / strideFloats);

                vertices.push_back(p.x);
                vertices.push_back(p.y);
                vertices.push_back(p.z);

                if (hasNormals) {
                    if (vnIndex >= 0 &&
                        static_cast<std::size_t>(vnIndex) < normals.size()) {
                        const Vec3 &n =
                            normals[static_cast<std::size_t>(vnIndex)];
                        vertices.push_back(n.x);
                        vertices.push_back(n.y);
                        vertices.push_back(n.z);
                    } else {
                        vertices.push_back(0.f);
                        vertices.push_back(0.f);
                        vertices.push_back(0.f);
                    }
                }

                if (hasColors) {
                    if (color.has_value()) {
                        const glm::vec3 c = *color;
                        vertices.push_back(c.r);
                        vertices.push_back(c.g);
                        vertices.push_back(c.b);
                    } else {
                        vertices.push_back(currentMtlColor.r);
                        vertices.push_back(currentMtlColor.g);
                        vertices.push_back(currentMtlColor.b);
                    }
                }

                if (hasTexcoords) {
                    if (vtIndex >= 0 &&
                        static_cast<std::size_t>(vtIndex) < texcoords.size()) {
                        const Vec2 &t =
                            texcoords[static_cast<std::size_t>(vtIndex)];
                        vertices.push_back(t.x);
                        vertices.push_back(t.y);
                    } else {
                        vertices.push_back(0.f);
                        vertices.push_back(0.f);
                    }
                }

                vertexMap.emplace(key, newIndex);
                return newIndex;
            };

            // Triangulate polygon via fan
            const unsigned int i0 = emitVertexIndex(face[0]);
            for (std::size_t i = 1; i + 1 < face.size(); ++i) {
                const unsigned int i1 = emitVertexIndex(face[i]);
                const unsigned int i2 = emitVertexIndex(face[i + 1]);
                indices.push_back(i0);
                indices.push_back(i1);
                indices.push_back(i2);
            }
            continue;
        }
    }

    if (vertices.empty() || indices.empty()) {
        return std::nullopt;
    }

    Mesh mesh(vertices, indices, hasNormals, hasColors, hasTexcoords, minX,
              maxX, minY, maxY, minZ, maxZ);
    return mesh;
}

} // namespace mesh_loader

#endif
