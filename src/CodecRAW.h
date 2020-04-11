#pragma once
#include "CodecBase.h"
#include <cstring>

namespace transmitter {
  class RAWEncoder : public EncoderBase {
    float mInterleaved[2880 * 2] = { 0 }; // Max opus frame size at 48kHz
    float mPreInterleave[2880] = { 0 };
    int mFrameSize = 240; // The size of the blocks handed over to the encoder
  public:
    RAWEncoder() {
      strcpy(mName, "RAWC");
    }

    ~RAWEncoder() {
    }

  private:
    int encodeImpl(float** samples, int count, unsigned char* result) override {
      mBuffer[0].add(samples[0], count);
      mBuffer[1].add(samples[1], count);
      if (mFrameSize <= mBuffer[0].inBuffer()) {
        for (int c = 0; c < 2; c++) {
          mBuffer[c].get(mPreInterleave, mFrameSize);
          for (int i = c, s = 0; s < mFrameSize; i += 2, s++) {
            mInterleaved[i] = mPreInterleave[s]; // interleave the signal
          }
        }
        int size = 0;
        return size;
      }
      return 0;
    }
  };

  class RAWDecoder : public DecoderBase {
    float mInterleaved[2880 * 2] = { 0 }; // Interleaved buffer to encode data
    float mPostInterleave[2880] = { 0 }; // Buffer to use for interleaving
    /**
     * This is the local buffer. The opus decoder will not always
     * produce samples if the packets arrive in the wrong order.
     * So this will trade latency for smoother playback
     */
    int mBufferSize = 2048;
  public:
    RAWDecoder() {
      strcpy(mName, "RAWC");
      resizeBuffer(4096);
    }

    ~RAWDecoder() {
    }

  private:
    int decodeImpl(const unsigned char* data, const int size, float** result, int requestedSamples) override {
      const int frames = 0;
      if (frames > 0) {
        for (int c = 0; c < 2; c++) {
          for (int i = c, s = 0; s < frames; i += 2, s++) {
            mPostInterleave[s] = mInterleaved[i];
          }
          mBuffer[c].add(mPostInterleave, frames);
        }
      }
      int inbuf = mBuffer[0].inBuffer();
      iplug::DBGMSG("decode buffer %i\n", inbuf);
      if (mBuffer[0].inBuffer() >= requestedSamples) {
        for (int c = 0; c < 2; c++) {
          mBuffer[c].get(result[c], requestedSamples);
        }
        return requestedSamples;
      }
      return 0;
    }
  };
}