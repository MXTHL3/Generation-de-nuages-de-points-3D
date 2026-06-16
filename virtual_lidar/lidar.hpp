#ifndef LIDAR_HPP
#define LIDAR_HPP

#include <vector>
#include <string>
#include <variant>
#include <nlohmann/json.hpp>

#include "noise_model.hpp"

#include "horizontal_step.hpp"

/// @brief Configuration abstraite de base d'un lidar.
/// Contient les paramètres communs de tout les types de lidars.
/// Chaque sous classe ajoute les paramètres spécifiques à sa technologie.
class LidarConfig {
public:
    std::string m_name; ///< Identifiant/nom du modèle.
    double m_min_dist;  ///< Distance minimale de détection (mètres).
    double m_max_dist;  ///< Distance maximale de détection (mètres).
    double m_accuracy;  ///< Précision du capteur.
    NoiseProfile m_noise_profile; ///< Profil de bruit.
    
    LidarConfig(std::string name, double min_dist, double max_dist, double accuracy);

    virtual ~LidarConfig();

    /// @brief Serialise le configuration complète en JSON.
    virtual void serialize(nlohmann::json& data) const = 0;

    virtual std::string to_string() const = 0;

protected:

    /// @brief Serialise le profil de bruit.
    void serialize_noise_profile(nlohmann::json& data) const;

    /// @brief Serialise les paramètres communs aux configs.
    void serialize_base(nlohmann::json& data) const;

    std::string noise_to_string() const;
};

/// @brief Configuration d'un lidar mécanique rotatif.
class MechanicalLidarConfig : public LidarConfig{
public:
    /// @brief Structure 1 laser physique du capteur.
    struct Laser
    {
        double v_rad;   ///< Angle vertical/élévation en radians. 
        double h_off;   //< Décalage angulaire horizontal en radians. 
        double d_off;   //< Décalage de distance par rapport à "l'origine" du capteur.
    };


    std::unique_ptr<HorizontalStepSource> m_h_step;

    std::vector<Laser> m_lasers;    ///< lasers du Lidar.
    double m_rotation_rate = 10.0;  ///< Tours par seconde (Hz).

    MechanicalLidarConfig(std::string name, double min_dist, double max_dist, double accuracy,
         double rotation_rate);

    void serialize(nlohmann::json& data) const override;

    std::string to_string() const override;

    /// @brief Ajoute un laser au capteur.
    /// @param v_rad Angle vertical/élévation (radians).
    /// @param h_rad Décalage azimutal (radians).
    /// @param d_off Décalage de distance para rapport à l'origine (mètres).
    void addLaser(double v_rad, double h_rad, double d_off);
    
    /// @brief Retourne la résolution verticale/azimutale en degrés à partir de "m_h_step". 
    double horizontal_step() const;
};

/// @brief Configuration d'un lidar flash (sans balayage).
class FlashLidarConfig : public LidarConfig {
public:    
    int m_resolution_h; ///< Nombre de pixels horizontaux.
    int m_resolution_v; ///< Nombre de pixels verticaux.
    double m_fov_h_min; ///< Borne inférieure champ de vision horizontal (radians).
    double m_fov_h_max; ///< Borne supérieure champ de vision horizontal (radians).
    double m_fov_v_min; ///< Borne inférieure champ de vision vertical (radians).
    double m_fov_v_max; ///< Borne inférieure champ de vision vertical (radians).

    FlashLidarConfig(std::string name, double min_dist, double max_dist, double accuracy, 
        int res_h, int res_v, double fov_h_min, double fov_h_max, double fov_v_min, double fov_v_max);
    
    void serialize(nlohmann::json& data) const override;

    std::string to_string() const override;
};

struct LissajouParams {
    double m_amplitude_h;           ///< Amplitude de balayage horizontal (radians)
    double m_amplitude_v;           ///< Amplitude de balayage vertical (radians)  
    double m_freq_h;                ///< Fréquence d'oscillation horizontale (Hz)
    double m_freq_v;                ///< Fréquence d'oscillation verticale (Hz)
    double m_phase_diff;            ///< "Déphasage" entre 2 axes (radians)
};

struct RasterParams {
    int resolution_h = 0;           ///< Nombre de colonnes
    int resolution_v = 0;           ///< Nombre de lignes
};

/// @brief Configuration d'un lidar à miroir (balayage de Lissajous)
class MirroredLidarConfig : public LidarConfig {
public:
    int m_points_per_second;        ///< Tir par secondes

    double m_fov_h_min; ///< Borne inférieure champ de vision horizontal (radians).
    double m_fov_h_max; ///< Borne supérieure champ de vision horizontal (radians).
    double m_fov_v_min; ///< Borne inférieure champ de vision vertical (radians).
    double m_fov_v_max; ///< Borne inférieure champ de vision vertical (radians).
    double m_integration_time;      ///< Durée d'acumulation d'une frame (secondes)

    std::variant<LissajouParams, RasterParams> m_mirrored_scan_params; ///< contient les paramètres pour lissajou ou raster

    bool is_lissajou() const;
    bool is_raster() const;

    MirroredLidarConfig(std::string name, double min_dist, double max_dist, double accuracy,
        int points_per_second, double fov_h_min, double fov_h_max, double fov_v_min, double fov_v_max, double integration_time);

    void serialize(nlohmann::json& data) const override;

    std::string to_string() const override;
};

#endif