#ifndef HORIZONTAL_STEP_HPP
#define HORIZONTAL_STEP_HPP

#include <nlohmann/json.hpp>
#include <string>


/// @brief Interface pour le calcul de la résolution (horizontale) azimuth d'un LidarMecanique.
/// Les fiches techniques des constructeurs donnent 3 résolution horizontale de manière différente.
class HorizontalStepSource {
public:
    virtual ~HorizontalStepSource()= default;

    /// @brief Calcule la résolution horizontale en degrés.
    virtual double compute_h_step() const = 0;

    /// @brief Sérialize la résolution horizontale dans l'objet JSON source.
    /// @param data L'objet JSON.
    virtual void serialize(nlohmann::json& data) const = 0;
};

/// @brief Résolution à partir du nombre de colonnes par tour.
class ColumnCountSource : public HorizontalStepSource{
public:
    /// @param columns Nombre de colonnes par tour 
    ColumnCountSource(unsigned int columns);
   
    double compute_h_step() const override;

    void serialize(nlohmann::json& data) const override;

private:
    int m_columns;
};

/// @brief Résolution angulaire donnée directmement (pas de calcul)
class DirectResolutionSource : public HorizontalStepSource{
public:
    /// @param resolution_deg Résolution horizontale (degrés).
    DirectResolutionSource(double resolution_deg);
 
    double compute_h_step() const override;

    void serialize(nlohmann::json& data) const override;

private:
    double m_resolution;
};


/// @brief Résolution à partir du nombre de points par seconde
class PointsPerSecondSource : public HorizontalStepSource{
public:
    /// @param points_per_second nombres de points par seconde
    /// @param rotation_rotate vitesse de rotation (Hz)
    /// @param n_lasers Nombres de lasers (channels) du capteur
    PointsPerSecondSource(unsigned int points_per_second, double rotation_rotate, size_t n_lasers);

    double compute_h_step() const override;

    void serialize(nlohmann::json& data) const override;

private:
    unsigned int m_points_per_second;
    double m_rotation_rotate;
    size_t m_n_lasers;
};

#endif