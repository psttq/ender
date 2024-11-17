#include "VertexArray.hpp"
#include <CGAL/boost/iterator/transform_iterator.hpp>
#include <Utilities.hpp>

namespace ENDER
{
    namespace Utils
    {
        typedef std::function<glm::vec3(float, float)> ParametricSurfFunc;

        // Point3D convert_to_cgal_point(const glm::vec3 &glm_point) {
        //   return Point3D(glm_point.x, glm_point.y, glm_point.z);
        // }

        Plane_3 computeBestFitPlane(const std::vector<Point_3> &points)
        {
            Plane_3 bestFitPlane;
            CGAL::linear_least_squares_fitting_3(points.begin(), points.end(), bestFitPlane, CGAL::Dimension_tag<0>());
            return bestFitPlane;
        }

        // Вычисление базиса плоскости
        void computePlaneBasis(const Plane_3 &plane, glm::vec3 &origin, glm::vec3 &u, glm::vec3 &v)
        {
            // Точка на плоскости
            auto planePoint = plane.point();
            origin = glm::vec3(planePoint.x(), planePoint.y(), planePoint.z());

            // Нормаль плоскости
            auto normal = plane.orthogonal_vector();
            glm::vec3 n(normal.x(), normal.y(), normal.z());

            // Выбираем произвольный вектор, не коллинеарный нормали
            glm::vec3 temp = (std::abs(n.x) < std::abs(n.z)) ? glm::vec3(1, 0, 0) : glm::vec3(0, 0, 1);

            // Создаём базисные векторы
            u = glm::normalize(glm::cross(temp, n));
            v = glm::normalize(glm::cross(n, u));
        }

        // Преобразование точки из 3D в локальные координаты плоскости
        Point_2 projectToLocal(const glm::vec3 &point, const glm::vec3 &origin, const glm::vec3 &u, const glm::vec3 &v)
        {
            glm::vec3 relative = point - origin;
            double x = glm::dot(relative, u);
            double y = glm::dot(relative, v);
            return Point_2(x, y);
        }

