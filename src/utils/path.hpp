#pragma once
#include <vector>
#include <list>
#include <queue>
#include <unordered_map>
#include <limits>

// 前向声明 World 和 Location
class World;
struct Location;

using Dir = std::pair<int, int>;
const std::vector<Dir> directions = {{0, 1}, {0, -1}, {1, 0}, {-1, 0}};
using Path = std::list<Dir>;
using Locations = std::vector<Location>;
using Field = std::pair<Location, Location>;

class Router {
public:
    Router(World& new_world);

    // 公共接口
    Path find_path(Location start_pos, Location end_pos);
    Path find_path_to_locations(Location start, const Locations& end_locations);
    Locations get_locations_around(const Location& pos);
    bool is_in_field(const Location& pos, const Field& field);

private:
    World& world_;
};