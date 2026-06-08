#ifndef HORIZONTAL_STEP_HPP
#define HORIZONTAL_STEP_HPP

#include <nlohmann/json.hpp>
#include <string>

class HorizontalStepSource {
public:
    virtual ~HorizontalStepSource()= default;

    virtual double compute_h_step() const = 0;

    virtual void serialize(nlohmann::json& data) const = 0;
};

class ColumnCountSource : public HorizontalStepSource{
public:
    ColumnCountSource(unsigned int columns);
    
    double compute_h_step() const override;

    void serialize(nlohmann::json& data) const override;

private:
    int m_columns;
};

class DirectResolutionSource : public HorizontalStepSource{
public:
    DirectResolutionSource(double resolution_deg);

    double compute_h_step() const override;

    void serialize(nlohmann::json& data) const override;

private:
    double m_resolution;
};

class PointsPerSecondSource : public HorizontalStepSource{
public:
    PointsPerSecondSource(unsigned int points_per_second, double rotation_rotate, size_t n_lasers);

    double compute_h_step() const override;

    void serialize(nlohmann::json& data) const override;

private:
    unsigned int m_points_per_second;
    double m_rotation_rotate;
    size_t m_n_lasers;
};

#endif