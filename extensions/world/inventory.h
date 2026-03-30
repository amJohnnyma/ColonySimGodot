#ifndef INVENTORY_H
#define INVENTORY_H

#include <unordered_map>
#include <utility>   // std::pair

// Custom hash for std::pair<int, int>
struct PairHash {
    template <class T1, class T2>
    std::size_t operator()(const std::pair<T1, T2>& p) const {
        auto h1 = std::hash<T1>{}(p.first);
        auto h2 = std::hash<T2>{}(p.second);
        // Good mixing function (from boost::hash_combine style)
        return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
    }
};

class Inventory
{
private:
    std::unordered_map<std::pair<int, int>, int, PairHash> items;

public:
    Inventory() = default;                    
    explicit Inventory(size_t reserve_size) {
        items.reserve(reserve_size);
    }

    void add(int type, int id, int count);
    void remove(int type, int id, int count);
    
    int get_count(int type, int id) const;
    bool has_item(int type, int id, int required_count = 1) const;
    void clear();

    bool is_empty() const { return items.empty(); }
    const auto& get_all_items() const { return items; }
};

#endif
