#ifndef SCAN_STRATEGY_HPP
#define SCAN_STRATEGY_HPP

#include "lidar_scanner.hpp"
#include "entity.hpp"
#include "scene.hpp"
#include <vector>

/// @brief Interface de stratégie de scan.
/// Définit comment un scan est executé par un capteur.
class ScanStrategy{
public:
    virtual ~ScanStrategy() = default;

    /// @brief Execute le scan selon la stratégie.
    /// @param scanner Le scanner qui va réaliser scan de la scene.
    /// @param lidar_ent Le capteur Lidar (génération de rayons).
    /// @param scene La scene construite ! (build() l'arbre AABB avant).
    /// @return Nuage de points résultat.
    virtual std::vector<Point3> execute(const LidarScanner& scanner, const LidarEntity& lidar_ent, const Scene& scene) const = 0;
};

/// @brief Scan complet : 1 tour lidar mécanique, 1 frame (lidar mirroir et flash)
class ScanThreeSixty : public ScanStrategy{
public:    
    std::vector<Point3> execute(const LidarScanner& scanner, const LidarEntity& lidar_ent, const Scene& scene) const override;
};

/// @brief Scan sur une durée paramétrable en secondes
class ScanTimed : public ScanStrategy {
public:
    /// @param duration_in_seconds La durée du scan en secondes
    ScanTimed(double duration_in_seconds);

    std::vector<Point3> execute(const LidarScanner& scanner, const LidarEntity& lidar_ent, const Scene& scene) const override;

private:
    double m_time = 1.0;
};

/// @brief N scans complets concaténés.
class ScanMultiple: public ScanStrategy {
public:
    /// @param n_scans nombre de scans à générer et contaténer.
    ScanMultiple(int n_scans);

    std::vector<Point3> execute(const LidarScanner& scanner, const LidarEntity& lidar_ent, const Scene& scene) const override;

private:
    int m_n_scans;
};

/// @brief Execute le scan avec une stratégie
class SessionScan {
public:
    /// @param strategy Strategie de scan   
    SessionScan(std::unique_ptr<ScanStrategy> strategy);

    /// @brief Lance le scan avec la stratégie configurée.
    /// @return Le nuage de point résultat du scan.
    std::vector<Point3> run(const LidarScanner& scanner, const LidarEntity& lidar, const Scene& scene) const;
private:
    std::unique_ptr<ScanStrategy> m_strategy;   
};

#endif