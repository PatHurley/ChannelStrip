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
  kDyn1Thresh, kDyn1Attack, kDyn1Release, kDyn1Ratio, kDyn1Alt,
  kDyn2Thresh, kDyn2Attack, kDyn2Release, kDyn2Ratio, kDyn2Alt,
  kDynMeter,
  kNumParams
};

enum EControlTags
{
  kCtrlTagInMeter = 0,
  kCtrlTagOutMeter
  //kCtrlTagDynMeter
};

using namespace iplug;
using namespace igraphics;

class ChannelStrip final : public Plugin
{
public:
  ChannelStrip(const InstanceInfo& info);

  void ChannelStrip::AttachMeterControls(
  IGraphics* pGraphics, IRECT gainBounds, IRECT meterBounds,
  int gainIndex, int meterCtrlTag, IVStyle knobStyle, IVStyle meterStyle,
  const char* label);

void ChannelStrip::AttachBandControls(
  IGraphics* pGraphics, ISVG knobFront, ISVG knobBack, IVStyle switchStyle, IRECT bandRect,
  int gainParamIndex, int freqParamIndex, int qParamIndex,
  int modeParamIndex,  const char* altLabel);

void ChannelStrip::AttachDynControls(
  IGraphics* pGraphics, ISVG knobFront, ISVG knobBack, IVStyle buttonStyle,  IVStyle ratioIndStyle,
  IRECT threshBounds, IRECT modeBounds, IRECT attackBounds, IRECT releaseBounds, IRECT ratioSelBounds, IRECT ratioIndBounds,
  int threshIndex, int modeIndex, int attackIndex, int releaseIndex, int ratioIndex,
  const char* normLabel, const char* altLabel);

#if IPLUG_DSP // http://bit.ly/2S64BDd
public:
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
  void OnIdle() override;
  void OnReset() override;

private:
  IPeakAvgSender<2> mPeakAvgInMeterSender;
  IPeakAvgSender<2> mPeakAvgOutMeterSender;
  //IPeakAvgSender<2> mPeakAvgDynMeterSender;

  double mSampleRate;
#endif
};
