#pragma once
#include "../thirdparty/netlib/src/netlib.h"
#include "../thirdparty/json.hpp"

#include "../thirdparty/httplib.h"

#include <iostream>
#include "OpusWrapper.h"

namespace transmitter {
  class SessionManagerBase {
  protected:
    udp_socket mSocket = nullptr;
    udp_packet* mPacket = nullptr;
    ip_address mAddress;
    int mLocalPort;
    bool mUdpReady = false; // Means the socket can send and recieve data
    bool mCanSend = false;
    bool mInitDone = false;
    WrappedOpusDecoder mDecoder;
    WrappedOpusEncoder mEncoder;
  public:

    virtual ~SessionManagerBase() {
      if (mUdpReady) {
        netlib_udp_close(mSocket);
      }
      netlib_free_packet(mPacket);
      netlib_quit();
    }
    /**
     * Will set up all the sockets and a http client
     * Needs to be called before anything else
     */
    virtual bool init(std::string masterServer, int udpPort, int apiPort = 0, int localUdpPort = 0) {
      /**
       * The udp init is in the base class since both direct and relay based
       * communication will use udp to send opus packets
       */
      if (netlib_init() == -1) {
        // DBGMSG("Can't init sdl net!\n");
        return false;
      }

      mPacket = netlib_alloc_packet(MAX_PACKET_SIZE);
      if (!mPacket) {
        // DBGMSG("SDLNet_AllocPacket: %s\n", netlib_get_error());
        return false;
      }

      if (netlib_resolve_host(&mAddress, masterServer.c_str(), udpPort) == -1) {
        assert(false);
      }
      else {
        mPacket->address.host = mAddress.host; // These are both in big endian
        mPacket->address.port = mAddress.port; // These are both in big endian
        mSocket = netlib_udp_open(localUdpPort); // if it's zero, a random free port will be opened
        mLocalPort = netlib_swap_BE16(netlib_udp_get_peer_address(mSocket, -1)->port);
        mUdpReady = true;
      }
    }

    /**
     * If successful, opus packets can be send
     */
    virtual bool start(std::string id = "") { return true; }

    /**
     * Will tell the relay server to send packets from the client with id provided
     */
    virtual bool connectAsListener(std::string id) { return true; }

    void pushSamples(float** samples, int count) {
      if (!mPacket || !mUdpReady || !mCanSend) {
        return;
      }
      mEncoder.pushSamples(samples, count);
      mPacket->len = mEncoder.popPacket(mPacket->data);
      if (mPacket->len) {
        netlib_udp_send(mSocket, -1, mPacket);
      }
    }

    void pollSamples(float** result, int count) {
      if (!mPacket || !mUdpReady) {
        return;
      }
      while (netlib_udp_recv(mSocket, mPacket)) { // we might have a few packets queued
        mDecoder.pushPacket(mPacket->data, mPacket->len);
      }
      mDecoder.popSamplesOrSilence(result, count);
    }
  };

  /**
   * This one will not involve a relay sever and means firewalls might create some problems
   */
  class DirectConnectionManager : public SessionManagerBase {
  public:
    bool init(std::string masterServer, int udpPort, int apiPort, int localUdpPort) override {
      return SessionManagerBase::init(masterServer, udpPort, 0, udpPort);
    }

    bool start(std::string id = "") override {
      mUdpReady = true;
      mCanSend = true;
    }
  };

  /**
   * This one will use a central server to distribute
   */
  class SessionManager : public SessionManagerBase {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
    httplib::SSLClient* mClient = nullptr; // No idea how this will work with multiple sessions
#else
    httplib::Client* mClient = nullptr; // No idea how this will work with multiple sessions
#endif
    std::string mMasterServer;
    std::string mId;
  public:
    bool init(std::string masterServer, int udpPort, int apiPort, int localUdpPort = 0) override {
      if (mInitDone) { return true; }
      if (mClient == nullptr) {
        mMasterServer = masterServer;
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        mClient = new httplib::SSLClient(mMasterServer, apiPort);
        // mClient->set_ca_cert_path("./ca-bundle.crt");
        mClient->enable_server_certificate_verification(false);
#else
        mClient = new httplib::Client(mMasterServer, apiPort);
#endif
      }
      mInitDone = SessionManagerBase::init(masterServer, udpPort, apiPort, localUdpPort);
      return mInitDone;
    }

    bool start(std::string id = "") override {
      if (!mInitDone) { return false; }
      std::string req = "/register?port=" + std::to_string(mLocalPort);
      if (!id.empty()) {
        req += "&id=" + id;
      }

      const auto res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        nlohmann::json json = nlohmann::json::parse(res->body);
        try {
          mId = json["id"].get<std::string>();
          std::cout << res->body << std::endl;
          mCanSend = true;
          return true;
        }
        catch (...) {
          mCanSend = false;
          return false;
        }
      }
      return false;
    }

    bool connectAsListener(std::string id) override {
      std::string req = "/connect_listener?id=" + mId + "&peer=" + id;
      const auto res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        nlohmann::json json = nlohmann::json::parse(res->body);
        std::cout << res->body << std::endl;
        if (json["success"].get<bool>()) {
          return true;
        }
      }
      return false;
    }

    std::string getId() const {
      return mId;
    }

    ~SessionManager() {
      delete mClient;
    }
  };

}
