#include "../include/strategy.hpp"

class NullStrategy : public csot::Strategy{
public:
    virtual void on_init() {}
    std::pmr::vector<csot::Order> on_tick(const csot::Tick& ,std::pmr::memory_resource *) override {return {};}
    virtual void on_fill(const csot::Order& o,
                         double        fill_price,
                         uint32_t      fill_qty) {
        (void)o; (void)fill_price; (void)fill_qty;
    }
};


/*This snippet defines a factory function that creates a strategy object for dynamic loading. 
The return type is csot::Strategy*, which indicates the caller receives a pointer to the base strategy interface, not the concrete class. 
That enables polymorphism: the engine can work with any strategy implementation through the same interface.

extern "C" is the key interoperability detail. 
It disables C++ name mangling for this function, so the exported symbol name is predictable (create_strategy).
A host application can then find it reliably with APIs like "dlsym" when loading a shared library at runtime.

Inside the function, new NullStrategy{} allocates a NullStrategy on the heap and returns it as a csot::Strategy*.
This is a common plugin pattern: the plugin owns construction, while the host typically owns destruction (often via delete or a paired destroy function).
The main gotcha is lifetime management—whoever receives this pointer must free it correctly to avoid leaks.*/
extern "C" csot::Strategy* create_strategy() {
    return new NullStrategy{};
}