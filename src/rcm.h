/* src/rcm.h
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

#ifndef RCM_H
#define RCM_H

#include <stdlib.h>
#include <stdint.h>

/* Format of the model file.
file header meta data:
  magic number,           2 byte [0][1] 0xde 0xad
  version (major, minor), 2 byte [2][3] 0x0 0x1
  object count,           1 byte -> numberOfMeshes + number of textures
  unused                  1 byte
per object meta data:
  type:                   1 byte
      - model (struct of arrays) 0x1  STRUCT_OF_ARRAYS
      - model (array of structs) 0x2  ARRAY_OF_STRUCTS
      - textures?
      - normal maps?
per model meta data:
  vertex size),   1 byte
  flags:          2 byte
      - position  0x0001              HAS_POSITIONS
      - normals   0x0002              HAS_NORMALS
      - uv 0      0x0010              HAS_UV0
      - uv 1      0x0020              HAS_UV1
      - uv 2      0x0040              HAS_UV2
      - uv 3      0x0080              HAS_UV3
      - color 0   0x0100              HAS_COLOR0
      - color 1   0x0200              HAS_COLOR1
      - color 2   0x0400              HAS_COLOR2
      - color 3   0x0800              HAS_COLOR3
      - tan+bitan 0x1000              HAS_TAN_AND_BITAN
      - bones     0x2000              HAS_BONES
      - halffloat 0x8000              USES_HALF_FLOAT
  vertex count, 4 byte
  index count,  4 byte
  bone count,   4 byte
per model data:
  vertex count * (positions, normals, uvs...)
  index count * (unsigned short)
  bone count * (whatever a bone will be...)
*/

const unsigned char kMagicNumber[] = {0xDE, 0xAD};
const unsigned char kFileFormatVersionMajor = 0x0;
const unsigned char kFileFormatVersionMinor = 0x1;

const unsigned int kMaxNumTexCoords = 4;
const unsigned int kMaxNumColors = 4;

/* 1 byte */
enum ObjectType {
      STRUCT_OF_ARRAYS = 0x1,
      ARRAY_OF_STRUCTS = 0x2,
};

struct FileHeader {
    uint8_t magicNumber[2];
    uint8_t version[2];
    uint8_t objectCount;
    uint8_t unused;
};

struct ObjectHeader {
    uint8_t type;
    uint8_t vertexSize;
    uint16_t vertexFlags;
    uint32_t vertexCount;
    uint32_t indexCount;
    uint32_t boneCount;
};


/* 2 bytes */
enum ModelDataFlags {
    HAS_POSITIONS = 0x0001,
    HAS_NORMALS = 0x0002,
    HAS_UV0 = 0x0010,
    HAS_UV1 = 0x0020,
    HAS_UV2 = 0x0040,
    HAS_UV3 = 0x0080,
    HAS_COLOR0 = 0x0100,
    HAS_COLOR1 = 0x0200,
    HAS_COLOR2 = 0x0400,
    HAS_COLOR3 = 0x0800,
    HAS_TAN_AND_BITAN = 0x1000,
    HAS_BONES = 0x2000,
    USES_HALF_FLOAT = 0x8000,
};

const int kPositionSize = 3;
const int kNormalsSize = 3;
const int kTanSize = 3;
const int kBitanSize = 3;
// TODO: check whether we need 3 floats for volume textures
const int kTextureSize = 2;
const int kColorSize = 4;

inline bool hasPositions(uint16_t vertexFlags) {
    return (vertexFlags & HAS_POSITIONS);
}

inline void setHasPositions(uint16_t &vertexFlags) {
    vertexFlags |= HAS_POSITIONS;
}

inline int positionOffset() {
    return 0;
}

inline bool hasNormals(uint16_t vertexFlags) {
    return (vertexFlags & HAS_NORMALS);
}

inline void setHasNormals(uint16_t &vertexFlags) {
    vertexFlags |= HAS_NORMALS;
}

inline int normalsOffset() {
    return kPositionSize;;
}

// set the number of flags required, if there are 0 tex coords, no flag is set
// if there is one, the first one is set and so on...
inline void setHasTexCoords(uint16_t &vertexFlags, unsigned int number) {
    int texCoordCount = (number > kMaxNumTexCoords) ? kMaxNumTexCoords : number;
    if (texCoordCount > 0) {
        for (int i = 0; i < texCoordCount; i++) {
            vertexFlags |= (HAS_UV0 << i);
        }
    }
}

inline bool hasTexCoords0(uint16_t vertexFlags) {
    return (vertexFlags & HAS_UV0);
}

inline int texCoords0Offset(uint16_t vertexFlags) {
    //int offset = normalsOffset(vertexFlags);;
    return hasNormals(vertexFlags) ? normalsOffset() + kNormalsSize : normalsOffset();
}

inline bool hasTexCoords1(uint16_t vertexFlags) {
    return (vertexFlags & HAS_UV1);
}

