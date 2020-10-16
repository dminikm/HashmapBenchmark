cmake_minimum_required(VERSION 3.10)
project(HashmapBenchmark VERSION 0.0.1)

# Add executable
add_executable(HashmapBenchmark src/main.cpp)

# Enable C++17
target_compile_features(HashmapBenchmark PUBLIC cxx_std_17)
set_target_properties(HashmapBenchmark PROPERTIES CXX_EXTENSIONS OFF)

# Add Libcuckoo
target_include_directories(HashmapBenchmark PUBLIC "libs/libcuckoo")

# Add Intel TBB
set(TBB_BUILD_TESTS OFF CACHE BOOL "Build TBB's tests" FORCE)
add_subdirectory("libs/tbb")
target_include_directories(HashmapBenchmark PUBLIC "libs/tbb/include")
target_link_libraries(HashmapBenchmark PUBLIC tbb)

# Add Junction
add_subdirectory("libs/junction")
target_include_directories(HashmapBenchmark PUBLIC ${JUNCTION_ALL_INCLUDE_DIRS})
target_link_libraries(HashmapBenchmark PUBLIC ${JUNCTION_ALL_LIBRARIES})

# Add cxxopts
target_include_directories(HashmapBenchmark PUBLIC "libs/cxxopts/include")

# Install
set(CMAKE_INSTALL_PREFIX ${PROJECT_SOURCE_DIR})
install(TARGETS HashmapBenchmark DESTINATION bin)