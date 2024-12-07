cmake_minimum_required(VERSION 3.27)
set (PROJECT_NAME FATChecker)
project(${PROJECT_NAME} )

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME} main.cpp
        includes/AnalyzersFAT16.hpp
        includes/FATstructs.hpp
        sources/AnalyzersFAT16.cpp
        includes/AnalyzersFAT32.hpp
        sources/AnalyzersFAT32.cpp
        sources/AnalyzersFAT12.cpp
        includes/AnalyzersFAT12.hpp
        includes/FATTableStructs.hpp
        sources/FixFAT16.cpp
        includes/FixFAT16.hpp
        sources/readFunctions.cpp
        includes/readFunctions.hpp
        includes/file.hpp
        sources/file.cpp
        FAT_fixer/FATFileSystem.cpp
)

target_include_directories(${PROJECT_NAME} PRIVATE includes)
