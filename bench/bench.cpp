
#include <benchmark/benchmark.h>
#include "../include/strategy.hpp"
#include "../strategies/spec_strategy.cpp"
#include <vector>
#include <numeric>
#include<memory_resource>
#include <array>

// 1. The Benchmark Function Signature
static void BM_TickTest(benchmark::State& state) {
    // 2. Setup Phase (NOT timed)
    
    SpecStrategy st;
    st.on_init();

    // 2. Pre-allocate the benchmark arena
    std::array<std::byte, 1024> buffer;
    std::pmr::monotonic_buffer_resource arena(
        buffer.data(), 
        buffer.size(), 
        std::pmr::null_memory_resource()
    );
    // 3. The Timing Loop
    for (auto _ : state) {
    // ONLY the code inside this loop is measured for time.
        // Reset memory pointer instantly, exactly as the engine does
        arena.release();
        csot::Tick tick{1700000000000000000,"SYM0",99.9921,100.0021,474,483};
        auto result {st.on_tick(tick,&arena)};
    // 4. The Compiler Defense
        benchmark::DoNotOptimize(result);
    }
    // 5. Throughput Calculation
    state.SetItemsProcessed(state.iterations() *1);
}
//6. Registration
BENCHMARK(BM_TickTest);
// 7. Auto-generated Main
BENCHMARK_MAIN();