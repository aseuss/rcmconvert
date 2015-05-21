/* src/rcmwriter.cpp
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

#include "internal/rcm_internal.h"
#include <iostream>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

std::vector<Mesh*>* loadModel(const char *path, bool useAssimpOptimization) {
    unsigned int importerFlags = 0;
    if (useAssimpOptimization) {
        importerFlags |= aiProcess_JoinIdenticalVertices;
    }
    //importerFlags |= aiProcess_GenUVCoords;
    importerFlags |= aiProcess_Triangulate;
    importerFlags |= aiProcess_FixInfacingNormals;
    Assimp::Importer importer;

    const aiScene *scene = importer.ReadFile(path, importerFlags);
    if (!scene) {
        std::cerr << importer.GetErrorString() << std::endl;
        return 0;
    }

    if (scene->HasMeshes()) {
        unsigned int numMeshes = scene->mNumMeshes;
        std::vector<Mesh*> *meshes = new std::vector<Mesh*>();
        Mesh *mesh;
        aiMesh *aimesh;
        for (int i = 0; i < numMeshes; i++) {
            aimesh = 0;
            mesh = 0;
            aimesh = scene->mMeshes[i];
            mesh = convertAiMesh(aimesh);
            if (mesh) {
                meshes->push_back(mesh);
            } else {
                std::cerr << "problem converting mesh" << std::endl;
            }
        }
        importer.FreeScene();
        return meshes;
    } else {
        std::cerr << "scene does not have meshes" << std::endl;
    }
    importer.FreeScene();
    return 0;
}

Mesh* convertAiMesh(const aiMesh *aimesh) {
    if (!aimesh) {
        std::cerr << "aimesh is null" << std::endl;
        return 0;
    }
    uint32_t numVertices = aimesh->mNumVertices;
    uint32_t numIndices = aimesh->mNumFaces * 3;
    uint32_t vertexSize = 0;
    uint32_t posOffset = 0;
    uint32_t normalOffset = 0;
    uint32_t texOffset = 0;

    uint16_t vertexFlags = 0;

    if (aimesh->HasPositions()) {
        vertexSize += kPositionSize;
        setHasPositions(vertexFlags);
    }
    if (aimesh->HasNormals()) {
        vertexSize += kNormalsSize;
        setHasNormals(vertexFlags);
    }

    unsigned int numTexCoords = aimesh->GetNumUVChannels();
    numTexCoords = (numTexCoords > kMaxNumTexCoords) ? kMaxNumTexCoords : numTexCoords;
    vertexSize += numTexCoords * kTextureSize;
    setHasTexCoords(vertexFlags, numTexCoords);

    unsigned int numColors = aimesh->GetNumColorChannels();
    numColors = (numColors > kMaxNumColors) ? kMaxNumColors : numColors;
    vertexSize += numColors * kColorSize;
    setHasColors(vertexFlags, numColors);

    if (aimesh->HasTangentsAndBitangents()) {
        vertexSize += kTanSize + kBitanSize;
        setHasTanBitan(vertexFlags);
    }

    float *vertices = new float[vertexSize * numVertices];
    uint32_t offset = 0;
    for (int i = 0, n = 0; i < numVertices; i++, n = i * vertexSize) {
        if (aimesh->HasPositions()) {
            offset = n + positionOffset();
            aiVector3D aipos = aimesh->mVertices[i];
            vertices[offset] = aipos.x;
            vertices[offset + 1] = aipos.y;
            vertices[offset + 2] = aipos.z;
        }
        if (aimesh->HasNormals()) {
            offset = n + normalsOffset();
            aiVector3D ainormal = aimesh->mNormals[i];
            vertices[offset] = ainormal.x;
            vertices[offset + 1] = ainormal.y;
            vertices[offset + 2] = ainormal.z;
        }
        for (int k = 0; k < numTexCoords; k++) {
            if (aimesh->HasTextureCoords(k)) {
                offset = n + texCoords0Offset(vertexFlags) + k * kTextureSize;
                aiVector3D texCoord = aimesh->mTextureCoords[k][i];
                vertices[offset] = texCoord.x;
                // we want to use this with OpenGL. OpenGL uses the lower left
                // corner of an image as origin whereas plain images have their
                // origin in the upper left corner. So we have to subtract the
                // uv.y value from 1.
                vertices[offset + 1] = 1 - texCoord.y;
            }
        }
        for (int k = 0; k < numColors; k++) {
            if (aimesh->HasVertexColors(k)) {
                offset = n + color0Offset(vertexFlags) + k * kColorSize;
                aiColor4D color = aimesh->mColors[k][i];
                vertices[offset] = color.r;
                vertices[offset + 1] = color.g;
                vertices[offset + 2] = color.b;
                vertices[offset + 3] = color.a;
            }
        }
        if (aimesh->HasTangentsAndBitangents()) {
            // tangent
            offset = n + tanOffset(vertexFlags);
            aiVector3D tangent = aimesh->mTangents[i];
            vertices[offset] = tangent.x;
            vertices[offset + 1] = tangent.y;
            vertices[offset + 2] = tangent.z;

            // and bitangent
            offset = n + bitanOffset(vertexFlags);
            aiVector3D bitangent = aimesh->mBitangents[i];
            vertices[offset] = bitangent.x;
            vertices[offset + 1] = bitangent.y;
            vertices[offset + 2] = bitangent.z;
        }
    }
    uint16_t *indices = new uint16_t[numIndices];
    for (int i = 0, f = 0; i < numIndices; i += 3, f++) {
        indices[i] = aimesh->mFaces[f].mIndices[0];
        indices[i+1] = aimesh->mFaces[f].mIndices[1];
        indices[i+2] = aimesh->mFaces[f].mIndices[2];
    }

    Mesh *mesh = new Mesh();
    
    mesh->flags = vertexFlags;
    mesh->numVertices = numVertices;
    mesh->numIndices = numIndices;
    mesh->numColors = numColors;
    mesh->numTexCoords = numTexCoords;
    mesh->vertexSize = vertexSize;
    mesh->vertices = vertices;
    mesh->indices = indices;

    return mesh;
}

// TODO: write inline function that claculates vertex size from vertex flags
ObjectData* convertArrayOfStructsToStructOfArrays(std::vector<Vertex<float> > vertices,
        unsigned short vertexFlags, unsigned int vertexSize) {
    ObjectData *object = new ObjectData();
    object->vertexFlags = vertexFlags;
    for (int i = 0; i < vertices.size(); i++) {
        Vertex<float> vertex = vertices.at(i);
        int index = 0;
        if (hasPositions(vertexFlags)) {
            object->position.push_back(vertex.array[index++]);
            object->position.push_back(vertex.array[index++]);
            object->position.push_back(vertex.array[index++]);
        }
        if (hasNormals(vertexFlags)) {
            object->normals.push_back(vertex.array[index++]);
            object->normals.push_back(vertex.array[index++]);
            object->normals.push_back(vertex.array[index++]);
        }
        if (hasTexCoords0(vertexFlags)) {
            object->uvs0.push_back(vertex.array[index++]);
            object->uvs0.push_back(vertex.array[index++]);
        }
        if (hasTexCoords1(vertexFlags)) {
            object->uvs1.push_back(vertex.array[index++]);
            object->uvs1.push_back(vertex.array[index++]);
        }
        if (hasTexCoords2(vertexFlags)) {
            object->uvs2.push_back(vertex.array[index++]);
            object->uvs2.push_back(vertex.array[index++]);
        }
        if (hasTexCoords3(vertexFlags)) {
            object->uvs3.push_back(vertex.array[index++]);
            object->uvs3.push_back(vertex.array[index++]);
        }
        if (hasColor0(vertexFlags)) {
            object->color0.push_back(vertex.array[index++]);
            object->color0.push_back(vertex.array[index++]);
            object->color0.push_back(vertex.array[index++]);
            object->color0.push_back(vertex.array[index++]);
        }
        if (hasColor1(vertexFlags)) {
            object->color1.push_back(vertex.array[index++]);
            object->color1.push_back(vertex.array[index++]);
            object->color1.push_back(vertex.array[index++]);
            object->color1.push_back(vertex.array[index++]);
        }
        if (hasColor2(vertexFlags)) {
            object->color2.push_back(vertex.array[index++]);
            object->color2.push_back(vertex.array[index++]);
            object->color2.push_back(vertex.array[index++]);
            object->color2.push_back(vertex.array[index++]);
        }
        if (hasColor3(vertexFlags)) {
            object->color3.push_back(vertex.array[index++]);
            object->color3.push_back(vertex.array[index++]);
            object->color3.push_back(vertex.array[index++]);
            object->color3.push_back(vertex.array[index++]);
        }
        if (hasTanBitan(vertexFlags)) {
            object->tangents.push_back(vertex.array[index++]);
            object->tangents.push_back(vertex.array[index++]);
            object->tangents.push_back(vertex.array[index++]);

            object->bitangents.push_back(vertex.array[index++]);
            object->bitangents.push_back(vertex.array[index++]);
            object->bitangents.push_back(vertex.array[index++]);
        }
    }
    return object;
}

int writeArrayOfStructsData(std::ofstream &out, std::vector<Vertex<float> > &vertices) {
    int size = vertices[0].size * sizeof(float);
    for (int n = 0; n < vertices.size(); n++) {
        out.write((char*) vertices[n].array, size);
    }
    return size;
}

int writeStructOfArraysData(std::ofstream &out, const ObjectData *data) {
    int size = 0;
    out.write((char*) &data->position[0], data->position.size() * sizeof(float));
    size += data->position.size() * sizeof(float);
    if (hasNormals(data->vertexFlags)) { 
        out.write((char*) &data->normals[0], data->normals.size() * sizeof(float));
        size += data->normals.size() * sizeof(float);
    }
    if (hasTexCoords0(data->vertexFlags)) {
        out.write((char*) &data->uvs0[0], data->uvs0.size() * sizeof(float));
        size += data->uvs0.size() * sizeof(float);
    }
    if (hasTexCoords1(data->vertexFlags)) {
        out.write((char*) &data->uvs1[0], data->uvs1.size() * sizeof(float));
        size += data->uvs1.size() * sizeof(float);
    }
    if (hasTexCoords2(data->vertexFlags)) {
        out.write((char*) &data->uvs2[0], data->uvs2.size() * sizeof(float));
        size += data->uvs2.size() * sizeof(float);
    }
    if (hasTexCoords3(data->vertexFlags)) {
        out.write((char*) &data->uvs3[0], data->uvs3.size() * sizeof(float));
        size += data->uvs3.size() * sizeof(float);
    }
    if (hasColor0(data->vertexFlags)) {
        out.write((char*) &data->color0[0], data->color0.size() * sizeof(float));
        size += data->color0.size() * sizeof(float);
    }
    if (hasColor1(data->vertexFlags)) {
        out.write((char*) &data->color1[0], data->color1.size() * sizeof(float));
        size += data->color1.size() * sizeof(float);
    }
    if (hasColor2(data->vertexFlags)) {
        out.write((char*) &data->color2[0], data->color2.size() * sizeof(float));
        size += data->color2.size() * sizeof(float);
    }
    if (hasColor3(data->vertexFlags)) {
        out.write((char*) &data->color3[0], data->color3.size() * sizeof(float));
        size += data->color3.size() * sizeof(float);
    }
    if (hasTanBitan(data->vertexFlags)) {
        out.write((char*) &data->tangents[0], data->tangents.size() * sizeof(float));
        size += data->tangents.size() * sizeof(float);
        out.write((char*) &data->bitangents[0], data->bitangents.size() * sizeof(float));
        size += data->bitangents.size() * sizeof(float);
    }
    return size;
}

bool optimizeArrayOfStructs(float *vertices,
            size_t vertexSize,
            size_t vertexCount,
            std::vector<unsigned short> &indicesOut,
            std::vector<Vertex<float> > &verticesOut
            ) {
    // TODO: move the following stuff into an export function
    std::map<struct Vertex<float>, unsigned short> mymap;
    unsigned short index;
    for (int i = 0; i < vertexCount; i++) {
        struct Vertex<float> vertex(8);
        float *a = vertices + (i * vertexSize);
        memcpy(vertex.array, a, vertexSize * sizeof(float));
        bool found = findVertexIndex(mymap, vertex, index);
        if (!found) {
            verticesOut.push_back(vertex);
            unsigned short newIndex =
                    (unsigned short) verticesOut.size() - 1;
            indicesOut.push_back(newIndex);
            mymap[vertex] = newIndex;
        } else {
            indicesOut.push_back(index);
        }
    }
    return true;
}

void writeElementArray(std::ofstream &out, const Mesh *mesh, unsigned int offset, size_t elementSize) {
    float *vertices = mesh->vertices;
    unsigned int numVertices = mesh->numVertices;
    size_t vertexSize = mesh->vertexSize;

    for (int i = offset; i < numVertices * vertexSize; i += vertexSize) {
        out.write((char*) &vertices[i], elementSize * sizeof(float));
    }
}

bool writeObject(std::ofstream &out, const Mesh* mesh,
        bool doOptimize, bool useStructOfArrays) {

    if (!mesh) {
        std::cerr << "mesh is null" << std::endl;
        return false;
    }

    const unsigned short vertexFlags = mesh->flags;

    if (doOptimize) {
        std::vector<unsigned short> indicesOut;
        std::vector<Vertex<float> > verticesOut;
        optimizeArrayOfStructs(mesh->vertices, mesh->vertexSize,
                               mesh->numVertices,
                               indicesOut, verticesOut);
        ObjectHeader *header = createObjectHeader(mesh->flags, verticesOut.size(),
                                                 indicesOut.size(), mesh->numBones,
                                                 mesh->vertexSize, useStructOfArrays);
        writeObjectHeader(out, header);
        delete header;

        // after this point create struct of arrays or leave as is
        if (useStructOfArrays) {
            ObjectData *data = convertArrayOfStructsToStructOfArrays(verticesOut, vertexFlags,
                                                                     mesh->vertexSize);
            writeStructOfArraysData(out, data);
            delete data;
        } else {
            writeArrayOfStructsData(out, verticesOut);
        }
        out.write((char*) &indicesOut[0],
            indicesOut.size() * sizeof(uint16_t));
    } else {
        ObjectHeader *header = createObjectHeader(mesh->flags, mesh->numVertices,
                                                 mesh->numIndices, mesh->numBones,
                                                 mesh->vertexSize, useStructOfArrays);
        writeObjectHeader(out, header);
        delete header;

        // write struct of arrays
        if (useStructOfArrays) {
            if (hasPositions(vertexFlags)) {
                writeElementArray(out, mesh, positionOffset(), kPositionSize);
            }
            if (hasNormals(vertexFlags)) {
                writeElementArray(out, mesh, normalsOffset(), kNormalsSize);
            }
            if (hasTexCoords0(vertexFlags)) {
                writeElementArray(out, mesh, texCoords0Offset(vertexFlags), kTextureSize);
            }
            if (hasTexCoords1(vertexFlags)) {
                writeElementArray(out, mesh, texCoords1Offset(vertexFlags), kTextureSize);
            }
            if (hasTexCoords2(vertexFlags)) {
                writeElementArray(out, mesh, texCoords2Offset(vertexFlags), kTextureSize);
            }
            if (hasTexCoords3(vertexFlags)) {
                writeElementArray(out, mesh, texCoords3Offset(vertexFlags), kTextureSize);
            }
            if (hasColor0(vertexFlags)) {
                writeElementArray(out, mesh, color0Offset(vertexFlags), kColorSize);
            }
            if (hasColor1(vertexFlags)) {
                writeElementArray(out, mesh, color1Offset(vertexFlags), kColorSize);
            }
            if (hasColor2(vertexFlags)) {
                writeElementArray(out, mesh, color2Offset(vertexFlags), kColorSize);
            }
            if (hasColor3(vertexFlags)) {
                writeElementArray(out, mesh, color3Offset(vertexFlags), kColorSize);
            }
            if (hasTanBitan(vertexFlags)) {
                writeElementArray(out, mesh, tanOffset(vertexFlags), kTanSize);
                writeElementArray(out, mesh, bitanOffset(vertexFlags), kBitanSize);
            }
        } else {
            // write array of structs
            out.write((char*) mesh->vertices,
                      mesh->numVertices * mesh->vertexSize * sizeof(float));
        }
        // write indices
        out.write((char*) mesh->indices, mesh->numIndices * sizeof(uint16_t));
    }
    return true;
}

bool writeFile(const char *path, const std::vector<Mesh*> *meshes,
        bool doOptimize, bool useStructOfArrays) {
    
    std::ofstream out(path, std::ios::trunc | std::ios::binary);
    if (!out.is_open()) {
        std::cerr << "could not open file: " << path << std::endl;
        return false;
    }
    FileHeader *fileHeader = createFileHeader(meshes->size());
    writeFileHeader(out, fileHeader);
    delete fileHeader;

    for (int i = 0; i < meshes->size(); i++) {
        writeObject(out, meshes->at(i), doOptimize, useStructOfArrays);
    }
    out.close();
}

int writeFileHeader(std::ofstream &out, const FileHeader *header) {
    if (!header) {
        return -1;
    }
    const int size = sizeof(FileHeader);
    // write file meta data
    out.write((char*) header, size);
    return size;
}

// TODO: think about writing a constructor
FileHeader* createFileHeader(unsigned int numObjects) {
    FileHeader *header = new FileHeader();
    header->magicNumber[0] = kMagicNumber[0];
    header->magicNumber[1] = kMagicNumber[1];
    header->version[0] = kFileFormatVersionMajor;
    header->version[1] = kFileFormatVersionMinor;
    header->objectCount = numObjects;
    header->unused = 0;
    return header;
}

int writeObjectHeader(std::ofstream &out, const ObjectHeader* header) {
    if (!header) {
        return -1;
    }
    int size = sizeof(ObjectHeader);
    out.write((char*) header, size);
    return size;
}

// TODO: think about writing a constructor
ObjectHeader*  createObjectHeader(
        unsigned short vertexFlags,
        unsigned int vertexCount,
        unsigned int indexCount,
        unsigned int boneCount,
        unsigned char vertexSize,
        bool useStructOfArrays) {

    ObjectHeader *header = new ObjectHeader();
    if (useStructOfArrays) {
        header->type = (uint8_t) STRUCT_OF_ARRAYS;
    } else {
        header->type = (uint8_t) ARRAY_OF_STRUCTS;
    }
    // use half floats?
    // TODO: all the half float implementation
    header->vertexSize = vertexSize;
    header->vertexFlags = vertexFlags;
    header->vertexCount = vertexCount;
    header->indexCount = indexCount;
    header->boneCount = boneCount;
    return header;
}

