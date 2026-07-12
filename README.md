# CSoT'26 Low Latency — Week 1 Project

## Overview
This repository contains a high-performance C++ execution engine and a deterministic mean-reversion quantitative trading strategy. The architecture separates the engine and the strategy using a dynamic linker (`.so`), features a zero-allocation hot path, utilizes a circular ring-buffer for O(1) state updates, and leverages native hardware cycle counters (`__rdtsc`) to eliminate OS measurement overhead.

## Build Instructions

The project uses CMake and requires a C++20 compliant compiler.

**1. Configure the Build System (Release Mode)**
```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
```

**2. Compile the Engine and Strategies**
```bash
cmake --build build -j
```

**3. Run the Full Engine**
```bash
./build/quant_runner ./build/spec_strategy.so data/synthetic_large.csv
```

**4. Run Isolated Microbenchmarks**
```bash
./build/quant_bench
```

---

## Hardware & Environment

* **CPU:** 12-Core Processor @ 2.68 GHz (L1d: 48 KiB, L1i: 32 KiB, L2: 1.25 MB, L3: 12 MB)
* **OS:** Ubuntu (WSL2 environment)
* **Compiler:** GNU GCC 13.3.0
* **Build Flags:** `-O2`, `-march=native`, `-flto` (Link-Time Optimization enabled)

---

## Performance Metrics

### Headline Engine Latency (10,000,000 Ticks)
Measured on the `synthetic_large.csv` dataset, capturing the full pipeline (CSV routing, symbol lookup, algorithmic execution, and order processing) using `__rdtsc`. 

* **Total Processed:** 10,000,000 ticks
* **Median (p50):** <= 32 ns
* **90th Percentile (p90):** <= 32 ns
* **99th Percentile (p99):** <= 64 ns
* **99.9th Percentile (p999):** <= 64 ns

### Isolated Strategy Microbenchmark (Google Benchmark)
Measured using Google Benchmark to capture the pure algorithmic execution of `on_tick` without file I/O or engine overhead.

* **Execution Time (Per tick iteration):** 4.3 ns
* **Throughput:** 221.567 Million items/second

---

## Correctness Validation

The emitted order stream has been strictly validated against the reference specification. The strategy perfectly matches the judge's deterministic output for the following datasets:
* `tiny.csv` (Basic logic and order bounds)
* `synthetic_small.csv` (10,000 tick sanity and memory leak check)
* `public.csv` (Official CSoT leaderboard gate check)

---

## Profiling & Hardware Counters

High-level `perf stat` diagnostics captured during a full run on the 10-million tick dataset to verify baseline OS interactions and thread utilization:

```text
$ perf stat -d ./build/quant_runner ./build/spec_strategy.so data/synthetic_large.csv

Loading ticks...
Successfully loaded 10000000 ticks.
10000000 ticks were processed.

 Performance counter stats for './build/quant_runner ./build/spec_strategy.so data/synthetic_large.csv':

       6231.21 msec task-clock:u               #    0.998 CPUs utilized              
             0      context-switches:u         #    0.000 /sec                       
             0      cpu-migrations:u           #    0.000 /sec                       
        313952      page-faults:u              #   50.384 K/sec                      

       6.242361285 seconds time elapsed

       5.436187000 seconds user
       0.795442000 seconds sys
```