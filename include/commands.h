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
#include "spdlog/spdlog.h"
#include <string>
#include "base_types.h"
#include "utils.h"
#include <glaze/core/context.hpp>
#include <glaze/json/write.hpp>
#include <ixwebsocket/IXWebSocket.h>
#include <random>
#include <spdlog/spdlog.h>
#include <stdexcept>
#include <commands.h>

namespace miningbots::client
{

    class Command
    {
    public:
        Command(const std::string &hostname, int port) : hostname(hostname), port(port) {
        }

        template <typename RequestType>
        void addRequest(const RequestType &request, State &state)
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            requests.push([this, request, &state]()
                          { sendRequest(request, state); });
        }

        void processQueue()
        {
            while (!requests.empty())
            {
                std::function<void()> request;
                {
                    std::lock_guard<std::mutex> lock(queue_mutex);
                    request = requests.front();
                    requests.pop();
                }
                // the request is a lambda function, and it only runs sendRequest once, need to do error handling
                request();
            }
        }

        std::string hostname;
        int port = 9003;
        std::queue<std::function<void()>> requests;
        std::queue<Update> updates;
        std::vector<int> commandcount;//0->move 1->mine 2->scan 3->transfer 4->build
        std::mutex queue_mutex;
        std::condition_variable cv;
        std::mutex update_mutex;
        void addUpdate(const Update &update)
        {
            std::lock_guard<std::mutex> lock(update_mutex);
            updates.push(update);
            cv.notify_one();
        }
        template <typename RequestType>
        void sendRequest(const RequestType &request, State &state)
        {
            auto json_str = glz::write_json(request);
            std::string endpoint = getEndpoint(request,state);

            // Use fmt::format for constructing the URL
            std::string url = fmt::format("http://{}:{}/{}", hostname, port, endpoint);

            cpr::Response response = cpr::PostAsync(
                                         cpr::Url{url},
                                         cpr::Header{{"Content-Type", "application/json"}},
                                         cpr::Parameters{{"request", json_str}})
                                         .get();
            JobId job_id = ReadJson<miningbots::json::ActionResponse>(response.text);
            if (job_id == 0 or job_id == 2)
            {
                // Useless callback
                state.bots_state[request.bot_id] = job_id;
                struct miningbots::json::Update *update = new miningbots::json::Update;
                *update = miningbots::json::Update{UpdateType::kUseless,0,0,{},{},{}};
                addUpdate(*update);
            }
            else
            {
                state.current_jobs[job_id] = request.bot_id;
                if (response.status_code != 200)
                {
                    SPDLOG_ERROR("Failed to send request: {} - {}", response.status_code, response.text);
                }
                else
                {
                    // SPDLOG_INFO("Request sent successfully: {}", response.text);
                }
            }
        }

        template <typename RequestType>
        std::string getEndpoint(const RequestType &request,State &state)
        {
            if constexpr (std::is_same_v<RequestType, json::ActionExplodeRequest>)
            {
                return "explode";
            }
            else if constexpr (std::is_same_v<RequestType, json::ActionBuildBotRequest>)
            {   
                state.commandcount[4]++;
                return "build_bot";
            }
            else if constexpr (std::is_same_v<RequestType, json::ActionMineRequest>)
            {   
                state.commandcount[1]++;
                return "mine";
            }
            else if constexpr (std::is_same_v<RequestType, json::ActionMoveRequest>)
            {
                state.commandcount[0]++;
                return "move";
            }
            else if constexpr (std::is_same_v<RequestType, json::ActionScanRequest>)
            {
                state.commandcount[2]++;
                return "scan";
            }
            else if constexpr (std::is_same_v<RequestType, json::ActionTransferRequest>)
            {    
                state.commandcount[3]++;
                return "transfer";
            }

            throw std::runtime_error("Unknown request type");
        }
    };
} // namespace miningbots::client
