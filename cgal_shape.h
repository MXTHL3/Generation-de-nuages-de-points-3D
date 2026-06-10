#pragma once

#include "cgal.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/triangulate_faces.h>

class CgalShape : public Cgal {
public:
    void build_cube_mesh() override;
    void build_mesh_from_file(const std::string& filename) override;
};