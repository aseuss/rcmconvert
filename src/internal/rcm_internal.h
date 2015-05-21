/* src/internal/rcm_internal.h
 *
 * Copyright 2014,2015 Andreas Seuss
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * */

#ifndef RCM_INTERNAL_H
#define RCM_INTERNAL_H

#include <assimp/scene.h>
#include <fstream>
#include <map>
#include "../rcm.h"
#include "../rcmwriter.h"

struct ObjectData {
    unsigned short vertexFlags;
    std::vector<float> position;
    std::vector<float> normals;
    std::vector<float> uvs0;
    std::vector<float> uvs1;
    std::vector<float> uvs2;
    std::vector<float> uvs3;
    std::vector<float> color0;
    std::vector<float> color1;
    std::vector<float> color2;
    std::vector<float> color3;
    std::vector<float> tangents;
    std::vector<float> bitangents;
};

template<typename T>
struct Vertex {
    size_t size;
    T *array;

    Vertex(size_t vertexSize) {
        size = vertexSize;
        array = new T[size];
    }

    ~Vertex() {
        delete[] array;
    }

    Vertex(const Vertex &other) {
        size = other.size;
        array = new T[size];
        memcpy(array, other.array, sizeof(T) * size);
    }

    Vertex operator=(const Vertex &other) {
        size = other.size;
        array = new T[size];
        memcpy(array, other.array, sizeof(T) * size);
    }

    bool operator<(const Vertex &that) const {
        return memcmp((void*) this->array, (void*) that.array,
                      sizeof(T) * size)>0;
    };

    bool operator==(const Vertex &that) const {
        return memcmp((void*) this->array, (void*) that.array, sizeof(T) * size) == 0;
    };
};

int writeFileHeader(std::ofstream &out, const FileHeader *header);

FileHeader* createFileHeader(unsigned int numObjects);

int writeObjectHeader(std::ofstream &out, const ObjectHeader* header);

ObjectHeader* createObjectHeader( 
        unsigned short vertexFlags,
        unsigned int vertexCount,
        unsigned int indexCount,
        unsigned int boneCount,
        unsigned char vertexSize,
        bool useStructOfArrays = false);

ObjectData* convertArrayOfStructsToStructOfArrays(std::vector<Vertex<float> > vertices,
        unsigned short vertexFlags, unsigned int vertexSize);

int writeArrayOfStructsData(std::ofstream &out, std::vector<Vertex<float> > &vertices);

int writeStructOfArraysData(std::ofstream &out, const ObjectData *data);

Mesh* convertAiMesh(const aiMesh *aimesh);

void writeFileHeader(std::ofstream &out, unsigned int numObjects);

bool writeObject(std::ofstream &out, const Mesh* mesh,
        bool doOptimize = true, bool useStructOfArrays = false);

template<typename T>
bool findVertexIndex(std::map<T, unsigned short> &vertices,
        T &vertex, unsigned short &result) {
    typename std::map<T, unsigned short>::iterator it = vertices.find(vertex);
    if (it == vertices.end()) {
        return false;
    } else {
        result = it->second;
        return true;
    }
}

bool optimizeArrayOfStructs(float *vertices,
            size_t vertexSize,
            size_t vertexCount,
            std::vector<unsigned short> &indicesOut,
            std::vector<Vertex<float> > &verticesOut);

#endif // RCM_INTERNAL_H
