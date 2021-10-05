cmake_minimum_required(VERSION 2.8)
project(onsem)


set(CMAKE_CXX_FLAGS "-std=gnu++11" CACHE STRING "" FORCE)

## Command line for compilation
# cmake ../ && make -j4


include(onsem-config.cmake)
add_subdirectory(common)

add_subdirectory(typeallocatorandserializer)
add_subdirectory(lingdbeditor)

add_subdirectory(chatbotplanner)
add_subdirectory(texttosemantic)
add_subdirectory(semantictotext)
add_subdirectory(streamdatabaseaccessor)
add_subdirectory(semanticdebugger)
add_subdirectory(tests)
