cmake_minimum_required(VERSION 3.22)

project(target)

set(CMAKE_CXX_STANDARD 20)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/Renderer/*.cpp ${PROJECT_SOURCE_DIR}/src/Geometry/*.cpp)
file(GLOB GLAD_FILES ${PROJECT_SOURCE_DIR}/3rd/glad/src/glad.c)
file(GLOB IMGUI_FILES ${PROJECT_SOURCE_DIR}/3rd/imgui/*.cpp ${PROJECT_SOURCE_DIR}/3rd/imgui/*.h ${PROJECT_SOURCE_DIR}/3rd/imgui/misc/cpp/imgui_stdlib.* ${PROJECT_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_glfw.cpp ${PROJECT_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_opengl3.cpp)
file(GLOB IMGUIZMO_FILES ${PROJECT_SOURCE_DIR}/3rd/ImGuizmo/*.cpp)


add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/spdlog)


add_executable(target ${SRC_FILES} ${GLAD_FILES} ${IMGUI_FILES} ${IMGUIZMO_FILES}
        include/Renderer/OrthographicCamera.hpp
        src/Renderer/OrthographicCamera.cpp
        src/Geometry/Spline1.cpp
        include/Geometry/Spline1.hpp)

target_include_directories(target PUBLIC ${PROJECT_SOURCE_DIR}/include/Renderer ${PROJECT_SOURCE_DIR}/3rd/glad/include ${PROJECT_SOURCE_DIR}/3rd/stb ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/3rd ${PROJECT_SOURCE_DIR}/3rd/imgui ${PROJECT_SOURCE_DIR}/3rd/ImGuizmo ${PROJECT_SOURCE_DIR}/include/Geometry)

target_link_libraries(target PUBLIC glfw glm spdlog)


