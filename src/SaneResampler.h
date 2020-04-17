#pragma once
#include "./TConfig.h"
#include "../thirdparty/HOresample.h"

namespace transmitter {
  class SaneResampler {
    const int mPadding = 10;
    WDL_Resampler mRs;
    WDL_Resampler mLs;
    void _setUp(WDL_Resampler& r, double srIn, double srOut, bool inputDriven) {
      r.SetMode(true, 2, false);
      r.SetFilterParms(1, 1);
      r.SetFeedMode(inputDriven);
      r.SetRates(srIn, srOut); // We'll always output 48kHz for opus
    }
    sample mBufL[MAX_BUFFER_SIZE * 4]; // samplerates up to 4 times the base should fit in here
    sample mBufR[MAX_BUFFER_SIZE * 4];
  public:
    sample* buffer[2] = { mBufL, mBufR }; // Some scratch buffer which can be used for convenience

    void setUp(double srIn, double srOut, bool inputDriven = true) {
      _setUp(mRs, srIn, srOut, inputDriven);
      _setUp(mLs, srIn, srOut, inputDriven);
    }

    int ProcessBlock(sample** in, sample** out, int nFrames) {
      sample* buf[2];
      const int n = _resamplePrepare(buf, nFrames);
      return _resample(in, out, buf, n);
    }

    int _resamplePrepare(sample** buf, int nFrames) {
      int nl = mRs.ResamplePrepare(nFrames, 1, &buf[0]);
      int nr = mLs.ResamplePrepare(nFrames, 1, &buf[1]);
      return nl;
    }

    int _resample(sample** in, sample** out, sample** buf, int n) {
      memcpy(buf[0], in[0], n * sizeof(float));
      memcpy(buf[1], in[1], n * sizeof(float));
      int out1 = mLs.ResampleOut(out[0], n, 512 * 4, 1);
      int out2 = mRs.ResampleOut(out[1], n, 512 * 4, 1);
      return out1;
    }
  };
}