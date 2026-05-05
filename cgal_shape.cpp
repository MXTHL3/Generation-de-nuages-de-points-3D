#include "cgal_shape.h"

void CgalShape::build_mesh()
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