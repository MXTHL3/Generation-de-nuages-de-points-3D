#pragma once

#include "lidar_scanner.h"
#include "entity.h"
#include "scene.h"
#include <vector>

/// @brief Interface de stratégie de scan.
class ScanStrategy {
public:
    virtual ~ScanStrategy() = default;
    /// @brief Execute le scan selon la stratégie.
    /// @param scanner Le scanner qui réalise le scan.
    /// @param lidar_ent Le capteur Lidar.
    /// @param scene La scène construite (build() appelé avant).
    /// @return Nuage de points résultat.
    virtual std::vector<Point3> execute(const LidarScanner& scanner,
        const LidarEntity& lidar_ent, const Scene& scene) const = 0;
};

/// @brief Scan complet : 1 tour lidar mécanique, 1 frame flash/miroir.
class ScanThreeSixty : public ScanStrategy {
public:
    std::vector<Point3> execute(const LidarScanner& scanner,
        const LidarEntity& lidar_ent, const Scene& scene) const override;
};

/// @brief Scan sur une durée paramétrable en secondes.
class ScanTimed : public ScanStrategy {
public:
    /// @param duration_in_seconds Durée du scan en secondes.
    ScanTimed(double duration_in_seconds);
    std::vector<Point3> execute(const LidarScanner& scanner,
        const LidarEntity& lidar_ent, const Scene& scene) const override;
private:
    double m_time = 1.0;
};

/// @brief N scans complets concaténés.
class ScanMultiple : public ScanStrategy {
public:
    /// @param n_scans Nombre de scans à générer et concaténer.
    ScanMultiple(int n_scans);
    std::vector<Point3> execute(const LidarScanner& scanner,
        const LidarEntity& lidar_ent, const Scene& scene) const override;
private:
    int m_n_scans;
};

/// @brief Exécute un scan avec une stratégie choisie.
class SessionScan {
public:
    /// @param strategy Stratégie de scan à utiliser.
    SessionScan(std::unique_ptr<ScanStrategy> strategy);
    /// @brief Lance le scan avec la stratégie configurée.
    /// @return Nuage de points résultat.
    std::vector<Point3> run(const LidarScanner& scanner,
        const LidarEntity& lidar, const Scene& scene) const;
private:
    std::unique_ptr<ScanStrategy> m_strategy;
};