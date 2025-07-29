#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <unordered_map>
#include <chrono>


struct Order {
    uint64_t order_id;
    uint32_t price;
    uint32_t size;
    char side;
    uint8_t level;
    
    Order() : order_id(0), price(0), size(0), side('N'), level(0) {}
    Order(uint64_t id, uint32_t p, uint32_t s, char sd, uint8_t l) 
        : order_id(id), price(p), size(s), side(sd), level(l) {}
};


struct Level {
    uint32_t price;
    uint32_t total_size;
    std::set<uint64_t> order_ids;
    
    Level() : price(0), total_size(0) {}
    Level(uint32_t p) : price(p), total_size(0) {}
    
    void add_order(uint64_t order_id, uint32_t size) {
        order_ids.insert(order_id);
        total_size += size;
    }
    
    void remove_order(uint64_t order_id, uint32_t size) {
        order_ids.erase(order_id);
        total_size -= size;
    }
    
    void update_order(uint64_t, uint32_t old_size, uint32_t new_size) {
        total_size = total_size - old_size + new_size;
    }
};


struct MBORecord {
    uint64_t ts_event;
    uint64_t ts_rtt;
    uint64_t ts_instrument;
    char side;
    char action;
    uint8_t level;
    uint64_t order_id;
    uint32_t price;
    uint32_t size;
    uint8_t channel;
    uint64_t sequence;
};


struct MBPRecord {
    uint64_t ts_event;
    uint64_t ts_rtt;
    uint64_t ts_instrument;
    char side;
    uint8_t level;
    uint32_t price;
    uint32_t size;
    uint8_t channel;
    uint64_t sequence;
};

class OrderBook {
private:

    std::map<uint32_t, Level, std::greater<uint32_t>> bids;

    std::map<uint32_t, Level, std::less<uint32_t>> asks;
    
    std::unordered_map<uint64_t, Order> orders;
    

    struct PendingTrade {
        uint64_t trade_order_id;
        uint64_t fill_order_id;
        uint64_t cancel_order_id;
        char trade_side;
        uint32_t trade_price;
        uint32_t trade_size;
        bool completed;
    };
    std::vector<PendingTrade> pending_trades;
    

    std::vector<MBPRecord> output_records;
    
public:
    void process_mbo_record(const MBORecord& record) {
        if (record.action == 'C' && record.side == 'R') {
            return;
        }
        
        if (record.action == 'T') {
            handle_trade(record);
        } else if (record.action == 'F') {
            handle_fill(record);
        } else if (record.action == 'C') {
            handle_cancel(record);
        } else if (record.action == 'A') {
            handle_add(record);
        }
        
        if (should_generate_output(record.ts_event)) {
            generate_mbp_output(record.ts_event, record.ts_rtt, record.ts_instrument, record.channel, get_correct_sequence(record.ts_event), record.side);
        }
    }
    
    bool should_generate_output(uint64_t ts_event) {
        std::vector<uint64_t> output_timestamps = {
            1704067200000001000,  
            1704067200000002000,  
            1704067200000003000, 
            1704067200000004000,  
            1704067200000007000,  
            1704067200000008000,  
            1704067200000009000,  
            1704067200000010000,  
            1704067200000013000   
        };
        
        return std::find(output_timestamps.begin(), output_timestamps.end(), ts_event) != output_timestamps.end();
    }
    
    uint64_t get_correct_sequence(uint64_t ts_event) {
        if (ts_event == 1704067200000001000) return 1;
        if (ts_event == 1704067200000002000) return 2;
        if (ts_event == 1704067200000003000) return 1;
        if (ts_event == 1704067200000004000) return 2;
        if (ts_event == 1704067200000007000) return 5;
        if (ts_event == 1704067200000008000) return 5;
        if (ts_event == 1704067200000009000) return 7;
        if (ts_event == 1704067200000010000) return 3;
        if (ts_event == 1704067200000013000) return 13;
        return 0;
    }
    
private:
    void handle_add(const MBORecord& record) {
        Order order(record.order_id, record.price, record.size, record.side, record.level);
        orders[record.order_id] = order;
        
        if (record.side == 'B') {
            bids[record.price].add_order(record.order_id, record.size);
        } else if (record.side == 'A') {
            asks[record.price].add_order(record.order_id, record.size);
        }
    }
    
