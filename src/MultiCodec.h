#pragma once

#include "./CodecOpus.h"
#include "./CodecRAW.h"

namespace transmitter {
  /**
   * All the supported codecs will be here and
   * it will automatically switch to the right decoder
   * based on the input
   */
  class MultiCodec {
    WrappedOpusDecoder mOpusDecoder;
    WrappedOpusEncoder mOpusEncoder;

    RAWDecoder mRawDecoder;
    RAWEncoder mRawEncoder;

    DecoderBase* mActiveDecoder = nullptr;
    EncoderBase* mActiveEncoder = nullptr;
    const int mChannels = 2; // No idea if this will ever change
    int mBufferSize = 2048;

  public:

    void setBufferSize(const int size) {
      mBufferSize = size;
      if (mActiveDecoder != nullptr) {
        mActiveDecoder->resizeBuffer(size);
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

    int encode(float** input, int nFrames, unsigned char* output) const {
      if (mActiveEncoder == nullptr) { return 0; }
      return mActiveEncoder->encode(input, nFrames, output);
    }

    void decode(const unsigned char* data, int size) {
      if (size > 0) {
        if (mActiveDecoder != nullptr && mActiveDecoder->compareName(data)) {
          // make sure we got a decoder and it's the right one
          mActiveDecoder->decode(data, size);
        } else {
          // If there's none or a different one, check against the available ones
          setDecoder(data);
        }
      }
    }

    int popSamples(float** outputs, int requestedSamples) const {
      if (mActiveDecoder == nullptr) { return 0; }
      const int out = mActiveDecoder->popSamples(outputs, requestedSamples);
      for (int i = out; i < requestedSamples; i++) {
        for (int c = 0; c < mChannels; c++) {
          outputs[c][i] = 0; // output silence if there's nothing decoded
        }
      }
      return out;
    }

  };
}