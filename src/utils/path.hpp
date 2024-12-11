#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <algorithm>
#include <limits.h>
#include "world/world.hpp"

using Dir = std::pair<int, int>;
using Path = std::vector<Dir>;
const std::vector<Dir> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};

class Router {
public:
    Router(World& new_world) : world_(new_world) {}
    bool is_in_field(const Location& pos, const Field& field);
    Path find_path(const Location& start, const Location& end);
    Path find_path_from_locations(const Location& start, const Locations& end_locations) {

private:
    World& world_;
};

bool Router::is_in_field(const Location& pos, const Field& field) {
    int minX = std::min(field.first.x, field.second.x);
    int maxX = std::max(field.first.x, field.second.x);
    int minY = std::min(field.first.y, field.second.y);
    int maxY = std::max(field.first.y, field.second.y);
    return pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY;
}

Path Router::find_path(const Location& start, const Location& end) {
    if (start == end) {
        return {}; // 如果起点和终点相同，返回空路径
    }

    std::queue<Location> queue;
    std::unordered_map<Location, Location> came_from; // 用于追踪路径
    queue.push(start);
    came_from[start] = start; // 起点的前驱是它自己

    while (!queue.empty()) {
        Location current = queue.front();
        queue.pop();

        for (const auto& direction : directions) {
            Location next = {current.x + direction.first, current.y + direction.second};

            // 检查下一个位置是否在地图范围内并且没有被访问过
            if (world_.is_valid_position(next) && came_from.find(next) == came_from.end()) {
                queue.push(next);
                came_from[next] = current;

                // 如果找到目标位置，构建路径
                if (next == end) {
                    Path path;
                    for (Location step = end; step != start; step = came_from[step]) {
                        path.push_back({step.x, step.y});
                    }
                    std::reverse(path.begin(), path.end());
                    return path;
                }
            }
        }
    }

    return {}; // 如果没有找到路径，返回空路径
}

Path Router::find_path_from_locations(const Location& start, const Locations& end_locations) {
    std::vector<Path> paths;
    int min_distance = INT_MAX;
    int min_index = -1;
    for (int i = 0; i < end_locations.size(); ++i) {
        paths.push_back(find_path(start, end_locations[i]));
        if (paths.back().size() < min_distance) {
            min_distance = paths.back().size();
            min_index = i;
        }
    }
    return paths[min_index];
}

Locations Router::get_locations_around(const Location& pos) {
    Locations locations;
    for(int i = -1; i <= 1; ++i) {
        for(int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0 || pos.x + i < 0 || pos.x + i >= MAP_SIZE || pos.y + j < 0 || pos.y + j >= MAP_SIZE) continue;
            locations.push_back({pos.x + i, pos.y + j});
        }
    }
    return locations;
}

