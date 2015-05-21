set (WriterTestSources Writer_test.cpp Header_test.cpp)
set (ReaderTestSources Reader_test.cpp)

include_directories (../src/)

add_executable (Writer_test ${WriterTestSources})
target_link_libraries (Writer_test gtest gtest_main assimp rcmwriter)

add_executable (Reader_test ${ReaderTestSources})
target_link_libraries (Reader_test gtest gtest_main rcmreader rcmwriter assimp)
