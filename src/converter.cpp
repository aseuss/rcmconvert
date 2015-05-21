/* src/converter.cpp
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

#include <iostream>
#include <vector>
#include <iomanip>

#include "rcmreader.h"
#include "rcmwriter.h"

#include "command_parser.h"

static const char* kArraysOption = "-a";
static const char* kHalfFloatOption = "-f";
static const char* kHelpOption = "-h";
static const char* kDisplayInfoOption = "-i";
static const char* kNoOptimizationOption = "-n";
static const char* kOutputFileOption = "-o";
static const char* kStructsOption = "-s";
static const char* kVerboseOption = "-v";

static const char* kDefaultFileExtension = ".rcm";

static const int kFormatWidth = 17;
static const int kInfoFormatWidth = 13;
static const int kInfoDataFormatWidth = 12;

// TODO: enhance such that input files with multiple objects can be supported
void readAndDisplayInfo(const std::string &fileName) {
    std::ifstream in(fileName.c_str(), std::ios::binary);
    if (in) {
        FileHeader* fileHeader = readFileHeader(in);
        if (!fileHeader) {
            std::cerr << "could not read file header" << std::endl;
            return;
        }

        ObjectHeader* objectHeader = readObjectHeader(in);
        if (!objectHeader) {
            std::cerr << "could not read object header" << std::endl;
            return;
        }

        const int majorVersion = fileHeader->version[0];
        const int minorVersion = fileHeader->version[1];
        const int objectCount = fileHeader->objectCount;

        std::cout << std::endl << "file: " << fileName << ":" << std::endl;
        std::cout << std::left;
        std::cout << "  " << std::setw(kInfoFormatWidth);
        std::cout << "file version" << ": " << majorVersion << "." << minorVersion << std::endl;
        std::cout << "  " << std::setw(kInfoFormatWidth);
        std::cout << "object count" << ": " << objectCount << std::endl << std::endl;

        delete fileHeader;

        std::string type = "array of structs";
        if (objectHeader->type == STRUCT_OF_ARRAYS) {
            type = "struct of arrays";
        }
        const int vertexSize = objectHeader->vertexSize;
        std::cout << "  " << std::setw(kInfoFormatWidth);
        std::cout << "type" << ": " << type << std::endl;
        std::cout << "  " << std::setw(kInfoFormatWidth);
        std::cout << "vertex size" << ": " << vertexSize << std::endl;
        std::cout << "  " << std::setw(kInfoFormatWidth);
        std::cout << "vertex count" << ": " << objectHeader->vertexCount << std::endl;
        std::cout << "  " << std::setw(kInfoFormatWidth);
        std::cout << "index count" << ": " << objectHeader->indexCount << std::endl << std::endl;


        uint16_t vertexFlags = objectHeader->vertexFlags;

        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "positions" << ": " << (hasPositions(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "normals" << ": " << (hasNormals(vertexFlags) ? "yes" : "no") << std::endl;

        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "uvs0" << ": " << (hasTexCoords0(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "uvs1" << ": " << (hasTexCoords1(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "uvs2" << ": " << (hasTexCoords2(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "uvs3" << ": " << (hasTexCoords3(vertexFlags) ? "yes" : "no") << std::endl;

        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "color0" << ": " << (hasColor0(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "color1" << ": " << (hasColor1(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "color2" << ": " << (hasColor2(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "color3" << ": " << (hasColor3(vertexFlags) ? "yes" : "no") << std::endl;

        std::cout << "    " << std::setw(kInfoDataFormatWidth);
        std::cout << "tan & bitan" << ": " << (hasTanBitan(vertexFlags) ? "yes" : "no") << std::endl;
        std::cout << std::right << std::endl;

        delete objectHeader;

        in.close();
    }

}

int main(int argc, char **argv) {

    CommandParser parser(argc, argv);

    parser.addBoolOption(kArraysOption, "export as struct of arrays. [-a | -s]");
    // parser.addBoolOption(kHalfFloatOption, "use half float (16-bit)");
    parser.addHelpOption(kHelpOption, "display this help screen");
    parser.addBoolOption(kDisplayInfoOption, "show meta data of input file");
    parser.addBoolOption(kNoOptimizationOption, "do not optimize model");
    parser.addValueOption(kOutputFileOption, "FILE", "export model to FILE");
    parser.addBoolOption(kStructsOption, "export as array of structs (default). [-s | -a]");
    parser.addBoolOption(kVerboseOption, "enable verbose output");

    //parser.setUsageString("hey, this is my awesome usgae string");
    parser.appendToPreDescText("This tool can be used to convert standard 3D models from");
    parser.appendToPreDescText("different commercial and open source tools to a flat binary");
    parser.appendToPreDescText("format optimized for size. The tool theoretically supports");
    parser.appendToPreDescText("all formats supported by the AssImp library.");

    if (parser.parse()) {
        return 1;
    }

    const bool showHelp = parser.boolOption(kHelpOption);
    if (showHelp) {
        parser.showHelpDialog();
        return 0;
    }

    bool exportStructOfArrays = false;
    if (parser.boolOption(kArraysOption) && !parser.boolOption(kStructsOption)) {
        exportStructOfArrays = true;
    }

    const bool doOptimize = !parser.boolOption(kNoOptimizationOption);
    const bool useHalfFloat = parser.boolOption(kHalfFloatOption);

    std::list<std::string> trailingArgs = parser.trailingArgs();
    if (trailingArgs.empty()) {
        std::stringstream error;
        error << "no input file given";
        parser.showError(error);
        return 0;
    }

    const std::string inFile = trailingArgs.front();

    if (parser.boolOption(kDisplayInfoOption)) {
        readAndDisplayInfo(inFile);
        return 0;
    }

    size_t dotIndex = inFile.find(".");
    const std::string defaultOutFile = inFile.substr(0, dotIndex).append(kDefaultFileExtension);

    const std::string outFile = parser.valueOption(kOutputFileOption, defaultOutFile);

    if (parser.boolOption("-v")) {
        std::cout << std::endl << "exporting with following options:" << std::left << std::endl;
        std::cout << "  " << std::setw(kFormatWidth);
        std::cout << "export from" << ": " << inFile << std::endl;
        std::cout << "  " << std::setw(kFormatWidth);
        std::cout << "export to" << ": " << outFile << std::endl;
        std::cout << "  " << std::setw(kFormatWidth);
        std::cout << "optimization" << ": " << (doOptimize ? "yes" : "no") << std::endl;
        std::cout << "  " << std::setw(kFormatWidth);
        std::cout << "use half-float" << ": " << (useHalfFloat ? "yes" : "no") << std::endl;
        std::cout << "  " << std::setw(kFormatWidth);
        std::cout << "struct of arrays" << ": " << (exportStructOfArrays ? "yes" : "no") << std::endl;
        std::cout << "  " << std::setw(kFormatWidth);
        std::cout << "array of structs" << ": " << (!exportStructOfArrays ? "yes" : "no") << std::endl;
        std::cout << std::endl << std::right;
    }

    // now after loads of boiler plate, do the im- and export
    std::vector<Mesh*> *meshes = loadModel(inFile.c_str());
    if (!meshes) {
        std::cerr << "model could not be loaded" << std::endl;
        return 1;
    }
    writeFile(outFile.c_str(), meshes, doOptimize, exportStructOfArrays);

    // clear all meshes
    std::vector<Mesh*>::iterator it = meshes->begin();
    while (it != meshes->end()) {
        delete *it;
        ++it;
    }
    meshes->clear();
    delete meshes;

    return 0;
}

