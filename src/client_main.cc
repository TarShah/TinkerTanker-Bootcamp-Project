#include "algo.h"
#include "analyze.h"
#include "base_types.h"
#include "client/connection.h"
#include "commands.h"
#include "spdlog/fmt/bundled/core.h"
#include "state.h"
#include "utils.h"
#include "json/action_build_bot.h"
#include "json/action_move_request.h"
#include "json/action_response.h"
#include "json/action_scan_request.h"
#include "json/client_subscribe_request.h"
#include "json/create_map_request.h"
#include "json/error_response.h"
#include "json/game_status.h"
#include "json/get_games_response.h"
#include "json/join_game_request.h"
#include "json/join_game_response.h"
#include "json/map_config.h"
#include "json/position.h"
#include "json/resource_config.h"
#include "json/server_config.h"
#include "json/update.h"
#include <chrono>
#include <cpr/cpr.h>
#include <glaze/core/context.hpp>
#include <glaze/json/write.hpp>
#include <ixwebsocket/IXWebSocket.h>
#include <mutex>
#include <random>
#include <spdlog/spdlog.h>
#include <stdexcept>

using namespace miningbots;

std::default_random_engine rng{std::random_device{}()};
std::uniform_int_distribution<Id> id_generator{};

std::string config_path{"../config/"};
auto str_buffer = ReadFile(config_path + "server_config.json");

auto server_config = ReadJson<miningbots::json::ServerConfig>(str_buffer);

std::string hostname = server_config.hostname;
int port = server_config.port; 
GameId game_id;
miningbots::client::Command command(hostname, port);
std::mutex analyze_mutex;

std::vector<Id> generate_player_keys(int n) {
  std::vector<Id> player_keys{};
  for (int i = 0; i < n; i++)
    player_keys.push_back(id_generator(rng));
  return player_keys;
}
// string_view is and advanced better string class which is more efficient
void ThrowExceptionIfError(std::string_view response_text) {
  if (response_text.starts_with("{\"err")) {
    auto error_response =
        ReadJson<miningbots::json::ErrorResponse>(response_text);
    throw std::runtime_error(std::string{response_text});
  }
}

