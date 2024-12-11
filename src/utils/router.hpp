#include "path.hpp"
#include "world/world.h" // 现在只在 .cpp 文件中包含 world/world.h

// 构造函数实现
Router::Router(World& new_world) : world_(new_world) {}

Path Router::find_path(Location start_pos, Location end_pos) {
    std::queue<std::pair<int, int>> q;
    q.push({start_pos.x, start_pos.y});

    std::unordered_map<int, bool> visited;
    std::unordered_map<int, Dir> predecessors;

    visited[start_pos.x * 1000 + start_pos.y] = true;

    bool found = false;
    while (!q.empty() && !found) {
        auto [x, y] = q.front();
        q.pop();

        for (const auto& dir : directions) {
            int new_x = x + dir.first;
            int new_y = y + dir.second;

            if (world_.is_valid_position(new_x, new_y) && 
                visited.find(new_x * 1000 + new_y) == visited.end()) {
                visited[new_x * 1000 + new_y] = true;
                predecessors[new_x * 1000 + new_y] = dir;

                if (new_x == end_pos.x && new_y == end_pos.y) {
                    found = true;
                    break;
                }

                q.push({new_x, new_y});
            }
        }
    }

    Path path;
    if (found) {
        int current_x = end_pos.x;
        int current_y = end_pos.y;
        while (current_x != start_pos.x || current_y != start_pos.y) {
            Dir dir = predecessors[current_x * 1000 + current_y];
            path.push_front(dir);
            current_x -= dir.first;
            current_y -= dir.second;
        }
    }

    return path;
}

Path Router::find_path_to_locations(Location start, const Locations& end_locations) {
    Path shortest_path;
    size_t shortest_len = std::numeric_limits<size_t>::max();

    for (const auto& end : end_locations) {
        Path path = find_path(start, end);
        if (path.size() < shortest_len) {
            shortest_len = path.size();
            shortest_path = path;
        }
    }

    return shortest_path;
}

Locations Router::get_locations_around(const Location& pos) {
    Locations locations;
    for (int i = -1; i <= 1; ++i) {
        for (int j = -1; j <= 1; ++j) {
            if (i == 0 && j == 0 ||
                pos.x + i < 0 ||
                pos.x + i >= MAP_SIZE ||
                pos.y + j < 0 ||
                pos.y + j >= MAP_SIZE)
                continue;
            locations.push_back({pos.x + i, pos.y + j});
        }
    }
    return locations;
}

bool Router::is_in_field(const Location& pos, const Field& field) {
    if (field.first == field.second)
        return pos == field.first;
    auto minX = std::min(field.first.x, field.second.x);
    auto maxX = std::max(field.first.x, field.second.x);
    auto minY = std::min(field.first.y, field.second.y);
    auto maxY = std::max(field.first.y, field.second.y);
    return pos.x >= minX && pos.x <= maxX && pos.y >= minY && pos.y <= maxY;
}