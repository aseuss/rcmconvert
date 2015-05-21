set (WriterSources rcmwriter.cpp)
set (ReaderSources rcmreader.cpp)

#include_directories (/usr/local/include)

add_library (writerobjects OBJECT ${WriterSources})
add_library (readerobjects OBJECT ${ReaderSources})
add_library (rcmwriter STATIC $<TARGET_OBJECTS:writerobjects>)
add_library (rcmreader STATIC $<TARGET_OBJECTS:readerobjects>)

add_executable (rcmconvert converter.cpp command_parser.cpp)
target_link_libraries (rcmconvert rcmwriter rcmreader assimp)

