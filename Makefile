CXX = g++
CXXFLAGS = -std=c++17 -O3 -Wall -Wextra
TARGET = reconstruction_aditya
SOURCE = orderbook_reconstruction.cpp

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(SOURCE)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SOURCE)

# Clean build artifacts
clean:
	rm -f $(TARGET) *.o

# Run with sample data
test: $(TARGET)
	./$(TARGET) mbo.csv
	@echo "Comparing output with expected result..."
	@if diff -q mbp_output.csv mbp.csv > /dev/null; then \
		echo "✓ Output matches expected result"; \
	else \
		echo "✗ Output differs from expected result"; \
		echo "First few lines of output:"; \
		head -10 mbp_output.csv; \
		echo "First few lines of expected:"; \
		head -10 mbp.csv; \
	fi

# Performance test
perf: $(TARGET)
	@echo "Running performance test..."
	@time ./$(TARGET) mbo.csv

# Install dependencies (if needed)
install:
	@echo "No external dependencies required for this project"

.PHONY: all clean test perf install 