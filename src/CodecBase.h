#pragma once


#include "./Types.h"
#include "../thirdparty/RingBuffer.h"

namespace transmitter {
  const int MAX_CHANNELS = 2;

  class EncoderBase {

  protected:
    // unsigned char mPacket[MAX_PACKET_SIZE]; // The encoded packet
    char mName[5] = "NAME";

  private:
    TRANSMITTER_NO_COPY(EncoderBase)
    /**
     * Actual implementation of the encoder
     */
    virtual int encodeImpl(MultiRingBuffer<sample, 2>* mBuffer, unsigned char* result) = 0;

  public:
    EncoderBase() = default;

    /**
     * Will push samples to the buffer and encode them
     * if the frame size is reached
     */
    int encode(MultiRingBuffer<sample, 2>* mBuffer, unsigned char* result) {
      const int size = encodeImpl(mBuffer, result + 4); // Keep space for the name
      if (size == 0) { return 0; }
      memcpy(result, mName, 4); // Add the codec name
      return size + 4; // add the codec name
    };

    virtual ~EncoderBase() = default;

    bool compareName(const void* name) const {
      return strncmp(mName, static_cast<const char*>(name), 4) == 0;
    }

    /**
     * Tells the multi codec how large the input buffer needs to be
     */
    const char* getName() const {
      return mName;
    }

    virtual int getMaxBlockSize() const = 0;
  };

  class DecoderBase {
  public:
    typedef std::function<void(int count, float* buf, int channelindex)> DecodeCallback;
  protected:
    char mName[5] = "NAME";
    virtual int decodeImpl(const unsigned char* data, int size, DecodeCallback& c) = 0;

  private:
    TRANSMITTER_NO_COPY(DecoderBase)

  public:
    
    DecoderBase() = default;
    /**
     * Will decode the packet provided and add into the buffer
     */
    int decode(const unsigned char* data, int size, DecodeCallback c) {
      return decodeImpl(data + 4, size - 4, c); // crop away the codec name
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
