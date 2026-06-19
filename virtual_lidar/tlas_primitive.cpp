#include "tlas_primitive.hpp"

TlasPrimitive::Id TlasPrimitive::id() const
{
    return m_id;
}

TlasPrimitive::Datum TlasPrimitive::datum() const
{
    return m_cuboid;
}

TlasPrimitive::Point TlasPrimitive::reference_point() const
{
    return Point((m_cuboid.xmin() + m_cuboid.xmax()) / 2.0,
                (m_cuboid.ymin() + m_cuboid.ymax()) / 2.0,
                (m_cuboid.zmin() + m_cuboid.zmax()) / 2.0
    );
}
