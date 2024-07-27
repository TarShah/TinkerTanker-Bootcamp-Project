#pragma once

#include "base_types.h"
#include "json/action_move_request.h"
#include "json/client_subscribe_request.h"
#include "json/get_games_response.h"
#include "json/join_game_request.h"
#include "json/join_game_response.h"
#include "json/map_config.h"
#include "json/resource_config.h"
#include "json/action_build_bot.h"
#include "json/action_mine_request.h"
#include "json/action_transfer_request.h"
#include "json/position.h"
#include "json/update.h"
#include "json/action_response.h"
#include <bits/stdc++.h>
#include <bits/stdc++.h>
#include "utils.h"
#include "algo.h"
#include <chrono>
#include <random>
#include <stdexcept>

using namespace miningbots::json;

class State
{
public:
    State()
    {
        scan_centers.resize(4);
        grid_corner.resize(1);
        resourcequeue.resize(4);
        counter = 0;
        build_check = true;
        start = std::chrono::high_resolution_clock::now();
        commandcount.resize(5);
        std::fill(commandcount.begin(), commandcount.end(), 0);
    };
    // For each position, return the sub-grid it belongs to
    int subgrid_index(Position position)
    {
        for (int i = 0; i < 1; i++)
        {
            if (position.x >= std::min(grid_corner[i].first.x, grid_corner[i].second.x) && position.x <= std::max(grid_corner[i].first.x, grid_corner[i].second.x))
                if (position.y >= std::min(grid_corner[i].first.y, grid_corner[i].second.y) && position.y <= std::max(grid_corner[i].first.y, grid_corner[i].second.y))
                    return i;
        }
    }
    void win_cond_update()
    {
        for (auto chunk : mapconfig.win_condition)
        {
            win_amount[chunk.id] = chunk.amount;
            permanent_win_amount[chunk.id] = chunk.amount;
        }
        scanned.resize(mapconfig.max_x, std::vector<int>(mapconfig.max_y));
        for (auto chunk : mapconfig.action_buildbot_resource_cost){
            one_build_amount[chunk.id] = chunk.amount;
            build_amount[chunk.id] = chunk.amount*num_build_bots;
        }
        int maxx = mapconfig.max_x;
        int maxy = mapconfig.max_y;
        int centerx = mapconfig.max_x / 2;
        int centery = mapconfig.max_y / 2;
        grid_corner[0] = {{0, 0}, {Kilometer(maxx - 1), Kilometer(maxy - 1)}};
        // grid_corner[1] = {{0, Kilometer(centery + 1)}, {Kilometer(centerx), Kilometer(maxy - 1)}};
        // grid_corner[2] = {{Kilometer(centerx + 1), 0}, {Kilometer(maxx - 1), Kilometer(centery)}};
        // grid_corner[3] = {{Kilometer(centerx + 1), Kilometer(centery + 1)}, {Kilometer(maxx - 1), Kilometer(maxy - 1)}};
        for (int i = 0; i < 1; i++)
        {
            Algorithm CurrAlgo(mapconfig);
            auto scanpos = CurrAlgo.scan_positions(grid_corner[i].second, grid_corner[i].first);
            scan_centers[i].clear();
            for (auto pos : scanpos)
                scan_centers[i].push_back(pos);
        }
        max_bots = std::min(mapconfig.max_x, mapconfig.max_y) / 2;
    }

