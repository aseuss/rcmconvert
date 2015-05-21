cmake_minimum_required (VERSION 2.8)
project (RCMConvert)

#find_package(OpenGL REQUIRED)

set (ModelExporter_VERSION_MAJOR 0)
set (ModelExporter_VERSION_MINOR 1)

include_directories (${gtest_SOURCE_DIR}/include)
include_directories (/usr/local/include)
link_directories (/usr/local/lib)

#add_subdirectory (lib/gtest-1.7.0/)
#add_subdirectory (tests/)

add_subdirectory (src/)
add_subdirectory (external/gtest-1.7.0/)
add_subdirectory (tests/)

#set (WriterSources rcmwriter.cpp)
#set (ReaderSources rcmreader.cpp)

#add_library (writerobjects OBJECT ${WriterSources})
#add_library (readerobjects OBJECT ${ReaderSources})
#add_library (rcmwriter STATIC $<TARGET_OBJECTS:writerobjects>)
#add_library (rcmreader STATIC $<TARGET_OBJECTS:readerobjects>)

#add_executable (rcmconvert converter.cpp hfloat.cpp command_parser.cpp)
#target_link_libraries (rcmconvert rcmwriter rcmreader assimp)

#add_executable (renderer demo/renderer.cpp demo/shader.c demo/dds.c demo/png.c demo/texture_loader.c)
#target_link_libraries (renderer SDL2-2.0 ${OPENGL_LIBRARIES} rcmreader png)
