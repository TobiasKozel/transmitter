#pragma once
#define WDL_RESAMPLE_TYPE float
/**
 * Comment this out to compile with out ssl
 * Connections to a master server with ssl will fail obviously
 */
#define CPPHTTPLIB_OPENSSL_SUPPORT

#include "../thirdparty/httplib.h" // Needs to be included here because of windows socket define bullshit



#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "./src/TextControl.h"
#include "./src/TConfig.h"
#include "./src/MasterServerSession.h"
#include "./src/Types.h"
#include "thirdparty/SpeexResampler.h"


const int kNumPrograms = 1;

enum EParams {
  kFrameSize = 0,
  kBitRate,
  kPacketLoss,
  kComplexity,
  kSilenceThreshold,
  kNoPhaseInversion,
  kVolume,
  kVolumeRemote,
  kBufferSize,
  kNumParams
};

class Transmitter final : public iplug::Plugin {
  transmitter::SpeexResampler mUp;
  transmitter::SpeexResampler mDown;
  bool mResamplingSetup = false;
  transmitter::MasterServerSession* mMSession = nullptr;
  WDL_PtrList<iplug::igraphics::IControl> mMainTab, mDirectTab;
  iplug::igraphics::IVButtonControl* mMainTabButton = nullptr, * mDirectTabButton = nullptr;
  iplug::igraphics::IGraphics* mGraphics = nullptr;
  transmitter::TextControl* mMasterServer = nullptr;
  transmitter::TextControl* mMasterPeer = nullptr;
  transmitter::IVButtonControl2* mMasterId = nullptr;
  transmitter::ITextControl* mOwnIp = nullptr;
  transmitter::TextControl* mDirectPeer = nullptr;
  int mMaxBlockSize = 512;
  int mChannelCount = 2;

  transmitter::sample* mSliceBufferSubIn[2];
  transmitter::sample* mSliceBufferSubOut[2];
  transmitter::sample** mSliceBuffer[2] = { mSliceBufferSubIn, mSliceBufferSubOut };
  transmitter::sample** mResamplingBuffer = nullptr;

  void switchTab(bool directTab);
  void connect(bool keepId);
  void setupResampling();
public:
  Transmitter(const iplug::InstanceInfo& info);
  ~Transmitter();

  void OnUIClose() override;

  /**
   * Called from outside when a state needs to be saved
   */
  bool SerializeState(iplug::IByteChunk& chunk) const override;

  /**
   * Called from outside with a byte chunk to load
   */
  int UnserializeState(const iplug::IByteChunk& chunk, int startPos) override;

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(iplug::sample** inputs, iplug::sample** outputs, int nFrames) override;
#endif
};
