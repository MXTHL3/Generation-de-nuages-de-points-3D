#ifndef ENTITY_HPP
#define ENTITY_HPP

#include "pose.hpp"
#include "lidar.hpp"
#include "noise_model.hpp"

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

    /// @brief Calcule la matrice de transformation pour placement dans la scène.
    Transform3 transform() const;    // Récupère la position dans la scene.
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
    /// @param p ///< Position et rotation initale.
    StaticEntity(std::string name, std::shared_ptr<Object> obj, Pose p);
    std::unique_ptr<IEntity> clone() const override; 
    const std::string& name() const override final {return m_name;};

    /// @brief Retourne un accès en lecture aux triangles du maillage
    const std::vector<Triangle3>& meshTriangles() const { return m_object->m_triangles; }
    
    /// @brief Remplace le maillage par un nouveau (utilisé dans KeyframeLayoutAugmentation !)
    void update_mesh(std::shared_ptr<Object> new_obj) { m_object = new_obj; }

private:
    std::string m_name; ///< Identifiant unique dans la scène.
    std::shared_ptr<Object> m_object; ///< Maillage partagé.
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
    /// @param duration durée du scan (secondes).
    /// Mécanique : angle total = duration * rotation_rate * 360°
    /// Miroir lissajou : n_points = n_points_per_second * duration
    /// Flash : ignoré car capture instantanée
    virtual std::vector<Ray3> generate_rays(double duration = -1.0) const = 0;

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

/// @brief Capteur lidar mécanique rotatif (Velodyne, Ouster, Hesai).
class MechanicalLidarEntity : public LidarEntity {
public:
    MechanicalLidarEntity(std::shared_ptr<MechanicalLidarConfig> config, Pose p);

    /// @brief Accès typé à la config mécanique.
    const MechanicalLidarConfig& config() const{ return static_cast<const MechanicalLidarConfig&>(*m_config); }

    /// @brief Génère les rayons pour un angle azimutal (horizontal) donné
    std::vector<Ray3> scan(double h_angle_deg) const;

    std::vector<Ray3> generate_rays(double duration = -1.0) const override;
};

/// @brief Génère une grille régulière de rayons (partagé entre Flash et Miroir raster).
/// @param origin Position du capteur.
/// @param wolrd_xf Transformation monde du capteur.
/// @param fov_h_min ///< Borne inférieure champ de vision horizontal (radians).
/// @param fov_h_max ///< Borne supérieure champ de vision horizontal (radians).
/// @param fov_v_min ///< Borne inférieure champ de vision vertical (radians).
/// @param fov_v_max ///< Borne inférieure champ de vision vertical (radians).
/// @param res_h ///< Nombre de colonnes.
/// @param res_v ///< Nombre de lignes.
std::vector<Ray3> generate_ray_grid(
    const Point3& origin,
    const Transform3& wolrd_xf,
    double fov_h_min, double fov_h_max,
    double fov_v_min, double fov_v_max,
    int res_h, int res_v
);

/// @brief Capteur lidar flash solid-state (Continental HFL110)
class FlashLidarEntity : public LidarEntity {
public:
    FlashLidarEntity(std::shared_ptr<FlashLidarConfig> config, Pose p);

    const FlashLidarConfig& config() const{ return static_cast<const FlashLidarConfig&>(*m_config); };

    std::vector<Ray3> generate_rays(double duration = -1.0) const override;
};
/// @brief Capteur lidar à Miroir oscillant (Livox, Robosense)
class MirroredLidarEntity : public LidarEntity {
public:
    MirroredLidarEntity(std::shared_ptr<MirroredLidarConfig> config, Pose p);

    const MirroredLidarConfig& config() const{ return static_cast<const MirroredLidarConfig&>(*m_config); };
    
    std::vector<Ray3> generate_rays(double duration = -1.0) const override;

private:
    /// @brief Génère les points le long d'une courbe de Lissajous.
    std::vector<Ray3> generate_lissajou(double duration = -1.0) const;
    /// @brief Génère une grille régulière (comme le Flash lidar) en déléguant à generate_ray_grid 
    std::vector<Ray3> generate_raster() const;
};


#endif