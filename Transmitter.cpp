#include "Transmitter.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "./thirdparty/json.hpp"

Transmitter::Transmitter(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms)) {
  GetParam(kGain)->InitDouble("Gain", 0., 0., 100.0, 0.01, "%");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, 1.);
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
    mSessionManager.init("127.0.0.1", 55556, 55555);
    if (mSessionManager.getId().empty()) {
      mSessionManager.start();
    }
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), mSessionManager.getId().c_str(), IText(50)));
    if (mSessionManager.getId() == "!1") {
      mSessionManager.connectAsListener("!0");
    }
  };
#endif
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

#if IPLUG_DSP
void Transmitter::ProcessBlock(sample** inputs, sample** outputs, int nFrames) {
  const int nChans = NOutChansConnected();
  mSessionManager.pushSamples(inputs, nFrames);
  mSessionManager.pollSamples(outputs, nFrames);
  for (int i = 0; i < nFrames; i++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][i] += inputs[c][i]; // Add the old signal on top
    }
  }
}
#endif
