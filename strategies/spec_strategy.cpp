#include "../include/strategy.hpp"
#include <array>
#include <cmath>
#include <iostream>
#include <unordered_map>
#include <memory_resource>
#include <vector>
constexpr uint32_t WINDOW {64};
constexpr double WINDOW_D {64.0};
constexpr float ENTRY_Z{2.0f};
constexpr float EXIT_Z{0.5f};
constexpr uint32_t ORDER_QTY{1};
constexpr uint32_t MAX_ABS_POSITION{1};
constexpr double EPSILON_STDDEV{1e-9};
constexpr std::string_view START_SYMBOL{"SYM0"};

constexpr double ENTRY_Z_SQ { ENTRY_Z * ENTRY_Z};
constexpr double EXIT_Z_SQ {EXIT_Z * EXIT_Z};
constexpr double EPSILON_VAR {EPSILON_STDDEV * EPSILON_STDDEV};
constexpr double WINDOW_INV {1.0/64.0};
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
    std::pmr::vector<csot::Order> on_tick(const csot::Tick& t,std::pmr::memory_resource* arena) override{
        // 1.per tick algorithm
        int symbol_idx = parse_symbol_as_int(t.symbol);
        SymbolState& s {symbols[symbol_idx]};
        double mid = (t.bid_px + t.ask_px) * 0.5;

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
        if(s.count < WINDOW) return std::pmr::vector<csot::Order> (arena);;
        // The tick that fills the 64th entry is eligible for trading. In other words, append first, then compute.

        // 4.Compute mean and standard deviation
        double mean { s.sum * WINDOW_INV}; // Fast multiplication with WINDOW_INV instead of slow division by 64.0

        double variance {(s.sqsum * WINDOW_INV) - (mean*mean)};
        
        // double stddev {std::sqrt(variance)}; erase this to optimise math

        // if(stddev < EPSILON_STDDEV) return orders;
        // this above line is replaced by below :
        if(variance < EPSILON_VAR) return std::pmr::vector<csot::Order> (arena);;

        // Step 5
        // Compute z-score (erased)
        // double z { (mid - mean) / stddev};

        // instead Compute the raw difference and its square
        double diff = mid - mean;
        double diff_sq = diff * diff;

        // step 6 : entry logic
        
        if(s.position==0) {
            if (diff >= 0 && (diff_sq >= ENTRY_Z_SQ * variance)){
                // construct the vector only when we trade
                std::pmr::vector<csot::Order> orders(arena);
                orders.emplace_back(csot::Order::Side::SELL,t.symbol,t.bid_px,1);
                return orders;
            }else if (diff < 0 && (diff_sq >= ENTRY_Z_SQ * variance)){
                std::pmr::vector<csot::Order> orders(arena);
                orders.emplace_back(csot::Order::Side::BUY,t.symbol,t.ask_px,1); 
                return orders;
            }
        }// step 7 : exit logic 
        else {
            if (diff_sq <= EXIT_Z_SQ * variance) {
                std::pmr::vector<csot::Order> orders(arena);
                if (s.position > 0) {
                    orders.emplace_back(csot::Order::Side::SELL, t.symbol, t.bid_px, static_cast<uint32_t>(s.position));
                } else {
                    orders.emplace_back(csot::Order::Side::BUY, t.symbol, t.ask_px, static_cast<uint32_t>(-s.position));
                }
                return orders;
            }
        }
        // this returns empty vector
        return std::pmr::vector<csot::Order> (arena);
        
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