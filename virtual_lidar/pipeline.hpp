#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "scene.hpp"
#include "asset_manager.hpp"

class PipelineStep {
public:
    virtual ~PipelineStep(){}

    virtual std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) = 0;
};

// Génération de scènes aléatoires avec positions différentes
class PositionLayoutAugmentation : public PipelineStep{
private:
    size_t m_number_of_variations; // nombre de variations (de scène généré par augmentations) par scènes
    std::mt19937 m_gen;

public:    
    PositionLayoutAugmentation(size_t number_of_variations, unsigned int seed = 42)
        :m_number_of_variations(number_of_variations), m_gen(seed){}

    // TODO:: A voir pour le domaine d'exclusion si c'est une radius ou une zone rectangulaire pour l'instant en dur
    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

// Génération de scènes aléatoires avec positions différentes
class RotationLayoutAugmentation : public PipelineStep{
private:
    size_t m_number_of_variations; // nombre de variations (de scène généré par augmentations) par scènes

public:    
    RotationLayoutAugmentation(size_t number_of_variations)
        :m_number_of_variations(number_of_variations){}

    // TODO:: A voir pour le domaine d'exclusion si c'est une radius ou une zone rectangulaire pour l'instant en dur
    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

// Réprésente les différentes "keyframes"(fichiers .ply) qui forme l'animation d'un personnage
struct ObjectAnimation {
    std::string object_name; // "m_name" ! de l'"Entity" de la scène
    std::vector<std::string> keyframes_paths; // path vers les fichiers PLY
};

struct KeyframesConfig {
    std::string scenario_name; // nom utile juste pour les logs pour l'instant
    std::vector<ObjectAnimation> objects_animation; // les "animations" avec leurs "keyframes" associé au nom d'objet
};


class KeyframeLayoutAugmentation : public PipelineStep {
private:
    std::vector<KeyframesConfig> m_keyframes_configs; // listes des animations à appliquer

    std::unique_ptr<Scene> apply_keyframe(const Scene& input_scene, const std::string &entity_name, const std::string &keyframe_path, AssetManager &assets);
    std::vector<std::unique_ptr<Scene>> generate_combinations(const Scene& input_scene, const KeyframesConfig &config, AssetManager& assets);

public:
    KeyframeLayoutAugmentation(std::vector<KeyframesConfig> keyframes_configs)
        : m_keyframes_configs(keyframes_configs){}

    std::vector<std::unique_ptr<Scene>> process(std::vector<std::unique_ptr<Scene>> input_scenes, AssetManager& assets) override;
};

class Pipeline {
private:
    // Etapes à executer correspondant à une augmentation à chaque fois des données pour l'instant
    std::vector<std::unique_ptr<PipelineStep>> m_steps;

public:
    void add_step(std::unique_ptr<PipelineStep> step);

    std::vector<std::unique_ptr<Scene>> execute(std::unique_ptr<Scene> input_scene, AssetManager& assets);
};

#endif