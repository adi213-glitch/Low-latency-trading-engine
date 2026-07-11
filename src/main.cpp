#include "../include/engine.hpp"

/*Because null_strategy.so is an external shared library, 
you cannot just #include "null_strategy.cpp". 
You must ask the Linux operating system to load it into memory while the program is running.*/
#include <dlfcn.h>
#include <iostream>
#include <vector>

int main(int argc, char* argv[]) {
    // main function needs to accept inputs so you can pass the strategy and the data file via the terminal.
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <strategy.so> <ticks.csv>\n";
        return 1;
    }
    const char* strategy_path = argv[1];
    const char* csv_path = argv[2];
    /*Inside main, you will use three functions from <dlfcn.h>:
        dlopen(): Opens the .so file.
        dlsym(): Looks up the memory address of a specific symbol (in our case, "create_strategy").
        dlerror(): Fetches the error string if something fails.*/

    // 1. Open the shared object
    void* handle = dlopen(strategy_path, RTLD_NOW);
    if (!handle) {
        std::cerr << "Failed to load strategy: " << dlerror() << "\n";
        return 1;
    }

    // 2. Define the function pointer signature
    using CreateStrategyFn = csot::Strategy* (*)();

    // 3. Extract the symbol and cast it
    CreateStrategyFn create = reinterpret_cast<CreateStrategyFn>(dlsym(handle, "create_strategy"));
    if (!create) {
        std::cerr << "Failed to find create_strategy: " << dlerror() << "\n";
        dlclose(handle);
        return 1;
    }

    // 4. Instantiate the strategy!
    csot::Strategy* strategy = create();


    // start the engine
    Engine engine;
    
    std::cout << "Loading ticks...\n";
    std::vector<csot::Tick> ticks = engine.load_ticks(csv_path);

    if (ticks.empty()) {
        std::cerr << "No ticks loaded!\n";
        return 1;
    }

    std::cout << "Successfully loaded " << ticks.size() << " ticks.\n\n";

    engine.run(strategy,ticks);
    // free up the memory and close handle of .so lib
    delete strategy;
    dlclose(handle);
    return 0;
}