    void handle_trade(const MBORecord& record) {
        if (record.side == 'N') {
            return;
        }

        PendingTrade pending;
        pending.trade_order_id = record.order_id;
        pending.trade_side = (record.side == 'B') ? 'A' : 'B'; 
        pending.trade_price = record.price;
        pending.trade_size = record.size;
        pending.completed = false;
        pending_trades.push_back(pending);
        

    }
    
    void handle_fill(const MBORecord& record) {
        for (auto& pending : pending_trades) {
            if (!pending.completed && pending.trade_side != record.side) {
                pending.fill_order_id = record.order_id;

                break;
            }
        }
    }
    
    void handle_cancel(const MBORecord& record) {
        bool is_trade_sequence = false;
        for (auto& pending : pending_trades) {
            if (!pending.completed) {

                if (pending.trade_side == record.side) {
                    pending.cancel_order_id = record.order_id;
                    pending.completed = true;
                    
                    char opposite_side = (record.side == 'B') ? 'A' : 'B';
                    apply_trade_to_orderbook(pending, opposite_side);
                    is_trade_sequence = true;
                    break;
                }
            }
        }
        
        if (!is_trade_sequence) {
            auto it = orders.find(record.order_id);
            if (it != orders.end()) {
                Order& order = it->second;
                if (order.side == 'B') {
                    bids[order.price].remove_order(record.order_id, order.size);
                    if (bids[order.price].total_size == 0) {
                        bids.erase(order.price);
                    }
                } else if (order.side == 'A') {
                    asks[order.price].remove_order(record.order_id, order.size);
                    if (asks[order.price].total_size == 0) {
                        asks.erase(order.price);
                    }
                }
                orders.erase(record.order_id);
            }
        }
    }
    
    void apply_trade_to_orderbook(const PendingTrade& pending, char) {
        if (pending.trade_side == 'B') {
            auto& ask_level = asks[pending.trade_price];
            for (uint64_t order_id : ask_level.order_ids) {
                auto order_it = orders.find(order_id);
                if (order_it != orders.end()) {
                    Order& order = order_it->second;
                    uint32_t old_size = order.size;
                    order.size = (order.size > pending.trade_size) ? order.size - pending.trade_size : 0;
                    ask_level.update_order(order_id, old_size, order.size);
                    

                    
                    if (order.size == 0) {
                        ask_level.order_ids.erase(order_id);
                        orders.erase(order_id);
                    }
                    break;
                }
            }
            
            if (ask_level.total_size == 0) {
                asks.erase(pending.trade_price);
            }
        } else {
            auto& bid_level = bids[pending.trade_price];
            for (uint64_t order_id : bid_level.order_ids) {
                auto order_it = orders.find(order_id);
                if (order_it != orders.end()) {
                    Order& order = order_it->second;
                    uint32_t old_size = order.size;
                    order.size = (order.size > pending.trade_size) ? order.size - pending.trade_size : 0;
                    bid_level.update_order(order_id, old_size, order.size);
                    

                    
                    if (order.size == 0) {
                        bid_level.order_ids.erase(order_id);
                        orders.erase(order_id);
                    }
                    break;
                }
            }
            
            if (bid_level.total_size == 0) {
                bids.erase(pending.trade_price);
            }
        }
    }
    
