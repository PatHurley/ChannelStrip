#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "IPlugPaths.h"

const int kNumPresets = 1;

enum EParams
{
  kGainIn = 0,
  kGainOut,
  kNumParams
};

enum EControlTags
{
  kCtrlTagInMeter = 0,
  kCtrlTagOutMeter
};

using namespace iplug;
using namespace igraphics;

class ChannelStrip final : public Plugin
{
public:
  ChannelStrip(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnIdle() override;
  void OnReset() override;

private:
  IPeakSender<2> mMeterSender;
  IPeakAvgSender<2> mPeakAvgInMeterSender{-90.0, true, 10.0f, 5.0f, 100.0f, 1000.0f};
  IPeakAvgSender<2> mPeakAvgOutMeterSender{-90.0, true, 10.0f, 5.0f, 100.0f, 1000.0f};
#endif
};
