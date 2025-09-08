import pandas as pd
import matplotlib.pyplot as plt

def main():
    try:
        df = pd.read_csv('battery_simulation.csv')
    except FileNotFoundError:
        print("❌ battery_simulation.csv not found. Run 'make run' first.")
        return

    t = df['time']
    V = df['voltage']
    soc = df['soc']
    h = df['hysteresis']

    plt.figure(figsize=(14, 10))

    plt.subplot(3, 1, 1)
    plt.plot(t, V, 'b-', linewidth=1.5)
    plt.ylabel('Terminal Voltage (V)')
    plt.grid(True)
    plt.title('LFP Battery Simulation: Charge → Rest → Discharge')

    plt.subplot(3, 1, 2)
    plt.plot(t, soc, 'g-', linewidth=1.5)
    plt.ylabel('SOC')
    plt.grid(True)

    plt.subplot(3, 1, 3)
    plt.plot(t, h, 'r-', linewidth=1.5)
    plt.ylabel('Hysteresis State (h)')
    plt.xlabel('Time (s)')
    plt.grid(True)

    plt.tight_layout()
    plt.savefig('simulation_plot.png', dpi=150)
    plt.show()

    print("✅ Plot generated: simulation_plot.png")

if __name__ == "__main__":
    main()