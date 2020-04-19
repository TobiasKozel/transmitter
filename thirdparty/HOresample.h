/*
    WDL - resample.h
    Copyright (C) 2010 and later Cockos Incorporated

    This software is provided 'as-is', without any express or implied
    warranty.  In no event will the authors be held liable for any damages
    arising from the use of this software.

    Permission is granted to anyone to use this software for any purpose,
    including commercial applications, and to alter it and redistribute it
    freely, subject to the following restrictions:

    1. The origin of this software must not be misrepresented; you must not
       claim that you wrote the original software. If you use this software
       in a product, an acknowledgment in the product documentation would be
       appreciated but is not required.
    2. Altered source versions must be plainly marked as such, and must not be
       misrepresented as being the original software.
    3. This notice may not be removed or altered from any source distribution.

    You may also distribute this software under the LGPL v2 or later.

*/

#pragma once

#include "./HOheapbuf.h"
#include <algorithm>

class WDL_Resampler {
public:
  typedef float WDL_ResampleSample;
  WDL_Resampler() {
    Reset();
  }

  ~WDL_Resampler() {
    delete m_iirfilter;
  }
  // if sinc set, it overrides interp or filtercnt
  void SetMode(int filtercnt) {
    m_filtercnt = std::max(0, std::min(filtercnt, 4));

    if (m_filtercnt == 0) {
      delete m_iirfilter;
      m_iirfilter = nullptr;
    }
  }

  // used for filtercnt>0 but not sinc
  void SetFilterParms(float filterpos = 0.693, float filterq = 0.707) {
    m_filterpos = filterpos;
    m_filterq = filterq;
  }

  // if true, that means the first parameter to ResamplePrepare will specify however much input you have, not how much you want
  void SetFeedMode(bool wantInputDriven) {
    m_feedmode = wantInputDriven;
  } 

  void Reset(double fracpos = 0.0) {
    m_last_requested = 0;
    m_filtlatency = 0;
    m_fracpos = fracpos;
    m_samples_in_rsinbuf = 0;
    if (m_iirfilter != nullptr) {
      m_iirfilter->Reset();
    }
  }

  void SetRates(double rate_in, double rate_out) {
    if (rate_in < 1.0) { rate_in = 1.0; }
    if (rate_out < 1.0) { rate_out = 1.0; }

    if (rate_in != m_sratein || rate_out != m_srateout) {   
      m_sratein = rate_in;
      m_srateout = rate_out;
      m_ratio = m_sratein / m_srateout;
    }
  }

  // amount of input that has been received but not yet converted to output, in seconds
  double GetCurrentLatency() const {
    double v = (double(m_samples_in_rsinbuf) - m_filtlatency) / m_sratein;

    if (v < 0.0) { v = 0.0; }
    return v;
  }

  // req_samples is output samples desired if !wantInputDriven, or if wantInputDriven is input samples that we have
  // returns number of samples desired (put these into *inbuffer)
  // note that it is safe to call ResamplePrepare without calling ResampleOut (the next call of ResamplePrepare will function as normal)
  int ResamplePrepare(int out_samples, WDL_ResampleSample** inbuffer) {
    int sreq = 0;

    if (!m_feedmode) {
      sreq = int(m_ratio * out_samples) + 4 - m_samples_in_rsinbuf;
    }
    else {
      sreq = out_samples;
    }

    if (sreq < 0) {
      sreq = 0;
    }

    m_rsinbuf.resize((m_samples_in_rsinbuf + sreq), false);

    *inbuffer = m_rsinbuf.get() + m_samples_in_rsinbuf;

    m_last_requested = sreq;
    return sreq;
  }


