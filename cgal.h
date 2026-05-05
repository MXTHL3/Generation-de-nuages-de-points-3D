#pragma once

#include <CGAL/Simple_cartesian.h>
#include <CGAL/Surface_mesh.h>
#include <vector>

typedef CGAL::Simple_cartesian<double> Kernel;
typedef Kernel::Point_3 Point;
typedef CGAL::Surface_mesh<Point> SurfaceMesh;

class Cgal {
public:
    virtual ~Cgal() = default;
    virtual void build_mesh() = 0;
    std::vector<float> to_vertex_data() const;
    const SurfaceMesh& mesh() const { return m_mesh; }

protected:
    SurfaceMesh m_mesh;
    SurfaceMesh::Vertex_index add_vertex(double x, double y, double z);
    void add_triangle(SurfaceMesh::Vertex_index a,
                      SurfaceMesh::Vertex_index b,
                      SurfaceMesh::Vertex_index c);
};