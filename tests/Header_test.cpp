/* tests/Header_test.cpp
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
#include "rcm.h"

TEST(HeaderTest, modelDataFlags) {
    EXPECT_EQ(0x0001, HAS_POSITIONS);
    EXPECT_EQ(0x0002, HAS_NORMALS);

    // check UV flags
    EXPECT_EQ(0x0010, HAS_UV0);
    EXPECT_EQ(0x0020, HAS_UV1);
    EXPECT_EQ(0x0040, HAS_UV2);
    EXPECT_EQ(0x0080, HAS_UV3);

    // check color flags
    EXPECT_EQ(0x0100, HAS_COLOR0);
    EXPECT_EQ(0x0200, HAS_COLOR1);
    EXPECT_EQ(0x0400, HAS_COLOR2);
    EXPECT_EQ(0x0800, HAS_COLOR3);

    EXPECT_EQ(0x1000, HAS_TAN_AND_BITAN);
    EXPECT_EQ(0x2000, HAS_BONES);
    EXPECT_EQ(0x8000, USES_HALF_FLOAT);
}

TEST(HeaderTest, vertexDataSizes) {
    EXPECT_EQ(3, kPositionSize);
    EXPECT_EQ(3, kNormalsSize);
    EXPECT_EQ(3, kTanSize);
    EXPECT_EQ(3, kBitanSize);
    EXPECT_EQ(2, kTextureSize);
    EXPECT_EQ(4, kColorSize);
}

TEST(HeaderTest, hasXXX) {
    const unsigned short flags = 0xFFFF;
    EXPECT_TRUE(flags & HAS_POSITIONS);
    EXPECT_TRUE(hasPositions(flags));
    EXPECT_TRUE(hasPositions(HAS_POSITIONS));

    EXPECT_TRUE(flags & HAS_NORMALS);
    EXPECT_TRUE(hasNormals(flags));
    EXPECT_TRUE(hasNormals(HAS_NORMALS));

    // has textures
    EXPECT_TRUE(flags & HAS_UV0);
    EXPECT_TRUE(hasTexCoords0(flags));
    EXPECT_TRUE(hasTexCoords0(HAS_UV0));

    EXPECT_TRUE(flags & HAS_UV1);
    EXPECT_TRUE(hasTexCoords1(flags));
    EXPECT_TRUE(hasTexCoords1(HAS_UV1));

    EXPECT_TRUE(flags & HAS_UV2);
    EXPECT_TRUE(hasTexCoords2(flags));
    EXPECT_TRUE(hasTexCoords2(HAS_UV2));

    EXPECT_TRUE(flags & HAS_UV3);
    EXPECT_TRUE(hasTexCoords3(flags));
    EXPECT_TRUE(hasTexCoords3(HAS_UV3));

    // has colors
    EXPECT_TRUE(flags & HAS_COLOR0);
    EXPECT_TRUE(hasColor0(flags));
    EXPECT_TRUE(hasColor0(HAS_COLOR0));

    EXPECT_TRUE(flags & HAS_COLOR1);
    EXPECT_TRUE(hasColor1(flags));
    EXPECT_TRUE(hasColor1(HAS_COLOR1));

    EXPECT_TRUE(flags & HAS_COLOR2);
    EXPECT_TRUE(hasColor2(flags));
    EXPECT_TRUE(hasColor2(HAS_COLOR2));

    EXPECT_TRUE(flags & HAS_COLOR3);
    EXPECT_TRUE(hasColor3(flags));
    EXPECT_TRUE(hasColor3(HAS_COLOR3));


    EXPECT_TRUE(flags & HAS_TAN_AND_BITAN);
    EXPECT_TRUE(hasTanBitan(flags));
    EXPECT_TRUE(hasTanBitan(HAS_TAN_AND_BITAN));

    EXPECT_TRUE(flags & HAS_BONES);
    EXPECT_TRUE(hasBones(flags));
    EXPECT_TRUE(hasBones(HAS_BONES));
}

TEST(HeaderTest, usesHalfFloat) {
    const unsigned short flags = 0xFFFF;
    EXPECT_TRUE(flags & USES_HALF_FLOAT);
    EXPECT_TRUE(usesHalfFloat(flags));
    EXPECT_TRUE(usesHalfFloat(USES_HALF_FLOAT));
}

TEST(HeaderTest, setUsesHalfFloat) {
    unsigned short flags = 0;
    setUsesHalfFloat(flags);
    EXPECT_TRUE(usesHalfFloat(flags));
}

TEST(HeaderTest, setHasXXX) {
    unsigned short flags = 0;
    setHasPositions(flags);
    EXPECT_TRUE(hasPositions(flags));
    flags = 0;

    setHasNormals(flags);
    EXPECT_TRUE(hasNormals(flags));
    flags = 0;

    setHasTanBitan(flags);
    EXPECT_TRUE(flags);
    flags = 0;

    setHasBones(flags);
    EXPECT_TRUE(hasBones(flags));
}

TEST(HeaderTest, setHasTexCoords) {
    unsigned short flags = 0;
    const unsigned short kZeroTexCoords = 0;
    const unsigned short kOneTexCoord = kZeroTexCoords | HAS_UV0;
    const unsigned short kTwoTexCoords = kOneTexCoord | HAS_UV1;
    const unsigned short kThreeTexCoords = kTwoTexCoords | HAS_UV2;
    const unsigned short kFourTexCoords = kThreeTexCoords | HAS_UV3;

    setHasTexCoords(flags, 0);
    EXPECT_EQ(0, flags);

    setHasTexCoords(flags, 1);
    EXPECT_EQ(kOneTexCoord, flags);
    flags = 0;
    
    setHasTexCoords(flags, 2);
    EXPECT_EQ(kTwoTexCoords, flags);
    flags = 0;
    
    setHasTexCoords(flags, 3);
    EXPECT_EQ(kThreeTexCoords, flags);
    flags = 0;

    setHasTexCoords(flags, 4);
    EXPECT_EQ(kFourTexCoords, flags);
    flags = 0;

    setHasTexCoords(flags, 5);
    EXPECT_EQ(kFourTexCoords, flags);
    flags = 0;
}

TEST(HeaderTest, setHasColors) {
    unsigned short flags = 0;
    const unsigned short kZeroColors = 0;
    const unsigned short kOneColor = kZeroColors | HAS_COLOR0;
    const unsigned short kTwoColors = kOneColor | HAS_COLOR1;
    const unsigned short kThreeColors = kTwoColors | HAS_COLOR2;
    const unsigned short kFourColors = kThreeColors | HAS_COLOR3;

    setHasColors(flags, 0);
    EXPECT_EQ(0, flags);

    setHasColors(flags, 1);
    EXPECT_EQ(kOneColor, flags);
    flags = 0;

    setHasColors(flags, 2);
    EXPECT_EQ(kTwoColors, flags);
    flags = 0;

    setHasColors(flags, 3);
    EXPECT_EQ(kThreeColors, flags);
    flags = 0;

    setHasColors(flags, 4);
    EXPECT_EQ(kFourColors, flags);
    flags = 0;

    setHasColors(flags, 5);
    EXPECT_EQ(kFourColors, flags);
    flags = 0;
}

TEST(HeaderTest, offsetsAllFlagsSet) {
    const unsigned short flags = 0xFFFF;
    EXPECT_EQ(0, positionOffset());
    EXPECT_EQ(3, normalsOffset());
    EXPECT_EQ(6, texCoords0Offset(flags));
    EXPECT_EQ(8, texCoords1Offset(flags));
    EXPECT_EQ(10, texCoords2Offset(flags));
    EXPECT_EQ(12, texCoords3Offset(flags));
    EXPECT_EQ(14, color0Offset(flags));
    EXPECT_EQ(18, color1Offset(flags));
    EXPECT_EQ(22, color2Offset(flags));
    EXPECT_EQ(26, color3Offset(flags));
    EXPECT_EQ(30, tanOffset(flags));
    EXPECT_EQ(33, bitanOffset(flags));
    // bone offset missing implementation
}

TEST(HeaderTest, offsetTypicalFlagsSet) {
    unsigned short flags = 0;
    setHasPositions(flags);
    setHasNormals(flags);
    setHasTexCoords(flags, 1);
    setHasTanBitan(flags);

    EXPECT_EQ(0, positionOffset());
    EXPECT_EQ(3, normalsOffset());
    EXPECT_EQ(6, texCoords0Offset(flags));
    EXPECT_EQ(8, tanOffset(flags));
    EXPECT_EQ(11, bitanOffset(flags));
}

TEST(HeaderTest, offsetNoNormalsSet) {
    unsigned short flags = 0;
    setHasPositions(flags);
    setHasTexCoords(flags, 1);
    setHasColors(flags, 2);

    EXPECT_EQ(0, positionOffset());
    EXPECT_EQ(3, texCoords0Offset(flags));
    EXPECT_EQ(5, color0Offset(flags));
    EXPECT_EQ(9, color1Offset(flags));
}

