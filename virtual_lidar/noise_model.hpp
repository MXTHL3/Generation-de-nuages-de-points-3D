#ifndef NOISE_MODEL_HPP
#define NOISE_MODEL_HPP

#include <random>
#include <cmath>
#include <nlohmann/json.hpp>

struct NoiseStep{
    double max_distance; // <= x
    double sigma; // écart type du bruit gaussien
};

struct NoiseProfile{
    std::vector<NoiseStep> steps;
    double resolution = 0.0;
};


class NoiseModel {
public:
    NoiseModel();
    virtual ~NoiseModel() = default;
    NoiseModel(const NoiseProfile& profile, unsigned int seed = 42);

    void profile(const NoiseProfile& profile);
    const NoiseProfile& profile() const;
    // Applique le bruit a la distance initiale
    double apply(double real_distance)const;

private:
    double find_sigma(double distance) const;

    NoiseProfile m_profile;
    mutable std::mt19937 m_gen;
};

/*
class NullNoiseModel : public NoiseModel {
public :
    double apply(double real_distance) override{
        return real_distance;
    }
};

class OusterOS2Noise : public NoiseModel {
private:
    std::mt19937_64 m_gen;
    double m_resolution = 0.003;

public:
    OusterOS2Noise() : m_gen(std::random_device{}()) {}

    double apply(double real_distance) override;
};*/

#endif