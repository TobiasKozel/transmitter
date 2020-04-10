#pragma once

#include "thirdparty/RingBuffer.h"


namespace transmitter {
  const int MAX_PACKET_SIZE = 1464; // We'll just go with the max udp packet size without fragmentation
  const int MAX_CHANNELS = 2;
  class EncoderBase {
  protected:
    RingBuffer<float> mBuffer[MAX_CHANNELS]; // The audio buffer
    unsigned char mPacket[MAX_PACKET_SIZE]; // The encoded packet
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
    virtual void pushSamples(float** samples, int count) = 0;

    /**
     * Should be called after each time pushSamples is called
     * Will provide a packet if the encoder has enough samples
     */
    virtual int popPacket(unsigned char* result) = 0;
    virtual ~EncoderBase() = default;
  };

  class DecoderBase {
  protected:
    RingBuffer<float> mBuffer[MAX_CHANNELS]; // The audio buffer
    unsigned char mPacket[MAX_PACKET_SIZE]; // The encoded packet
  public:
    /**
     * Will decode the packet provided and add into the buffer
     */
    virtual void pushPacket(const unsigned char* data, int size) = 0;

    /**
     * This function will provide the requested samples
     */
    virtual int popSamples(float** result, int size) = 0;

    virtual bool compareName(const void* name) const = 0;

    /**
     * Larger buffers will result in higher latencies but smoother playback usually
     */
    void resizeBuffer(int size) {
      mBuffer->setSize(size);
    }

    DecoderBase() {
      for (int i = 0; i < MAX_CHANNELS; i++) {
        mBuffer[i].setSize(2048);
      }
    }

    virtual ~DecoderBase() = default;
  };
}
