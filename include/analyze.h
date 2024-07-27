#pragma once

#include <vector>
#include <queue>
#include <cpr/cpr.h>
#include <future>
#include <mutex>
#include "json/action_explode_request.h"
#include "json/action_build_bot.h"
#include "json/action_mine_request.h"
#include "json/action_move_request.h"
#include "json/action_scan_request.h"
#include "json/action_transfer_request.h"
#include "json/map_config.h"
#include "spdlog/spdlog.h"
#include <string>
#include "base_types.h"
#include "utils.h"
#include <glaze/core/context.hpp>
#include <glaze/json/write.hpp>
#include <ixwebsocket/IXWebSocket.h>
#include <random>
#include <stdexcept>
#include "state.h"
#include "commands.h"
#include "algo.h"
#include <unordered_map>
#include <chrono>
#include <condition_variable>
#include "client/connection.h"

using namespace miningbots;
class Analyze
{
public:
    Analyze(State &state, miningbots::client::Command &command, Algorithm &Algo) : state(state), command(command), Algo(Algo) {}

    void run()
    {
        std::thread([this]()
                    {
            while (true) {   
                Update update;
                std::unique_lock<std::mutex> lock(command.update_mutex);
                command.cv.wait(lock, [this]() { return !command.updates.empty(); });
                if(command.updates.size() > 0){
                    update = command.updates.front();
                    command.updates.pop();
                    processUpdate(update);
                }
                analyze();
            } })
            .detach();
    }

    int inside_grid(Position& pos)
    {
        bool check = true;
        check = check && (pos.x >= 0);
        check = check && (pos.x < state.mapconfig.max_x);
        check = check && (pos.y >= 0);
        check = check && (pos.y < state.mapconfig.max_y);
        return check;
    }

    int cargo_bots(int resource_index)
    {
        int total_cargo = 0;
        for (auto& bot_pair: state.bot_map){
            int bot_idx = bot_pair.first;
            for (auto& bot_cargo: state.bot_map[bot_idx].cargo)
                if (bot_cargo.id == resource_index)
                    total_cargo += bot_cargo.amount;
        }
        return total_cargo;
    }

    int score(int resource_index)
    {
        if (state.permanent_win_amount.find(resource_index) == state.permanent_win_amount.end())
            return 0;
        int resource_score = std::max(0, state.permanent_win_amount[resource_index] - cargo_bots(resource_index));
        return resource_score;
    }

    double granite_op(const Position& pos)
    {
        double granite_score = 0;
        int radius = state.mapconfig.max_x / 20;
        for (int dx = -radius; dx <= radius; dx++)
            for (int dy = -radius; dy <= radius; dy++)
            {
                Position curr = {pos.x + dx, pos.y + dy};
                if (!inside_grid(curr) || state.land_map.find(curr) == state.land_map.end())
                    continue;
                for (auto& curr_cargo: state.land_map[curr].resources)
                    granite_score += double(score(curr_cargo.id)) * (1.0 / double(1.0 + (dx * dx) + (dy * dy)));
            }
        return granite_score;
    }

private:

    bool buildbotcheck()
    {
        int n = state.mapconfig.resource_configs.size();
        std::vector<double> per_bot_cargo(n, 0), curr_build_amount(n, 0), curr_win_amount(n, 0);
        for (auto& bot_pair: state.bots_state){
            BotId bot_id = bot_pair.first;
            for (auto& bot_cargo: state.bot_map[bot_id].cargo){
                per_bot_cargo[bot_cargo.id] += bot_cargo.amount;
            }
        }
        for (int i = 0; i < n; i++){
            per_bot_cargo[i] /= double(state.counter);
        }
        for (auto& cargo: state.mapconfig.action_buildbot_resource_cost){
            curr_build_amount[cargo.id] += cargo.amount;
        }
        for (auto& cargo_id_kg: state.win_amount){
            curr_win_amount[cargo_id_kg.first] += cargo_id_kg.second;
        }
        double time_with_build = 0.0, time_without_build = 0.0;
        for (int i = 0; i < n; i++){
            double numerator = curr_win_amount[i] + curr_build_amount[i];
            double denominator = (state.counter + 1) * per_bot_cargo[i] + curr_build_amount[i] * (state.counter - 2);
            if (denominator < 1e-9 || curr_win_amount[i] == 0)
                continue;
            time_with_build = std::max(numerator / denominator, time_with_build);
        }
        for (int i = 0; i < n; i++){
            double numerator = curr_win_amount[i] + curr_build_amount[i];
            double denominator = state.counter * per_bot_cargo[i] + curr_build_amount[i] * (state.counter - 2);
            if (denominator < 1e-9 || curr_win_amount[i] == 0)
                continue;
            time_without_build = std::max(numerator / denominator, time_without_build);
        }
        double multiplier = 1.0 + (5.0 / double(state.mapconfig.max_x));
        return ((time_with_build * multiplier)  <= time_without_build);
    }

