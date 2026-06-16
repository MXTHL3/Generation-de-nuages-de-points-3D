#pragma once

#include "scene.h"
#include "asset_manager.h"

/// @brief Étape abstraite du pipeline de génération de scènes
/// Chaque implémentation transforme un ensemble de scènes en un nouveau ensemble plus grand (augmentation de l'ensemble).
/// Les étapes sont chaînées.
class PipelineStep {
public:
    virtual ~PipelineStep(){}

    /// @brief Traite les scènes d'entrée et produit les scènes augmentées.
    /// @param input_scenes Scènes à transformer
    /// @param assets Gestionnaire de ressources (meshes, configs)
    /// @return Scènes augmentées
    virtual std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) = 0;
};

/// @brief Placement aléatoire des entités dans une scène avec zone d'exclusion.
class PositionLayoutAugmentation : public PipelineStep{
private:
    size_t m_number_of_variations; // nombre de variations (de scène généré par augmentations) par scènes
    std::mt19937 m_gen;

public: 
    /// @param number_of_variations Nombre de variantes par scène d'entrée.
    /// @param seed Graine du générateur aléatoire.
    PositionLayoutAugmentation(size_t number_of_variations, unsigned int seed = 42)
        :m_number_of_variations(number_of_variations), m_gen(seed){}

    // TODO:: A voir pour le domaine d'exclusion si c'est une radius ou une zone rectangulaire pour l'instant en dur
    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

/// @brief Rotations uniforme d'une entité ciblée (pas = 360°/N) avec N le nombre de rotations uniforme choisi.
class RotationLayoutAugmentation : public PipelineStep{
private:
    size_t m_number_of_variations; // nombre de variations (de scène généré par augmentations) par scènes
    double m_start_angle, m_end_angle; // Domaine des angles de rotations générés
    std::mt19937 m_gen;

public:
    /// @param number_of_variations Nombre de variantes de rotation.
    RotationLayoutAugmentation(size_t number_of_variations, double start_angle, double end_angle, unsigned int seed = 42)
        :m_number_of_variations(number_of_variations), m_start_angle(start_angle), m_end_angle(end_angle), m_gen(seed){}
    
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
    std::vector<KeyframesConfig> m_keyframes_configs; // listes des animations à appliquer

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

/// @brief Exécute une chaîne d'étapes d'augmentation sur une scène de base.
/// Chaque étape (PipelineStep) reçoit l'ensemble des scènes produites
/// par l'étape précédente et retourne un ensemble plus grand.
class Pipeline {
private:
    std::vector<std::unique_ptr<PipelineStep>> m_steps; ///< Étapes à exécuter dans l'ordre.

public:
    /// @brief ajoute une nouvelle étape à la fin de la liste.
    void add_step(std::unique_ptr<PipelineStep> step);

    /// @brief Execute toutes les étapes du pipeline.
    /// @param input_scene Scène de base
    /// @param assets  Gestionnaire de ressources.
    /// @return Toutes les scènes augmentées.
    std::vector<std::unique_ptr<Scene>> execute(std::unique_ptr<Scene> input_scene, AssetManager& assets);
};