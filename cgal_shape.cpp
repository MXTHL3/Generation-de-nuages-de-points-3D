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
    size_t last_occur = filename.rfind(".");
    if (last_occur == std::string::npos) {
        std::cerr << "Pas d'extension trouvée : " << filename << "\n";
        return;
    }

    std::string ext = filename.substr(last_occur);

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
    else {
        std::cerr << "Format non supporté : " << ext << "\n";
    }
}