/* src/rcmreader.h
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

#ifndef RCM_READER_H
#define RCM_READER_H

#include "rcm.h"
#include <fstream>

struct Bla {
    ObjectHeader *header;
    float **vertices;
    unsigned short *indices;
};

FileHeader* readFileHeader(std::ifstream &in);
ObjectHeader* readObjectHeader(std::ifstream &in);
Bla* readArrayOfStructs(std::ifstream &in, const ObjectHeader *object);
Bla* readStructOfArrays(std::ifstream &in, const ObjectHeader *object);

#endif // RCM_READER_H
