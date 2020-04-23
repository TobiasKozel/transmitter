#pragma once
#include "CodecBase.h"
#include <cstring>

namespace transmitter {
  class RAWEncoder : public EncoderBase {
    static constexpr int mFrameSize = MAX_PACKET_SIZE / 10;
    float mInterleaved[mFrameSize * 2] = { 0 }; // Max opus frame size at 48kHz
    float mPreInterleave[mFrameSize] = { 0 };
  public:
    RAWEncoder() {
      strcpy(mName, "RAWC");
    }

    ~RAWEncoder() {
    }

    int getMaxBlockSize() const override {
      return  MAX_PACKET_SIZE / 10;
    }

  private:
    int encodeImpl(MultiRingBuffer<sample, 2>* mBuffer, unsigned char* result) override {
      if (mFrameSize <= mBuffer->inBuffer()) {
        const int packetSize = mFrameSize * sizeof(float) * 2; // two channels
        for (int c = 0; c < 2; c++) {
          mBuffer->get(mPreInterleave, mFrameSize, c);
          for (int i = c, s = 0; s < mFrameSize; i += 2, s++) {
            mInterleaved[i] = mPreInterleave[s]; // interleave the signal
          }
        }
        // don't care about endiannes in this day and age
        memcpy(result, mInterleaved, packetSize);
        return packetSize;
      }
      return 0;
    }
  };

  class RAWDecoder : public DecoderBase {
    static constexpr int mFrameSize = MAX_PACKET_SIZE / 10;
    float mInterleaved[mFrameSize * 2] = { 0 }; // Interleaved buffer to encode data
    float mPostInterleave[mFrameSize] = { 0 }; // Buffer to use for interleaving
    /**
     * This is the local buffer. The opus decoder will not always
     * produce samples if the packets arrive in the wrong order.
     * So this will trade latency for smoother playback
     */
    int mBufferSize = 2048;
  public:
    RAWDecoder() {
      strcpy(mName, "RAWC");
    }

    ~RAWDecoder() {
    }

  private:
    int decodeImpl(const unsigned char* data, const int size, DecodeCallback& callback) override {
      const int frames = size / sizeof(float) / 2; // Two channels
      if (frames > 0) {
        memcpy(mInterleaved, data, size);
        for (int c = 0; c < 2; c++) {
          for (int i = c, s = 0; s < frames; i += 2, s++) {
            mPostInterleave[s] = mInterleaved[i];
          }
          callback(frames, mPostInterleave, c);
        }
      }
      return frames;
    }
  };
}