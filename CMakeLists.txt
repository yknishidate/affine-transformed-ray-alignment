cmake_minimum_required(VERSION 3.10)
project(affine_transformed_ray_alignment LANGUAGES CXX)
set(CMAKE_CXX_STANDARD 20)

add_subdirectory(external/glfw)

file(GLOB sources ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
file(GLOB headers ${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp)
add_executable(${PROJECT_NAME} 
    ${sources}
    ${headers}
)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC 
    ${EMBREE_PATH}/lib/embree3.lib
    glfw
)

target_include_directories(
    ${PROJECT_NAME} PUBLIC
    ${PROJECT_SOURCE_DIR}/external/tinyobjloader
    ${PROJECT_SOURCE_DIR}/external/glm
    ${PROJECT_SOURCE_DIR}/external/glfw/include
    ${EMBREE_PATH}/include
)

file(
    COPY 
    ${EMBREE_PATH}/bin/embree3.dll
    ${EMBREE_PATH}/bin/tbb12.dll
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)
