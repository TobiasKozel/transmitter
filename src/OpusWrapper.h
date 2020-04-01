#pragma once
#include "../thirdparty/opus/include/opus.h"
#include "../thirdparty/RingBuffer.h"
#include <cstring>
#include <cassert>

#define OPUS_CHANNELS 2
#define MAX_PACKET_SIZE 4000
#define MAX_FRAMESIZE 2880

namespace transmitter {
  class WrappedOpusEncoder {
    OpusEncoder* mEncoder = nullptr; // The encoder itself
    float mInterleaved[MAX_FRAMESIZE * OPUS_CHANNELS];
    float mPreInterleave[MAX_FRAMESIZE];
    RingBuffer<float> mBuffer[OPUS_CHANNELS]; // The audio buffer
    unsigned char mPacket[MAX_PACKET_SIZE]; // The encoded packet
    int mPacketSize = 0; // The actual size of the packet
    int mFrameSize = 480; // The size of the blocks handed over to the encoder
  public:

    WrappedOpusEncoder() {
      mBuffer[0].setSize(MAX_FRAMESIZE);
      mBuffer[1].setSize(MAX_FRAMESIZE);
      int err;
      mEncoder = opus_encoder_create(48000, OPUS_CHANNELS, OPUS_APPLICATION_RESTRICTED_LOWDELAY, &err);
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
     * Will push samples to the buffer and encode them
     * if the frame size is reached
     */
    void pushSamples(float** samples, int count) {
      mBuffer[0].add(samples[0], count);
      mBuffer[1].add(samples[1], count);
      if (mFrameSize >= mBuffer[0].inBuffer()) {
        for (int c = 0; c < OPUS_CHANNELS; c++) {
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

    int popPacket(unsigned char* result) {
      const int size = mPacketSize;
      if (size) {
        memcpy(result, mPacket, size);
      }
      mPacketSize = 0;
      return size;
    }

    /**
     * Change the encoder bit rate in bits up to 512000
     */
    void changeBitRate(int rate) {
      opus_encoder_ctl(mEncoder, OPUS_SET_BITRATE(rate));
    }

    /**
     * Change the expected packet loss from 0-100 %
     */
    void changePacketLoss(int loss) {
      opus_encoder_ctl(mEncoder, OPUS_SET_PACKET_LOSS_PERC(loss));
    }
  };

  class WrappedOpusDecoder {
    OpusDecoder* mDecoder;
    float mInterleaved[MAX_FRAMESIZE * OPUS_CHANNELS]; // Interleaved buffer to encode data
    float mPostInterleave[MAX_FRAMESIZE]; // Buffer to use for interleaving
    RingBuffer<float> mBuffer[OPUS_CHANNELS];
    /**
     * This is the local buffer. The opus decoder will not always
     * produce samples if the packets arrive in the wrong order.
     * So this will trade latency for smoother playback
     */
    int mBufferSize = 960;
  public:
    WrappedOpusDecoder() {
      int err;
      mDecoder = opus_decoder_create(48000, OPUS_CHANNELS, &err);
      if (err < 0) {
        assert(false);
      }
      mBuffer[0].setSize(mBufferSize);
      mBuffer[1].setSize(mBufferSize);
    }

    ~WrappedOpusDecoder() {
      opus_decoder_destroy(mDecoder);
    }

    void pushPacket(const unsigned char* data, int size) {
      int frames = opus_decode_float(mDecoder, data, size, mInterleaved, MAX_FRAMESIZE, 0);
      if (frames > 0) {
        for (int c = 0; c < OPUS_CHANNELS; c++) {
          for (int i = c, s = 0; s < frames; i += 2, s++) {
            mPostInterleave[s] = mInterleaved[i];
          }
          mBuffer[c].add(mPostInterleave, frames);
        }
      }
    }

    /**
     * This function will provide the requested samples, or silence if 
     * not enough samples are available
     */
    void popSamplesOrSilence(float** result, int size) {
      if (mBuffer[0].inBuffer() >= size) {
        for (int c = 0; c < OPUS_CHANNELS; c++) {
          mBuffer[c].get(result[c], size);
        }
      }
      else {
        for (int c = 0; c < OPUS_CHANNELS; c++) {
          for (int i = 0; i < size; i++) {
            result[c][i] = 0; // output silence if there's nothing decoded
          }
        }
      }
    }


  };
}