cmake_minimum_required(VERSION 3.10)
project(CoreTest)

set(CMAKE_CXX_STANDARD 17)

add_executable (linpackTest main.cpp process_library.cpp cpu_info.cpp)
configure_file(libiomp5md.dll library/libiomp5md.dll COPYONLY)
configure_file(linpack_xeon64.exe library/linpack_xeon64.exe COPYONLY)
