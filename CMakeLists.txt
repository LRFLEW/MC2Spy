project(MC2Spy)

#Requires ASIO, Bind, Endian, Functional, System, and Utility
find_package(Boost REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

file(GLOB source_files "src/*.cpp" "src/*.hpp")
add_executable(mc2spy ${source_files})
target_link_libraries(mc2spy ${Boost_LIBRARIES})
