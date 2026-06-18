#include "cgal_shape.h"

void CgalShape::build_cube_mesh()
{
    m_mesh.clear();

    auto v0 = add_vertex(-1, -1, -1);
    auto v1 = add_vertex( 1, -1, -1);
    auto v2 = add_vertex( 1,  1, -1);
    auto v3 = add_vertex(-1,  1, -1);

    auto v4 = add_vertex(-1, -1,  1);
    auto v5 = add_vertex( 1, -1,  1);
    auto v6 = add_vertex( 1,  1,  1);
    auto v7 = add_vertex(-1,  1,  1);

    add_triangle(v4, v5, v6);
    add_triangle(v4, v6, v7);

    add_triangle(v1, v0, v3);
    add_triangle(v1, v3, v2);

    add_triangle(v0, v4, v7);
    add_triangle(v0, v7, v3);

    add_triangle(v5, v1, v2);
    add_triangle(v5, v2, v6);

    add_triangle(v3, v7, v6);
    add_triangle(v3, v6, v2);

    add_triangle(v0, v1, v5);
    add_triangle(v0, v5, v4);
}

void CgalShape::build_mesh_from_file(const std::string& filename)
{
    std::string ext = std::filesystem::path(filename).extension().string();
    if (ext.empty()) {
        std::cerr << "Pas d'extension trouvée : " << filename << "\n";
        return;
    }
    std::transform(ext.begin(), ext.end(), ext.begin(),
                [](unsigned char c){ return static_cast<char>(std::tolower(c)); });

    if (ext == ".obj") {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Impossible d'ouvrir : " << filename << "\n";
            return;
        }

        m_mesh.clear();

        std::vector<SurfaceMesh::Vertex_index> vertices;
        std::string line;

        while (std::getline(file, line)) {
            std::istringstream iss(line);
            std::string token;
            iss >> token;

            if (token == "v") {
                double x, y, z;
                iss >> x >> y >> z;
                vertices.push_back(add_vertex(x, y, z));
            }
            else if (token == "f") {
                std::vector<int> indices;
                std::string part;
                while (iss >> part) {
                    int idx = std::stoi(part.substr(0, part.find('/')));
                    indices.push_back(idx - 1);
                }
                for (size_t i = 1; i + 1 < indices.size(); ++i)
                    add_triangle(vertices[indices[0]],
                                 vertices[indices[i]],
                                 vertices[indices[i + 1]]);
            }
        }

        std::cout << "Maillage chargé : " << vertices.size() << " sommets, "
                  << m_mesh.number_of_faces() << " faces\n";
    }
    else if (ext == ".ply" || ext == ".off" || ext == ".stl") {
        m_mesh.clear();

        if (!CGAL::IO::read_polygon_mesh(filename, m_mesh, CGAL::parameters::verbose(true))) {
            std::cerr << "Impossible de lire : " << filename << "\n";
            return;
        }

        if (!CGAL::is_triangle_mesh(m_mesh)) {
            CGAL::Polygon_mesh_processing::triangulate_faces(m_mesh);
        }

        std::cout << "Maillage chargé : " << m_mesh.number_of_vertices() << " sommets, "
                  << m_mesh.number_of_faces() << " faces\n";
    }
    else if (ext == ".las" || ext == ".laz") {
        pdal::Options opts;
        opts.add("filename", filename);

        pdal::LasReader reader;
        reader.setOptions(opts);

        pdal::PointTable table;
        reader.prepare(table);
        pdal::PointViewSet viewSet = reader.execute(table);

        typedef CGAL::Projection_traits_xy_3<Kernel> Gt;
        typedef CGAL::Delaunay_triangulation_2<Gt>   Delaunay;

        std::vector<Point> raw_points;
        for (const auto& view : viewSet) {
            raw_points.reserve(raw_points.size() + view->size());
            for (pdal::PointId i = 0; i < view->size(); ++i) {
                raw_points.emplace_back(
                    view->getFieldAs<double>(pdal::Dimension::Id::X, i),
                    view->getFieldAs<double>(pdal::Dimension::Id::Y, i),
                    view->getFieldAs<double>(pdal::Dimension::Id::Z, i));
            }
        }

        if (raw_points.empty()) {
            std::cerr << "Aucun point lu dans : " << filename << "\n";
            return;
        }

        Delaunay dt;
        dt.insert(raw_points.begin(), raw_points.end());

        if (dt.number_of_faces() == 0) {
            std::cerr << "Triangulation vide (points coplanaires ou colinéaires ?) : " << filename << "\n";
            return;
        }

        m_mesh.clear();

        std::map<Delaunay::Vertex_handle, SurfaceMesh::Vertex_index> vh_map;
        for (auto vit = dt.finite_vertices_begin(); vit != dt.finite_vertices_end(); ++vit)
            vh_map[vit] = m_mesh.add_vertex(vit->point());

        for (auto fit = dt.finite_faces_begin(); fit != dt.finite_faces_end(); ++fit)
            m_mesh.add_face(vh_map[fit->vertex(0)],
                            vh_map[fit->vertex(1)],
                            vh_map[fit->vertex(2)]);

        std::cout << "Maillage LAS/LAZ chargé : " << m_mesh.number_of_vertices()
                << " sommets, " << m_mesh.number_of_faces() << " faces <- " << filename << "\n";
    }
    else {
        std::cerr << "Format non supporté : " << ext << "\n";
    }
}