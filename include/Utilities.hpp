#pragma once

#include "BufferLayout.hpp"
#include <Ender.hpp>
#include <ender_types.hpp>
#include <memory>

namespace ENDER {
    namespace Utils {
        typedef std::function<glm::vec3(float, float)> ParametricSurfFunc;

        inline unsigned int *generateParametricSurfaceGrid(int rows, int cols) {
            auto *indices = new unsigned int[(rows - 1) * (cols - 1) * 2 * 3];
            for (int row = 0; row < rows - 1; row++) {
                for (int col = 0; col < cols - 1; col++) {
                    unsigned int f = col + row * cols;
                    unsigned int s1 = col + (row + 1) * cols;
                    unsigned int s2 = s1 + 1;
                    unsigned int t = f + 1;
                    indices[(col + row * (cols - 1)) * 6] = f;
                    indices[(col + row * (cols - 1)) * 6 + 1] = s1;
                    indices[(col + row * (cols - 1)) * 6 + 2] = s2;
                    indices[(col + row * (cols - 1)) * 6 + 3] = f;
                    indices[(col + row * (cols - 1)) * 6 + 4] = s2;
                    indices[(col + row * (cols - 1)) * 6 + 5] = t;

                    // printf("%d %d %d\n", f, s1, s2);
                    // printf("%d %d %d\n", f, s2, t);
                }
            }
            return indices;
        }

        inline sptr<Object> createParametricSurface(ParametricSurfFunc func,float u_min, float v_min, float u_max,
                                                    float v_max, uint rows, uint cols) {
            auto indices = generateParametricSurfaceGrid(rows, cols);


            float *vertices = new float[rows * cols * 3];

            float h_u = (u_max - u_min) / (cols - 1);
            float h_v = (v_max - v_min) / (rows - 1);

            for (auto i = 0; i < rows; i++) {
                for (auto j = 0; j < cols; j++) {
                    float u = u_min + h_u * j;
                    float v = v_min + h_v * i;
                    auto vertice = func(u, v);
                    vertices[(j + i * cols) * 3] = vertice.x; // x
                    vertices[(j + i * cols) * 3 + 1] = vertice.y; // x
                    vertices[(j + i * cols) * 3 + 2] = vertice.z; // x

                    // printf("%0.2f %0.2f %0.2f\t", vertice.x, vertice.y, vertice.z);
                }
                // printf("\n");
            }

            // for(auto i = 0; i < (rows - 1) * (cols - 1) * 2*3; i+=3) {
            //   printf("%0.2f %0.2f %0.2f\t", vertices[indices[i]*3],vertices[indices[i]*3+1] ,vertices[indices[i]*3+2]);
            //   printf("%0.2f %0.2f %0.2f\t", vertices[indices[i+1]*3],vertices[indices[i+1]*3+1] ,vertices[indices[i+1]*3+2]);
            //   printf("%0.2f %0.2f %0.2f\t", vertices[indices[i+2]*3],vertices[indices[i+2]*3+1] ,vertices[indices[i+2]*3+2]);
            // printf("\n==================================\n");
            // }

            auto layout = uptr<BufferLayout>(new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}}));
            auto vbo = std::make_unique<VertexBuffer>(std::move(layout));
            vbo->setData(vertices, sizeof(float) * rows * cols * 3);

            auto ibo = std::make_unique<IndexBuffer>(indices, (rows - 1) * (cols - 1) * 2 * 3);

            auto vao = std::make_shared<VertexArray>();
            vao->addVBO(std::move(vbo));
            vao->setIndexBuffer(std::move(ibo));

            auto object = ENDER::Object::create("ParametricSurface", vao);

            return object;
        }

        void applyImguiTheme() {
            ImGuiIO& io = ImGui::GetIO();
            io.Fonts->AddFontFromFileTTF("../resources/font.ttf", 14);

            ImGuiStyle &style = ImGui::GetStyle();
            ImVec4 *colors = style.Colors;
            colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
            colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
            colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
            colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
            colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.25f, 0.25f, 0.25f, 0.54f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
            colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
            colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
            colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
            colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
            colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
            colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
            colors[ImGuiCol_Button] = ImVec4(0.30f, 0.30f, 0.30f, 0.54f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
            colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
            colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
            colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
            colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
            colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
            colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
            colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
            colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
            colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
            colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
            colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
            colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
            colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
            colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
            colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

            style.WindowPadding = ImVec2(8.00f, 8.00f);
            style.FramePadding = ImVec2(5.00f, 2.00f);
            style.CellPadding = ImVec2(6.00f, 6.00f);
            style.ItemSpacing = ImVec2(6.00f, 6.00f);
            style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
            style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
            style.IndentSpacing = 25;
            style.ScrollbarSize = 15;
            style.GrabMinSize = 10;
            style.WindowBorderSize = 1;
            style.ChildBorderSize = 1;
            style.PopupBorderSize = 1;
            style.FrameBorderSize = 1;
            style.TabBorderSize = 1;
            style.WindowRounding = 7;
            style.ChildRounding = 4;
            style.FrameRounding = 3;
            style.PopupRounding = 4;
            style.ScrollbarRounding = 9;
            style.GrabRounding = 3;
            style.LogSliderDeadzone = 4;
            style.TabRounding = 4;
        }
    } // namespace Utils
} // namespace ENDER
