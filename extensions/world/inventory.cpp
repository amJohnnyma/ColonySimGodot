#include "inventory.h"
#include <algorithm>

void Inventory::add(int type, int id, int count)
{
    if (count <= 0) return;
    auto key = std::make_pair(type, id);
    items[key] += count;        // automatically inserts 0 if new
}

void Inventory::remove(int type, int id, int count)
{
    if (count <= 0) return;
    auto key = std::make_pair(type, id);
    auto it = items.find(key);
    if (it != items.end()) {
        if (it->second > count) {
            it->second -= count;
        } else {
            items.erase(it);
        }
    }
}

int Inventory::get_count(int type, int id) const
{
    auto key = std::make_pair(type, id);
    auto it = items.find(key);
    return (it != items.end()) ? it->second : 0;
}

bool Inventory::has_item(int type, int id, int required_count) const
{
    return get_count(type, id) >= required_count;
}

void Inventory::clear()
{
    items.clear();
}
