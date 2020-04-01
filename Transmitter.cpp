#include "Transmitter.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "./thirdparty/json.hpp"

Transmitter::Transmitter(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPrograms)), mSessionManager("localhost", 55555, 55556)
{
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
    pGraphics->AttachControl(new ITextControl(b.GetMidVPadded(50), "Hello iPlug 2!", IText(50)));
    pGraphics->AttachControl(new IVKnobControl(b.GetCentredInside(100).GetVShifted(-100), kGain));
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
void Transmitter::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gain = GetParam(kGain)->Value() / 100.;
  const int nChans = NOutChansConnected();
  
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      outputs[c][s] = inputs[c][s] * gain;
    }
  }
}
#endif
