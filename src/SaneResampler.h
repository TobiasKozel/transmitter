#pragma once
#include "./TConfig.h"
#include "../thirdparty/HOresample.h"

namespace transmitter {
  class SaneResampler {
    const int mPadding = 10;
    WDL_Resampler mRs;
    WDL_Resampler mLs;
    bool mFeedmode = true;
    void _setUp(WDL_Resampler& r, double srIn, double srOut, bool inputDriven) {
      r.SetMode(4);
      // r.SetFilterParms(1, 1);
      r.SetFilterParms();
      r.SetFeedMode(inputDriven);
      r.SetRates(srIn, srOut);
    }
    sample mBufL[MAX_BUFFER_SIZE * 4]; // samplerates up to 4 times the base should fit in here
    sample mBufR[MAX_BUFFER_SIZE * 4];
  public:
    sample* buffer[2] = { mBufL, mBufR }; // Some scratch buffer which can be used for convenience

    void setUp(double srIn, double srOut, bool inputDriven = true) {
      mFeedmode = inputDriven;
      _setUp(mRs, srIn, srOut, inputDriven);
      _setUp(mLs, srIn, srOut, inputDriven);
    }

    int ProcessBlock(sample** in, sample** out, int nFrames) {
      sample* buf[2];
      const int n = _resamplePrepare(buf, nFrames);
      memcpy(buf[0], in[0], n * sizeof(float));
      memcpy(buf[1], in[1], n * sizeof(float));
      return _resample(out, buf, n, nFrames);
    }

    int _resamplePrepare(sample** buf, int nFrames) {
      int nl = mRs.ResamplePrepare(nFrames, &buf[0]);
      int nr = mLs.ResamplePrepare(nFrames, &buf[1]);
      return nl;
    }

    int _resample(sample** out, sample** buf, int n, int nFrames) {
      if (mFeedmode) {
        int out1 = mLs.ResampleOut(out[0], n, 1024);
        int out2 = mRs.ResampleOut(out[1], n, 1024);
        return out1;
      }
      int out1 = mLs.ResampleOut(out[0], nFrames, n);
      int out2 = mRs.ResampleOut(out[1], nFrames, n);
    }
  };
}