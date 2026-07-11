#include "../include/strategy.hpp"

class NullStrategy : public csot::Strategy{
public:
    virtual void on_init() {}
    std::vector<csot::Order> on_tick(const csot::Tick& ) override {return {};}
    virtual void on_fill(const csot::Order& o,
                         double        fill_price,
                         uint32_t      fill_qty) {
        (void)o; (void)fill_price; (void)fill_qty;
    }
};

extern "C" csot::Strategy* create_strategy() {
    return new NullStrategy{};
}