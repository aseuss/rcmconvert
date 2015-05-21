/* tests/Writer_test.cpp
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

#include <gtest/gtest.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include "internal/rcm_internal.h"
#include <unistd.h>

#define TEST_MODEL "suzanne.obj"
#define TEST_MODEL_INVALID "does_not_exist"
#define TEST_FILE_HEADER_FILE "/tmp/123456fh"
#define TEST_OBJECT_HEADER_FILE "/tmp/123456oh"
#define TEST_ARRAYS_DATA_FILE "/tmp/123456arrays"
#define TEST_STRUCTS_DATA_FILE "/tmp/123456structs"


class WriterTest : public ::testing::Test {
public:
    WriterTest() : aimesh(0) {}
protected:
    virtual void SetUp() {
        // load model without optimizing the vertex count
        const aiScene *scene = importer.ReadFile(TEST_MODEL, 0);
        if (scene) {
            aimesh = scene->mMeshes[0];
            rcmmesh = convertAiMesh(aimesh);
        } else {
            std::cerr << importer.GetErrorString() << std::endl;
        }
        // load model with optimized vertex count
        const aiScene *scene2 = importer2.ReadFile(TEST_MODEL, aiProcess_JoinIdenticalVertices);
        if (scene) {
            aimeshOpt = scene2->mMeshes[0];
        } else {
            std::cerr << importer.GetErrorString() << std::endl;
        }
    }

    virtual void TearDown() {
        if (aimesh) {
//            delete aimesh;
        }
    }

    Assimp::Importer importer;
    Assimp::Importer importer2;
    aiMesh *aimesh;
    aiMesh *aimeshOpt;
    Mesh *rcmmesh;
};

TEST_F(WriterTest, convertAiMesh) {
    ASSERT_NE((aiMesh*) 0, aimesh);
    Mesh *mesh = convertAiMesh(aimesh);
    ASSERT_NE((Mesh*) 0, mesh);

    EXPECT_EQ(aimesh->mNumVertices, mesh->numVertices);
    EXPECT_EQ(aimesh->mNumFaces * 3, mesh->numIndices);

    unsigned int posOffset = positionOffset();
    unsigned int normOffset = normalsOffset();
    unsigned int texCoordsOffset = texCoords0Offset(mesh->flags);;
    unsigned int vertexSize = mesh->vertexSize;
    unsigned int offset = 0;

    for (int i = 0, n = 0; i < aimesh->mNumVertices; i++, n = i * vertexSize) {
        offset = n + posOffset;
        aiVector3D aipos = aimesh->mVertices[i];
        EXPECT_EQ(aipos.x, mesh->vertices[offset]);
        EXPECT_EQ(aipos.y, mesh->vertices[offset+1]);
        EXPECT_EQ(aipos.z, mesh->vertices[offset+2]);

        offset = n + normOffset;
        aiVector3D ainormal = aimesh->mNormals[i];
        EXPECT_EQ(ainormal.x, mesh->vertices[offset]);
        EXPECT_EQ(ainormal.y, mesh->vertices[offset+1]);
        EXPECT_EQ(ainormal.z, mesh->vertices[offset+2]);

        offset = n + texCoordsOffset;
        aiVector3D texCoord = aimesh->mTextureCoords[0][i];
        EXPECT_EQ(texCoord.x, mesh->vertices[offset]);
        EXPECT_EQ(texCoord.y, mesh->vertices[offset+1]);
    }
}

TEST_F(WriterTest, loadModelOptimized) {
    unsigned short expectedFlags = 0;
    std::vector<Mesh*> *meshes = loadModel(TEST_MODEL, true);
    ASSERT_EQ(1, meshes->size());
    Mesh *mesh = meshes->at(0);

    if (aimesh->HasPositions()) {
        expectedFlags |= HAS_POSITIONS;
    }
    if (aimesh->HasNormals()) {
        expectedFlags |= HAS_NORMALS;
    }
    if (aimesh->HasTextureCoords(0)) {
        expectedFlags |= HAS_UV0;
    }

    EXPECT_EQ(aimeshOpt->mNumVertices, mesh->numVertices);
    EXPECT_EQ(aimeshOpt->mNumFaces * 3, mesh->numIndices);
    EXPECT_EQ(aimeshOpt->GetNumColorChannels(), mesh->numColors);
    EXPECT_EQ(aimeshOpt->GetNumUVChannels(), mesh->numTexCoords);
    EXPECT_EQ(expectedFlags, mesh->flags);
    EXPECT_NE((float*) 0, mesh->vertices);
    EXPECT_NE((unsigned short*) 0, mesh->indices);
}

TEST_F(WriterTest, loadModelOptimizedFail) {
    std::vector<Mesh*> *meshes = loadModel(TEST_MODEL_INVALID, true);
    ASSERT_EQ((std::vector<Mesh*>*) 0, meshes);
}

TEST_F(WriterTest, loadModelNotOptimized) {
    unsigned short expectedFlags = 0;
    std::vector<Mesh*> *meshes = loadModel(TEST_MODEL, false);
    ASSERT_EQ(1, meshes->size());
    Mesh *mesh = meshes->at(0);

    if (aimesh->HasPositions()) {
        expectedFlags |= HAS_POSITIONS;
    }
    if (aimesh->HasNormals()) {
        expectedFlags |= HAS_NORMALS;
    }
    if (aimesh->HasTextureCoords(0)) {
        expectedFlags |= HAS_UV0;
    }

    EXPECT_EQ(aimesh->mNumVertices, mesh->numVertices);
    EXPECT_EQ(aimesh->mNumFaces * 3, mesh->numIndices);
    EXPECT_EQ(aimesh->GetNumColorChannels(), mesh->numColors);
    EXPECT_EQ(aimesh->GetNumUVChannels(), mesh->numTexCoords);
    EXPECT_EQ(expectedFlags, mesh->flags);
    EXPECT_NE((float*) 0, mesh->vertices);
    EXPECT_NE((unsigned short*) 0, mesh->indices);
}

TEST_F(WriterTest, loadModelNotOptimizedFail) {
    std::vector<Mesh*> *meshes = loadModel(TEST_MODEL_INVALID, false);
    ASSERT_EQ((std::vector<Mesh*>*) 0, meshes);
}

TEST_F(WriterTest, createFileHeader) {
    const uint8_t objectCount = 12;
    FileHeader *header = createFileHeader(objectCount);
    EXPECT_EQ(kMagicNumber[0], header->magicNumber[0]);
    EXPECT_EQ(kMagicNumber[1], header->magicNumber[1]);
    EXPECT_EQ(kFileFormatVersionMajor, header->version[0]);
    EXPECT_EQ(kFileFormatVersionMinor, header->version[1]);
    EXPECT_EQ(objectCount, header->objectCount);
    EXPECT_EQ(0, header->unused);
}

TEST_F(WriterTest, createObjectHeader) {
    const unsigned short vertexFlags = 0x0F6F;
    const unsigned int vertexCount = 2266;
    const unsigned int indexCount = 4096;
    const unsigned int boneCount = 32;
    const unsigned char vertexSize = 13;
    const bool useStructs =  true;
    const bool dontUseStructs = false;
    ObjectHeader *header =  createObjectHeader(
            vertexFlags, vertexCount, indexCount,
            boneCount, vertexSize, useStructs);

    EXPECT_EQ(vertexFlags, header->vertexFlags);
    EXPECT_EQ(vertexCount, header->vertexCount);
    EXPECT_EQ(indexCount, header->indexCount);
    EXPECT_EQ(boneCount, header->boneCount);
    EXPECT_EQ(vertexSize, header->vertexSize);
    EXPECT_EQ(STRUCT_OF_ARRAYS, header->type);
    delete header;

    header =  createObjectHeader(
            vertexFlags, vertexCount, indexCount,
            boneCount, vertexSize, dontUseStructs);

    EXPECT_EQ(vertexFlags, header->vertexFlags);
    EXPECT_EQ(vertexCount, header->vertexCount);
    EXPECT_EQ(indexCount, header->indexCount);
    EXPECT_EQ(boneCount, header->boneCount);
    EXPECT_EQ(vertexSize, header->vertexSize);
    EXPECT_EQ(ARRAY_OF_STRUCTS, header->type);
    delete header;
}

TEST_F(WriterTest, optimizeArrayOfStructs) {
    ASSERT_NE((aiMesh*) 0, aimesh);
    Mesh *mesh = convertAiMesh(aimesh);
    ASSERT_NE((Mesh*) 0, mesh);

    std::vector<unsigned short> indicesOut;
    std::vector<Vertex<float> > verticesOut;
    optimizeArrayOfStructs(mesh->vertices, mesh->vertexSize,
                           mesh->numVertices,
                           indicesOut, verticesOut);
    EXPECT_EQ(aimesh->mNumFaces * 3, indicesOut.size());
    // the 590 is typical for the test object, but not flexible
    EXPECT_EQ(590, verticesOut.size());
    for (int i = 0; i < indicesOut.size(); i++) {
        unsigned int index = indicesOut.at(i);
        Vertex<float> vertex = verticesOut.at(index);

        // TODO: make it more flexible? work with params to test different models?
        aiVector3D aipos  = aimesh->mVertices[i];
        aiVector3D ainorm = aimesh->mNormals[i];
        aiVector3D aitexcoord = aimesh->mTextureCoords[0][i];

        EXPECT_EQ(aipos.x, vertex.array[0]);
        EXPECT_EQ(aipos.y, vertex.array[1]);
        EXPECT_EQ(aipos.z, vertex.array[2]);

        EXPECT_EQ(ainorm.x, vertex.array[3]);
        EXPECT_EQ(ainorm.y, vertex.array[4]);
        EXPECT_EQ(ainorm.z, vertex.array[5]);

        EXPECT_EQ(aitexcoord.x, vertex.array[6]);
        EXPECT_EQ(aitexcoord.y, vertex.array[7]);
    }
}

TEST_F(WriterTest, convertArrayOfStructsToStructOfArrays) {
    ASSERT_NE((aiMesh*) 0, aimesh);
    Mesh *mesh = convertAiMesh(aimesh);
    ASSERT_NE((Mesh*) 0, mesh);

    std::vector<unsigned short> indicesOut;
    std::vector<Vertex<float> > verticesOut;
    optimizeArrayOfStructs(mesh->vertices, mesh->vertexSize,
                           mesh->numVertices,
                           indicesOut, verticesOut);
    EXPECT_EQ(aimesh->mNumFaces * 3, indicesOut.size());
    ObjectData *data = convertArrayOfStructsToStructOfArrays(verticesOut, mesh->flags,
                                                             mesh->vertexSize);
    ASSERT_NE((ObjectData*) 0, data);
    for (int i = 0; i < indicesOut.size(); i++) {
        unsigned int index = indicesOut.at(i);
        unsigned int offset = 0;

        unsigned int ind = mesh->indices[i];
        aiVector3D aipos  = aimesh->mVertices[ind];
        aiVector3D ainorm = aimesh->mNormals[ind];
        aiVector3D aitexcoord = aimesh->mTextureCoords[0][ind];

        ASSERT_NE(0, data->position.size());

        offset = index * kPositionSize;
        EXPECT_EQ(aipos.x, data->position.at(offset++));
        EXPECT_EQ(aipos.y, data->position.at(offset++));
        EXPECT_EQ(aipos.z, data->position.at(offset));

        ASSERT_NE(0, data->normals.size());

        offset = index * kNormalsSize;
        EXPECT_EQ(ainorm.x, data->normals.at(offset++));
        EXPECT_EQ(ainorm.y, data->normals.at(offset++));
        EXPECT_EQ(ainorm.z, data->normals.at(offset));

        ASSERT_NE(0, data->uvs0.size());

        offset = index * kTextureSize;
        EXPECT_EQ(aitexcoord.x, data->uvs0.at(offset++));
        EXPECT_EQ(aitexcoord.y, data->uvs0.at(offset));
    }
}

TEST_F(WriterTest, writeFileHeader) {
    FileHeader *header = createFileHeader(2);
    ASSERT_NE((FileHeader*) 0, header);
    EXPECT_EQ(2, header->objectCount);
    
    std::ofstream out(TEST_FILE_HEADER_FILE, std::ios::trunc | std::ios::binary);
    writeFileHeader(out, header);
    out.close();
    std::ifstream in(TEST_FILE_HEADER_FILE, std::ios::binary);
    FileHeader inheader;
    in.read((char*) &inheader, sizeof(inheader));
    in.close();
    unlink(TEST_FILE_HEADER_FILE);

    EXPECT_EQ(header->magicNumber[0], inheader.magicNumber[0]);
    EXPECT_EQ(header->magicNumber[1], inheader.magicNumber[1]);
    EXPECT_EQ(header->version[0], inheader.version[0]);
    EXPECT_EQ(header->version[1], inheader.version[1]);
    EXPECT_EQ(header->objectCount, inheader.objectCount);
    EXPECT_EQ(header->unused, inheader.unused);
}

TEST_F(WriterTest, writeObjectHeader) {
    unsigned char vertexSize = 0x0f;
    unsigned short vertexFlags = 0xdead;
    unsigned int vertexCount = 36;
    unsigned int indexCount = 128;
    unsigned int boneCount = 18;
    bool useStructOfArrays = false;

    ObjectHeader *header = createObjectHeader(vertexFlags,
                                              vertexCount,
                                              indexCount,
                                              boneCount,
                                              vertexSize,
                                              useStructOfArrays);
    ASSERT_NE((ObjectHeader*) 0, header);
    std::ofstream out(TEST_OBJECT_HEADER_FILE, std::ios::trunc | std::ios::binary);
    writeObjectHeader(out, header);
    out.close();
    std::ifstream in(TEST_OBJECT_HEADER_FILE, std::ios::binary);
    ObjectHeader inheader;
    in.read((char*) &inheader, sizeof(inheader));
    in.close();
    unlink(TEST_OBJECT_HEADER_FILE);

    EXPECT_EQ(header->type, inheader.type);
    EXPECT_EQ(header->vertexSize, inheader.vertexSize);
    EXPECT_EQ(header->vertexFlags, inheader.vertexFlags);
    EXPECT_EQ(header->vertexCount, inheader.vertexCount);
    EXPECT_EQ(header->indexCount, inheader.indexCount);
    EXPECT_EQ(header->boneCount, inheader.boneCount);
}

// test write object data array of structs
TEST_F(WriterTest, writeArrayOfStructsData) {
    ASSERT_NE((Mesh*) 0, rcmmesh);
    std::vector<unsigned short> indicesOut;
    std::vector<Vertex<float> > verticesOut;
    unsigned int vertexSize = rcmmesh->vertexSize;
    optimizeArrayOfStructs(rcmmesh->vertices, vertexSize,
                           rcmmesh->numVertices,
                           indicesOut, verticesOut);

    ASSERT_NE(0, verticesOut.size());
    std::ofstream out(TEST_STRUCTS_DATA_FILE, std::ios::trunc | std::ios::binary);
    writeArrayOfStructsData(out, verticesOut);
    out.close();

    std::ifstream in(TEST_STRUCTS_DATA_FILE, std::ios::binary);
    Vertex<float> vertex(vertexSize);
    for (int i = 0; i < verticesOut.size(); i++) {
        in.read((char*) vertex.array, vertexSize * sizeof(float));
        EXPECT_EQ(verticesOut.at(i), vertex);
    }
    in.close();
    unlink(TEST_STRUCTS_DATA_FILE);
}

// test write object data struct of arrays
TEST_F(WriterTest, writeStructOfArraysData) {
    ASSERT_NE((Mesh*) 0, rcmmesh);
    std::vector<unsigned short> indicesOut;
    std::vector<Vertex<float> > verticesOut;
    unsigned int vertexSize = rcmmesh->vertexSize;
    optimizeArrayOfStructs(rcmmesh->vertices, vertexSize,
                           rcmmesh->numVertices,
                           indicesOut, verticesOut);

    ASSERT_NE(0, verticesOut.size());
    ObjectData *data = convertArrayOfStructsToStructOfArrays(verticesOut, rcmmesh->flags,
                                                             rcmmesh->vertexSize);

    ASSERT_NE((ObjectData*) 0, data);
    unsigned int expectedPositionSize = verticesOut.size() * kPositionSize;
    unsigned int expectedNormalsSize = verticesOut.size() * kNormalsSize;
    unsigned int expectedTexSize = verticesOut.size() * kTextureSize;

    ASSERT_EQ(expectedPositionSize, data->position.size());
    ASSERT_EQ(expectedNormalsSize, data->normals.size());
    ASSERT_EQ(expectedTexSize, data->uvs0.size());

    std::ofstream out(TEST_ARRAYS_DATA_FILE, std::ios::trunc | std::ios::binary);
    writeStructOfArraysData(out, data);
    out.close();

    std::ifstream in(TEST_ARRAYS_DATA_FILE, std::ios::binary);
    // check positions
    for (int i = 0; i < expectedPositionSize; i++) {
        float position;
        in.read((char*) &position, sizeof(float));
        EXPECT_EQ(data->position.at(i), position);
    }
    // check normals
    for (int i = 0; i < expectedNormalsSize; i++) {
        float normal;
        in.read((char*) &normal, sizeof(float));
        EXPECT_EQ(data->normals.at(i), normal);
    }
    // check uvs
    for (int i = 0; i < expectedTexSize; i++) {
        float uv;
        in.read((char*) &uv, sizeof(float));
        EXPECT_EQ(data->uvs0.at(i), uv);
    }

    in.close();
    unlink(TEST_ARRAYS_DATA_FILE);
}

// TODO: write code to check unoptimized write cases