inline int texCoords1Offset(uint16_t vertexFlags) {
    int offset = texCoords0Offset(vertexFlags);;
    return hasTexCoords0(vertexFlags) ? offset + kTextureSize : offset; 
}

inline bool hasTexCoords2(uint16_t vertexFlags) {
    return (vertexFlags & HAS_UV2);
}

inline int texCoords2Offset(uint16_t vertexFlags) {
    int offset = texCoords1Offset(vertexFlags);;
    return hasTexCoords1(vertexFlags) ? offset + kTextureSize : offset; 
}

inline bool hasTexCoords3(uint16_t vertexFlags) {
    return (vertexFlags & HAS_UV3);
}

inline int texCoords3Offset(uint16_t vertexFlags) {
    int offset = texCoords2Offset(vertexFlags);;
    return hasTexCoords2(vertexFlags) ? offset + kTextureSize : offset; 
}

// set the number of flags required, if there are 0 colors, no flag is set
// if there is one, the first one is set and so on...
inline void setHasColors(uint16_t &vertexFlags, unsigned int number) {
    int colorCount = (number > kMaxNumColors) ? kMaxNumColors : number;
    if (colorCount > 0) {
        for (int i = 0; i < colorCount; i++) {
            vertexFlags |= (HAS_COLOR0 << i);
        }
    }
}

inline bool hasColor0(uint16_t vertexFlags) {
    return (vertexFlags & HAS_COLOR0);
}

inline int color0Offset(uint16_t vertexFlags) {
    int offset = texCoords3Offset(vertexFlags);
    return hasTexCoords3(vertexFlags) ? offset + kTextureSize : offset; 
}

inline bool hasColor1(uint16_t vertexFlags) {
    return (vertexFlags & HAS_COLOR1);
}

inline int color1Offset(uint16_t vertexFlags) {
    int offset = color0Offset(vertexFlags);
    return hasColor0(vertexFlags) ? offset + kColorSize : offset;
}

inline bool hasColor2(uint16_t vertexFlags) {
    return (vertexFlags & HAS_COLOR2);
}

inline int color2Offset(uint16_t vertexFlags) {
    int offset = color1Offset(vertexFlags);
    return hasColor1(vertexFlags) ? offset + kColorSize : offset;
}

inline bool hasColor3(uint16_t vertexFlags) {
    return (vertexFlags & HAS_COLOR3);
}

inline int color3Offset(uint16_t vertexFlags) {
    int offset = color2Offset(vertexFlags);
    return hasColor2(vertexFlags) ? offset + kColorSize : offset;
}

inline bool hasTanBitan(uint16_t vertexFlags) {
    return (vertexFlags & HAS_TAN_AND_BITAN);
}

inline void setHasTanBitan(uint16_t &vertexFlags) {
    vertexFlags |= HAS_TAN_AND_BITAN;
}

inline int tanOffset(uint16_t vertexFlags) {
    int offset = color3Offset(vertexFlags);
    return hasColor3(vertexFlags) ? offset + kColorSize : offset;
}

inline int bitanOffset(uint16_t vertexFlags) {
    int offset = tanOffset(vertexFlags);
    return hasTanBitan(vertexFlags) ? offset + kTanSize : offset;
}

inline bool hasBones(uint16_t vertexFlags) {
    return (vertexFlags & HAS_BONES);
}

inline void setHasBones(uint16_t &vertexFlags) {
    vertexFlags |= HAS_BONES;
}

inline bool usesHalfFloat(uint16_t vertexFlags) {
    return (vertexFlags & USES_HALF_FLOAT);
}

inline void setUsesHalfFloat(uint16_t &vertexFlags) {
    vertexFlags |= USES_HALF_FLOAT;
}

inline unsigned int calcVertexSize(unsigned short vertexFlags) {
    unsigned short vertexSize = 0;
    if (hasPositions(vertexFlags)) {
        vertexSize += kPositionSize;
    }
    if (hasNormals(vertexFlags)) {
        vertexSize += kNormalsSize;
    }
    if (hasTexCoords0(vertexFlags)) {
        vertexSize += kTextureSize;
    }
    if (hasTexCoords1(vertexFlags)) {
        vertexSize += kTextureSize;
    }
    if (hasTexCoords2(vertexFlags)) {
        vertexSize += kTextureSize;
    }
    if (hasTexCoords3(vertexFlags)) {
        vertexSize += kTextureSize;
    }
    if (hasColor0(vertexFlags)) {
        vertexSize += kColorSize;
    }
    if (hasColor1(vertexFlags)) {
        vertexSize += kColorSize;
    }
    if (hasColor2(vertexFlags)) {
        vertexSize += kColorSize;
    }
    if (hasColor3(vertexFlags)) {
        vertexSize += kColorSize;
    }
    if (hasTanBitan(vertexFlags)) {
        vertexSize += kTanSize + kBitanSize;
    }
    return vertexSize;
}

#endif // RCM_H

