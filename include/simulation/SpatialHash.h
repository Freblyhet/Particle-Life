#pragma once

#include <unordered_map>
#include <vector>
#include <cmath>

class SpatialHash {
private:
    float cellSize;
    std::unordered_map<int, std::vector<int>> grid;
    
    int hash(int x, int y) const {
        return x * 73856093 ^ y * 19349663;
    }
    
public:
    SpatialHash(float size) : cellSize(size) {}
    
    void clear() { grid.clear(); }
    
    void insert(int idx, float x, float y) {
        int cx = static_cast<int>(std::floor(x / cellSize));
        int cy = static_cast<int>(std::floor(y / cellSize));
        grid[hash(cx, cy)].push_back(idx);
    }
    
    std::vector<int> query(float x, float y, float radius) const {
        std::vector<int> result;
        int minX = static_cast<int>(std::floor((x - radius) / cellSize));
        int maxX = static_cast<int>(std::floor((x + radius) / cellSize));
        int minY = static_cast<int>(std::floor((y - radius) / cellSize));
        int maxY = static_cast<int>(std::floor((y + radius) / cellSize));
        
        for (int cy = minY; cy <= maxY; ++cy) {
            for (int cx = minX; cx <= maxX; ++cx) {
                auto it = grid.find(hash(cx, cy));
                if (it != grid.end()) {
                    result.insert(result.end(), it->second.begin(), it->second.end());
                }
            }
        }
        return result;
    }
};