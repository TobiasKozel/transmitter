#pragma once
/**
 * Based on the "circbuf.h" implementation from Cockos WDL but without their heap buffer
 */

#include <cstring>

namespace transmitter {
  template <typename T>
  class RingBuffer {
    T* mBuffer = nullptr;
    int mSize = 0;
    int mHead = 0;
    int mTail = 0;
  public:
    RingBuffer() = default;

    ~RingBuffer() {
      setSize(0);
    }

    /**
     * Won't allow copying for now
     */
    RingBuffer(const RingBuffer&) = delete;
    RingBuffer(const RingBuffer*) = delete;
    RingBuffer(RingBuffer&&) = delete;
    RingBuffer& operator= (const RingBuffer&) = delete;
    RingBuffer& operator= (RingBuffer&&) = delete;

    /**
     * Will resize the buffer an clear it
     */
    void setSize(const int size) {
      if (mSize == size) { return; }
      if (mBuffer != nullptr) {
        delete[] mBuffer;
        mBuffer = nullptr;
      }
      mTail = 0;
      mHead = 0;
      mSize = size;
      if (0 < size) {
        mBuffer = new T[size];
      }
    }

    /**
     * Puts a number of elements in the array provided
     */
    int peek(T* out, int elements, int offset = 0) {
      if (offset < 0) { return 0; } // only use positive offsets
      const int head = mHead - offset; // Offset the head
      if (elements > head) {
        elements = head; // Clamp the elements to peek to the elements in the buffer
      }
      if (elements > 0) {
        int tailStart = mTail - head; // This should always be negative when the offset is 0
        if (tailStart < 0) {
          tailStart += mSize; // So move it back
        }
        const int spaceLeft = mSize - tailStart;
        if (spaceLeft < elements) {
          // Means it wraps around and split in two moves
          memmove(out, mBuffer + tailStart, spaceLeft * sizeof(T));
          memmove(out + spaceLeft, mBuffer, (elements - spaceLeft) * sizeof(T));
        }
        else {
          // Enough space left and can be done in one step
          memmove(out, mBuffer + tailStart, elements * sizeof(T));
        }
      }
      return elements;
    }

    /**
     * Pops a number of elements and puts them in the buffer provided
     */
    int get(T* out, const int elements) {
      const int elementsOut = peek(out, elements);
      mHead -= elementsOut; // Move the head back, can't exceed bounds since it was clamped in peek
      return elementsOut;
    }

    /**
     * Adds a number of elements
     */
    int add(const T* in, int elements) {
      const int spaceLeftHead = mSize - mHead; // Space left before exceeding upper buffer bounds
      if (elements > spaceLeftHead) {
        /**
         * Clamp the elements added to the buffer to it's bounds
         * This ring buffer will stop adding elements if that's the case instead of wrapping around
         */
        elements = spaceLeftHead; 
      }
      if (elements > 0) {
        const int spaceLeftTail = mSize - mTail;
        if (spaceLeftTail < elements) {
          // We'll need to wrap around since there's not enough space left
          memmove(mBuffer + mTail, in, spaceLeftTail * sizeof(T));
          memmove(mBuffer, in + spaceLeftTail, (elements - spaceLeftTail) * sizeof(T));
          mTail = elements - spaceLeftTail;
        }
        else { // If there's enough space left we can just copy the buffer starting at the tail index
          memmove(mBuffer + mTail, in, elements * sizeof(T));
          // If we end up taking all space left for the tail, move it back to the start, otherwise move it forward
          mTail = (spaceLeftTail == elements) ? 0 : mTail + elements;
        }
        mHead += elements; // Move the head forward, can't exceed the bounds since we clamped it
      }
      return elements;
    }

    /**
     * Returns how many more elements the buffer can hold
     */
    int nFree() const {
      return mSize - mHead;
    }

    /**
     * Returns how many elements are in the buffer
     */
    int inBuffer() const {
      return mHead;
    }
  };

  template <typename T, int channels>
  class MultiRingBuffer {
    RingBuffer<T> mBuffers[channels];
  public:
  	
    void setSize(const int size) {
      for (int c = 0; c < channels; c++) {
        mBuffers[c].setSize(size);
      }
    }

    int peek(T** out, int elements, int offset = 0) {
      int ret;
      for (int c = 0; c < channels; c++) {
        ret = mBuffers[c].peek(out[c], elements, offset);
      }
      return ret;
    }

    int get(T* out, const int elements, const int channel) {
      return mBuffers[channel].get(out, elements);
    }

    int get(T** out, const int elements) {
      int ret;
      for (int c = 0; c < channels; c++) {
        ret = mBuffers[c].get(out[c], elements);
      }
      return ret;
    }

    int add(const T* in, const int elements, const int channel) {
      return mBuffers[channel].add(in, elements);
    }
  	
    int add(const T** in, int elements) {
      int ret;
    	for (int c = 0; c < channels; c++) {
			ret = mBuffers[c].add(in[c], elements);
    	}
      return ret;
    }

    int nFree() const {
      return mBuffers[0].nFree();
    }

    int inBuffer() const {
      return mBuffers[0].inBuffer();
    }
  };
}