    void processUpdate(Update &update)
    {
        // Process the update and update the state
        std::lock_guard<std::mutex> lock(state_mutex);
        state.processupdate(update);
    }

    void analyze()
    {
        auto firststart = std::chrono::high_resolution_clock::now();
        // std::lock_guard<std::mutex> lock(state_mutex);
        // first iteration to move all bots to center
        if (move_initial and state.counter > 0)
        {
            move_initial = false;
            for(auto &bot_pair : state.bots_state){
                BotId bot_id = bot_pair.first;
                Position center = {state.mapconfig.max_x / 2, state.mapconfig.max_y / 2};
                if (state.bot_map[bot_id].position != center){
                    json::ActionMoveRequest move_request(state.game_id, state.player_id, state.player_key, bot_id, center);
                    command.addRequest(move_request, state);
                    state.bots_state[bot_id] = 1;
                }
            }
        }
        // write ananlyze function here
        for (auto &bot_pair : state.bots_state)
        {
            if(bot_pair.second == 2){
                BotId bot_id = bot_pair.first;
                uint16_t bot_x = state.bot_map[bot_id].position.x;
                uint16_t bot_y = state.bot_map[bot_id].position.y;
                if(state.bot_map[bot_id].current_energy > state.mapconfig.action_scan_energy){
                    json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {bot_x,bot_y}};
                    command.addRequest(scan_request, state);
                    bot_pair.second = 1;
                }
                goto end;
            }
            else if (bot_pair.second == 0)
            {
                BotId bot_id = bot_pair.first;
                int& bot_index = state.bot_idx[bot_id];
                // Update build_check
                state.build_check = (buildbotcheck() && state.build_check && (state.counter < state.max_bots));
                // Calculate the cargo amount with factory bot
                std::map<ResourceId,Kilogram> cargo_amounts;
                for(auto bp : state.bots_state){
                    for(auto chunk : state.bot_map[bp.first].cargo){
                        cargo_amounts[chunk.id] += chunk.amount;
                    }
                } 
                // Update win conditions
                for(auto chunk : state.permanent_win_amount){
                    if(cargo_amounts[chunk.first] >= chunk.second){
                        state.win_amount[chunk.first] = 0;
                    }
                    else{
                        state.win_amount[chunk.first] = chunk.second;
                    }
                }
                // Transfer Actions
                bool over = true;
                for (auto chunk : state.win_amount){
                    if (chunk.second > 0){
                        over = false;
                    }
                }
                // factory bot
                if (state.bot_map[bot_id].variant == Variant::kFactoryBot)
                {
                    if(!over and state.bot_map[bot_id].current_energy >= state.mapconfig.action_buildbot_energy and state.counter < state.max_bots and state.build_check){
                        bool build_it = true;
                        std::map<ResourceId,Kilogram> factory_cargo;
                        for(auto chunk : state.bot_map[bot_id].cargo){
                            factory_cargo[chunk.id] = chunk.amount;
                        }
                        for(auto chunk : state.one_build_amount){
                            if(factory_cargo[chunk.first] < chunk.second){
                                build_it = false;
                            }
                        }
                        if(build_it){
                            json::ActionBuildBotRequest build_request{state.game_id, state.player_id, state.player_key, bot_id};
                            command.addRequest(build_request, state);
                            bot_pair.second = 1;
                            goto end;
                        }
                    }
                    uint16_t bot_x = state.bot_map[bot_id].position.x;
                    uint16_t bot_y = state.bot_map[bot_id].position.y;
                    for (auto pos : state.scan_centers[bot_index]){
                        if (((bot_x - pos.x)*(bot_x - pos.x)) + ((bot_y - pos.y)*(bot_y - pos.y)) <= (state.mapconfig.action_scan_max_distance*state.mapconfig.action_scan_max_distance)){
                            if (state.bot_map[bot_id].current_energy >= state.mapconfig.action_scan_energy){
                                #define all(v) v.begin(),v.end()
                                auto it = find(all(state.scan_centers[bot_index]),Position(pos));
                                if(it != state.scan_centers[bot_index].end()){
                                    state.scan_centers[bot_index].erase(it);
                                }
                                json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, pos};
                                command.addRequest(scan_request, state);
                                bot_pair.second = 1;
                            }
                            goto end;
                        }
                    }
                }
                // mining bot
                else
                {
                    uint16_t bot_x = state.bot_map[bot_id].position.x;
                    uint16_t bot_y = state.bot_map[bot_id].position.y;
                    // Move Actions
                    if ((int)state.move_path[bot_id].size() > 0){
                        if(over and (state.gotomine[bot_id] == 1 or state.mine_granite[bot_id] == 1)){
                            state.gotomine[bot_id] = 0;
                            state.mine_granite[bot_id] = 0;
                            while (!state.move_path[bot_id].empty()){
                                state.move_path[bot_id].pop();
                            }
                            goto finishgame;
                        }
                        Position next_move = state.move_path[bot_id].front();
                        // Next move cell not scanned
                        if (state.scanned[next_move.x][next_move.y] == 0){
                            while (!state.move_path[bot_id].empty()){
                                state.move_path[bot_id].pop();
                            }
                            if(state.gotomine[bot_id] == 1){
                                state.resourcequeue[0].push_back(state.gotomine_pos[bot_id]);
                                state.gotomine[bot_id] = 0;
                            }
                            if (state.bot_map[bot_id].current_energy >= state.mapconfig.action_scan_energy){
                                int sub_index = state.subgrid_index(next_move);
                                for (auto pos : state.scan_centers[sub_index]){
                                    if (std::max(std::abs(pos.x - next_move.x), std::abs(pos.y - next_move.y)) <= state.mapconfig.action_scan_radius){
                                        json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, pos};
                                        command.addRequest(scan_request, state);
                                        bot_pair.second = 1;
                                        goto end;
                                    }
                                }
                            }
                        }
                        // Next move cell already scanned
                        else{
                            if (state.bot_map[bot_id].current_energy >= state.mapconfig.terrain_configs[state.land_map[next_move].terrain_id].move_energy_per_interval){
                                state.move_path[bot_id].pop();
                                if(state.move_path[bot_id].size() == 0 and state.gotomine[bot_id] == 1){
                                    state.gotomine[bot_id] = 0;
                                }
                                json::ActionMoveRequest move_request{state.game_id, state.player_id, state.player_key, bot_id, next_move};
                                command.addRequest(move_request, state);
                                bot_pair.second = 1;
                            }
                        }
                        goto end;
                    }
                    
                    // Win condition reached
                    finishgame:
                    if (over){
                        // Bot within transfer radius of Factory bot
                        if (state.bot_map[bot_id].position.square_distance(state.bot_map[state.factory_id].position) <= (state.mapconfig.action_transfer_max_distance*state.mapconfig.action_transfer_max_distance)){
                            Position final = state.bot_map[state.factory_id].position;
                            if (state.bot_map[bot_id].cargo.size() > 0)
                            {
                                int curr_energy = state.bot_map[bot_id].current_energy;
                                auto& resource_chunk = *state.bot_map[bot_id].cargo.begin();
                                int energy_transfer = state.mapconfig.action_transfer_amount_per_interval * int(curr_energy / state.mapconfig.action_transfer_energy_per_interval);
                                int transfer = std::min((int)resource_chunk.amount, energy_transfer);
                                resource_chunk.amount -= transfer;
                                if(transfer > 0){
                                    json::ActionTransferRequest transfer_request{state.game_id, state.player_id, state.player_key, bot_id, final, state.factory_id, ResourceChunk{resource_chunk.id, Kilogram{transfer}}};
                                    command.addRequest(transfer_request, state);
                                    bot_pair.second = 1;
                                }
                                goto end;
                            }
                            goto end;
                        }
                        // Find a path to the factory bot
                        else{
                            std::vector<Position> full_path = Algo.find_path(false,state.mapconfig.action_transfer_max_distance, state.obstacles, state.bot_map[state.factory_id].position, state.bot_map[bot_id].position, state.land_map);
                            for (int i = 1; i < (int)full_path.size(); i++)
                            {
                                state.move_path[bot_id].push(full_path[i]);
                            }
                            // Useless callback
                            // SPDLOG_INFO("CALL5: {}", state.move_path[bot_id].size());
                            if (state.bot_map[bot_id].current_energy == state.mapconfig.max_bot_energy){
                                json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {bot_x,bot_y}};
                                command.addRequest(scan_request, state);
                                bot_pair.second = 1;
                            }
                            goto end; 
                        }
                    }
                    // 2nd prefernce to mine
                    for (int i = -state.mapconfig.action_mine_max_distance; i <= state.mapconfig.action_mine_max_distance; i++){
                        for (int j = -state.mapconfig.action_mine_max_distance; j <= state.mapconfig.action_mine_max_distance; j++){
                            if((i*i) + (j*j) > (state.mapconfig.action_mine_max_distance*state.mapconfig.action_mine_max_distance)){
                                continue;
                            }
                            int mine_x = bot_x + i;
                            int mine_y = bot_y + j;
                            if(state.land_map.find({Kilometer(mine_x), Kilometer(mine_y)}) == state.land_map.end()){
                                 continue;
                            }
                            if (!(mine_x <= state.mapconfig.max_x - 1 and mine_y <= state.mapconfig.max_y - 1)){
                                continue;
                            }
                            for (auto chunk : state.land_map[{Kilometer(mine_x), Kilometer(mine_y)}].resources){
                                if (state.win_amount[chunk.id] > 0){
                                    if(state.bot_map[bot_id].current_energy < 0){
                                         goto end;
                                    }
                                    Kilogram energy_mine_amount =  (int(state.bot_map[bot_id].current_energy / state.mapconfig.resource_configs[chunk.id].mine_energy_per_interval));
                                    energy_mine_amount *= state.mapconfig.resource_configs[chunk.id].mine_amount_per_interval;
                                    // state.win_amount removed from min
                                    Kilogram mine_amount = std::min({chunk.amount, energy_mine_amount});
                                    if (mine_amount > 0)
                                    {
                                        json::ActionMineRequest mine_request{state.game_id, state.player_id, state.player_key, bot_id, {Kilometer(mine_x), Kilometer(mine_y)}, ResourceChunk{chunk.id,Kilogram(mine_amount)}};
                                        command.addRequest(mine_request, state);
                                        bot_pair.second = 1;
                                        // SPDLOG_INFO("MINE: {}, [{}, {}]", mine_amount, mine_x, mine_y);
                                    }
                                    goto end;
                                }
                            }
                        }
                    }
                    // Update build bot conditions
                    bool send_build = true;
                    for (auto chunk : state.build_amount){
                        if (cargo_amounts[chunk.first] < chunk.second){
                            send_build = false;
                        }
                    }
                    if(state.bot_map[bot_id].cargo.size() == 0){
                         send_build = false;
                    }
                    if(send_build and state.counter < state.max_bots and state.build_check){
                         // Bot within transfer radius of Factory bot
                        if (state.bot_map[bot_id].position.square_distance(state.bot_map[state.factory_id].position) <= (state.mapconfig.action_transfer_max_distance*state.mapconfig.action_transfer_max_distance)){
                            Position final = state.bot_map[state.factory_id].position;
                            if (state.bot_map[bot_id].cargo.size() > 0)
                            {
                                int curr_energy = state.bot_map[bot_id].current_energy;
                                auto& resource_chunk = *state.bot_map[bot_id].cargo.begin();
                                if(resource_chunk.id == 0){
                                    state.bot_map[bot_id].cargo.erase(state.bot_map[bot_id].cargo.begin());
                                    if (state.bot_map[bot_id].cargo.size() > 0){
                                        auto& resource_chunk = *state.bot_map[bot_id].cargo.begin();
                                    }
                                    else goto end;
                                }
                                int energy_transfer = state.mapconfig.action_transfer_amount_per_interval * int(curr_energy / state.mapconfig.action_transfer_energy_per_interval);
                                int transfer = std::min((int)resource_chunk.amount, energy_transfer);
                                resource_chunk.amount -= transfer;
                                if(transfer > 0){
                                    json::ActionTransferRequest transfer_request{state.game_id, state.player_id, state.player_key, bot_id, final, state.factory_id, ResourceChunk{resource_chunk.id, Kilogram{transfer}}};
                                    command.addRequest(transfer_request, state);
                                    bot_pair.second = 1;
                                }
                                goto end;
                            }
                            goto end;
                        }
                        // Find a path to the factory bot
                        else{
                            std::vector<Position> full_path = Algo.find_path(false,state.mapconfig.action_transfer_max_distance, state.obstacles, state.bot_map[state.factory_id].position,  state.bot_map[bot_id].position, state.land_map);
                            for (int i = 1; i < (int)full_path.size(); i++)
                            {
                                state.move_path[bot_id].push(full_path[i]);
                            }
                            // Useless callback
                            // SPDLOG_INFO("CALL3 {}", state.move_path[bot_id].size());
                            // std::cout<<"CALL3"<<" "<<state.move_path[bot_id].size()<<"\n";
                            if (state.bot_map[bot_id].current_energy == state.mapconfig.max_bot_energy){
                                json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {bot_x,bot_y}};
                                command.addRequest(scan_request, state);
                                bot_pair.second = 1;
                            }
                            goto end; 
                        }
                    }
                    // Mine the Granite Cell
                    if(state.mine_granite[bot_id] == 1){
                        Position final = state.mine_granite_pos[bot_id];  
                        if(state.land_map[{Kilometer(final.x), Kilometer(final.y)}].resources.size() == 0){
                            if(state.bot_map[bot_id].current_energy > state.mapconfig.action_scan_energy){
                                state.mine_granite[bot_id] = 0;
                                json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {final.x,final.y}};
                                command.addRequest(scan_request, state);
                                bot_pair.second = 1;
                            }
                            goto end;
                        }
                        else{
                            for (auto chunk : state.land_map[{Kilometer(final.x), Kilometer(final.y)}].resources){
                                if(state.bot_map[bot_id].current_energy < 0){
                                    goto end;
                                }
                                Kilogram energy_mine_amount =  (int(state.bot_map[bot_id].current_energy / state.mapconfig.resource_configs[chunk.id].mine_energy_per_interval));
                                energy_mine_amount *= state.mapconfig.resource_configs[chunk.id].mine_amount_per_interval;
                                // state.win_amount removed from min
                                Kilogram mine_amount = std::min({chunk.amount, energy_mine_amount});
                                if (mine_amount > 0)
                                {
                                    json::ActionMineRequest mine_request{state.game_id, state.player_id, state.player_key, bot_id, {Kilometer(final.x), Kilometer(final.y)}, ResourceChunk{chunk.id,Kilogram(mine_amount)}};
                                    command.addRequest(mine_request, state);
                                    bot_pair.second = 1;
                                    // SPDLOG_INFO("MINE: {}, [{}, {}]", mine_amount, mine_x, mine_y);
                                }
                                goto end;
                            }
                        }
                        goto end;
                    }
                    // Mine Actions
                    auto start1 = std::chrono::high_resolution_clock::now();
                    #define all(v) v.begin(),v.end()
                    auto custom_resource = [&](const Position& a, const Position& b)
                    {
                        Position bot = {bot_x, bot_y};
                        return (bot.square_distance(a) < bot.square_distance(b));
                    };
                    sort(all(state.resourcequeue[bot_index]),custom_resource);
                    while (state.resourcequeue[bot_index].size() > 0){
                        Position resource_pos = state.resourcequeue[bot_index][0];
                        state.resourcequeue[bot_index].erase(state.resourcequeue[bot_index].begin());
                        state.granite_pos.push_back(resource_pos);
                        for (auto chunk : state.land_map[resource_pos].resources){
                            if (state.win_amount[chunk.id] > 0){
                                std::vector<Position> full_path = Algo.find_path(true,state.mapconfig.action_mine_max_distance, state.obstacles, resource_pos, state.bot_map[bot_id].position, state.land_map);
                                // No path to resoruce, jump to scan
                                if ((int)full_path.size() == 0){
                                    SPDLOG_INFO("MINEPOS NOT REACHABLE: [{}, {}] from [{}, {}]", resource_pos.x, resource_pos.y, bot_x, bot_y);
                                    break;
                                }
                                state.gotomine[bot_id] = 1;
                                state.gotomine_pos[bot_id] = resource_pos;
                                for (int i = 1; i < (int)full_path.size(); i++){
                                    state.move_path[bot_id].push(full_path[i]);
                                }
                                // Useless callback
                                auto end = std::chrono::high_resolution_clock::now();
                                std::chrono::duration<double> elapsed = end - start1;
                                double duration = double(elapsed.count());
                                if (duration > 1.0)
                                    // SPDLOG_INFO("******TIME ELAPSED FOR CALL1 : {}******", std::to_string(duration));
                                // SPDLOG_INFO("CALL1");
                                if (state.bot_map[bot_id].current_energy == state.mapconfig.max_bot_energy){
                                    json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {bot_x,bot_y}};
                                    command.addRequest(scan_request, state);
                                    bot_pair.second = 1;
                                }
                                goto end; 
                            }
                        }
                    }
                // Scan Actions
                scan:
                    // Scan any scan centers within radius
                    for (auto pos : state.scan_centers[bot_index]){
                        if (((bot_x - pos.x)*(bot_x - pos.x)) + ((bot_y - pos.y)*(bot_y - pos.y)) <= (state.mapconfig.action_scan_max_distance*state.mapconfig.action_scan_max_distance)){
                            if (state.bot_map[bot_id].current_energy >= state.mapconfig.action_scan_energy){
                                #define all(v) v.begin(),v.end()
                                auto it = find(all(state.scan_centers[bot_index]),Position(pos));
                                if(it != state.scan_centers[bot_index].end()){
                                    state.scan_centers[bot_index].erase(it);
                                } 
                                json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, pos};
                                command.addRequest(scan_request, state);
                                bot_pair.second = 1;
                            }
                            goto end;
                        }
                    }
                    // If all empty, see the granite_pos
                    if (state.scan_centers[bot_index].size() == 0){
                        if(state.resourcequeue[bot_index].size() == 0){
                            Position bot_pos = {bot_x,bot_y};
                            auto comp = [&] (const Position& pos1, const Position& pos2){
                                return (granite_op(pos1)  > granite_op(pos2));
                            };
                            sort(all(state.granite_pos), comp);
                            auto it = state.granite_pos.begin();
                            while(it != state.granite_pos.end()){
                                Position pos = (*it);
                                if(state.land_map[pos].resources.size() == 0){
                                    it = state.granite_pos.erase(it);
                                    continue;
                                }
                                std::vector<Position> full_path = Algo.find_path(true,state.mapconfig.action_mine_max_distance, state.obstacles, pos, state.bot_map[bot_id].position, state.land_map);
                                // No path to resoruce, jump to scan
                                if ((int)full_path.size() == 0){
                                    // SPDLOG_INFO("MINEPOS NOT REACHABLE: [{}, {}] from [{}, {}]", resource_pos.x, resource_pos.y, bot_x, bot_y);
                                    it++;
                                    continue;
                                }
                                else{
                                    it = state.granite_pos.erase(it);
                                    for (int i = 1; i < (int)full_path.size(); i++){
                                        state.move_path[bot_id].push(full_path[i]);
                                    }
                                    state.mine_granite_pos[bot_id] = pos;
                                    state.mine_granite[bot_id] = 1; 
                                    if (state.bot_map[bot_id].current_energy == state.mapconfig.max_bot_energy){
                                        json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {bot_x,bot_y}};
                                        command.addRequest(scan_request, state);
                                        bot_pair.second = 1;
                                    }
                                    goto end;
                                }
                            }
                        }
                        goto end;
                    }
                    // Find path to an unscanned center
                    auto start2 = std::chrono::high_resolution_clock::now();
                    #define all(v) v.begin(),v.end()
                    auto custom_scan = [&](const Position& a, const Position& b)
                    {
                        Position bot = {bot_x, bot_y};
                        return (bot.square_distance(a) < bot.square_distance(b));
                    };
                    sort(all(state.scan_centers[bot_index]),custom_scan);
                    while(state.scan_centers[bot_index].size() > 0){
                        Position unvis_pos = state.scan_centers[bot_index][0];
                        auto start4 = std::chrono::high_resolution_clock::now();
                        std::vector<Position> full_path = Algo.find_path(false,state.mapconfig.action_scan_max_distance, state.obstacles, unvis_pos, state.bot_map[bot_id].position, state.land_map);
                        if ((int)full_path.size() == 0){
                            // SPDLOG_INFO("SCANPOS NOT REACHABLE: [{}, {}] from [{}, {}]", unvis_pos.x, unvis_pos.y, bot_x, bot_y);
                            state.scan_centers[bot_index].erase(state.scan_centers[bot_index].begin());
                            continue;
                        }
                        else{
                            auto start3 = std::chrono::high_resolution_clock::now();
                            for (int i = 1; i < (int)full_path.size(); i++){
                                state.move_path[bot_id].push(full_path[i]);
                            }
                            auto end = std::chrono::high_resolution_clock::now();
                            std::chrono::duration<double> elapsed_seconds = end - start3;
                            double duration = double(elapsed_seconds.count());
                            if(duration > 1.0){ 
                                // SPDLOG_INFO("******TIME ELAPSED FOR ELSE STATEMENT: {}******", std::to_string(duration));
                            }
                            break;
                        }
                    }
                    auto end = std::chrono::high_resolution_clock::now();
                    std::chrono::duration<double> elapsed = end - start2;
                    double duration = double(elapsed.count());
                    // if (duration > 1.0)
                        // SPDLOG_INFO("******TIME ELAPSED FOR CALL4 : {}******", std::to_string(duration));
                    // Useless callback 
                    // SPDLOG_INFO("CALL4");
                    if (state.bot_map[bot_id].current_energy == state.mapconfig.max_bot_energy){
                        json::ActionScanRequest scan_request{state.game_id, state.player_id, state.player_key, bot_id, {bot_x,bot_y}};
                        command.addRequest(scan_request, state);
                        bot_pair.second = 1;
                    }
                    goto end;
                }
            }
        end:
        }
        auto firstend = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> firstelapsed = firstend - firststart;
        double analyze_duration = double(firstelapsed.count());
        // if (analyze_duration > 1.0)
            // SPDLOG_INFO("******TIME ELAPSED FOR ANALYSE : {}******", std::to_string(analyze_duration));
        
    }
    bool move_initial = true;
    State &state;
    Algorithm &Algo;
    miningbots::client::Command &command;
    std::mutex state_mutex; // Protect state during analyze and update
};
