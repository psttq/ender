cmake_minimum_required(VERSION 3.22)

project(target)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp)
file(GLOB GLAD_FILES ${PROJECT_SOURCE_DIR}/3rd/glad/src/glad.c)

add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/spdlog)

add_executable(target ${SRC_FILES} ${GLAD_FILES}
        src/Scene.cpp
        include/FirstPersonCamera.hpp
        src/FirstPersonCamera.cpp
        src/Camera.cpp
        include/Camera.hpp)

target_include_directories(target PUBLIC ${PROJECT_SOURCE_DIR}/3rd/glad/include ${PROJECT_SOURCE_DIR}/3rd/stb ${PROJECT_SOURCE_DIR}/include)

target_link_libraries(target PUBLIC glfw glm spdlog)


