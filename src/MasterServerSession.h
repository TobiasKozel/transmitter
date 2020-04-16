#pragma once

#include "../thirdparty/json.hpp"

#include "../thirdparty/httplib.h"

#include <iostream>

#include "./AudioCommunicator.h"

#include "./URLParser.h"

namespace transmitter {
  class MasterServerSession {
    const float API_VESION = 0.1;
    httplib::Client* mClient = nullptr; // No idea how this will work with multiple sessions

    struct {
      std::string displayName;
      std::string address;
      int udpPort = 0;
      int apiPort = 0;
    } mMasterServer;


    std::string mId; // The own id of the client
    std::string mPeer; // The id of the client peer we're listening to

    std::string mOwnAddress; // Address shown to copy paste

    AudioCommunicator* mCommunicator = nullptr;

    bool mError = true;
  public:
    MasterServerSession(std::string masterServer, std::string ownId = "") {
      URLParser url(masterServer);
      if (!url.valid) { return; }
      mMasterServer.displayName = url.reconstruct(true, true, true);
      mMasterServer.address = url.host;
      mMasterServer.apiPort = url.port;

      if (url.ssl) {
#ifdef CPPHTTPLIB_OPENSSL_SUPPORT
        httplib::SSLClient* sslclient = new httplib::SSLClient(mMasterServer.address, mMasterServer.apiPort);
        mClient = sslclient;
        // sslclient ->set_ca_cert_path("./ca-bundle.crt");
        sslclient->enable_server_certificate_verification(false);
#else
        assert(false); // SSL master server, but not compiled with ssl support
        return;
#endif
      } else {
        mClient = new httplib::Client(mMasterServer.address, mMasterServer.apiPort);
      }

      /**
       * Get the udp port for the audio packets
       */
      auto res = mClient->Get("/get_api_info");
      if (res && res->status == 200) {
        nlohmann::json json = nlohmann::json::parse(res->body);
        try {
          mMasterServer.udpPort = json["port"].get<int>();
          if (json["version"].get<float>() > API_VESION) {
            assert(false);
            return;
          }
        } catch (...) { return; }
      } else { return; }


      /**
       * We now know where to send the UDP packets, so open up a socket
       */
      mCommunicator = new AudioCommunicator(mMasterServer.address, mMasterServer.udpPort);

      /**
       * Get the client id from the server
       */
      std::string req = "/get_id?port=" + std::to_string(mCommunicator->getLocalPort());
      if (!ownId.empty()) { req += "&id=" + ownId; }

      res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        nlohmann::json json = nlohmann::json::parse(res->body);
        try {
          mId = json["id"].get<std::string>();
        } catch (...) { return; }
      } else { return; }

      mOwnAddress = url.reconstruct(true, false, false) + "/" + mId;

      mError = false;

      connectAsListener(url.path);
    }

    ~MasterServerSession() {
      delete mClient;
      delete mCommunicator;
    }

    bool connectAsListener(std::string id) {
      const auto idIndex = id.find('!');
      if (idIndex != std::string::npos) {
        id = id.substr(idIndex);
      }
      if (id.size() < 1) {
        mPeer = "";
        return false; // We'll only consider IDs longer than 4 characters valid
      }

      std::string req = "/connect_as_listener?id=" + mId + "&peer=" + id;
      const auto res = mClient->Get(req.c_str());
      if (res && res->status == 200) {
        nlohmann::json json = nlohmann::json::parse(res->body);
        std::cout << res->body << std::endl;
        if (json["success"].get<bool>()) {
          mPeer = id;
          return true;
        }
      }
      mPeer = "";
      return false;
    }

    const char* getId() const {
      return mId.c_str();
    }

    const char* getOwnAddress() const {
      return mOwnAddress.c_str();
    }

    bool getError() const {
      return mError;
    }

    void setBufferSize(double size) {
      if (mCommunicator != nullptr) {
        mCommunicator->setBufferSize(size);
      }
      
    }

    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) const {
      if (mCommunicator != nullptr) {
        mCommunicator->ProcessBlock(inputs, outputs, nFrames);
      }
    }
  };
}