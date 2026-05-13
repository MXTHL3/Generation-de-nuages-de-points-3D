#include "pose.hpp"

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

