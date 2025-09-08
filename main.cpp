#include "LfpBatteryModel.hpp"
#include <iostream>
#include <fstream>
#include <vector>

int main() {
    LfpBatteryModel battery(52.0); // 5Ah cell
    double dt = 1.0;              // 1 second step
    double T = 25.0;              // 25°C

    std::vector<double> time, voltage, soc, hysteresis;

    std::cout << "Simulating...\n";

    // Charge at 1A for 1 hour
    for (int t = 0; t < 3600; ++t) {
        double V = battery.step(-26.0, T, dt); // charge
        time.push_back(t);
        voltage.push_back(V);
        soc.push_back(battery.getState().soc);
        hysteresis.push_back(battery.getState().h);
    }

    // Rest for 1 hour
    for (int t = 3600; t < 7200; ++t) {
        double V = battery.step(0.0, T, dt); // rest
        time.push_back(t);
        voltage.push_back(V);
        soc.push_back(battery.getState().soc);
        hysteresis.push_back(battery.getState().h);
    }

    // // Discharge at 2A for 1 hour
    // for (int t = 7200; t < 10800; ++t) {
    //     double V = battery.step(-2.0, T, dt); // discharge
    //     time.push_back(t);
    //     voltage.push_back(V);
    //     soc.push_back(battery.getState().soc);
    //     hysteresis.push_back(battery.getState().h);
    // }

    // Save to CSV
    std::ofstream file("battery_simulation.csv");
    file << "time,voltage,soc,hysteresis\n";
    for (size_t i = 0; i < time.size(); ++i) {
        file << time[i] << "," << voltage[i] << "," << soc[i] << "," << hysteresis[i] << "\n";
    }
    file.close();

    std::cout << "Simulation complete. Data saved to 'battery_simulation.csv'\n";
    return 0;
}