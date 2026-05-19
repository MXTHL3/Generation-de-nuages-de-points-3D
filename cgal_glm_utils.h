#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "pose.h"

struct ModelTransform;

inline Transform3 model_transform_to_cgal(const ModelTransform& tr)
{
    glm::mat4 m(1.0f);
    m = glm::translate(m, glm::vec3(tr.pos_x, tr.pos_y, tr.pos_z));
    m = glm::rotate(m, tr.angle_x, glm::vec3(1,0,0));
    m = glm::rotate(m, tr.angle_y, glm::vec3(0,1,0));
    m = glm::rotate(m, tr.angle_z, glm::vec3(0,0,1));
    m = glm::scale(m, glm::vec3(tr.scale));

    return Transform3(
        m[0][0], m[1][0], m[2][0], m[3][0],
        m[0][1], m[1][1], m[2][1], m[3][1],
        m[0][2], m[1][2], m[2][2], m[3][2]
    );
}