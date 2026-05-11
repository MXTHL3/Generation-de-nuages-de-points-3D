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
    data.reserve(m_mesh.number_of_faces() * 3 * 3);
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
            data.push_back(static_cast<float>(p.x()) + m_ox);
            data.push_back(static_cast<float>(p.y()) + m_oy);
            data.push_back(static_cast<float>(p.z()) + m_oz);
        }
    }

    return data;
}