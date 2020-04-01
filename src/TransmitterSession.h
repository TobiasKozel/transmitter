#pragma once
#include "../thirdparty/json.hpp"

#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../thirdparty/httplib.h"

#include "../thirdparty/netlib/src/netlib.h"
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
        mSocket = netlib_udp_open(0);
        mLocalPort = netlib_udp_get_peer_address(mSocket, -1)->port;
      }
      
    }

    void init(std::string id = "") {
      std::string req = "/register?port=" + std::to_string(mLocalPort);
      if (!id.empty()) {
        req += "&id=" + id;
      }

      const auto res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        mId = res->body;
        std::cout << res->body << std::endl;
      }
    }

    ~SessionManager() {
      delete mClient;
      netlib_udp_close(mSocket);
      netlib_free_packet(mPacket);
      netlib_quit();
    }
  };
}
