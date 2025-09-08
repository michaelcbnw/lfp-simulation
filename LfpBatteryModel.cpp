#include "LfpBatteryModel.hpp"
#include <stdexcept>
#include <cmath>

LfpBatteryModel::LfpBatteryModel(double Q_max, double T_ref)
    : Q_max(Q_max), T_ref(T_ref) {

    // Original descending SOC: 100, 95, ..., 0
    std::vector<double> soc_desc = {
        100, 95, 90, 85, 80, 75, 70, 65, 60, 55,
        50, 45, 40, 35, 30, 25, 20, 15, 10, 5, 0
    };

    // Flip to ascending for interpolation
    soc_levels = soc_desc;
    std::reverse(soc_levels.begin(), soc_levels.end());

    temps = {-10, 0, 10, 15, 25, 35, 45};

    // Original table (rows = SOC 100% → 0%)
    std::vector<std::vector<double>> ocv_desc = {
        {3.361, 3.335, 3.340, 3.352, 3.375, 3.354, 3.334}, // SOC=100%
        {3.320, 3.319, 3.325, 3.326, 3.329, 3.330, 3.331},
        {3.309, 3.318, 3.324, 3.325, 3.328, 3.329, 3.331},
        {3.309, 3.318, 3.324, 3.325, 3.328, 3.329, 3.331},
        {3.309, 3.318, 3.324, 3.325, 3.328, 3.329, 3.331},
        {3.309, 3.318, 3.324, 3.325, 3.328, 3.329, 3.331},
        {3.304, 3.314, 3.323, 3.324, 3.327, 3.329, 3.330},
        {3.293, 3.304, 3.316, 3.319, 3.324, 3.325, 3.327},
        {3.292, 3.293, 3.298, 3.302, 3.310, 3.306, 3.301},
        {3.285, 3.285, 3.288, 3.290, 3.294, 3.295, 3.297},
        {3.280, 3.282, 3.285, 3.287, 3.290, 3.293, 3.296},
        {3.277, 3.280, 3.284, 3.285, 3.289, 3.292, 3.295},
        {3.276, 3.279, 3.283, 3.285, 3.288, 3.291, 3.294},
        {3.274, 3.279, 3.282, 3.284, 3.288, 3.290, 3.293},
        {3.273, 3.277, 3.280, 3.281, 3.284, 3.280, 3.277},
        {3.272, 3.273, 3.272, 3.272, 3.272, 3.267, 3.261},
        {3.270, 3.264, 3.258, 3.256, 3.253, 3.248, 3.242},
        {3.266, 3.250, 3.236, 3.234, 3.230, 3.223, 3.217},
        {3.259, 3.230, 3.215, 3.213, 3.210, 3.205, 3.201},
        {3.246, 3.209, 3.188, 3.186, 3.180, 3.149, 3.117},
        {3.227, 3.175, 3.072, 3.024, 2.928, 2.830, 2.732}  // SOC=0%
    };

    // Flip rows to match ascending SOC
    ocv_table = ocv_desc;
    std::reverse(ocv_table.begin(), ocv_table.end());

    reset();
}

void LfpBatteryModel::reset() {
    state.soc = 1.0;  // 100%
    state.h = 0.0;
    state.V1 = 0.0;
    state.V2 = 0.0;
    state.V3 = 0.0;
}

double LfpBatteryModel::H_mag(double soc) const {
    double soc_percent = soc * 100.0;
    if (soc_percent < 10.0 || soc_percent > 90.0) {
        return 0.0;
    } else {
        return 0.015 * sin(PI * (soc_percent - 10.0) / 80.0);
    }
}

void LfpBatteryModel::updateParameters(double T, double& R0, double& R1, double& C1,
                                       double& R2, double& C2, double& R3, double& C3) const {
    double dT = T - T_ref;
    R0 = R0_ref * (1.0 + alpha_R * dT);
    R1 = R1_ref * (1.0 + alpha_R * dT);
    C1 = C1_ref * (1.0 + beta_C * dT);
    R2 = R2_ref * (1.0 + alpha_R * dT);
    C2 = C2_ref * (1.0 + beta_C * dT);
    R3 = R3_ref * (1.0 + alpha_R * dT);
    C3 = C3_ref * (1.0 + beta_C * dT);
}

