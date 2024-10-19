cmake_minimum_required(VERSION 3.22)

project(target)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/3rd/CGAL-6.0/cmake/modules)

set(CMAKE_CXX_STANDARD 23)

file(GLOB SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cpp ${PROJECT_SOURCE_DIR}/src/Renderer/*.cpp ${PROJECT_SOURCE_DIR}/src/Geometry/*.cpp ${PROJECT_SOURCE_DIR}/src/Geometry/Topology/*.cpp)
file(GLOB GLAD_FILES ${PROJECT_SOURCE_DIR}/3rd/glad/src/glad.c)
file(GLOB IMGUI_FILES ${PROJECT_SOURCE_DIR}/3rd/imgui/*.cpp ${PROJECT_SOURCE_DIR}/3rd/imgui/*.h ${PROJECT_SOURCE_DIR}/3rd/imgui/misc/cpp/imgui_stdlib.* ${PROJECT_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_glfw.cpp ${PROJECT_SOURCE_DIR}/3rd/imgui/backends/imgui_impl_opengl3.cpp)
file(GLOB IMGUIZMO_FILES ${PROJECT_SOURCE_DIR}/3rd/ImGuizmo/*.cpp)
file(GLOB ImGuiFileDialog_FILES ${PROJECT_SOURCE_DIR}/3rd/ImGuiFileDialog/ImGuiFileDialog.cpp})


add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/glfw)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/glm)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/spdlog)
add_subdirectory(${PROJECT_SOURCE_DIR}/3rd/tomlplusplus)

find_package(Boost REQUIRED COMPONENTS system filesystem)  # Add other components as needed
find_package(GMP REQUIRED)
find_package(MPFR REQUIRED)

include_directories(${Boost_INCLUDE_DIRS})

add_executable(target ${SRC_FILES} ${GLAD_FILES} ${IMGUI_FILES} ${IMGUIZMO_FILES} 3rd/ImGuiFileDialog/ImGuiFileDialog.cpp include/Renderer/OrthographicCamera.hpp
        src/Renderer/OrthographicCamera.cpp
        src/Geometry/Spline1.cpp
        include/Geometry/Spline1.hpp
        src/MyApplication.cpp
        3rd/font/IconsFontAwesome5.h
        src/Geometry/Sketch.cpp
        include/Geometry/Sketch.hpp
        include/Geometry/PivotPlane.hpp
        src/Geometry/PivotPlane.cpp)

      target_include_directories(target PUBLIC ${PROJECT_SOURCE_DIR}/3rd/ImGuiFileDialog  ${PROJECT_SOURCE_DIR}/include/Renderer ${PROJECT_SOURCE_DIR}/3rd/font ${PROJECT_SOURCE_DIR}/3rd/glad/include ${PROJECT_SOURCE_DIR}/3rd/stb ${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/3rd ${PROJECT_SOURCE_DIR}/3rd/imgui ${PROJECT_SOURCE_DIR}/3rd/ImGuizmo ${PROJECT_SOURCE_DIR}/include/Geometry ${PROJECT_SOURCE_DIR}/3rd/tomlplusplus/include/toml++ ${PROJECT_SOURCE_DIR}/3rd/CGAL-6.0/include/)

target_link_libraries(target PUBLIC glfw glm spdlog ${Boost_LIBRARIES} gmp mpfr)