  // if numsamples_in < the value return by ResamplePrepare(), then it will be flushed to produce all remaining valid samples
  // do NOT call with nsamples_in greater than the value returned from resamplerprpare()! the extra samples will be ignored.
  // returns number of samples successfully outputted to out
  int ResampleOut(WDL_ResampleSample* out, int nsamples_in, int nsamples_out) {
    if (m_filtercnt > 0) {
      if (m_ratio > 1.0 && nsamples_in > 0) {
        // filter input
        if (m_iirfilter == nullptr) {
          m_iirfilter = new WDL_Resampler_IIRFilter(); // TODO get this out of the audio thread
          m_iirfilter->setParms((1.0 / m_ratio) * m_filterpos, m_filterq);
        }

        int n = m_filtercnt;

        WDL_ResampleSample* buf = m_rsinbuf.get() + m_samples_in_rsinbuf;
        int offs = 0;
        for (int a = 0; a < n; a++) {
          m_iirfilter->Apply(buf, buf, nsamples_in, offs++);
        }
      }
    }

    // prevent the caller from corrupting the internal state
    m_samples_in_rsinbuf += nsamples_in < m_last_requested ? nsamples_in : m_last_requested;

    int rsinbuf_availtemp = m_samples_in_rsinbuf;

    // flush out to ensure we can deliver
    if (nsamples_in < m_last_requested) {
      int fsize = (m_last_requested - nsamples_in) * 2;

      int alloc_size = (m_samples_in_rsinbuf + fsize);
      WDL_ResampleSample* zb = m_rsinbuf.resize(alloc_size, false);
      if (m_rsinbuf.size() == alloc_size) {
        memset(zb + m_samples_in_rsinbuf, 0, fsize * sizeof(WDL_ResampleSample));
        rsinbuf_availtemp = m_samples_in_rsinbuf + fsize;
      }
    }

    int ret = 0;
    double srcpos = m_fracpos;
    double drspos = m_ratio;
    WDL_ResampleSample* localin = m_rsinbuf.get();

    WDL_ResampleSample* outptr = out;

    int ns = nsamples_out;

    int outlatadj = 0;

    // linear interpolation
    while (ns--) {
      int ipos = (int)srcpos;
      double fracpos = srcpos - ipos;

      if (ipos >= rsinbuf_availtemp - 1)
      {
        break; // quit decoding, not enough input samples
      }

      double ifracpos = 1.0 - fracpos;
      WDL_ResampleSample* inptr = localin + ipos;
      *outptr++ = inptr[0] * (ifracpos)+inptr[1] * (fracpos);
      srcpos += drspos;
      ret++;
    }


    if (m_filtercnt > 0) {
      // filter output
      if (m_ratio < 1.0 && ret>0) {
        if (m_iirfilter == nullptr) {
          m_iirfilter = new WDL_Resampler_IIRFilter;
        }

        int n = m_filtercnt;
        m_iirfilter->setParms(m_ratio * m_filterpos, m_filterq);

        int offs = 0;
        for (int a = 0; a < n; a++) {
          m_iirfilter->Apply(out, out, ret, offs++);
        }
      }
    }


    // we had to pad!!
    if (ret > 0 && rsinbuf_availtemp > m_samples_in_rsinbuf) {
      // check for the case where rsinbuf_availtemp>m_samples_in_rsinbuf, decrease ret down to actual valid samples
      double adj = (srcpos - m_samples_in_rsinbuf + outlatadj) / drspos;
      if (adj > 0) {
        ret -= (int)(adj + 0.5);
        if (ret < 0)ret = 0;
      }
    }

    int isrcpos = (int)srcpos;

    if (isrcpos > m_samples_in_rsinbuf) {
      isrcpos = m_samples_in_rsinbuf;
    }

    m_fracpos = srcpos - isrcpos;

    m_samples_in_rsinbuf -= isrcpos;

    if (m_samples_in_rsinbuf <= 0) {
      m_samples_in_rsinbuf = 0;
    }
    else {
      memmove(localin, localin + isrcpos, m_samples_in_rsinbuf * sizeof(WDL_ResampleSample));
    }
    return ret;
  }




private:
  class WDL_Resampler_IIRFilter {
  public:
    WDL_Resampler_IIRFilter() {
      Reset();
    }

    void Reset() {
      memset(m_hist, 0, sizeof(m_hist));
    }

    void setParms(double fpos, double Q) {
      if (fabs(fpos - m_fpos) < 0.000001) return;
      m_fpos = fpos;

      double pos = fpos * 3.1415926535897932384626433832795;
      double cpos = cos(pos);
      double spos = sin(pos);

      double alpha = spos / (2.0 * Q);

      double sc = 1.0 / (1 + alpha);
      m_b1 = (1 - cpos) * sc;
      m_b2 = m_b0 = m_b1 * 0.5;
      m_a1 = -2 * cpos * sc;
      m_a2 = (1 - alpha) * sc;
    }

    void Apply(WDL_ResampleSample* in1, WDL_ResampleSample* out1, int ns, int w) {
      const double b0 = m_b0, b1 = m_b1, b2 = m_b2, a1 = m_a1, a2 = m_a2;
      double* hist = m_hist[w];
      while (ns--) {
        const double in = *in1;
        in1 += 1;
        const double out = in * b0 + hist[0] * b1 + hist[1] * b2 - hist[2] * a1 - hist[3] * a2;
        hist[1] = hist[0]; hist[0] = in;
        hist[3] = hist[2]; *out1 = hist[2] = out;
        out1 += 1;
      }
    }

  private:
    double m_fpos = -1;
    double m_a1, m_a2;
    double m_b0, m_b1, m_b2;
    double m_hist[4][4];
  };


  double m_sratein = 44100.0;
  double m_srateout = 44100.0;
  double m_fracpos;
  double m_ratio = 1.0;
  double m_filter_ratio = -1.0;
  float m_filterq = 0.707f;
  float m_filterpos = 0.693f; // .792 ?
  transmitter::HeapBuffer<WDL_ResampleSample> m_rsinbuf;

  WDL_Resampler_IIRFilter* m_iirfilter = nullptr;

  int m_filter_coeffs_size = 0;
  int m_last_requested;
  int m_filtlatency;
  int m_samples_in_rsinbuf;
  int m_lp_oversize = 1;

  int m_filtercnt = 1;
  bool m_feedmode = false;

};