double LfpBatteryModel::interpolateOcv(double soc_percent, double T) const {
    // Clamp inputs to table bounds
    soc_percent = std::max(soc_levels.front(), std::min(soc_percent, soc_levels.back()));
    T = std::max(temps.front(), std::min(T, temps.back()));

    // Find SOC indices
    size_t i_soc_hi = 0;
    while (i_soc_hi < soc_levels.size() && soc_levels[i_soc_hi] < soc_percent) {
        i_soc_hi++;
    }
    if (i_soc_hi == 0) {
        i_soc_hi = 1; // Force to use first two points if at boundary
    }
    size_t i_soc_lo = i_soc_hi - 1;

    // Find Temp indices
    size_t i_temp_hi = 0;
    while (i_temp_hi < temps.size() && temps[i_temp_hi] < T) {
        i_temp_hi++;
    }
    if (i_temp_hi == 0) {
        i_temp_hi = 1;
    }
    size_t i_temp_lo = i_temp_hi - 1;

    // Get corner values
    double v00 = ocv_table[i_soc_lo][i_temp_lo];
    double v01 = ocv_table[i_soc_lo][i_temp_hi];
    double v10 = ocv_table[i_soc_hi][i_temp_lo];
    double v11 = ocv_table[i_soc_hi][i_temp_hi];

    // Interpolation weights
    double soc_lo = soc_levels[i_soc_lo];
    double soc_hi = soc_levels[i_soc_hi];
    double temp_lo = temps[i_temp_lo];
    double temp_hi = temps[i_temp_hi];

    double wx = (soc_hi == soc_lo) ? 0.0 : (soc_percent - soc_lo) / (soc_hi - soc_lo);
    double wy = (temp_hi == temp_lo) ? 0.0 : (T - temp_lo) / (temp_hi - temp_lo);

    // Bilinear interpolation
    double v0 = v00 + wx * (v10 - v00);
    double v1 = v01 + wx * (v11 - v01);
    double v = v0 + wy * (v1 - v0);

    return v;
}

double LfpBatteryModel::step(double I, double T, double dt) {
    // 1. Update hysteresis
    double dhdt;
    if (std::abs(I) > 1e-6) {
        double target_h = (I > 0) ? -1.0 : 1.0; // discharge → -1, charge → +1
        dhdt = (target_h - state.h) / tau_h_charge;
    } else {
        dhdt = -state.h / tau_h_rest;
    }

    state.h += dhdt * dt;
    state.h = std::clamp(state.h, -1.0, 1.0);

    // 2. Update SOC
    double dSOC = -I * dt / (Q_max * 3600.0); // A·s → Ah
    state.soc = std::clamp(state.soc + dSOC, 0.0, 1.0);

    // 3. Update parameters
    double R0, R1, C1, R2, C2, R3, C3;
    updateParameters(T, R0, R1, C1, R2, C2, R3, C3);

    // 4. Update RC voltages
    double dV1dt = -state.V1 / (R1 * C1) + I / C1;
    double dV2dt = -state.V2 / (R2 * C2) + I / C2;
    double dV3dt = -state.V3 / (R3 * C3) + I / C3;

    state.V1 += dV1dt * dt;
    state.V2 += dV2dt * dt;
    state.V3 += dV3dt * dt;

    // 5. Compute OCV
    double ocv_base = interpolateOcv(state.soc * 100.0, T);
    double ocv_hyst = state.h * H_mag(state.soc);
    double ocv = ocv_base + ocv_hyst;

    // 6. Terminal voltage
    double V_terminal = ocv + I * R0 + state.V1 + state.V2 + state.V3;

    return V_terminal;
}