# =====================================================================
# LFP Battery Model — Makefile with Plotting
# =====================================================================

# Compiler
CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I.

# Libraries
LDFLAGS = -lm

# Source files
SRCS = main.cpp LfpBatteryModel.cpp
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = battery_sim

# Plot script
PLOT_SCRIPT = plot_results.py

# Default target
all: $(TARGET)

# Link objects into executable
$(TARGET): $(OBJS)
	$(CXX) $(OBJS) $(LDFLAGS) -o $(TARGET)

# Compile .cpp to .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Run the simulation
run: $(TARGET)
	./$(TARGET)

# Plot results using Python
plot:
	@echo "Checking Python dependencies..."
	@python -c "import pandas, matplotlib" 2>/dev/null || \
		(echo "❌ Error: Required Python packages not found." && \
		 echo "👉 Install with: pip install pandas matplotlib" && \
		 exit 1)
	@echo "✅ Dependencies OK. Generating plot..."
	@python $(PLOT_SCRIPT)
	@echo "📈 Plot saved as 'simulation_plot.png'"

# Run simulation and plot
run-plot: run plot

# Clean build artifacts
clean:
	rm -f battery_simulation.csv simulation_plot.png battery_sim.exe LfpBatteryModel.o main.o

# Very clean
clean-all: clean

# Phony targets
.PHONY: all clean clean-all run plot run-plot