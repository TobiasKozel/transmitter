#include "Transmitter.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "./thirdparty/json.hpp"
#include "./src/GUIStyle.h"

using namespace transmitter;

Transmitter::Transmitter(const iplug::InstanceInfo& info) : iplug::Plugin(info, iplug::MakeConfig(kNumParams, kNumPrograms)) {
  GetParam(kVolume)->InitGain("Volume own");
  GetParam(kVolumeRemote)->InitGain("Volume remote");
  GetParam(kBitRate)->InitDouble("Bitrate", 128, 1, 512, 0.1, "kBit/s");
  GetParam(kComplexity)->InitDouble("Complexity", 10, 0, 10, 1);
  GetParam(kPacketLoss)->InitPercentage("Expected packet loss");
  GetParam(kBufferSize)->InitDouble("Buffer Size", 960, 1, 10000, 1);
  GetParam(kFrameSize)->InitEnum("Frame Size", 1, 4, "Samples", IParam::kFlagsNone, "", "120", "240", "480", "960");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    mGraphics = pGraphics;
    pGraphics->AttachCornerResizer(iplug::igraphics::EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(style::BACKGROUND);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    IRECT b = pGraphics->GetBounds();
    b.B = style::TAB_BAR_HEIGHT;
    /**
     * Tab Bar
     */
    mMainTabButton = new IVButtonControl(b.SubRectHorizontal(2, 0), [&](IControl* caller) {
      switchTab(false);
    }, "Main Tab");
    pGraphics->AttachControl(mMainTabButton);
    mDirectTabButton = new IVButtonControl(b.SubRectHorizontal(2, 1), [&](IControl* caller) {
      switchTab(true);
    }, "Direct Connect");
    pGraphics->AttachControl(mDirectTabButton);

    /**
     * Main tab controls
     */
    b.T = b.B;
    b.B += 300;

    mMasterServer = new TextControl(b.SubRectVertical(3, 0), [&](IControl* pCaller) {
      TextControl* c = dynamic_cast<TextControl*>(pCaller);
      if (c == nullptr) { return; }
      c->callback = [&](const char* host) {
        return host;
      };
      GetUI()->CreateTextEntry(*pCaller, IText(20.f), pCaller->GetRECT(), "MasterSeverPopup");
    }, "MasterSever");
    mMainTab.Add(mMasterServer);

    mMasterId = new ITextControl(b.SubRectVertical(3, 1), "MasterOwnID");
    mMainTab.Add(mMasterId);

    mMasterPeer = new TextControl(b.SubRectVertical(3, 2), [&](IControl* pCaller) {
      TextControl* c = dynamic_cast<TextControl*>(pCaller);
      if (c == nullptr) { return; }
      c->callback = [&](const char* host) {
        return host;
      };
      GetUI()->CreateTextEntry(*pCaller, IText(20.f), pCaller->GetRECT(), "MasterPeerPopup");
    }, "MasterPeer");
    mMainTab.Add(mMasterPeer);


    /**
     * Direct connection tab controls
     */
     // https://api.ipify.org/
    mOwnIp = new ITextControl(b.SubRectVertical(2, 0), "OwnIP");
    mDirectTab.Add(mOwnIp);

    mDirectPeer = new TextControl(b.SubRectVertical(2, 1), [&](IControl* pCaller) {
      TextControl* c = dynamic_cast<TextControl*>(pCaller);
      if (c == nullptr) { return; }
      c->callback = [&](const char* host) {
        return host;
      };
      GetUI()->CreateTextEntry(*pCaller, IText(20.f), pCaller->GetRECT(), "DirectPeerPopup");
    }, "DirectPeer");
    mDirectTab.Add(mDirectPeer);

    

    // mDirectTab.Add(new IVKnobControl())

    for (int i = 0; i < mDirectTab.GetSize(); i++) {
      pGraphics->AttachControl(mDirectTab.Get(i));
    }

    for (int i = 0; i < mMainTab.GetSize(); i++) {
      pGraphics->AttachControl(mMainTab.Get(i));
    }

    switchTab(false);


    /**
     * General opus controls
     */

    b.T = b.B;
    b.B = pGraphics->GetBounds().B;

    const int nRows = 3;
    const int nCols = 3;

    int cellIdx = -1;

    auto nextCell = [&]() {
      return b.GetGridCell(++cellIdx, nRows, nCols).GetHPadded(-5.);
    };

    auto sameCell = [&]() {
      return b.GetGridCell(cellIdx, nRows, nCols).GetHPadded(-5.);
    };

    pGraphics->AttachControl(new IVKnobControl(nextCell(), kVolume), true);
    pGraphics->AttachControl(new IVKnobControl(nextCell(), kVolumeRemote), true);
    pGraphics->AttachControl(new IVKnobControl(nextCell(), kBitRate), true);
    pGraphics->AttachControl(new IVKnobControl(nextCell(), kPacketLoss), true);
    pGraphics->AttachControl(new IVKnobControl(nextCell(), kComplexity), true);
    pGraphics->AttachControl(new ITextControl(nextCell().SubRectVertical(2, 0), "Frame Size"));
    pGraphics->AttachControl(new ICaptionControl(sameCell().SubRectVertical(2, 1), kFrameSize, IText(24.f), iplug::igraphics::DEFAULT_FGCOLOR));
    pGraphics->AttachControl(new IVKnobControl(nextCell(), kBufferSize), true);

    //pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
    //mSessionManager.init("127.0.0.1", 55556, 55555);
    //if (mSessionManager.getId().empty()) {
    //  mSessionManager.start();
    //}
    //pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), mSessionManager.getId().c_str(), IText(50)));
    //if (mSessionManager.getId() == "!1") {
    //  mSessionManager.connectAsListener("!0");
    //}
  };
#endif
}

void Transmitter::switchTab(bool directTab) {
  for (int i = 0; i < mDirectTab.GetSize(); i++) {
    mDirectTab.Get(i)->Hide(!directTab);
  }
  for (int i = 0; i < mMainTab.GetSize(); i++) {
    mMainTab.Get(i)->Hide(directTab);
  }
  mMainTabButton->SetStyle(!directTab ? style::TAB_TEXT_ACTIVE : style::TAB_TEXT_INACTIVE);
  mDirectTabButton->SetStyle(directTab ? style::TAB_TEXT_ACTIVE : style::TAB_TEXT_INACTIVE);
  mGraphics->SetAllControlsDirty();
}


bool Transmitter::SerializeState(iplug::IByteChunk& chunk) const {
  WDL_String serialized;
  if (serialized.GetLength() < 1) {
    return false;
  }
  chunk.PutStr(serialized.Get());
  return true;
}

int Transmitter::UnserializeState(const iplug::IByteChunk& chunk, int startPos) {
  WDL_String json_string;
  return startPos;
}

void Transmitter::OnUIClose() {
  /**
   * Everything attached to the graphics will also be deleted once the UI
   * is closed, so just clear out the references
   */
  mMainTab.Empty(false);
  mDirectTab.Empty(false);
  mGraphics = nullptr;;
}

#if IPLUG_DSP
void Transmitter::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  const int nChans = NOutChansConnected();
  mSessionManager.pushSamples(inputs, nFrames);
  mSessionManager.pollSamples(outputs, nFrames);
  for (int i = 0; i < nFrames; i++) {
    for (int c = 0; c < nChans; c++) {
      // outputs[c][i] += inputs[c][i]; // Add the old signal on top
      outputs[c][i] = 0;
    }
  }
}
#endif
