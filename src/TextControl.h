#pragma once

#include "./Types.h"

namespace transmitter {
  class TextControl : public IVButtonControl {
  public:
    TextControl(const IRECT& bounds, IActionFunction aF = SplashClickActionFunc, const char* label = "") :IVButtonControl(bounds, aF, label) {}
    std::function<const char* (const char* str)> callback;
    void OnTextEntryCompletion(const char* str, int valIdx) override {
      const char* res = callback(str);
      mLabelStr.Set(res);
    }

    const char* GetLabelString() {
      return mLabelStr.Get();
    }
  };
}