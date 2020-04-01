#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPrograms = 1;

enum EParams
{
  kGain = 0,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class Transmitter final : public Plugin
{
public:
  Transmitter(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
