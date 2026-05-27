#ifndef PIPELINE_HPP
#define PIPELINE_HPP

#include "scene.hpp"
#include "asset_manager.hpp"

class PipelineStep {
public:
    virtual ~PipelineStep(){}

    virtual std::vector<std::shared_ptr<Scene>> process(const std::vector<std::shared_ptr<Scene>>& input_scenes, AssetManager& assets) = 0;
};

class SpatialLayoutAugmentation : public PipelineStep{
private:
    size_t m_number_of_variations; // nombre de variations (de scène généré par augmentations) par scèness
    std::mt19937 m_gen;

public:
    SpatialLayoutAugmentation(size_t number_of_variations, unsigned int seed = 42)
        : m_number_of_variations(number_of_variations), m_gen(seed){}

    std::vector<std::shared_ptr<Scene>> process(const std::vector<std::shared_ptr<Scene>>& input_scenes, AssetManager& assets) override;
};

class Pipeline {
private:
    std::vector<std::shared_ptr<PipelineStep>> m_steps;

public:
    void add_step(std::shared_ptr<PipelineStep> step);

    std::vector<std::shared_ptr<Scene>> execute(std::shared_ptr<Scene> input_scene, AssetManager& assets);
};

#endif