/* tests/Reader_test.cpp
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
#include "rcmreader.h"
#include "internal/rcm_internal.h"

#define TEST_MODEL "suzanne.obj"
#define TEST_MODEL_INVALID "does_not_exist"
#define TEST_FILE_HEADER_FILE "/tmp/123456fh"
#define TEST_OBJECT_HEADER_FILE "/tmp/123456oh"
#define TEST_ARRAYS_DATA_FILE "/tmp/123456arrays"
#define TEST_ARRAYS_DATA_OPT_FILE "/tmp/123456arraysopt"
#define TEST_STRUCTS_DATA_FILE "/tmp/123456structs"
#define TEST_STRUCTS_DATA_OPT_FILE "/tmp/123456structsopt"

class ReaderTest : public ::testing::Test {
public:
    ReaderTest() : aimesh(0) {}
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
    }

    virtual void TearDown() {
        if (aimesh) {
//            delete aimesh;
        }
    }

    Assimp::Importer importer;
    aiMesh *aimesh;
    Mesh *rcmmesh;
};

TEST_F(ReaderTest, readFileHeader) {
    FileHeader *header = createFileHeader(2);
    ASSERT_NE((FileHeader*) 0, header);
    EXPECT_EQ(2, header->objectCount);
    
    std::ofstream out(TEST_FILE_HEADER_FILE, std::ios::trunc | std::ios::binary);
    writeFileHeader(out, header);
    out.close();
    std::ifstream in(TEST_FILE_HEADER_FILE, std::ios::binary);
    FileHeader *inheader = readFileHeader(in);
    in.close();
    unlink(TEST_FILE_HEADER_FILE);

    EXPECT_EQ(header->magicNumber[0], inheader->magicNumber[0]);
    EXPECT_EQ(header->magicNumber[1], inheader->magicNumber[1]);
    EXPECT_EQ(header->version[0], inheader->version[0]);
    EXPECT_EQ(header->version[1], inheader->version[1]);
    EXPECT_EQ(header->objectCount, inheader->objectCount);
    EXPECT_EQ(header->unused, inheader->unused);
}

TEST_F(ReaderTest, readObjectHeader) {
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
    ObjectHeader* inheader = readObjectHeader(in);
    in.close();
    unlink(TEST_OBJECT_HEADER_FILE);

    EXPECT_EQ(header->type, inheader->type);
    EXPECT_EQ(header->vertexSize, inheader->vertexSize);
    EXPECT_EQ(header->vertexFlags, inheader->vertexFlags);
    EXPECT_EQ(header->vertexCount, inheader->vertexCount);
    EXPECT_EQ(header->indexCount, inheader->indexCount);
    EXPECT_EQ(header->boneCount, inheader->boneCount);
}

TEST_F(ReaderTest, wrongMagicNumber) {
    FileHeader *header = new FileHeader();
    header->magicNumber[0] = 0xab;
    header->magicNumber[1] = 0xff;
    
    std::ofstream out(TEST_FILE_HEADER_FILE, std::ios::trunc | std::ios::binary);
    writeFileHeader(out, header);
    out.close();
    std::ifstream in(TEST_FILE_HEADER_FILE, std::ios::binary);
    FileHeader *inheader = readFileHeader(in);
    in.close();
    unlink(TEST_FILE_HEADER_FILE);

    EXPECT_EQ((FileHeader*) 0, inheader);
}

TEST_F(ReaderTest, readOptimizedArrayOfStructs) {
    std::vector<Mesh*> *meshes = loadModel("suzanne.obj");
    writeFile(TEST_STRUCTS_DATA_OPT_FILE, meshes);

    std::ifstream in(TEST_STRUCTS_DATA_OPT_FILE, std::ios::binary);
    FileHeader *fileHeader = readFileHeader(in);
    ObjectHeader *objHeader = readObjectHeader(in);
    ASSERT_EQ(ARRAY_OF_STRUCTS, objHeader->type);

    Bla *bla = readArrayOfStructs(in, objHeader);

    ASSERT_NE((Bla*) 0, bla);
    ASSERT_NE((ObjectHeader*) 0, bla->header);
    ASSERT_EQ(objHeader, bla->header);
    ASSERT_EQ(calcVertexSize(objHeader->vertexFlags), objHeader->vertexSize);
    ASSERT_EQ(calcVertexSize(objHeader->vertexFlags), bla->header->vertexSize);

    Mesh* mesh = meshes->at(0);
    EXPECT_NE((Mesh*) 0, mesh);
    unsigned int vertexSize = bla->header->vertexSize;
    unsigned int indexCount = bla->header->indexCount;

    // verify that original and read data are equal
    for (int n = 0; n < indexCount; n++) {
        unsigned int origIndex = mesh->indices[n] * vertexSize;
        unsigned int loadIndex = bla->indices[n] * vertexSize;
        for (int k = 0; k < vertexSize; k++) {
            EXPECT_EQ(mesh->vertices[origIndex + k], bla->vertices[0][loadIndex + k]);
        }
    }
    unlink(TEST_STRUCTS_DATA_OPT_FILE);
}

TEST_F(ReaderTest, readUnoptimizedArrayOfStructs) {
    std::vector<Mesh*> *meshes = loadModel("suzanne.obj");
    writeFile(TEST_STRUCTS_DATA_FILE, meshes, false, false);

    std::ifstream in(TEST_STRUCTS_DATA_FILE, std::ios::binary);
    FileHeader *fileHeader = readFileHeader(in);
    ObjectHeader *objHeader = readObjectHeader(in);
    ASSERT_EQ(ARRAY_OF_STRUCTS, objHeader->type);

    Bla *bla = readArrayOfStructs(in, objHeader);

    ASSERT_NE((Bla*) 0, bla);
    ASSERT_NE((ObjectHeader*) 0, bla->header);
    ASSERT_EQ(objHeader, bla->header);
    ASSERT_EQ(calcVertexSize(objHeader->vertexFlags), objHeader->vertexSize);
    ASSERT_EQ(calcVertexSize(objHeader->vertexFlags), bla->header->vertexSize);

    Mesh* mesh = meshes->at(0);
    EXPECT_NE((Mesh*) 0, mesh);
    unsigned int vertexSize = bla->header->vertexSize;
    unsigned int indexCount = bla->header->indexCount;

    // verify that original and read data are equal
    for (int n = 0; n < indexCount; n++) {
        unsigned int origIndex = mesh->indices[n] * vertexSize;
        unsigned int loadIndex = bla->indices[n] * vertexSize;
        for (int k = 0; k < vertexSize; k++) {
            EXPECT_EQ(mesh->vertices[origIndex + k], bla->vertices[0][loadIndex + k]);
        }
    }
    unlink(TEST_STRUCTS_DATA_FILE);
}

TEST_F(ReaderTest, readOptimizedStructOfArrays) {
    std::vector<Mesh*> *meshes = loadModel("suzanne.obj");
    writeFile(TEST_ARRAYS_DATA_OPT_FILE, meshes, true, true);

    std::ifstream in(TEST_ARRAYS_DATA_OPT_FILE, std::ios::binary);
    FileHeader *fileHeader = readFileHeader(in);
    ObjectHeader *objHeader = readObjectHeader(in);
    ASSERT_EQ(STRUCT_OF_ARRAYS, objHeader->type);

    Bla *bla = readStructOfArrays(in, objHeader);

    ASSERT_NE((Bla*) 0, bla);
    ASSERT_NE((ObjectHeader*) 0, bla->header);
    ASSERT_EQ(objHeader, bla->header);
    ASSERT_EQ(calcVertexSize(objHeader->vertexFlags), objHeader->vertexSize);

    Mesh* mesh = meshes->at(0);
    EXPECT_NE((Mesh*) 0, mesh);

    unsigned int vertexSize = bla->header->vertexSize;

    // check that values read are equal to those written
    for (int i = 0; i < bla->header->indexCount; i++) {
        unsigned int origIndex = mesh->indices[i] * vertexSize;
        unsigned int loadIndex = bla->indices[i];
        unsigned int posIndex = loadIndex * kPositionSize;
        unsigned int normIndex = loadIndex * kNormalsSize;
        unsigned int texIndex = loadIndex * kTextureSize;

        for (int p = 0; p < kPositionSize; p++) {
            EXPECT_EQ(mesh->vertices[origIndex + p], bla->vertices[0][posIndex + p]);
        }
        for (int n = 0; n < kNormalsSize; n++) {
            EXPECT_EQ(mesh->vertices[origIndex + kPositionSize + n], bla->vertices[1][normIndex + n]);
        }
        for (int t = 0; t < kTextureSize; t++) {
            EXPECT_EQ(mesh->vertices[origIndex + kPositionSize + kNormalsSize + t],
                      bla->vertices[2][texIndex + t]);
        }
    }
    unlink(TEST_ARRAYS_DATA_OPT_FILE);
}

TEST_F(ReaderTest, readUnoptimizedStructOfArrays) {
    std::vector<Mesh*> *meshes = loadModel("suzanne.obj");
    writeFile(TEST_ARRAYS_DATA_FILE, meshes, false, true);

    std::ifstream in(TEST_ARRAYS_DATA_FILE, std::ios::binary);
    FileHeader *fileHeader = readFileHeader(in);
    ObjectHeader *objHeader = readObjectHeader(in);
    ASSERT_EQ(STRUCT_OF_ARRAYS, objHeader->type);

    Bla *bla = readStructOfArrays(in, objHeader);

    ASSERT_NE((Bla*) 0, bla);
    ASSERT_NE((ObjectHeader*) 0, bla->header);
    ASSERT_EQ(objHeader, bla->header);
    ASSERT_EQ(calcVertexSize(objHeader->vertexFlags), objHeader->vertexSize);

    Mesh* mesh = meshes->at(0);
    EXPECT_NE((Mesh*) 0, mesh);

    unsigned int vertexSize = bla->header->vertexSize;

    // check that values read are equal to those written
    for (int i = 0; i < bla->header->indexCount; i++) {
        unsigned int origIndex = mesh->indices[i] * vertexSize;
        unsigned int loadIndex = bla->indices[i];
        unsigned int posIndex = loadIndex * kPositionSize;
        unsigned int normIndex = loadIndex * kNormalsSize;
        unsigned int texIndex = loadIndex * kTextureSize;

        for (int p = 0; p < kPositionSize; p++) {
            EXPECT_EQ(mesh->vertices[origIndex + p], bla->vertices[0][posIndex + p]);
        }
        for (int n = 0; n < kNormalsSize; n++) {
            EXPECT_EQ(mesh->vertices[origIndex + kPositionSize + n], bla->vertices[1][normIndex + n]);
        }
        for (int t = 0; t < kTextureSize; t++) {
            EXPECT_EQ(mesh->vertices[origIndex + kPositionSize + kNormalsSize + t],
                      bla->vertices[2][texIndex + t]);
        }
    }
    unlink(TEST_ARRAYS_DATA_FILE);
}