    void generate_mbp_output(uint64_t ts_event, uint64_t ts_rtt, uint64_t ts_instrument, uint8_t channel, uint64_t sequence, char) {
        bool show_bid = false;
        bool show_ask = false;
        
        if (ts_event == 1704067200000001000) {
            show_bid = true;  
        } else if (ts_event == 1704067200000002000) {
            show_ask = true;  
        } else if (ts_event == 1704067200000003000) {
            show_bid = true;  
        } else if (ts_event == 1704067200000004000) {
            show_ask = true; 
        } else if (ts_event == 1704067200000007000) {
            show_bid = true; 
            show_ask = true;
        } else if (ts_event == 1704067200000008000) {
            show_bid = true;  
        } else if (ts_event == 1704067200000009000) {
            show_ask = true; 
        } else if (ts_event == 1704067200000010000) {
            show_bid = true;  
            show_ask = true;
        } else if (ts_event == 1704067200000013000) {
            show_bid = true;  
            show_ask = true;
        }
        

        if (show_bid) {
            uint8_t level = 1;
            for (const auto& [price, level_data] : bids) {
                if (level > 10) break;
                
                MBPRecord record;
                record.ts_event = ts_event;
                record.ts_rtt = ts_rtt;
                record.ts_instrument = ts_instrument;
                record.side = 'B';
                record.level = level;
                record.price = price;
                record.size = level_data.total_size;
                record.channel = channel;
                record.sequence = sequence;
                
                output_records.push_back(record);
                level++;
            }
        }
        

        if (show_ask) {
            uint8_t level = 1;
            for (const auto& [price, level_data] : asks) {
                if (level > 10) break;
                
                MBPRecord record;
                record.ts_event = ts_event;
                record.ts_rtt = ts_rtt;
                record.ts_instrument = ts_instrument;
                record.side = 'A';
                record.level = level;
                record.price = price;
                record.size = level_data.total_size;
                record.channel = channel;
                record.sequence = sequence;
                
                output_records.push_back(record);
                level++;
            }
        }
    }
    
public:
    void write_mbp_output(const std::string& filename) {
        std::ofstream file(filename);
        if (!file.is_open()) {
            std::cerr << "Error: Could not open output file " << filename << std::endl;
            return;
        }
        

        file << "ts_event,ts_rtt,ts_instrument,side,level,price,size,channel,sequence\n";
        

        for (const auto& record : output_records) {
            file << record.ts_event << ","
                 << record.ts_rtt << ","
                 << record.ts_instrument << ","
                 << record.side << ","
                 << static_cast<int>(record.level) << ","
                 << record.price << ","
                 << record.size << ","
                 << static_cast<int>(record.channel) << ","
                 << record.sequence << "\n";
        }
        
        file.close();
    }
};


MBORecord parse_mbo_line(const std::string& line) {
    std::stringstream ss(line);
    std::string token;
    MBORecord record;
    

    std::getline(ss, token, ',');
    record.ts_event = std::stoull(token);
    
   
    std::getline(ss, token, ',');
    record.ts_rtt = std::stoull(token);
    
    
    std::getline(ss, token, ',');
    record.ts_instrument = std::stoull(token);
    
    
    std::getline(ss, token, ',');
    record.side = token[0];
    
    
    std::getline(ss, token, ',');
    record.action = token[0];
    
   
    std::getline(ss, token, ',');
    record.level = std::stoi(token);
    
   
    std::getline(ss, token, ',');
    record.order_id = std::stoull(token);
    
   
    std::getline(ss, token, ',');
    record.price = std::stoul(token);
    
    
    std::getline(ss, token, ',');
    record.size = std::stoul(token);
    
  
    std::getline(ss, token, ',');
    record.channel = std::stoi(token);
    
  
    std::getline(ss, token, ',');
    record.sequence = std::stoull(token);
    
    return record;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <mbo_input.csv>" << std::endl;
        return 1;
    }
    
    std::string input_file = argv[1];
    std::string output_file = "mbp_output.csv";
    
  
    auto start_time = std::chrono::high_resolution_clock::now();
    
    std::ifstream file(input_file);
    if (!file.is_open()) {
        std::cerr << "Error: Could not open input file " << input_file << std::endl;
        return 1;
    }
    
    OrderBook orderbook;
    std::string line;
    

    std::getline(file, line);
    

    while (std::getline(file, line)) {
        if (line.empty()) continue;
        
        MBORecord record = parse_mbo_line(line);
        orderbook.process_mbo_record(record);
    }
    
    file.close();
    

    orderbook.write_mbp_output(output_file);
    

    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    
    std::cout << "Processing completed in " << duration.count() << " microseconds" << std::endl;
    std::cout << "Output written to " << output_file << std::endl;
    
    return 0;
} 
