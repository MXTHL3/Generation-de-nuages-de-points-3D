#ifndef NOISE_MODEL_HPP
#define NOISE_MODEL_HPP

#include <random>
#include <cmath>

class NoiseModel {
public:
    virtual ~NoiseModel() = default;
    // Applique le bruit a la distance initiale
    virtual double apply(double real_distance) = 0;
};

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
};

#endif