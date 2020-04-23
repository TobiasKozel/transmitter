#pragma once
#include "../thirdparty/opus/include/opus.h"
#include "./CodecBase.h"
#include <cstring>
#include <cassert>

namespace transmitter {

  class WrappedOpusEncoder : public EncoderBase {
    OpusEncoder* mEncoder = nullptr; // The encoder itself
    float mInterleaved[2880 * 2] = { 0 }; // Max opus frame size at 48kHz
    float mPreInterleave[2880] = { 0 };
    int mFrameSize = 240; // The size of the blocks handed over to the encoder
  public:
    WrappedOpusEncoder() {
      strcpy(mName, "OPUS");
      int err;
      /**
       * We'll always use 2 channels at 48kHz
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

    int getMaxBlockSize() const override {
      return 960;
    }

  private:
    int encodeImpl(MultiRingBuffer<sample, 2>* mBuffer, unsigned char* result) override {
      if (mFrameSize <= mBuffer->inBuffer()) {
        for (int c = 0; c < 2; c++) {
          mBuffer->get(mPreInterleave, mFrameSize, c);
          for (int i = c, s = 0; s < mFrameSize; i += 2, s++) {
            mInterleaved[i] = mPreInterleave[s]; // interleave the signal
          }
        }
        int size = 0;
        if (mEncoder != nullptr) {
          size = opus_encode_float(mEncoder, mInterleaved, mFrameSize, result, MAX_PACKET_SIZE);
          if (size < 0) {
            assert(false);
          }
        }

        return size;
      }
      return 0;
    }
  };

  class WrappedOpusDecoder : public DecoderBase {
    OpusDecoder* mDecoder = nullptr;
    float mInterleaved[2880 * 2] = { 0 }; // Interleaved buffer to encode data
    float mPostInterleave[2880] = { 0 }; // Buffer to use for interleaving
    /**
     * This is the local buffer. The opus decoder will not always
     * produce samples if the packets arrive in the wrong order.
     * So this will trade latency for smoother playback
     */
    int mBufferSize = 2048;
  public:
    WrappedOpusDecoder() {
      strcpy(mName, "OPUS");
      int err;
      mDecoder = opus_decoder_create(48000, 2, &err);
      if (err < 0) {
        assert(false);
      }
    }

    ~WrappedOpusDecoder() {
      opus_decoder_destroy(mDecoder);
    }

  private:
    int decodeImpl(const unsigned char* data, const int size, DecodeCallback& callback) override {
      const int frames = opus_decode_float(mDecoder, data, size, mInterleaved, 2880 * 2, 0);
      if (frames > 0) {
        for (int c = 0; c < 2; c++) {
          for (int i = c, s = 0; s < frames; i += 2, s++) {
            mPostInterleave[s] = mInterleaved[i];
          }
          callback(frames, mPostInterleave, c);
        }
      }
      return 0;
    }
  };
}