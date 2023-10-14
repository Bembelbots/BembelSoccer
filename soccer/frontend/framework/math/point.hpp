#pragma once

#include <Eigen/Dense>
#include "coord.h"
#include "directed_coord.h"

namespace BB
{
    class Point2d : public Eigen::Vector2f
    {
        using Eigen::Vector2f::Vector2f;

        explicit Point2d(Coord &other)
        {
            m_storage.data()[0] = other.x;
            m_storage.data()[1] = other.y;
        }
    };

    class Point3d : public Eigen::Vector3f
    {
        using Eigen::Vector3f::Vector3f;

        explicit Point3d(DirectedCoord &other)
        {
            m_storage.data()[0] = other.coord.x;
            m_storage.data()[1] = other.coord.y;
            m_storage.data()[2] = other.angle.rad();
        }
    };

}