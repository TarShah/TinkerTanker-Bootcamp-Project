#pragma once
#include <vector>
#include <set>
#include <unordered_map>
#include <map>
#include <algorithm>
#include "spdlog/spdlog.h"
#include <chrono>
#include <cmath>
#include "json/position.h"
#include "json/map_config.h"
#include "json/land_update.h"

using namespace miningbots::json;

struct Direction
{
    int x;
    int y;
};

class Algorithm
{
public:
    MapConfig &mapconfig;
    Algorithm(MapConfig &mapconfig) : mapconfig(mapconfig){};

    bool is_valid(Position &pos, std::vector<Position> &obstacles)
    {
        bool check = true;
        check = check && (pos.x < mapconfig.max_x);
        check = check && (pos.x >= 0);
        check = check && (pos.y < mapconfig.max_y);
        check = check && (pos.y >= 0);
        check = check && (std::find(obstacles.begin(), obstacles.end(), pos) == obstacles.end());
        return check;
    }

    int get_weight(Position &pos, std::map<Position, LandUpdate> &land_map)
    {
        int max_weight = 0;
        for (auto &terrain : mapconfig.terrain_configs)
        {
            int interval = terrain.move_interval;
            int move_energy = (terrain.move_energy_per_interval / 100000);
            max_weight = std::max(max_weight, interval * move_energy);
        }
        if (land_map.find(pos) == land_map.end())
            return max_weight;
        auto &terrainconf = mapconfig.terrain_configs[land_map[pos].terrain_id];
        int interval = terrainconf.move_interval;
        int move_energy = (terrainconf.move_energy_per_interval / 100000);
        return (interval * move_energy);
    }

    int heuristic(Position a, Position b)
    {
        int dx = std::abs(a.x - b.x);
        int dy = std::abs(a.y - b.y);
        return dx + dy + (std::sqrt(2) - 2) * std::min(dx, dy);
    }

    std::vector<Position> a_star(Position& start, Position& goal, std::vector<Position>& obstacles, std::map<Position, LandUpdate>& land_map)
    {
        std::vector<Direction> directions = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {1, 1}, {-1, 1}, {1, -1}};
        using Node = std::pair<int, Position>;
        std::set<Node> open_set;
        std::unordered_map<Position, Position> came_from;
        std::unordered_map<Position, int> cost_so_far;

        open_set.insert({0, start});
        came_from[start] = start;
        cost_so_far[start] = 0;

        while (!open_set.empty())
        {
            Position current = open_set.begin()->second;
            open_set.erase(open_set.begin());

            if (current == goal)
            {
                std::vector<Position> path;
                for (Position at = current; at != start; at = came_from[at])
                    path.push_back(at);
                path.push_back(start);
                std::reverse(path.begin(), path.end());
                return path;
            }

            for (auto& dir : directions)
            {
                Position next = {current.x + dir.x, current.y + dir.y};
                if (is_valid(next, obstacles))
                {
                    int new_cost = cost_so_far[current] + get_weight(next, land_map);
                    if (cost_so_far.find(next) == cost_so_far.end() || new_cost < cost_so_far[next])
                    {
                        cost_so_far[next] = new_cost;
                        int priority = new_cost + heuristic(next, goal);
                        open_set.insert({priority, next});
                        came_from[next] = current;
                    }
                }
            }
        }
        return {};
    }

    std::vector<Position> find_path(bool mining, int radius, std::vector<Position> &obstacles, Position mine, Position start, std::map<Position, LandUpdate> &land_map)
    {
        auto starting = std::chrono::high_resolution_clock::now();
        std::vector<Position> all_valid;
        for (int dx = -radius; dx <= radius; dx++)
            for (int dy = -radius; dy <= radius; dy++)
            {
                if (dx == 0 && dy == 0 && mining){
                    continue;
                }
                Position pos = {mine.x + dx, mine.y + dy};
                if (pos.square_distance(mine) <= (radius * radius) && is_valid(pos, obstacles))
                    all_valid.push_back(pos);
            }

        std::sort(all_valid.begin(), all_valid.end(), [&mine, this](Position &a, Position &b)
        {
            return heuristic(a, mine) < heuristic(b, mine);
        });

        for (Position &end : all_valid)
        {
            std::vector<Position> path = a_star(start, end, obstacles, land_map);
            if (!path.empty() && path[0] == start && path.back() == end)
            {
                auto ending = std::chrono::high_resolution_clock::now();
                std::chrono::duration<double> elapsed_seconds = ending - starting;
                double duration = double(elapsed_seconds.count());
                SPDLOG_INFO("******TIME ELAPSED FOR FINDING PATH : {}******", std::to_string(duration));
                return path;
            }
        }
        return {};
    }

    std::vector<Position> scan_positions(Position bottom_corner, Position top_corner)
    {
        std::vector<Position> valid;
        Kilometer max_x = std::max(bottom_corner.x, top_corner.x);
        Kilometer min_x = std::min(bottom_corner.x, top_corner.x);
        Kilometer max_y = std::max(bottom_corner.y, top_corner.y);
        Kilometer min_y = std::min(bottom_corner.y, top_corner.y);
        int delta = (2 * mapconfig.action_scan_radius + 1);
        for (Kilometer i = min_x + ((delta - 1) / 2); i <= max_x; i += delta)
            for (Kilometer j = min_y + ((delta - 1) / 2); j <= max_y; j += delta)
                valid.push_back({i, j});
        for (Kilometer i = min_x + (((max_x - min_x) / delta) * delta); i <= max_x; i += delta)
            for (Kilometer j = min_y + ((delta - 1) / 2); j <= max_y; j += delta)
                valid.push_back({i, j});
        for (Kilometer i = min_x + ((delta - 1) / 2); i <= max_x; i += delta)
            for (Kilometer j = min_y + (((max_y - min_y) / delta) * delta); j <= max_y; j += delta)
                valid.push_back({i, j});
        for (Kilometer i = min_x + (((max_x - min_x) / delta) * delta); i <= max_x; i += delta)
            for (Kilometer j = min_y + (((max_y - min_y) / delta) * delta); j <= max_y; j += delta)
                valid.push_back({i, j});
        return valid;
    }
};
