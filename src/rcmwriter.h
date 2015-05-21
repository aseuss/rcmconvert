/* src/rcmwriter.h
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

#ifndef RCM_WRITER_H
#define RCM_WRITER_H

#include <vector>

struct Mesh {
    ~Mesh() {
        delete [] vertices;
        delete [] indices;
    }

    char name[32];
    unsigned short flags;
    unsigned int numVertices;
    unsigned int numIndices;
    unsigned int numBones;
    unsigned int numColors;
    unsigned int numTexCoords;
    size_t vertexSize;
    float *vertices;
    unsigned short *indices;
};

std::vector<Mesh*>* loadModel(const char *path, bool useAssimpOptimization = false);

bool writeFile(const char *path, const std::vector<Mesh*> *meshes,
        bool doOptimize = true, bool useStructOfArrays = false);

#endif
