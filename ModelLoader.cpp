// ModelLoader.cpp

#include "ModelLoader.h"
#include <tuple>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

bool loadOBJModel(const std::string& path, std::vector<Vertex>& vertices, std::vector<TextureCoord>& textures, std::vector<Normal>& normals, std::vector<Face>& faces) {
    std::ifstream file(path);
    if (!file.is_open()) {
        std::cerr << "Failed to open the file: " << path << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            Vertex v;
            iss >> v.x >> v.y >> v.z;
            vertices.push_back(v);
        }
        else if (prefix == "vt") {
            TextureCoord t;
            iss >> t.u >> t.v;
            textures.push_back(t);
        }
        else if (prefix == "vn") {
            Normal n;
            iss >> n.nx >> n.ny >> n.nz;
            normals.push_back(n);
        }
        else if (prefix == "f") {
            Face f;
            std::string vertexDef;
            while (iss >> vertexDef) {
                std::istringstream viss(vertexDef);
                std::string index;
                std::getline(viss, index, '/');
                f.vertexIndices.push_back(std::stoi(index));
                if (std::getline(viss, index, '/')) {
                    f.textureIndices.push_back(std::stoi(index));
                }
                if (std::getline(viss, index, '/')) {
                    f.normalIndices.push_back(std::stoi(index));
                }
            }
            faces.push_back(f);
        }
    }

    file.close();
    return true;
}
