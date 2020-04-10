#pragma once

#include "../thirdparty/netlib/src/netlib.h"
#include "./CodecOpus.h"
#include <string>

namespace transmitter {
  /**
   * Is able to send and receive the audio packets and will decode them
   * doesn't care about sessions or whatnot and has it's own udp socket
   */
  class AudioCommunicator {
    enum Encoders {
      OPUS_ENCODER,
      VBAN_ENCODER,
      ENCODER_COUNT
    };
    udp_socket mSocket = nullptr;
    udp_packet* mPacket = nullptr;
    ip_address mAddress; // Address to send the packets to
    int mLocalPort = 0;
    bool mUdpReady = false; // Means the socket can send and receive data

    WrappedOpusDecoder mOpusDecoder;
    WrappedOpusEncoder mOpusmEncoder;
    DecoderBase* mActiveDecoder = nullptr;
    EncoderBase* mActiveEncoder = nullptr;
    const int mChannels = 2; // No idea if this will ever change

  public:
    AudioCommunicator(const std::string& address, const int remotePort, const int localPort = 0) {
      mPacket = netlib_alloc_packet(MAX_PACKET_SIZE);
      if (!mPacket) {
        assert(false);
        return;
      }

      if (netlib_resolve_host(&mAddress, address.c_str(), remotePort) == -1) {
        assert(false);
      }
      else {
        mPacket->address.host = mAddress.host; // These are both in big endian
        mPacket->address.port = mAddress.port; // These are both in big endian
        mSocket = netlib_udp_open(localPort); // if it's zero, a random free port will be opened
        mLocalPort = netlib_swap_BE16(netlib_udp_get_peer_address(mSocket, -1)->port);
        mUdpReady = true;
      }

      setEncoder(OPUS_ENCODER);
    }

    ~AudioCommunicator() {
      if (mUdpReady) {
        netlib_udp_close(mSocket);
      }
      netlib_free_packet(mPacket);
    }

    void pushSamples(float** samples, const int count) const {
      if (!mUdpReady || mActiveEncoder == nullptr) {
        return;
      }
      mActiveEncoder->pushSamples(samples, count);
      mPacket->len = mActiveEncoder->popPacket(mPacket->data);
      if (mPacket->len > MAX_PACKET_SIZE) {
        assert(false);
      }
      if (mPacket->len) {
        netlib_udp_send(mSocket, -1, mPacket);
      }
    }

    void pollSamples(float** result, const int count) const {
      int out = 0;
      if (mUdpReady && mActiveDecoder != nullptr) {
        while (netlib_udp_recv(mSocket, mPacket)) { // we might have a few packets queued
          if (mActiveDecoder->compareName(mPacket->data)) {
            mActiveDecoder->pushPacket(mPacket->data, mPacket->len);
          }
          else {
            // TODO means we'll have to switch decoders
          }
        }
        out = mActiveDecoder->popSamples(result, count);
      }
      
      if (out < count) {
        for (int i = out; i < count; i++) {
          for (int c = 0; c < mChannels; c++) {
            result[c][i] = 0; // output silence if there's nothing decoded
          }
        }
      }
    }

    void setBufferSize(const int size) const {
      if (mActiveDecoder != nullptr) {
        mActiveDecoder->resizeBuffer(size);
      }
    }

    void setEncoder(Encoders enc) {
      if (enc == OPUS_ENCODER) {
        mActiveEncoder = &mOpusmEncoder;
      }
      if (enc == VBAN_ENCODER) {
        mActiveEncoder = nullptr;
      }
    }

    bool isReady() const {
      return mUdpReady;
    }

    int getLocalPort() const {
      return mLocalPort;
    }

  };
}
