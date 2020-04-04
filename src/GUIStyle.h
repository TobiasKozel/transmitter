#pragma once

#include "Types.h"
namespace transmitter {
  namespace style {
    const float TAB_BAR_HEIGHT = 32;
    const IColor ACCENT(255, 255, 150, 150);
    const IColor TEXTCOLOR(255, 60, 60, 60);
    const IColor BACKGROUND(255, 40, 40, 40);
    const IVStyle TAB_TEXT_ACTIVE{ ACCENT };
    const IVStyle TAB_TEXT_INACTIVE{ TEXTCOLOR };
    // const IText TAB_TEXT_ACTIVE{ 22, ACCENT, "Roboto-Regular", EAlign::Center, EVAlign::Bottom, 0 };
    // const IText TAB_TEXT_INACTIVE{ 18, TEXTCOLOR, "Roboto-Regular", EAlign::Center, EVAlign::Bottom, 0 };
  }
}