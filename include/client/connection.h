#pragma once

#include "base_types.h"

#include <cpr/cpr.h>
#include <ixwebsocket/IXWebSocket.h>
#include <string>

namespace miningbots::client
{

  class Connection
  {
  public:
    Connection() = delete;
    Connection(std::string hostname, int port, std::string end_point,
               ix::OnMessageCallback ws_callback);
    ~Connection();
    ix::WebSocketSendInfo
    sendWSText(std::string const &text,
               const ix::OnProgressCallback &onProgressCallback = nullptr);
    cpr::Response sendGet(cpr::Url &&url, cpr::Parameters &&parameters);

  private:
    std::string hostname_;
    int port_;
    ix::WebSocket websocket_;
    ix::OnMessageCallback ws_callback_;

    const Second kPingInterval = 45;
    const MilliSecond kConnectWait = 30000;
    const MilliSecond kConnectSleep = 50;
    const MilliSecond kMaxRecconectWait = 3600000;
    const MilliSecond kMinReconnectWait = 12800;
  };

} // namespace miningbots::client