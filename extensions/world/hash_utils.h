#pragma once

#include <godot_cpp/variant/vector2i.hpp>
#include <functional>
#include <unordered_map>
#include <unordered_set>

using namespace godot;

// Hash function for Vector2i to use in unordered containers
struct Vector2iHash {
    std::size_t operator()(const Vector2i& v) const {
        // Use a better hash combining algorithm
        std::size_t h1 = std::hash<int>()(v.x);
        std::size_t h2 = std::hash<int>()(v.y);
        return h1 ^ (h2 << 1);
    }
};

// Equality comparator for Vector2i
struct Vector2iEqual {
    bool operator()(const Vector2i& a, const Vector2i& b) const {
        return a.x == b.x && a.y == b.y;
    }
};

// Convenience type aliases for common Vector2i containers
template<typename T>
using Vector2iMap = std::unordered_map<Vector2i, T, Vector2iHash, Vector2iEqual>;

using Vector2iSet = std::unordered_set<Vector2i, Vector2iHash, Vector2iEqual>;

// Less-than comparator for Vector2i (for std::map, std::set)
struct Vector2iLess {
    bool operator()(const Vector2i& a, const Vector2i& b) const {
        if (a.x != b.x) return a.x < b.x;
        return a.y < b.y;
    }
};
