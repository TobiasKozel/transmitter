#pragma once
#define CPPHTTPLIB_OPENSSL_SUPPORT
#include "../thirdparty/httplib.h" // Needs to be included here because of windows socket define bullshit
#include "IPlug_include_in_plug_hdr.h"
#include "./src/TransmitterSession.h"

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
  transmitter::SessionManager mSessionManager;
  /**
   * Called from outside when a state needs to be saved
   */
  bool SerializeState(iplug::IByteChunk& chunk) const override;

  /**
   * Called from outside with a byte chunk to load
   */
  int UnserializeState(const iplug::IByteChunk& chunk, int startPos) override;

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
