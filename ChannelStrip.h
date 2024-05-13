#pragma once

#include "IPlug_include_in_plug_hdr.h"
#include "IControls.h"
#include "IPlugPaths.h"

const int kNumPresets = 1;

enum EParams
{
  kGainIn = 0,
  kGainOut,
  kEqBand1Gain, kEqBand1Freq, kEqBand1Q, kEqBand1Alt,
  kEqBand2Gain, kEqBand2Freq, kEqBand2Q, kEqBand2Alt,
  kEqBand3Gain, kEqBand3Freq, kEqBand3Q, kEqBand3Alt,
  kEqBand4Gain, kEqBand4Freq, kEqBand4Q, kEqBand4Alt,
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
  void AttachBandControls(IGraphics* pGraphics, IRECT bandRect, int gainParamIndex, int freqParamIndex, int qParamIndex, int modeParamIndex, const char* bandLabel, const char* altLabel);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnIdle() override;
  void OnReset() override;

private:
  IPeakAvgSender<2> mPeakAvgInMeterSender{-90.0, true, 10.0f, 5.0f, 100.0f, 1000.0f};
  IPeakAvgSender<2> mPeakAvgOutMeterSender{-90.0, true, 10.0f, 5.0f, 100.0f, 1000.0f};
#endif
};