#pragma once

#include "../thirdparty/RingBuffer.h"
#include "./Types.h"

namespace transmitter {
  const int MAX_CHANNELS = 2;

  class EncoderBase {

  protected:
    RingBuffer<float> mBuffer[MAX_CHANNELS]; // The audio buffer
    // unsigned char mPacket[MAX_PACKET_SIZE]; // The encoded packet
    char mName[5] = "NAME";

  private:
    TRANSMITTER_NO_COPY(EncoderBase)
    /**
     * Actual implementation of the encoder
     */
    virtual int encodeImpl(float** samples, int count, unsigned char* result) = 0;

  public:
    EncoderBase() {
      for (int i = 0; i < MAX_CHANNELS; i++) {
        mBuffer[i].setSize(MAX_PACKET_SIZE); // if byte in the udp packet could fit a sample
      }
    }

    /**
     * Will push samples to the buffer and encode them
     * if the frame size is reached
     */
    int encode(float** samples, int count, unsigned char* result) {
      const int size = encodeImpl(samples, count, result + 4); // Keep space for the name
      if (size == 0) { return 0; }
      memcpy(result, mName, 4); // Add the codec name
      return size + 4; // add the codec name
    };

    virtual ~EncoderBase() = default;

    bool compareName(const void* name) const {
      return strncmp(mName, static_cast<const char*>(name), 4) == 0;
    }

    const char* getName() const {
      return mName;
    }
  };

  class DecoderBase {

  protected:
    RingBuffer<float> mBuffer[MAX_CHANNELS]; // The audio buffer
    char mName[5] = "NAME";
    virtual void decodeImpl(const unsigned char* data, int size) = 0;

  private:
    TRANSMITTER_NO_COPY(DecoderBase)

  public:
    DecoderBase() = default;
    /**
     * Will decode the packet provided and add into the buffer
     */
    void decode(const unsigned char* data, int size) {
      decodeImpl(data + 4, size - 4);
    }

    int popSamples(float** outputs, int requestedSamples) {
      int inbuf = mBuffer[0].inBuffer();
      if (mBuffer[0].inBuffer() >= requestedSamples) {
        for (int c = 0; c < 2; c++) {
          mBuffer[c].get(outputs[c], requestedSamples);
        }
        return requestedSamples;
      }
      return 0;
    }

    /**
     * Larger buffers will result in higher latencies but smoother playback usually
     */
    void resizeBuffer(int size) {
      for (int i = 0; i < MAX_CHANNELS; i++) {
        mBuffer[i].setSize(size);
      }
    }

    bool compareName(const void* name) const {
      return strncmp(mName, static_cast<const char*>(name), 4) == 0;
    }

    const char* getName() const {
      return mName;
    }

    virtual ~DecoderBase() = default;
  };
}
