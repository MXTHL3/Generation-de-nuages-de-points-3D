#include "scan_strategy.hpp"
#include "logger.hpp"

std::vector<Point3> ScanThreeSixty::execute(const LidarScanner &scanner, const LidarEntity &lidar_ent, const Scene &scene) const
{
    return scanner.scan(lidar_ent, scene, true, -1.0, 4);
}

ScanTimed::ScanTimed(double duration_in_seconds): m_time(duration_in_seconds){}

std::vector<Point3> ScanTimed::execute(const LidarScanner &scanner, const LidarEntity &lidar_ent, const Scene &scene) const
{

    std::vector<Point3> cloud = scanner.scan(lidar_ent, scene, true, m_time, 6);

    SIM_DEBUG("ScanTimed : {:.3f}s", m_time);
    return cloud;
}

ScanMultiple::ScanMultiple(int n_scans)
    : m_n_scans(n_scans){}

std::vector<Point3> ScanMultiple::execute(const LidarScanner &scanner, const LidarEntity &lidar_ent, const Scene &scene) const
{
    std::vector<Point3> multi_scans;

    for(int i = 0; i < m_n_scans; i++){
        auto cloud = scanner.scan(lidar_ent, scene, true, -1.0, 6);
        multi_scans.insert(multi_scans.end(), cloud.begin(), cloud.end());        
    }

    SIM_DEBUG("ScanMultiple : {} scans donne {} points", m_n_scans, multi_scans.size());
    return multi_scans;
}

SessionScan::SessionScan(std::unique_ptr<ScanStrategy> strategy)
    : m_strategy(std::move(strategy)){}

std::vector<Point3> SessionScan::run(const LidarScanner &scanner, const LidarEntity &lidar_ent, const Scene &scene) const
{
    return m_strategy->execute(scanner, lidar_ent, scene);
}
