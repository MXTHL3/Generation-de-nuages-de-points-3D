#pragma once

#include "cgal.h"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class CgalShape : public Cgal {
public:
    void build_cube_mesh() override;
    void build_mesh_from_file(const std::string& filename) override;
};