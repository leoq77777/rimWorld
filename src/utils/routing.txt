#pragma once
#include <SFML/Graphics.hpp>
#include <queue>
#include <unordered_map>
#include <cmath>
#include <limits>
#include <algorithm>
#include <utility>
#include <vector>
using namespace std;
struct Node {
    int x, y;
    float distance;
    bool operator>(const Node& other) const { return distance > other.distance; }
};

//for what?
struct pair_hash {
    inline size_t operator()(const pair<int, int>& v) const {
        return v.first * 97 + v.second;
    }
};

vector<sf::Vector2i> findShortestPath(const sf::Vector2i& start, const sf::Vector2i& end, bool& is_unreachable, World& world) {
    using Cell = std::pair<int, int>;
    priority_queue<Node, vector<Node>, greater<Node>> pq;
    unordered_map<Cell, Cell, pair_hash> cameFrom;
    vector<vector<float>> distances(MAP_SIZE, vector<float>(MAP_SIZE, numeric_limits<float>::infinity()));

    pq.push({start.x, start.y, 0});
    distances[start.y][start.x] = 0;

    while (!pq.empty()) {
        Node current = pq.top(); pq.pop();

        if (current.x == end.x && current.y == end.y) {
            vector<sf::Vector2i> path;
            Cell cell = make_pair(end.x, end.y);
            while (cell != make_pair(start.x, start.y)) {
                path.push_back(sf::Vector2i(cell.first, cell.second));
                cell = cameFrom[cell];
            }
            path.push_back(sf::Vector2i(start.x, start.y));
            reverse(path.begin(), path.end());
            is_unreachable = false;
            return path;
        }

        vector<pair<int, int>> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
        for (auto& dir : directions) {
            int newX = current.x + dir.first;
            int newY = current.y + dir.second;

            if (newX >= 0 && newX < MAP_SIZE && newY >= 0 && newY < MAP_SIZE && world.is_valid_position(newX, newY)) {
                float newDist = current.distance + 1.0f;
                if (newDist < distances[newY][newX]) {
                    distances[newY][newX] = newDist;
                    pq.push({newX, newY, newDist});
                    cameFrom[{newX, newY}] = {current.x, current.y};
                }
            }
        }
    }
    
    is_unreachable = true;
    return {};
}


void search_task(Entity character, World& world) {
    auto& pos = world.component_manager_.get_component<locationComponent>(character).loc;
    bool area_clear = true;
    int distance = 0;
    while(!area_clear) {
        for(int d = 0; d < 4; ++d) {
            int newX = pos.x + d * distance;
            int newY = pos.y + d * distance;
            if(world.is_valid_position(newX, newY)) {
                area_clear = false;
            }
        }
    }
}

