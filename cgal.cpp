#include "cgal.h"

SurfaceMesh::Vertex_index Cgal::add_vertex(double x, double y, double z)
{
    return m_mesh.add_vertex(Point(x, y, z));
}

void Cgal::add_triangle(SurfaceMesh::Vertex_index a,
                        SurfaceMesh::Vertex_index b,
                        SurfaceMesh::Vertex_index c)
{
    m_mesh.add_face(a, b, c);
}

std::vector<float> Cgal::to_vertex_data() const
{
    std::vector<float> data;
    data.reserve(m_mesh.number_of_faces() * 3 * 6);
    auto positions = m_mesh.points();

    for (auto face : m_mesh.faces())
    {
        std::vector<SurfaceMesh::Vertex_index> verts;
        for (auto v : CGAL::vertices_around_face(m_mesh.halfedge(face), m_mesh))
            verts.push_back(v);

        if (verts.size() != 3) continue;

        for (auto vi : verts)
        {
            const Point& p = positions[vi];

            float x = static_cast<float>(p.x());
            float y = static_cast<float>(p.y());
            float z = static_cast<float>(p.z());
            data.push_back(x);
            data.push_back(y);
            data.push_back(z);
            data.push_back((x + 1.0f) * 0.5f);
            data.push_back((y + 1.0f) * 0.5f);
            data.push_back((z + 1.0f) * 0.5f);
        }
    }

    return data;
}