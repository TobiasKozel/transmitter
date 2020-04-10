#pragma once

#include "thirdparty/RingBuffer.h"


namespace transmitter {
  const int MAX_PACKET_SIZE = 1464; // We'll just go with the max udp packet size without fragmentation
  const int MAX_CHANNELS = 2;
  class EncoderBase {
  protected:
    RingBuffer<float> mBuffer[MAX_CHANNELS]; // The audio buffer
    // unsigned char mPacket[MAX_PACKET_SIZE]; // The encoded packet
    char mName[5] = "NAME";

  private:
    /**
     * Actual implementation of the encoder
     */
    virtual int pushSamplesImpl(float** samples, int count, unsigned char* result) = 0;
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
    int pushSamples(float** samples, int count, unsigned char* result) {
      const int size = pushSamplesImpl(samples, count, result + 4);
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
    virtual int pushPacketImpl(const unsigned char* data, int size, float** result, int requestedSamples) = 0;
    
  public:
    /**
     * Will decode the packet provided and add into the buffer
     */
    int pushPacket(const unsigned char* data, int size, float** result, int requestedSamples) {
      return pushPacketImpl(data + 4, size - 4, result, requestedSamples);
    }

    /**
     * Larger buffers will result in higher latencies but smoother playback usually
     */
    void resizeBuffer(int size) {
      mBuffer->setSize(size);
    }

    bool compareName(const void* name) const {
      return strncmp(mName, static_cast<const char*>(name), 4) == 0;
    }

    const char* getName() const {
      return mName;
    }


    DecoderBase() {
      for (int i = 0; i < MAX_CHANNELS; i++) {
        mBuffer[i].setSize(2048);
      }
    }

    virtual ~DecoderBase() = default;
  };
}