    void maxbotcount(){

    }
    // State(std::string host_name,int port,GameId id,PlayerId player_id);
    void processupdate(Update &update)
    {
        if(update.update_type == UpdateType::kUseless){
            return;
        }
        if(update.update_type == UpdateType::kEndInWin){
            auto end = std::chrono::high_resolution_clock::now();   
            std::chrono::duration<double> duration = std::chrono::duration_cast<std::chrono::duration<double>>(end - start);
            double dur = double(duration.count()); 
            SPDLOG_INFO("TIME TAKEN : {}",std::to_string(dur));
            SPDLOG_INFO("move count : {}",std::to_string(commandcount[0]));
            SPDLOG_INFO("mine count : {}",std::to_string(commandcount[1]));
            SPDLOG_INFO("scan count : {}",std::to_string(commandcount[2]));
            SPDLOG_INFO("transfer count : {}",std::to_string(commandcount[3]));
            SPDLOG_INFO("build count : {}",std::to_string(commandcount[4]));

        }
        for (int i = 0; i < (int)update.bot_updates.size(); i++)
        {
            if (update.bot_updates[i].variant == Variant::kMiningBot)
            {
                if (bots_state.find(update.bot_updates[i].id) == bots_state.end())
                {
                    bots_state[update.bot_updates[i].id] = 0;
                    bot_idx[update.bot_updates[i].id] = (counter % 1);
                    counter++;
                }
                bot_map[update.bot_updates[i].id] = update.bot_updates[i];
            }
            else if (update.bot_updates[i].variant == Variant::kFactoryBot)
            {
                factory_id = update.bot_updates[i].id;
                if (bots_state.find(update.bot_updates[i].id) == bots_state.end())
                {
                    bots_state[update.bot_updates[i].id] = 0;
                }
                bot_map[update.bot_updates[i].id] = update.bot_updates[i];
            }
        }
        std::vector<ResourceId> empty_resources;
        for (int i = 0; i < (int)update.land_updates.size(); i++)
        {
            Position land_position = update.land_updates[i].position;
            if (!update.land_updates[i].resources.empty())
            {
                // if(scanned[land_position.x][land_position.y] == 0){
                    bool useful = false;
                    for (auto chunk : update.land_updates[i].resources){
                        if (win_amount[chunk.id] > 0){
                            resourcequeue[0].push_back(land_position);
                            useful = true;
                            break;
                        }
                    }
                    if(!useful){
                        granite_pos.push_back(land_position);
                    }
                // }
            }
            // if this land is never discovered and not traversable, push back in obstacles vector
            if (scanned[land_position.x][land_position.y] == 0 and update.land_updates[i].is_traversable == false)
            {
                obstacles.push_back(update.land_updates[i].position);
            }
            scanned[land_position.x][land_position.y] = 1;
            // Erase the earlier resource chunks from the resource_map
            for (auto earlier_chunk : land_map[land_position].resources)
            {
                resource_map[earlier_chunk.id].erase({land_position, earlier_chunk.amount});
                if (resource_map[earlier_chunk.id].size() == 0)
                {
                    empty_resources.push_back(earlier_chunk.id);
                }
            }
            // remove the empty resources id from resource_map
            for (auto resource_id : empty_resources)
            {
                resource_map.erase(resource_id);
            }
            // Update the land_map
            land_map[land_position] = update.land_updates[i];
            // land_map[land_position].is_traversable = !land_map[land_position].is_traversable;
            // Insert the updated resource chunks into the resource_map
            for (auto updated_chunk : land_map[land_position].resources)
            {
                resource_map[updated_chunk.id].insert({land_position, updated_chunk.amount});
            }
            // Update Obstacles
            auto in_between = [&](Position& pos)
            {
                bool check = true;
                check = check && (pos.x >= 0);
                check = check && (pos.x < mapconfig.max_x);
                check = check && (pos.y >= 0);
                check = check && (pos.y < mapconfig.max_y);
                return check;
            };
            auto it = obstacles.begin();
            int radius = mapconfig.action_mine_max_distance;
            while (it != obstacles.end())
            {
                Position pos = (*it);
                if (land_map[pos].is_traversable)
                {
                    for (int dx = -radius; dx <= radius; dx++)
                        for (int dy = -radius; dy <= radius; dy++)
                        {
                            if (!(dx == 0 and dy == 0) and ((dx*dx) + (dy*dy) <= (radius*radius)))
                            {
                                Position neighbour = {pos.x + dx, pos.y + dy};
                                if (land_map.find(neighbour) == land_map.end() || !in_between(neighbour)){
                                    continue;
                                }
                                if (!land_map[neighbour].resources.empty()){
                                    resourcequeue[0].push_back(neighbour);
                                }
                            }
                        }
                    it = obstacles.erase(it);
                }
                else{
                    it++;
                }
            }
        }
        for (int i = 0; i < (int)update.job_updates.size(); i++)
        {
            // if (update.job_updates[i].status == JobStatus::kCancelledPathBlocked ||  update.job_updates[i].status == JobStatus::kCancelled or update.job_updates[i].status == JobStatus::kCompleted or update.job_updates[i].status == JobStatus::kBlocked or update.job_updates[i].status == JobStatus::kCancelledInsufficientEnergy or update.job_updates[i].status == JobStatus::kCancelledBotMovedTooFar)
            if(!(update.job_updates[i].status == JobStatus::kInProgress || update.job_updates[i].status == JobStatus::kReachedInterval || update.job_updates[i].status == JobStatus::kNotStarted))
            {
                if (update.job_updates[i].action == kScan and update.job_updates[i].status == JobStatus::kCompleted)
                {
                    int sub_index = subgrid_index(update.job_updates[i].target_position);
                    #define all(v) v.begin(),v.end()
                    auto it = find(all(scan_centers[sub_index]),update.job_updates[i].target_position);
                    if(it != scan_centers[sub_index].end()){
                        scan_centers[sub_index].erase(it);
                    }
                }
                bots_state[current_jobs[update.job_updates[i].id]] = 0;
                current_jobs.erase(update.job_updates[i].id);
            }
        }
    };
    std::string hostname_;
    int counter;
    int max_bots;
    int num_build_bots = 2;
    int port_;
    bool build_check;
    Key_t player_key;
    BotId factory_id;
    GameId game_id;
    PlayerId player_id;
    MapConfig mapconfig;
    std::chrono::high_resolution_clock::time_point start;
    std::vector<std::vector<int>> scanned;
    std::map<BotId,int> gotomine;
    std::map<BotId,Position> gotomine_pos;
    std::vector<Position> granite_pos;
    std::map<BotId, int> bots_state; // 0 -> No job , 1-> In a job
    // need to the bot to which the job is assigned
    std::map<JobId, BotId> current_jobs;
    std::map<BotId, bool> mine_granite;
    std::map<BotId,Position> mine_granite_pos; 
    // BotUpdate is basically our bot type, the name is misleading though
    std::map<BotId, BotUpdate> bot_map;
    // Stores the Land and its resources
    std::map<Position, LandUpdate> land_map;
    // Stores the Position and Amount of resources at some specific position
    std::map<ResourceId, std::set<std::pair<Position, Kilogram>>> resource_map;
    std::vector<Position> obstacles;

    // path we will use for moving the bot
    std::map<BotId, std::queue<Position>> move_path;
    // bot index for working square
    std::map<BotId, int> bot_idx;
    // a vector for marking the UL and BR corner of the rectangle
    std::vector<std::pair<Position, Position>> grid_corner;
    // Scan centers
    std::vector<std::vector<Position>> scan_centers;
    std::vector<std::vector<Position>> resourcequeue;
    // required resrources to win
    std::map<ResourceId, Kilogram> win_amount;
    std::map<ResourceId, Kilogram> permanent_win_amount;
    std::map<ResourceId, Kilogram> build_amount;
    std::map<ResourceId, Kilogram> one_build_amount;
    //count stats
    std::vector<int>commandcount;

};
