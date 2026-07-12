
#include <benchmark/benchmark.h>
#include "../include/strategy.hpp"
#include "../strategies/spec_strategy.cpp"
#include <vector>
#include <numeric>
// 1. The Benchmark Function Signature
static void BM_TickTest(benchmark::State& state) {
    // 2. Setup Phase (NOT timed)
    
    SpecStrategy st;
    st.on_init();
    // 3. The Timing Loop
    for (auto _ : state) {
    // ONLY the code inside this loop is measured for time.
        csot::Tick tick{1700000000000000000,"SYM0",99.9921,100.0021,474,483};
        auto result {st.on_tick(tick)};
    // 4. The Compiler Defense
        benchmark::DoNotOptimize(result);
    }
    // 5. Throughput Calculation
    state.SetItemsProcessed(state.iterations() *1);
}
//6. Registration
BENCHMARK(BM_TickTest);
// 7. Auto-generated Main








// ====================================================================
// Benchmark 1: The String Parser
// ====================================================================
static void BM_ParseSymbol(benchmark::State& state) {
    std::string_view sym{"SYM42"}; // Test the worst-case (2-digit) path
    
    for (auto _ : state) {
        int idx = (sym.size() == 4) ? (sym[3] - '0') : (10 * (sym[3] - '0') + (sym[4] - '0'));
        
        // Force compiler to compute idx
        benchmark::DoNotOptimize(idx); 
    }
}
BENCHMARK(BM_ParseSymbol);

// ====================================================================
// Benchmark 2: Ring Buffer Append
// ====================================================================
static void BM_RingBuffer(benchmark::State& state) {
    double mids[64]{};
    uint32_t head = 0;
    uint32_t count = 0;
    double dummy_mid = 150.5;
    double sum {10000.0} , sqsum {10000.0};
    for (auto _ : state) {

        double val {mids[head]};
        sum+= (dummy_mid -val);
        sqsum +=((dummy_mid*dummy_mid)-(val*val));
        mids[head] = dummy_mid;
        head = (head + 1) & 63;
        count = std::min(count + 1, 64u);
        
        // Clobber memory so the compiler doesn't realize we are just
        // overwriting the same array repeatedly in a loop.
        benchmark::DoNotOptimize(mids);
        benchmark::DoNotOptimize(head);
        benchmark::DoNotOptimize(count);
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_RingBuffer);

// ====================================================================
// Benchmark 3: Rolling Math (Mean, Variance, StdDev)
// ====================================================================
static void BM_StatsMath(benchmark::State& state) {
    double sum {10000.0} , sqsum {10000.0};

    for (auto _ : state) {
        
        double mean { sum/64.0};

        double variance {(sqsum/64.0) - (mean*mean)};
        
        double stddev {std::sqrt(variance)};

        // Force compiler to materialize the final standard deviation
        benchmark::DoNotOptimize(stddev);
    }
}
BENCHMARK(BM_StatsMath);

BENCHMARK_MAIN();