        sptr<VertexArray>
        createTriangulationSurfaceVAO(const std::vector<glm::vec3> &curve_points,
                                      const std::vector<glm::vec3> &surface_points)
        {
            std::vector<glm::vec3> points = curve_points;
            for (auto &point : surface_points)
            {
                points.push_back(point);
            }

            std::vector<Point_3> c_points;
            for (const auto &p : points)
            {
                c_points.emplace_back(p.x, p.y, p.z);
            }

            Plane_3 bestFitPlane = computeBestFitPlane(c_points);

            // 3. Вычисляем локальную систему координат плоскости
            glm::vec3 origin, u, v;
            computePlaneBasis(bestFitPlane, origin, u, v);

            std::vector<Point_2> wireProjectedPoints;
            for (const auto &p : curve_points)
            {
                auto point = projectToLocal(p, origin, u, v);
                wireProjectedPoints.push_back(point);
            }

            std::vector<Point_2> projectedPoints;

            std::vector<float> VBO;
            std::vector<unsigned int> IBO;

            std::map<Point_2, unsigned int> pointIndexMap;
            unsigned int index = 0;

            for (const auto &p : points)
            {
                auto point = projectToLocal(p, origin, u, v);
                projectedPoints.push_back(point);
                VBO.push_back(p.x);
                VBO.push_back(p.y);
                VBO.push_back(p.z);
                pointIndexMap[point] = index++;
            }

            Delaunay delaunay;
            delaunay.insert(projectedPoints.begin(), projectedPoints.end());

            auto facets_num = delaunay.number_of_faces();
            spdlog::error("FACETS: {}", facets_num);

            for (auto it = delaunay.finite_faces_begin();
                 it != delaunay.finite_faces_end(); ++it)
            {
                // CHECK IF FACET HAS ANY POINT INSIDE SURFACE

                auto p1 = it->vertex(0)->point();
                auto p2 = it->vertex(1)->point();
                auto p3 = it->vertex(2)->point();

                int pointsOnCurve = 0;

                for (auto &point : wireProjectedPoints)
                {
                    auto point_glm = glm::vec2{point.x(), point.y()};
                    if (glm::length(glm::vec2{p1.x(), p1.y()} - point_glm) <= 1e-7)
                    {
                        pointsOnCurve++;
                    }
                    if (glm::length(glm::vec2{p2.x(), p2.y()} - point_glm) <= 1e-7)
                    {
                        pointsOnCurve++;
                    }
                    if (glm::length(glm::vec2{p3.x(), p3.y()} - point_glm) <= 1e-7)
                    {
                        pointsOnCurve++;
                    }
                    if(pointsOnCurve > 2) break;
                }

               
                // Получаем три вершины грани
                if (pointsOnCurve <= 2)
                {

                    auto v1 = pointIndexMap[p1];
                    auto v2 = pointIndexMap[p2];
                    auto v3 = pointIndexMap[p3];

                    IBO.push_back(v1);
                    IBO.push_back(v2);
                    IBO.push_back(v3);
                }
            }

            auto layout = uptr<BufferLayout>(
                new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}}));
            auto vbo = std::make_unique<VertexBuffer>(std::move(layout));
            vbo->setData(&VBO[0], sizeof(float) * VBO.size());

            auto ibo = std::make_unique<IndexBuffer>(&IBO[0], IBO.size());

            auto vao = std::make_shared<VertexArray>();
            vao->addVBO(std::move(vbo));
            vao->setIndexBuffer(std::move(ibo));

            return vao;
        }

        unsigned int *generateParametricSurfaceGrid(int rows, int cols)
        {
            auto *indices = new unsigned int[(rows - 1) * (cols - 1) * 2 * 3];
            for (int row = 0; row < rows - 1; row++)
            {
                for (int col = 0; col < cols - 1; col++)
                {
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

        sptr<VertexArray> createParametricSurfaceVAO(ParametricSurfFunc func,
                                                     float u_min, float v_min,
                                                     float u_max, float v_max,
                                                     uint rows, uint cols)
        {
            auto indices = generateParametricSurfaceGrid(rows, cols);

            float *vertices = new float[rows * cols * 3];

            float h_u = (u_max - u_min) / (cols - 1);
            float h_v = (v_max - v_min) / (rows - 1);

            for (auto i = 0; i < rows; i++)
            {
                for (auto j = 0; j < cols; j++)
                {
                    float u = u_min + h_u * j;
                    float v = v_min + h_v * i;
                    auto vertice = func(u, v);
                    vertices[(j + i * cols) * 3] = vertice.x;     // x
                    vertices[(j + i * cols) * 3 + 1] = vertice.y; // x
                    vertices[(j + i * cols) * 3 + 2] = vertice.z; // x

                    // printf("%0.2f %0.2f %0.2f\t", vertice.x, vertice.y, vertice.z);
                }
                // printf("\n");
            }

            // for(auto i = 0; i < (rows - 1) * (cols - 1) * 2*3; i+=3) {
            //   printf("%0.2f %0.2f %0.2f\t",
            //   vertices[indices[i]*3],vertices[indices[i]*3+1]
            //   ,vertices[indices[i]*3+2]); printf("%0.2f %0.2f %0.2f\t",
            //   vertices[indices[i+1]*3],vertices[indices[i+1]*3+1]
            //   ,vertices[indices[i+1]*3+2]); printf("%0.2f %0.2f %0.2f\t",
            //   vertices[indices[i+2]*3],vertices[indices[i+2]*3+1]
            //   ,vertices[indices[i+2]*3+2]);
            // printf("\n==================================\n");
            // }

            auto layout = uptr<BufferLayout>(
                new ENDER::BufferLayout({{ENDER::LayoutObjectType::Float3}}));
            auto vbo = std::make_unique<VertexBuffer>(std::move(layout));
            vbo->setData(vertices, sizeof(float) * rows * cols * 3);

            auto ibo =
                std::make_unique<IndexBuffer>(indices, (rows - 1) * (cols - 1) * 2 * 3);

            auto vao = std::make_shared<VertexArray>();
            vao->addVBO(std::move(vbo));
            vao->setIndexBuffer(std::move(ibo));

            return vao;
        }

        sptr<Object> createParametricSurface(ParametricSurfFunc func, float u_min,
                                             float v_min, float u_max, float v_max,
                                             uint rows, uint cols)
        {

            auto vao =
                createParametricSurfaceVAO(func, u_min, v_min, u_max, v_max, rows, cols);
            auto object = ENDER::Object::create("ParametricSurface", vao);

            return object;
        }

        void applyImguiWhiteTheme()
        {
            ImGuiStyle &style = ImGui::GetStyle();

            style.Alpha = 1.0f;
            style.DisabledAlpha = 0.6000000238418579f;
            style.WindowPadding = ImVec2(9.100000381469727f, 8.0f);
            style.WindowBorderSize = 0.0f;
            style.WindowMinSize = ImVec2(20.0f, 32.0f);
            style.WindowTitleAlign = ImVec2(0.5f, 0.5f);
            style.WindowMenuButtonPosition = ImGuiDir_Left;
            style.ChildBorderSize = 1.0f;
            style.PopupBorderSize = 0.0f;
            style.FramePadding = ImVec2(2.099999904632568f, 1.0f);
            style.FrameBorderSize = 0.0f;
            style.ItemSpacing = ImVec2(3.299999952316284f, 3.700000047683716f);
            style.ItemInnerSpacing = ImVec2(3.200000047683716f, 4.0f);
            style.CellPadding = ImVec2(2.799999952316284f, 1.0f);
            style.IndentSpacing = 9.199999809265137f;
            style.ColumnsMinSpacing = 6.300000190734863f;
            style.ScrollbarSize = 15.5f;
            style.GrabMinSize = 10.89999961853027f;
            style.TabBorderSize = 1.0f;
            style.TabMinWidthForCloseButton = 0.0f;
            style.ColorButtonPosition = ImGuiDir_Right;
            style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
            style.SelectableTextAlign = ImVec2(0.0f, 0.5f);

            style.Colors[ImGuiCol_Text] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TextDisabled] = ImVec4(
                0.6000000238418579f, 0.6000000238418579f, 0.6000000238418579f, 1.0f);
            style.Colors[ImGuiCol_WindowBg] = ImVec4(
                0.9613733887672424f, 0.9531213045120239f, 0.9531213045120239f, 1.0f);
            style.Colors[ImGuiCol_ChildBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_PopupBg] =
                ImVec4(1.0f, 1.0f, 1.0f, 0.9800000190734863f);
            style.Colors[ImGuiCol_Border] = ImVec4(0.0f, 0.0f, 0.0f, 0.300000011920929f);
            style.Colors[ImGuiCol_BorderShadow] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_FrameBg] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            style.Colors[ImGuiCol_FrameBgHovered] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.4000000059604645f);
            style.Colors[ImGuiCol_FrameBgActive] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.6700000166893005f);
            style.Colors[ImGuiCol_TitleBg] =
                ImVec4(0.95686274766922f, 0.95686274766922f, 0.95686274766922f, 1.0f);
            style.Colors[ImGuiCol_TitleBgActive] = ImVec4(
                0.8196078538894653f, 0.8196078538894653f, 0.8196078538894653f, 1.0f);
            style.Colors[ImGuiCol_TitleBgCollapsed] =
                ImVec4(1.0f, 1.0f, 1.0f, 0.5099999904632568f);
            style.Colors[ImGuiCol_MenuBarBg] = ImVec4(
                0.8588235378265381f, 0.8588235378265381f, 0.8588235378265381f, 1.0f);
            style.Colors[ImGuiCol_ScrollbarBg] =
                ImVec4(0.9764705896377563f, 0.9764705896377563f, 0.9764705896377563f,
                       0.5299999713897705f);
            style.Colors[ImGuiCol_ScrollbarGrab] =
                ImVec4(0.686274528503418f, 0.686274528503418f, 0.686274528503418f,
                       0.800000011920929f);
            style.Colors[ImGuiCol_ScrollbarGrabHovered] =
                ImVec4(0.4862745106220245f, 0.4862745106220245f, 0.4862745106220245f,
                       0.800000011920929f);
            style.Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(
                0.4862745106220245f, 0.4862745106220245f, 0.4862745106220245f, 1.0f);
            style.Colors[ImGuiCol_CheckMark] = ImVec4(
                0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_SliderGrab] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.7799999713897705f);
            style.Colors[ImGuiCol_SliderGrabActive] =
                ImVec4(0.4588235318660736f, 0.5372549295425415f, 0.800000011920929f,
                       0.6000000238418579f);
            style.Colors[ImGuiCol_Button] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.4000000059604645f);
            style.Colors[ImGuiCol_ButtonHovered] = ImVec4(
                0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_ButtonActive] = ImVec4(
                0.05882352963089943f, 0.529411792755127f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_Header] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.3100000023841858f);
            style.Colors[ImGuiCol_HeaderHovered] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.800000011920929f);
            style.Colors[ImGuiCol_HeaderActive] = ImVec4(
                0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_Separator] =
                ImVec4(0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f,
                       0.6200000047683716f);
            style.Colors[ImGuiCol_SeparatorHovered] =
                ImVec4(0.1372549086809158f, 0.4392156898975372f, 0.800000011920929f,
                       0.7799999713897705f);
            style.Colors[ImGuiCol_SeparatorActive] = ImVec4(
                0.1372549086809158f, 0.4392156898975372f, 0.800000011920929f, 1.0f);
            style.Colors[ImGuiCol_ResizeGrip] =
                ImVec4(0.3490196168422699f, 0.3490196168422699f, 0.3490196168422699f,
                       0.1700000017881393f);
            style.Colors[ImGuiCol_ResizeGripHovered] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.6700000166893005f);
            style.Colors[ImGuiCol_ResizeGripActive] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.949999988079071f);
            style.Colors[ImGuiCol_Tab] = ImVec4(0.7607843279838562f, 0.7960784435272217f,
                                                0.8352941274642944f, 0.9309999942779541f);
            style.Colors[ImGuiCol_TabHovered] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.800000011920929f);
            style.Colors[ImGuiCol_TabActive] = ImVec4(
                0.5921568870544434f, 0.7254902124404907f, 0.8823529481887817f, 1.0f);
            style.Colors[ImGuiCol_TabUnfocused] =
                ImVec4(0.9176470637321472f, 0.9254902005195618f, 0.9333333373069763f,
                       0.9861999750137329f);
            style.Colors[ImGuiCol_TabUnfocusedActive] = ImVec4(
                0.7411764860153198f, 0.8196078538894653f, 0.9137254953384399f, 1.0f);
            style.Colors[ImGuiCol_PlotLines] = ImVec4(
                0.3882353007793427f, 0.3882353007793427f, 0.3882353007793427f, 1.0f);
            style.Colors[ImGuiCol_PlotLinesHovered] =
                ImVec4(1.0f, 0.4274509847164154f, 0.3490196168422699f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogram] =
                ImVec4(0.8980392217636108f, 0.6980392336845398f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_PlotHistogramHovered] =
                ImVec4(1.0f, 0.4470588266849518f, 0.0f, 1.0f);
            style.Colors[ImGuiCol_TableHeaderBg] = ImVec4(
                0.7764706015586853f, 0.8666666746139526f, 0.9764705896377563f, 1.0f);
            style.Colors[ImGuiCol_TableBorderStrong] = ImVec4(
                0.5686274766921997f, 0.5686274766921997f, 0.6392157077789307f, 1.0f);
            style.Colors[ImGuiCol_TableBorderLight] = ImVec4(
                0.6784313917160034f, 0.6784313917160034f, 0.7372549176216125f, 1.0f);
            style.Colors[ImGuiCol_TableRowBg] = ImVec4(0.0f, 0.0f, 0.0f, 0.0f);
            style.Colors[ImGuiCol_TableRowBgAlt] =
                ImVec4(0.2980392277240753f, 0.2980392277240753f, 0.2980392277240753f,
                       0.09000000357627869f);
            style.Colors[ImGuiCol_TextSelectedBg] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.3499999940395355f);
            style.Colors[ImGuiCol_DragDropTarget] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.949999988079071f);
            style.Colors[ImGuiCol_NavHighlight] =
                ImVec4(0.2588235437870026f, 0.5882353186607361f, 0.9764705896377563f,
                       0.800000011920929f);
            style.Colors[ImGuiCol_NavWindowingHighlight] =
                ImVec4(0.6980392336845398f, 0.6980392336845398f, 0.6980392336845398f,
                       0.699999988079071f);
            style.Colors[ImGuiCol_NavWindowingDimBg] =
                ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f,
                       0.2000000029802322f);
            style.Colors[ImGuiCol_ModalWindowDimBg] =
                ImVec4(0.2000000029802322f, 0.2000000029802322f, 0.2000000029802322f,
                       0.3499999940395355f);
        }

        void applyImguiTheme()
        {
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
