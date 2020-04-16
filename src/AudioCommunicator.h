#pragma once

#include "../thirdparty/netlib/src/netlib.h"

#include <string>
#include "./Types.h"
#include "./MultiCodec.h"

namespace transmitter {
  /**
   * Is able to send and receive the audio packets and will decode them
   * doesn't care about sessions or whatnot and has it's own udp socket
   */
  class AudioCommunicator {
    MultiCodec mCodec;
    udp_socket mSocket = nullptr;
    udp_packet* mPacket = nullptr;
    ip_address mAddress = { 0, 0 }; // Address to send the packets to
    int mLocalPort = 0;
    bool mUdpReady = false; // Means the socket can send and receive data
    TRANSMITTER_NO_COPY(AudioCommunicator)
  public:
    AudioCommunicator(const std::string& address, const int remotePort, const int localPort = 0) {
      mPacket = netlib_alloc_packet(MAX_PACKET_SIZE);
      if (!mPacket) {
        assert(false);
        return;
      }

      if (netlib_resolve_host(&mAddress, address.c_str(), remotePort) == -1) {
        assert(false);
      } else {
        mSocket = netlib_udp_open(localPort); // if it's zero, a random free port will be opened
        mLocalPort = netlib_swap_BE16(netlib_udp_get_peer_address(mSocket, -1)->port);
        mUdpReady = true;
      }

      // mCodec.setEncoder("RAWC");
    }

    ~AudioCommunicator() {
      if (mUdpReady) {
        netlib_udp_close(mSocket);
      }
      netlib_free_packet(mPacket);
    }

    void ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
      /**
       * Encode the input
       */
      if (mUdpReady) {
        mPacket->len = mCodec.encode(inputs, nFrames, mPacket->data); // push in all the samples to encode

        do {
          if (mPacket->len > MAX_PACKET_SIZE) {
            assert(false);
          }
          mPacket->address.host = mAddress.host; // These are both in big endian
          mPacket->address.port = mAddress.port; // These are both in big endian
          netlib_udp_send(mSocket, -1, mPacket);
          /**
           * There might still be more data left to send
           * that didn't fit into a single packet
           */
          mPacket->len = mCodec.encode(inputs, 0, mPacket->data);
        } while (mPacket->len > 0);
      }

      /**
       * Decode the remote packets
       */
      if (mUdpReady) {
        while (netlib_udp_recv(mSocket, mPacket)) {
          // we might have a few packets queued
          mCodec.decode(mPacket->data, mPacket->len);
        }
        mCodec.popSamples(outputs, nFrames);
      }
    }

    bool isReady() const {
      return mUdpReady;
    }

    int getLocalPort() const {
      return mLocalPort;
    }

    void setBufferSize(double size) {
      mCodec.setBufferSize(size);
    }

  };
}
