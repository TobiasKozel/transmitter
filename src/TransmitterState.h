#pragma once
#include <string>

namespace transmitter {
  struct State {
    bool directConnection = false;
    std::string hostname; // hostname of the relay server or peer for direct connections
    int port; // same as above for the port
    std::string id; // the own id if using a server
    std::string peer; // the peer id if using a server
  };
}
