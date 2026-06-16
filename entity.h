#pragma once

#include "pose.h"
#include "lidar.h"
#include "noise_model.h"

#include <memory>
#include <vector>

typedef K::Ray_3 Ray3;
typedef K::Triangle_3 Triangle3;

/// @brief Entité Abstraite présente dans la scène 3D.
/// Classe de base pour tout les objets dans la scène.
class IEntity {
public:
    /// @param pose Position et Rotation initiale dans le repère global.
    IEntity(Pose pose): m_pose(pose){};
    virtual ~IEntity(){};

    /// @brief Crée une copie de l'entité.
    virtual std::unique_ptr<IEntity> clone() const = 0;

    /// @brief Calcule la matrice de transformation pour placement dans la scène.
    Transform3 transform() const;    // Récupère la position dans la scene

    /// @brief Retourne l'identifiant de l'entité.
    virtual const std::string& name() const = 0;

    const Pose& pose() const { return m_pose; }
    void pose(const Pose& pose) { m_pose = pose; }


protected:
    Pose m_pose; ///< Position et rotation dans le repère global.
};

/// @brief Maillage 3D sous forme de triangles.
/// Le maillage peut être partagé entre les entités.
class Object {
public:
    std::vector<Triangle3> m_triangles; ///< Faces triangulaires du maillage.
};

/// @brief  Objet statique de la scène (ne bouge pas !).

/// Représente un élément géométrique.
/// Son maillage peut être remplacé (notamment avec l'augmentation par keyframes).
class StaticEntity : public IEntity {
public:
    /// @param name ///< Identifiant unique dans la scène.
    /// @param obj ///< Maillage partagé.
    /// @param p ///< Position et rotation initiale.
    StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p, std::string mesh_path = "");
    
    std::unique_ptr<IEntity> clone() const override; 
    const std::string& name() const override final {return m_name;};

    /// @brief Retourne un accès en lecture aux triangles du maillage
    const std::vector<Triangle3>& meshTriangles() const { return m_object->m_triangles; }
    
    const std::string& mesh_path() const { return m_mesh_path; }

    /// @brief Remplace le maillage par un nouveau (utilisé dans KeyframeLayoutAugmentation !)
    void update_mesh(std::shared_ptr<Object> new_obj) { m_object = new_obj; }

private:
    std::string m_name; ///< Identifiant unique dans la scène.
    std::shared_ptr<Object> m_object; ///< Maillage partagé.
    std::string m_mesh_path;
};

/// @brief Capteur Lidar abstrait dans la scène
/// Gère la configuration d'un lidar LidarConfig, son modèle de bruit et la génération des rayons à lancer pour le scan
class LidarEntity : public IEntity {
public:
    /// @param config Configuration d'un capteur Lidar partagée 
    /// @param p Position et rotation du capteur dans la scène
    LidarEntity(std::shared_ptr<LidarConfig> config, Pose p);

    virtual ~LidarEntity() = default;
    
    const std::string& name() const override final { return m_config->m_name; };

    const LidarConfig& config() const { return *m_config; };

    /// @brief Génère tous les rayons pour le scan.
    virtual std::vector<Ray3> generate_rays() const = 0;

    /// @brief Remplace le modèle de bruit
    /// @param model nouveau modèle
    void noise_model(const NoiseModel& model) {m_noise_model = model; }

    /// @brief Applique le bruit à une mesure de distance
    /// @return la distance bruitée
    double noisy_distance(double d) const{
        return m_noise_model.apply(d);
    }

protected:
    std::shared_ptr<LidarConfig> m_config; ///< La config partagée du lidar
    NoiseModel m_noise_model; ///< le profil de bruit du lidar
};

/// @brief Capteur lidar mécanique rotatif (Velodyne, Ouster).
class MechanicalLidarEntity : public LidarEntity {
public:
    MechanicalLidarEntity(std::shared_ptr<MechanicalLidarConfig> config, Pose p);

    /// @brief Accès typé à la config mécanique.
    const MechanicalLidarConfig& config() const{ return static_cast<const MechanicalLidarConfig&>(*m_config); }
    
    std::unique_ptr<IEntity> clone() const override;

    /// @brief Génère les rayons pour un angle azimutal (horizontal) donné
    std::vector<Ray3> scan(double parameter) const;

    std::vector<Ray3> generate_rays() const override;
};

/// @brief Génère une grille régulière de rayons (partagé entre Flash et Miroir raster).
/// @param origin Position du capteur.
/// @param wolrd_xf Transformation monde du capteur.
/// @param res_h ///< Nombre de colonnes.
/// @param res_v ///< Nombre de lignes.
std::vector<Ray3> generate_ray_grid(
    const Point3& origin,
    const Transform3& wolrd_xf,
    double fov_h, double fov_v,
    int res_h, int res_v
);

/// @brief Capteur lidar flash solid-state (Continental HFL110)
class FlashLidarEntity : public LidarEntity {
public:
    FlashLidarEntity(std::shared_ptr<FlashLidarConfig> config, Pose p);

    const FlashLidarConfig& config() const{ return static_cast<const FlashLidarConfig&>(*m_config); };

    std::unique_ptr<IEntity> clone() const override;

    std::vector<Ray3> generate_rays() const override;
};

/// @brief Capteur lidar à Miroir oscillant (Livox)
class MirroredLidarEntity : public LidarEntity {
public:
    MirroredLidarEntity(std::shared_ptr<MirroredLidarConfig> config, Pose p);

    const MirroredLidarConfig& config() const{ return static_cast<const MirroredLidarConfig&>(*m_config); };

    std::unique_ptr<IEntity> clone() const override;
    
    std::vector<Ray3> generate_rays() const override;

private:
    std::vector<Ray3> generate_lissajou() const;
    std::vector<Ray3> generate_raster() const;
};