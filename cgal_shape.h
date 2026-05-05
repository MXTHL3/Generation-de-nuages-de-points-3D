#pragma once

#include "cgal.h"

class CgalShape : public Cgal {
public:
    void build_mesh() override;
};