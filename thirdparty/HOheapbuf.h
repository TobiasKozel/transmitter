#pragma once

#include <cmath>
#include <cstring>

namespace transmitter {
  template <typename T>
  class HeapBuffer {
    T* mBuf = nullptr;
    int mSize = 0; // size of elements requested
    int mRealSize = 0; // the actually allocated size
    int mGranularity = 1024 / sizeof(T); // the space actually allocated will be a multiple of this


  public:
    HeapBuffer(int granularity = 0) {
      setGranularity(granularity);
    }

    ~HeapBuffer() {
      delete mBuf;
    }

    void setGranularity(int granularity) {
      if (granularity > 0) {
        mGranularity = granularity;
      }
    }

    T* get() {
      return mBuf;
    }

    T* resize(const int size, const bool downsize = true) {
      const int chunked = mGranularity * std::ceil(size / static_cast<float>(mGranularity));
      if (chunked != mRealSize) {
        if (size == 0) { // delete
          delete mBuf;
          mBuf = nullptr;
        } else {
          if (chunked > mRealSize) { // Size up
            T* temp = new T[chunked];
            memcpy(temp, mBuf, mSize * sizeof(T));
            memset(temp + mSize, 0, (chunked - mSize) * sizeof(T));
            delete[] mBuf;
            mBuf = temp;
          } else if(downsize) { // size down
            T* temp = new T[chunked];
            memcpy(temp, mBuf, chunked * sizeof(T));
            delete[] mBuf;
            mBuf = temp;
          }
        }
        mRealSize = chunked;
      }
      mSize = size;
      return mBuf;
    }

    int size() const {
      return mSize;
    }
  };

  template <typename T, int channels>
  class MultiHeapBuffer {
    HeapBuffer<T> mBuffers[channels];
    T* mBufferRef[channels] = { nullptr };
  public:
    MultiHeapBuffer(int granularity = 0) {
      setGranularity(granularity);
    }

    ~MultiHeapBuffer() {
      for (int c = 0; c < channels; c++) {
        mBufferRef[c] = nullptr;
      }
    }

    void setGranularity(int granularity = 0) {
      for (int c = 0; c < channels; c++) {
        mBuffers[c].setGranularity(granularity);
      }
    }

    T** get(size_t offset = 0) {
      for (int c = 0; c < channels; c++) {
        mBufferRef[c] = mBuffers[c].get() + offset;
      }
      return mBufferRef;
    }

    T* getChannel(int c) {
      return mBuffers[c].get();
    }

    T** resize(const int size, const bool downsize = true) {
      for (int c = 0; c < channels; c++) {
        mBuffers[c].resize(size, downsize);
        mBufferRef[c] = mBuffers[c].get();
      }
      return mBufferRef;
    }
  };
}