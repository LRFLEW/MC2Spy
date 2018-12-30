cmake_minimum_required(VERSION 3.2)
project(MC2Spy)

set(CMAKE_CXX_STANDARD 17)
# Requires ASIO, Endian, Functional, and System
find_package(Boost REQUIRED COMPONENTS system)

if(MSVC)
  add_definitions(-D_WIN32_WINNT=0x0600)
  add_definitions(-D_SILENCE_CXX17_ALLOCATOR_VOID_DEPRECATION_WARNING) # fix incorrect MSVC warning
endif(MSVC)

file(GLOB source_files "src/*.cpp" "src/*.hpp")
add_executable(mc2spy ${source_files})
target_include_directories(mc2spy PRIVATE ${Boost_INCLUDE_DIRS})
target_link_libraries(mc2spy ${Boost_LIBRARIES})
