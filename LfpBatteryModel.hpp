#pragma once
#include <vector>
#include <array>
#include <cmath>
#include <algorithm>
#include <iostream>

// Portable PI constant
static constexpr double PI = 3.14159265358979323846;

class LfpBatteryModel {
public:
    struct State {
        double soc;   // 0.0 - 1.0
        double h;     // hysteresis state [-1, 1]
        double V1;    // RC1 voltage
        double V2;    // RC2 voltage
        double V3;    // RC3 voltage (slow relaxation)
    };

    LfpBatteryModel(double Q_max = 5.0, double T_ref = 25.0);

    // Simulate one time step
    double step(double I, double T, double dt);

    // Get current state
    const State& getState() const { return state; }

    // Reset state
    void reset();

private:
    // OCV lookup table
    std::vector<double> soc_levels;   // ascending: 0, 5, 10, ..., 100
    std::vector<double> temps;        // ascending: -10, 0, 10, ..., 45
    std::vector<std::vector<double>> ocv_table; // [soc_index][temp_index]

    // Parameters
    double Q_max;
    double T_ref;

    // Hysteresis
    double tau_h_charge = 600.0;   // 10 minutes
    double tau_h_rest = 3600.0;    // 1 hour

    // ECM @ T_ref
    double R0_ref = 0.002;
    double R1_ref = 0.003;
    double C1_ref = 5000.0;
    double R2_ref = 0.008;
    double C2_ref = 1000.0;
    double R3_ref = 0.02;
    double C3_ref = 50000.0;

    // Temp coefficients
    double alpha_R = 0.01;
    double beta_C = -0.005;

    // State
    State state;

    // Interpolation
    double interpolateOcv(double soc_percent, double T) const;

    // Hysteresis magnitude
    double H_mag(double soc) const;

    // Update R and C with temperature
    void updateParameters(double T, double& R0, double& R1, double& C1,
                          double& R2, double& C2, double& R3, double& C3) const;
};