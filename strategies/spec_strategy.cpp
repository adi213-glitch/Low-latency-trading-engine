#include "../include/strategy.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <unordered_map>
constexpr uint32_t WINDOW {64};
constexpr double WINDOW_D {64.0};
constexpr float ENTRY_Z{2.0f};
constexpr float EXIT_Z{0.5f};
constexpr uint32_t ORDER_QTY{1};
constexpr uint32_t MAX_ABS_POSITION{1};
constexpr double EPSILON_STDDEV{1e-9};
constexpr std::string_view START_SYMBOL{"SYM0"};



class SpecStrategy : public csot::Strategy{ 
private : 
    struct SymbolState {
        double mids[WINDOW]{};      // rolling mid-price window
        double sum{};
        double sqsum{};
        uint32_t count{};       // number of valid entries seen so far, capped at 64
        uint32_t head{};        // index where the next mid-price will be written
        int32_t position{};     // -1, 0, or +1
    };
    std::array <SymbolState, 64> symbols ;
public:
    int parse_symbol_as_int (std::string_view s){
        return  (s.size() == 4) ? (s[3] - '0') : (10 * (s[3] - '0') + (s[4] - '0'));
    }
    void on_init() override {
        
    }
    std::vector<csot::Order> on_tick(const csot::Tick& t) override{
        // 1.per tick algorithm
        int symbol_idx = parse_symbol_as_int(t.symbol);
        SymbolState& s {symbols[symbol_idx]};
        double mid = (t.bid_px + t.ask_px) / 2.0;

        //2.add to rolling window
        
        //remove the current value at head from sum and sqsum and add new mid to the rolling counters
        double val {s.mids[s.head]};
        s.sum+= (mid -val);
        s.sqsum +=((mid*mid)-(val*val));
        // now continue with window update
        s.mids[s.head]=mid;
        // 63 in binary is 00111111. 
        // This wraps head perfectly back to 0 without division.
        s.head = (s.head + 1) & 63;
        s.count = std::min(s.count+1, WINDOW);

        // 3.warmup period
        if(s.count < WINDOW) return {};
        // The tick that fills the 64th entry is eligible for trading. In other words, append first, then compute.

        // 4.Compute mean and standard deviation
        double mean { s.sum/WINDOW_D};

        double variance {(s.sqsum/WINDOW_D) - (mean*mean)};
        
        double stddev {std::sqrt(variance)};

        if(stddev < EPSILON_STDDEV) return {};
        //Step 5 — Compute z-score
        double z { (mid - mean) / stddev};

        // step 6 : entry logic

        if(s.position==0) {
            if(z>=+ENTRY_Z) return {csot::Order{csot::Order::Side::SELL,t.symbol,t.bid_px,1}};
            else if(z<=-ENTRY_Z) return {csot::Order{csot::Order::Side::BUY,t.symbol,t.ask_px,1}};
            else return {};
        }

        // step 7 : exit logic 
        if(s.position > 0 && std::abs(z) <= EXIT_Z) 
            return {csot::Order{csot::Order::Side::SELL,t.symbol,t.bid_px,static_cast<uint32_t>(s.position)}};

        else if(s.position < 0 && std::abs(z) <= EXIT_Z) 
            return {csot::Order{csot::Order::Side::BUY,t.symbol,t.ask_px,static_cast<uint32_t>(-s.position)}};
        
        else return{};
    }


    void on_fill(const csot::Order& o,
            double        fill_price,
            uint32_t      fill_qty) override  {

        (void)fill_price; // Explicitly silence the unused parameter warning
        SymbolState& s {symbols[parse_symbol_as_int(o.symbol)]};
        switch(o.side){
            case csot::Order::Side::BUY :
                s.position+=static_cast<int32_t>(fill_qty);
                break;
            default:
                s.position-=static_cast<int32_t>(fill_qty);
                break;
        }
    }
};

extern "C" csot::Strategy* create_strategy(){
    return new SpecStrategy{};
}