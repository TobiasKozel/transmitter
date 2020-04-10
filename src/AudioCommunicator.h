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

    WrappedOpusDecoder mOpusDecoder;
    WrappedOpusEncoder mOpusEncoder;

    DecoderBase* mActiveDecoder = nullptr;
    EncoderBase* mActiveEncoder = nullptr;

    udp_socket mSocket = nullptr;
    udp_packet* mPacket = nullptr;
    ip_address mAddress; // Address to send the packets to
    int mLocalPort = 0;
    bool mUdpReady = false; // Means the socket can send and receive data


    const int mChannels = 2; // No idea if this will ever change
    int mBufferSize = 2048;
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
        mPacket->address.host = mAddress.host; // These are both in big endian
        mPacket->address.port = mAddress.port; // These are both in big endian
        mSocket = netlib_udp_open(localPort); // if it's zero, a random free port will be opened
        mLocalPort = netlib_swap_BE16(netlib_udp_get_peer_address(mSocket, -1)->port);
        mUdpReady = true;
      }

      setEncoder(mOpusDecoder.getName());
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
      if (mUdpReady || mActiveEncoder != nullptr) {
        mPacket->len = mActiveEncoder->pushSamples(inputs, nFrames, mPacket->data);

        if (mPacket->len > MAX_PACKET_SIZE) {
          assert(false);
        }
        if (mPacket->len > 0) {
          netlib_udp_send(mSocket, -1, mPacket);
        }
      }

      /**
       * Decode the remote packets
       */
      int out = 0;
      if (mUdpReady) {
        while (netlib_udp_recv(mSocket, mPacket)) { // we might have a few packets queued
          if (mActiveDecoder != nullptr && mActiveDecoder->compareName(mPacket->data)) {
            out = mActiveDecoder->pushPacket(mPacket->data, mPacket->len, outputs, nFrames);
          } else {
            /**
             * Check against the available decoders and load the right one
             */
            if (mOpusDecoder.compareName(mPacket->data)) {
              mActiveDecoder = &mOpusDecoder;
            }
          }
        }
      }

      if (out < nFrames) { // If we didn't get enough samples or none at all, fill the output with silence
        for (int i = out; i < nFrames; i++) {
          for (int c = 0; c < mChannels; c++) {
            outputs[c][i] = 0; // output silence if there's nothing decoded
          }
        }
      }
    }

    void setBufferSize(const int size) {
      mBufferSize = size;
      if (mActiveDecoder != nullptr) {
        mActiveDecoder->resizeBuffer(size);
      }
    }

    void setEncoder(const char* name) {
      if (mOpusEncoder.compareName(name)) {
        mActiveEncoder = &mOpusEncoder;
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
