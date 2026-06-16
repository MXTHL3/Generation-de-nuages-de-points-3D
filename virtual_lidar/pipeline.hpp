#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "scene.hpp"
#include "asset_manager.hpp"

/// @brief Etape abstraite du pipeline de generation de scènes
/// Chaque implémentation transforme un ensemble de scène en un nouveau ensemble plus grand (augmentation de l'ensemble).
/// Les étapes sont chainés.
class PipelineStep {
public:
    virtual ~PipelineStep(){}

    /// @brief Traite les scènes d'entrée et produit les scènes augmentées.
    /// @param input_scenes Scènes à transformer
    /// @param assets Gestionnaire de ressources (meshes, configs)
    /// @return Scènes augmentés
    virtual std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) = 0;
};

/// @brief Placement aléatoire des entités dans une scène avec zone d'exclusion.
/// Attention cette classe est dépréciée ! Utiliser la classe GridPositionLayoutAugmentation à la place.
class PositionLayoutAugmentation : public PipelineStep{
private:
    size_t m_number_of_variations;
    std::mt19937 m_gen;

public:
    /// @param number_of_varaitions Nombre de variantes par scène d'entrée.
    /// @param seed Graine du générateur aléatoire.    
    PositionLayoutAugmentation(size_t number_of_variations, unsigned int seed = 42)
        :m_number_of_variations(number_of_variations), m_gen(seed){}

    // TODO:: A voir pour le domaine d'exclusion si c'est une radius ou une zone rectangulaire pour l'instant en dur
    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

/// @brief Rotations uniforme d'une entité ciblée (pas = 360°/N) avec N le nombre de rotations uniforme choisi.
class RotationLayoutAugmentation : public PipelineStep{
private:
    std::string m_object_name;
    size_t m_number_of_variations; // nombre de variations (de scène généré par augmentations) par scènes

public:
    /// @param object_name Nom de l'entité à tourner.
    /// @param number_of_variations Nombre de variantes de rotation.
    RotationLayoutAugmentation(const std::string& object_name, size_t number_of_variations)
        :m_object_name(std::move(object_name)), m_number_of_variations(number_of_variations){}

    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

/// @brief Animation d'un objet : liste des fichiers de ses "keyframes"
struct ObjectAnimation {
    std::string object_name; ///< Nom de l'entité dans la scène.
    std::vector<std::string> keyframes_paths; ///< Animations à appliquer.
};

/// @brief Configurations'un scénario d'animation
struct KeyframesConfig {
    std::string scenario_name; ////< Nom du scénario.
    std::vector<ObjectAnimation> objects_animation; ///< Animations à appliquer.
};

/// @brief Génération de scènes par substitution de meshes (keyframes d'animation).
class KeyframeLayoutAugmentation : public PipelineStep {
private:
    std::vector<KeyframesConfig> m_keyframes_configs; // listes des animations à appliquer.

    /// @brief Applique une keyframe/ un nouveau mesh à une copie de scène.
    std::unique_ptr<Scene> apply_keyframe(const Scene& input_scene, const std::string &entity_name, const std::string &keyframe_path, AssetManager &assets);
    
    /// @brief Génère toutes les combinaisons de keyframes pour un scénario.
    std::vector<std::unique_ptr<Scene>> generate_combinations(const Scene& input_scene, const KeyframesConfig &config, AssetManager& assets);

public:
    /// @param keyframes_configs Liste de scénarios d'animation.
    KeyframeLayoutAugmentation(std::vector<KeyframesConfig> keyframes_configs)
        : m_keyframes_configs(keyframes_configs){}

    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

/// @brief Placement sur une grille déterministe avec un rayon d'exclusion.
///
/// Génère une nouvelle position pour une entité pour chaque points de la grille.
class GridPositionLayoutAugmentation : public PipelineStep {
public:
    /// @param object_name Nom de l'entité à déplacer.
    /// @param min_x, max_x Bornes X de la grille (mètres).
    /// @param step_x Pas en X (mètres).
    /// @param min_y, max_y Bornes Y de la grille (mètres).
    /// @param step_y Pas en Y (mètres).
    /// @param exclusion_radius Rayon d'exclusion autour de l'origine (mètres).
    GridPositionLayoutAugmentation(std::string object_name, 
                                    double min_x, double max_x, double step_x,
                                    double min_y, double max_y, double step_y, 
                                    double exclusion_radius = 0.0);

    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
private:
        std::string m_object_name;
        double m_min_x, m_max_x, m_step_x;
        double m_min_y, m_max_y, m_step_y;
        double m_exclusion_radius;
};

/// @brief Pipeline séquentiel d'étapes d'augmentation de données.
///
/// Execute les étapes par ordre d'ajout : chaque étape reçoit les scènes produites par la précédente et les augmente.
class Pipeline {
private:
    // Etapes à executer correspondant à une augmentation à chaque fois des données pour l'instant
    std::vector<std::unique_ptr<PipelineStep>> m_steps;

public:
    /// @brief ajoute une nouvelle étape à la fin de la liste.
    void add_step(std::unique_ptr<PipelineStep> step);

    /// @brief Execute toutes les étapes du pipeline.
    /// @param input_scene Scène de base
    /// @param assets  Gestionnaire de ressources.
    /// @return Toutes les scènes augmentées.
    std::vector<std::unique_ptr<Scene>> execute(std::unique_ptr<Scene> input_scene, AssetManager& assets);
};

/// @brief Construit un Pipeline depuis le bloc "pipeline" du JSON de scène
class PipelineFactory{
    public:
        /// @brief Crée le pipeline à partir du contenu JSON
        /// @param data Le contenu de json
        static Pipeline create_from_json(const nlohmann::json& data);
    private:
        static std::unique_ptr<PipelineStep> parse_step(const nlohmann::json &step);
        static std::unique_ptr<PipelineStep> parse_rotation(const nlohmann::json& rotation_step);
        static std::unique_ptr<PipelineStep> parse_grid_position(const nlohmann::json& grid_position_step);
        static std::unique_ptr<PipelineStep> parse_keyframe(const nlohmann::json& keyframe_step);
};

#endif