int main(int, char **) {
  State state;
  state.player_key = 3067498284;
  std::cout<<port<<"\n";
  Algorithm Algo(state.mapconfig);
  Analyze analyze_obj(state, command, Algo);
  analyze_obj.run();
  spdlog::set_pattern("[%D %H:%M:%S.%F] [%^%l%$] [%s %!:%#] [%oms] [%ius]  %v");
  spdlog::set_level(spdlog::level::debug);
  std::string json_str{};

  std::string config_path{"../config/"};
  auto str_buffer = ReadFile(config_path + "player_keys.json");
  auto valid_player_keys_ = ReadJson<std::vector<Key_t>>(str_buffer);

  // std::vector<miningbots::json::ResourceConfig> resource_configs{{.name =
  // "gold"}, {.name = "iron"}}; json_str = glz::write_json(resource_configs);
  // fmt::println("{}", json_str);

  struct miningbots::json::Update *update = new miningbots::json::Update;
  ix::OnMessageCallback websocket_message_callback =
      ([update, &state, &analyze_obj](const ix::WebSocketMessagePtr &message) {
        if (message->type == ix::WebSocketMessageType::Message) {
          // SPDLOG_INFO("Received ws: {}", message->str);
          std::string info = message->str;
          if (message->str.starts_with("{\"err")) {
            auto error_response =
                ReadJson<miningbots::json::ErrorResponse>(message->str);
          } else if (message->str.starts_with("{\"upd")) {
            *update = ReadJson<miningbots::json::Update>(message->str);
            command.addUpdate(*update);
          }
        } else if (message->type == ix::WebSocketMessageType::Fragment) {
          // no op
        } else if (message->type == ix::WebSocketMessageType::Error) {
          auto error_message = fmt::format(
              "Error: {}\nretries: {}\nWait time(ms): {}\nHTTP Status: {}",
              message->errorInfo.reason, message->errorInfo.retries,
              message->errorInfo.wait_time, message->errorInfo.http_status);
          SPDLOG_ERROR(error_message);
        } else if (message->type == ix::WebSocketMessageType::Open)
          SPDLOG_INFO("Connected {}:{}", hostname, port);
        else if (message->type == ix::WebSocketMessageType::Close)
          SPDLOG_INFO("Received close.");
        else if (message->type == ix::WebSocketMessageType::Ping) {
          // no op
        } else if (message->type == ix::WebSocketMessageType::Pong) {
          // no op
        }
      });

  auto response =
      cpr::Get(cpr::Url{fmt::format("http://{}:{}/games", hostname, port)});
  SPDLOG_INFO("{}", response.text);
  ThrowExceptionIfError(response.text);
  auto get_games_response =
      ReadJson<miningbots::json::GetGamesResponse>(response.text);
  for (auto &g : get_games_response)
    if (g.game_status == miningbots::json::GameStatus::kNotStarted ||
        g.game_status == miningbots::json::GameStatus::kOpen) {
      game_id = g.game_id;
      break;
    }
  if (game_id == 0) {
    SPDLOG_ERROR("No open games found. {}", response.text);
    return 1;
  }
   json::CreateMapRequest create_map_request{10,
                                          10,
                                          {{1, 1}},
                                          {{"XXXXXXXXXX"},
                                           {"XXXXXXXXXX"},
                                           {"XXXXXXXXXX"},
                                           {"          "},
                                           {"          "},
                                           {"          "},
                                           {"          "},
                                           {"XXXXXXXXXX"},
                                           {"XXXXXXXXXX"},
                                           {"XXXXXXXXXX"}}};
  // restart the game
  if (port == 9005) {
    response =
        cpr::Post(cpr::Url{fmt::format("http://{}:{}/restart_game", hostname,
        port)},
                  cpr::Header{{"Content-Type", "application/json"}},
                  cpr::Parameters{
                      {"game_id", glz::write_json(game_id)},
                      // {"create_map_request",glz::write_json(create_map_request)}
                  });
    SPDLOG_INFO("RESTARTED : {}", response.text);
    // take the game id again
    response =
        cpr::Get(cpr::Url{fmt::format("http://{}:{}/games", hostname, port)});
    SPDLOG_INFO("{}", response.text);
    ThrowExceptionIfError(response.text);
    get_games_response =
        ReadJson<miningbots::json::GetGamesResponse>(response.text);
    for (auto &g : get_games_response)
      if (g.game_status == miningbots::json::GameStatus::kNotStarted ||
          g.game_status == miningbots::json::GameStatus::kOpen) {
        game_id = g.game_id;
        break;
      }
    if (game_id == 0) {
      SPDLOG_ERROR("No open games found. {}", response.text);
      return 1;
    }
    SPDLOG_INFO("Found game id {}", game_id);

  }
  state.game_id = game_id;

  json::JoinGameRequest join_game_request{game_id, "Team's Team", 3067498284};
  response =
      cpr::Put(cpr::Url{fmt::format("http://{}:{}/join_game", hostname, port)},
               cpr::Header{{"Content-Type", "application/json"}},
               cpr::Parameters{
                   {"request", glz::write_json(join_game_request)},
               });
  ThrowExceptionIfError(response.text);
  auto player_id_1 = ReadJson<json::JoinGameResponse>(response.text);
  SPDLOG_INFO("Received player_id: {} for {}", player_id_1,
              join_game_request.player_name);
  client::Connection my_connection1(hostname, port, "update",
                                    websocket_message_callback);
  json::ClientSubscribeRequest client_subscribe_request{game_id, 3067498284,
                                                        player_id_1};
  my_connection1.sendWSText(glz::write_json(client_subscribe_request));
  state.player_id = player_id_1;
  // Get map config
  response =
      cpr::Get(cpr::Url{fmt::format("http://{}:{}/map_config", hostname, port)},
               cpr::Header{{"Content-Type", "application/json"}},
               cpr::Parameters{
                   {"game_id", glz::write_json(game_id)},
               });
  state.mapconfig = ReadJson<json::MapConfig>(response.text);
  state.win_cond_update();
  SPDLOG_INFO("{}", response.text);
  // json_str = glz::write_json(state.mapconfig);
  // fmt::println("{}", json_str);

  while (true) {
    command.processQueue();
  }
}
