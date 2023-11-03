// ModelLoader.h

#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include <vector>
#include <string>

struct Vertex {
    float x, y, z;
};

struct TextureCoord {
    float u, v;
};

struct Normal {
    float nx, ny, nz;
};

struct Face {
    std::vector<int> vertexIndices;
    std::vector<int> textureIndices;
    std::vector<int> normalIndices;
};

bool loadOBJModel(const std::string& path,
    std::vector<Vertex>& vertices,
    std::vector<TextureCoord>& textures,
    std::vector<Normal>& normals,
    std::vector<Face>& faces);

#endif // MODEL_LOADER_H
