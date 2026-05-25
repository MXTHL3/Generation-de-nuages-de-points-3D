#pragma once

#include "pose.h"

#include <pdal/PointTable.hpp>
#include <pdal/PointView.hpp>
#include <pdal/io/LasWriter.hpp>
#include <pdal/Options.hpp>
#include <pdal/io/BufferReader.hpp>
#include <vector>
#include <string>

class IPointCloudExporter {
public:
    virtual ~IPointCloudExporter() {};
    virtual void save(const std::string& filename, const std::vector<Point3>& points) = 0;
};

class PlyExporter : public IPointCloudExporter {
public:
    void save(const std::string& filename, const std::vector<Point3>& points) override;
};

class LasExporter : public IPointCloudExporter {
public:
    void save(const std::string& filename, const std::vector<Point3>& points) override;
};

