#pragma once

#ifndef __EMSCRIPTEN__
#include "IControl.h"
#include "IControls.h"
#include "IGraphicsStructs.h"
#endif

namespace transmitter {
  const int MAX_PACKET_SIZE = 1464; // We'll just go with the max udp packet size without fragmentation

  typedef float sample;
#ifndef __EMSCRIPTEN__
  typedef iplug::igraphics::IGraphics IGraphics;
  typedef iplug::igraphics::IControl IControl;
  typedef iplug::igraphics::IActionFunction IActionFunction;
  inline void SplashClickActionFunc(IControl* pCaller) {
    iplug::igraphics::SplashClickActionFunc(pCaller);
  }
  typedef iplug::IParam IParam;
  typedef iplug::igraphics::IURLControl IURLControl;
  typedef iplug::igraphics::ITextControl ITextControl;
  typedef iplug::igraphics::ICaptionControl ICaptionControl;
  typedef iplug::igraphics::IKnobControlBase  IKnobControlBase;
  typedef iplug::igraphics::IVButtonControl IVButtonControl;
  typedef iplug::igraphics::ITextToggleControl ITextToggleControl;
  typedef iplug::igraphics::IGraphics IGraphics;
  typedef iplug::igraphics::IRECT IRECT;
  typedef iplug::igraphics::IColor IColor;
  typedef iplug::igraphics::IText IText;
  typedef iplug::igraphics::EAlign EAlign;
  typedef iplug::igraphics::EVAlign EVAlign;
  typedef iplug::igraphics::IVKnobControl IVKnobControl;
  typedef iplug::igraphics::IVectorBase IVectorBase;
  typedef iplug::igraphics::ISVG ISVG;
  typedef iplug::igraphics::IBitmap IBitmap;
  typedef iplug::igraphics::ILayerPtr ILayerPtr;
  typedef iplug::igraphics::IBlend IBlend;
  typedef iplug::igraphics::EBlend EBlend;
  typedef iplug::igraphics::EDirection EDirection;
  typedef iplug::igraphics::IMouseMod IMouseMod;
  typedef iplug::igraphics::IVStyle IVStyle;
  typedef iplug::igraphics::IPopupMenu IPopupMenu;
  const IVStyle DEFAULT_STYLE = IVStyle();
#endif

  /**
   * A macro to disable all kinds of implicit copy mechanisms
   */
  #define TRANSMITTER_NO_COPY(name) \
  name(const name&) = delete; \
  name(const name*) = delete; \
  name(name&&) = delete; \
  name& operator= (const name&) = delete; \
  name& operator= (name&&) = delete;
}