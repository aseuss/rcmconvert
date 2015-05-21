/* src/rcmreader.cpp
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

#include "rcmreader.h"
#include <iostream>

FileHeader* readFileHeader(std::ifstream &in) {
    if (!in.is_open()) {
        return 0;
    }
    in.seekg(0, in.end);
    size_t length = in.tellg();
    in.seekg(0, in.beg);
    if (length < sizeof(struct FileHeader)) {
        std::cerr << "file too small even for header. abort" << std::endl;
        in.close();
        return 0;
    }
    FileHeader *header = new FileHeader();
    in.read((char*) header, sizeof(FileHeader));
    if (header->magicNumber[0] != kMagicNumber[0] ||
        header->magicNumber[1] != kMagicNumber[1]) {
        std::cerr << "magic number mismatch. abort import" << std::endl;
        in.close();
        return NULL;
    }
    return header;
}

ObjectHeader* readObjectHeader(std::ifstream &in) {
    if (!in.is_open()) {
        return 0;
    }
    ObjectHeader *header = new ObjectHeader();
    in.read((char*) header, sizeof(ObjectHeader));
    return header;
}

Bla* readArrayOfStructs(std::ifstream &in, const ObjectHeader *object) {
    if (!in.is_open()) {
        return 0;
    }
    uint32_t vertexCount = object->vertexCount;
    uint32_t indexCount = object->indexCount;
    uint32_t vertexSize = calcVertexSize(object->vertexFlags);

    Bla *bla = new Bla();
    bla->header = (ObjectHeader*) object;
    bla->vertices = new float*[1];
    const unsigned int size = vertexCount * vertexSize;
    bla->vertices[0] = new float[size];
    bla->indices = new unsigned short[indexCount];
    in.read((char*) bla->vertices[0], size * sizeof(float));
    in.read((char*) bla->indices, indexCount * sizeof(unsigned short));

    return bla;
}

void readData(std::ifstream &in, float **data, unsigned int elementSize, unsigned int vertexCount) {
    unsigned int size = elementSize * vertexCount;
    *data = new float[size];
    in.read((char*) *data, size * sizeof(float));
}

Bla* readStructOfArrays(std::ifstream &in, const ObjectHeader *object) {
    if (!in.is_open()) {
        return 0;
    }
    unsigned short vertexFlags = object->vertexFlags;
    uint32_t vertexCount = object->vertexCount;
    uint32_t indexCount = object->indexCount;
    uint32_t vertexSize = calcVertexSize(object->vertexFlags);

    // TODO: figure out how many arrays there are by vertexFlags
    // then allocate those arrays directly into array
    unsigned int numVertexElements = 12;
    // TODO: clean this up, max num of vertex data, define global indices into this array
    Bla *bla = new Bla();
    bla->header = (ObjectHeader*) object;
    bla->vertices = new float*[numVertexElements];
    bla->indices = new unsigned short[indexCount];
    for (int i = 0; i < numVertexElements; i++) {
        // initialize all to 0
        bla->vertices[i] = 0;
    }

    if (hasPositions(vertexFlags)) {
        readData(in, &bla->vertices[0], kPositionSize, vertexCount);
    }
    if (hasPositions(vertexFlags)) {
        readData(in, &bla->vertices[1], kNormalsSize, vertexCount);
    }
    if (hasTexCoords0(vertexFlags)) {
        readData(in, &bla->vertices[2], kTextureSize, vertexCount);
    }
    if (hasTexCoords1(vertexFlags)) {
        readData(in, &bla->vertices[3], kTextureSize, vertexCount);
    }
    if (hasTexCoords2(vertexFlags)) {
        readData(in, &bla->vertices[4], kTextureSize, vertexCount);
    }
    if (hasTexCoords3(vertexFlags)) {
        readData(in, &bla->vertices[5], kTextureSize, vertexCount);
    }
    if (hasColor0(vertexFlags)) {
        readData(in, &bla->vertices[6], kColorSize, vertexCount);
    }
    if (hasColor1(vertexFlags)) {
        readData(in, &bla->vertices[7], kColorSize, vertexCount);
    }
    if (hasColor2(vertexFlags)) {
        readData(in, &bla->vertices[8], kColorSize, vertexCount);
    }
    if (hasColor3(vertexFlags)) {
        readData(in, &bla->vertices[9], kColorSize, vertexCount);
    }
    if (hasTanBitan(vertexFlags)) {
        readData(in, &bla->vertices[10], kTanSize, vertexCount);
        readData(in, &bla->vertices[11], kBitanSize, vertexCount);
    }
    in.read((char*) bla->indices, indexCount * sizeof(unsigned short));

    return bla;
}
