#pragma once
#include "../thirdparty/netlib/src/netlib.h"
#include "../thirdparty/json.hpp"

#include "../thirdparty/httplib.h"

#include <iostream>
#include "OpusWrapper.h"

namespace transmitter {
  class SessionManager {
    httplib::SSLClient* mClient;
    std::string mMasterServer;
    std::string mId;

    udp_socket mSocket;
    udp_packet* mPacket;
    ip_address mAddress;

    int mLocalPort;

  public:
    SessionManager(std::string masterServer, int apiPort, int udpPort) {
      mMasterServer = masterServer;
      mClient = new httplib::SSLClient(mMasterServer, apiPort);
      // mClient->set_ca_cert_path("./ca-bundle.crt");
      mClient->enable_server_certificate_verification(false);

      if (netlib_init() == -1) {
        // DBGMSG("Can't init sdl net!\n");
        return;
      }

      mPacket = netlib_alloc_packet(MAX_PACKET_SIZE);
      if (!mPacket) {
        // DBGMSG("SDLNet_AllocPacket: %s\n", netlib_get_error());
        return;
      }

      if (netlib_resolve_host(&mAddress, masterServer.c_str(), apiPort)) {
        mPacket->address.host = mAddress.host;
        mPacket->address.port = mAddress.port;
        mSocket = netlib_udp_open(0);
        mLocalPort = netlib_udp_get_peer_address(mSocket, -1)->port;
      }
      
    }

    bool init(std::string id = "") {
      std::string req = "/register?port=" + std::to_string(mLocalPort);
      if (!id.empty()) {
        req += "&id=" + id;
      }

      const auto res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        nlohmann::json json(res->body);
        mId = json["id"].get<std::string>();
        std::cout << res->body << std::endl;
        return true;
      }
      return false;
    }

    bool connect(std::string id) {
      std::string req = "/connect?id=" + mId + "&peer=" + id;
      const auto res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        nlohmann::json json(res->body);
        std::cout << res->body << std::endl;
        if (json["success"].get<bool>()) {
          return true;
        }
      }
      return false;
    }

    void sendOpusPacket(unsigned char* data, int length) const {
      memcpy(mPacket->data, data, length);
      mPacket->data = data;
      mPacket->len = length;
      netlib_udp_send(mSocket, -1, mPacket);
    }

    int pollOpusPacket(unsigned char* data) const {
      if (netlib_udp_recv(mSocket, mPacket)) {
        memcpy(data, mPacket->data, mPacket->len);
        return mPacket->len;
      }
      return 0;
    }

    ~SessionManager() {
      delete mClient;
      netlib_udp_close(mSocket);
      netlib_free_packet(mPacket);
      netlib_quit();
    }
  };
}
