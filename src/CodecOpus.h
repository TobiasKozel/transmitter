#pragma once
#include "../thirdparty/opus/include/opus.h"
#include "CodecBase.h"
#include <cstring>
#include <cassert>

namespace transmitter {

  class WrappedOpusEncoder : public EncoderBase {
    OpusEncoder* mEncoder = nullptr; // The encoder itself
    float mInterleaved[2880 * 2]; // Max opus frame size at 48kHz
    float mPreInterleave[2880];
    int mPacketSize = 0; // The actual size of the packet
    int mFrameSize = 480; // The size of the blocks handed over to the encoder
  public:

    WrappedOpusEncoder() {
      int err;
      /**
       * We'll always use 2 channels at 48000kHz
       */
      mEncoder = opus_encoder_create(48000, 2, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &err);
      // opus_encoder_ctl(mEncoder, OPUS_SET_EXPERT_FRAME_DURATION(OPUS_FRAMESIZE_5_MS)); // TODO find out if this does anything for the latency
      opus_encoder_ctl(mEncoder, OPUS_SET_PHASE_INVERSION_DISABLED(1)); // Without this, mono sounds really bad at lower bit rates
      opus_encoder_ctl(mEncoder, OPUS_SET_BITRATE(128000)); // 128 kBit/s
      opus_encoder_ctl(mEncoder, OPUS_SET_SIGNAL(OPUS_SIGNAL_MUSIC));
      // opus_encoder_ctl(mEncoder, OPUS_SET_VBR(0)); // TODO find out if this does anything for the latency
    }

    ~WrappedOpusEncoder() {
      opus_encoder_destroy(mEncoder);
    }

    void pushSamples(float** samples, int count) override {
      mBuffer[0].add(samples[0], count);
      mBuffer[1].add(samples[1], count);
      if (mFrameSize >= mBuffer[0].inBuffer()) {
        for (int c = 0; c < 2; c++) {
          mBuffer[c].get(mPreInterleave, mFrameSize);
          for (int i = c, s = 0; s < mFrameSize; i += 2, s++) {
            mInterleaved[i] = mPreInterleave[s]; // interleave the signal
          }
        }
        mPacketSize = opus_encode_float(mEncoder, mInterleaved, mFrameSize, mPacket, MAX_PACKET_SIZE);
        if (mPacketSize < 0) {
          assert(false);
        }
      }
    }

    int popPacket(unsigned char* result) override {
      const int size = mPacketSize;
      if (size) {
        memcpy(result, "OPUS", 4);
        memcpy(result + 4, mPacket, size);
        mPacketSize = 0;
        return size + 4; // add the codec name
      }
      return 0;
    }

    /**
     * Change the encoder bit rate in bits up to 512000
     */
    void changeBitRate(int rate) const {
      opus_encoder_ctl(mEncoder, OPUS_SET_BITRATE(rate));
    }

    /**
     * Change the expected packet loss from 0-100 %
     */
    void changePacketLoss(int loss) const {
      opus_encoder_ctl(mEncoder, OPUS_SET_PACKET_LOSS_PERC(loss));
    }

    void changeFrameSize(int frameSize) {
      mFrameSize = frameSize;
    }
  };

  class WrappedOpusDecoder : public DecoderBase {
    OpusDecoder* mDecoder;
    float mInterleaved[2880 * 2]; // Interleaved buffer to encode data
    float mPostInterleave[2880]; // Buffer to use for interleaving
    /**
     * This is the local buffer. The opus decoder will not always
     * produce samples if the packets arrive in the wrong order.
     * So this will trade latency for smoother playback
     */
    int mBufferSize = 960;
  public:
    WrappedOpusDecoder() {
      int err;
      mDecoder = opus_decoder_create(48000, 2, &err);
      if (err < 0) {
        assert(false);
      }
    }

    ~WrappedOpusDecoder() {
      opus_decoder_destroy(mDecoder);
    }

    bool compareName(const void* name) const override {
      return strncmp("OPUS", static_cast<const char*>(name), 4) == 0;
    }

    void pushPacket(const unsigned char* data, int size) override {
      size -= 4; // first 4 bytes are codec identification
      if (size <= 0) { return; }
      memcpy(mPacket, data + 4, size);
      int frames = opus_decode_float(mDecoder, mPacket, size, mInterleaved, 2880 * 2, 0);
      if (frames > 0) {
        for (int c = 0; c < 2; c++) {
          for (int i = c, s = 0; s < frames; i += 2, s++) {
            mPostInterleave[s] = mInterleaved[i];
          }
          mBuffer[c].add(mPostInterleave, frames);
        }
      }
    }

    int popSamples(float** result, int size) override {
      if (mBuffer[0].inBuffer() >= size) {
        for (int c = 0; c < 2; c++) {
          mBuffer[c].get(result[c], size);
        }
        return size;
      }
      return 0;
    }
  };
}