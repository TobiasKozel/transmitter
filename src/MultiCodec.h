#pragma once

#include "./CodecOpus.h"
#include "./CodecRAW.h"
#include "../thirdparty/RingBuffer.h"

#ifdef __EMSCRIPTEN__
#include "../thirdparty/SpeexResampler.h"
#include "../thirdparty/HOheapbuf.h"
#endif

namespace transmitter {
  /**
   * All the supported codecs will be here and
   * it will automatically switch to the right decoder
   * based on the input
   */
  class MultiCodec {
    /**
     * Encoders usually need a certain number of frames to encode a packet
     * They'll need init the size accordingly
     */
    MultiRingBuffer<sample, 2> mBufferIn;
    /**
     * The decoded samples will be stored here and can be controlled by the user
     * This will affect latency
     */
    MultiRingBuffer<sample, 2> mBufferOut;
#ifdef __EMSCRIPTEN__
    SpeexResampler mRsOut;
    HeapBuffer<float> mResampleBuf;
    bool mResamplerSetup = false;
#endif

    WrappedOpusDecoder mOpusDecoder;
    WrappedOpusEncoder mOpusEncoder;

    RAWDecoder mRawDecoder;
    RAWEncoder mRawEncoder;

    DecoderBase* mActiveDecoder = nullptr;
    EncoderBase* mActiveEncoder = &mOpusEncoder;

    const int mChannels = 2; // No idea if this will ever change
    int mBufferSize = 2048; // Decoder buffer size

  public:
    MultiCodec() {
      mBufferOut.setSize(mBufferSize);
    }
    /**
     * Sets the buffer size for the decoding buffer
     * Lower buffer sizes may cause drop outs but will also lower the latency
     */
    void setBufferSize(const int size) {
      if (size != mBufferSize) {
        mBufferOut.setSize(size);
        mBufferSize = size;
      }
    }

    void setEncoder(const void* name) {
      if (mOpusEncoder.compareName(name)) {
        mActiveEncoder = &mOpusEncoder;
        return;
      }
      if (mRawEncoder.compareName(name)) {
        mActiveEncoder = &mRawEncoder;
      }
    }

    /**
     * The decoder doesn't need to be set from the outside since
     * it will automatically choose the right one for the packets
     * received
     */
    void setDecoder(const void* name) {
      if (mOpusDecoder.compareName(name)) {
        mActiveDecoder = &mOpusDecoder; // we only got opus for now
        return;
      }
      if (mRawDecoder.compareName(name)) {
        mActiveDecoder = &mRawDecoder; // we only got opus for now
      }
    }

    void setBitRate(int bitRate) {
      
    }

#ifdef __EMSCRIPTEN__
    /**
     * If compiling fo the web, the decoder will directly resample to the sample rate set here
     */
    void setSampleRate(int sampleRate) {
      if (sampleRate != 48000) {
        printf("Set the sample rate for the decoder to enable resampling");
        int err;
        mRsOut.init(2, 48000.0, sampleRate, 3, &err);
        mResamplerSetup = true;
        mResampleBuf.resize(1024);
      }
    }
#endif

    /**
     * Will encode the stereo frames provided and put them
     * in the buffer with the decoder information added
     */
    int encode(const float** input, int nFrames, unsigned char* output) {
      if (mActiveEncoder == nullptr) { return 0; }
      mBufferIn.add(input, nFrames);
      return mActiveEncoder->encode(&mBufferIn, output);
    }

    /**
     * Will decode the packet provided with the right decoder and store
     * the samples internally
     */
    void decode(const unsigned char* data, int size) {
      if (size > 0) {
        if (mActiveDecoder != nullptr && mActiveDecoder->compareName(data)) {
          // make sure we got a decoder and it's the right one
          mActiveDecoder->decode(data, size, [&](int frames, float* buf, int channel) {
#ifdef __EMSCRIPTEN__
            if (mResamplerSetup) {
              unsigned int inf = frames;
              unsigned int outf = 1024;
              mRsOut.process(channel, buf, &inf, mResampleBuf.get(), &outf);
              mBufferOut.add(mResampleBuf.get(), outf, channel);
            }
            else
#endif
            {
              mBufferOut.add(buf, frames, channel);
            }
          });
        } else {
          // If there's none or a different one, check against the available ones
          setDecoder(data);
        }
      }
    }

    /**
     * Will fill the buffer with the requested amount of stereo frames
     * if not enough frames are decoded, it will fill up the rest with zeroes
     */
    int popSamples(float** outputs, int requestedSamples) {
      int out = mBufferOut.get(outputs, requestedSamples);

      for (int i = out; i < requestedSamples; i++) {
        for (int c = 0; c < mChannels; c++) {
          outputs[c][i] = 0; // output silence if there's nothing decoded
        }
      }
      return out;
    }

  };
}