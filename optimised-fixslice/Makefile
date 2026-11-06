# BAKSHEESH-GIFT Fixslice Makefile

CC = gcc
CFLAGS = -std=c99 -O3 -march=native -mtune=native -Wall -Wextra
LDFLAGS = -lm

# Source files
SOURCES = baksheesh_gift_fixslice.c
HEADERS = baksheesh_gift_fixslice.h

# Target binaries
CORRECTNESS = correctness_test
BENCHMARK   = comprehensive_benchmark

.PHONY: all clean test benchmark help

all: $(CORRECTNESS) $(BENCHMARK)

# Known-answer correctness test (GIFT heritage)
$(CORRECTNESS): correctness_test.c $(SOURCES) $(HEADERS)
	@echo "Building BAKSHEESH correctness test..."
	$(CC) $(CFLAGS) -o $@ correctness_test.c $(SOURCES) $(LDFLAGS)

# Throughput benchmark
$(BENCHMARK): comprehensive_benchmark.c $(SOURCES) $(HEADERS)
	@echo "Building BAKSHEESH benchmark..."
	$(CC) $(CFLAGS) -o $@ comprehensive_benchmark.c $(SOURCES) $(LDFLAGS)

# Run the known-answer correctness test
test: $(CORRECTNESS)
	@echo "=== Running BAKSHEESH Known-Answer Tests ==="
	./$(CORRECTNESS)

# Run the throughput benchmark
benchmark: $(BENCHMARK)
	@echo "=== Running BAKSHEESH Benchmark ==="
	./$(BENCHMARK)

help:
	@echo "BAKSHEESH-GIFT Fixslice Makefile"
	@echo ""
	@echo "Available targets:"
	@echo "  all        - Build the correctness test and benchmark"
	@echo "  test       - Run the known-answer correctness test"
	@echo "  benchmark  - Run the throughput benchmark"
	@echo "  clean      - Remove all built binaries"
	@echo "  help       - Show this help message"

clean:
	@echo "Cleaning build artifacts..."
	rm -f $(CORRECTNESS) $(BENCHMARK) *.o
	@echo "Clean complete"
