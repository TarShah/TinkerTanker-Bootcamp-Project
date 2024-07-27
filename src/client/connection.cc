#include "client/connection.h"
#include "cpr/api.h"
#include "cpr/response.h"
#include "ixwebsocket/IXWebSocketSendInfo.h"

#include <ixwebsocket/IXWebSocket.h>
#include <spdlog/spdlog.h>

namespace miningbots::client {

Connection::Connection(std::string hostname, const int port,
                       std::string end_point, ix::OnMessageCallback ws_callback)
    : hostname_(hostname), port_(port), websocket_(),
      ws_callback_(ws_callback) {
  ix::SocketTLSOptions tls_options;

  tls_options.certFile = "../certs/selfsigned-client-crt.pem";
  tls_options.keyFile = "../certs/selfsigned-client-key.pem";
  tls_options.caFile = "NONE";
  tls_options.tls = true;

  websocket_.setTLSOptions(tls_options);

  websocket_.setUrl(fmt::format("ws://{}:{}/{}", hostname, port, end_point));
  SPDLOG_INFO("Connecting {}:{}", hostname_, port_);

  // Optional heart beat, sent every 45 seconds when there is not any traffic
  // To make sure that load balancers do not kill an idle connection.
  websocket_.setPingInterval(kPingInterval);

  // Per message deflate connection is enabled by default. You can tweak its
  // Parameters or disable it
  websocket_.disablePerMessageDeflate();

  websocket_.setOnMessageCallback(ws_callback_);
  websocket_.setMaxWaitBetweenReconnectionRetries(kMaxRecconectWait);
  websocket_.setMinWaitBetweenReconnectionRetries(kMinReconnectWait);

  websocket_.start();

  int remaining_wait_time_ms = kConnectWait;
  while (websocket_.getReadyState() != ix::ReadyState::Open) {
    std::this_thread::sleep_for(std::chrono::milliseconds(kConnectSleep));
    remaining_wait_time_ms -= kConnectSleep;
    if (remaining_wait_time_ms < 0) {
      auto error_message =
          fmt::format("Connection timeout {}:{}", hostname, port);
      SPDLOG_ERROR(error_message);
      throw std::runtime_error(error_message);
    }
  }
};

Connection::~Connection() { websocket_.stop(); }

ix::WebSocketSendInfo
Connection::sendWSText(std::string const &text,
                       const ix::OnProgressCallback &on_progress_callback) {
  return websocket_.sendText(text, on_progress_callback);
}

cpr::Response Connection::sendGet(cpr::Url &&url,
                                  cpr::Parameters &&parameters) {
  cpr::Response my_response = cpr::Get(url, parameters);
  fmt::println("status code: {}, content-type: {}", my_response.status_code,
               my_response.header["content-type"]);
  SPDLOG_INFO("{}", my_response.text);
  return my_response;
}
}
