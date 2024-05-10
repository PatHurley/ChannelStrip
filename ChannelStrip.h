#pragma once

#include "IPlug_include_in_plug_hdr.h"

const int kNumPresets = 1;

enum EParams
{
  kGainIn = 0,
  kGainOut,
  eqBand1Gain,
  eqBand1Freq,
  eqBand1Q,
  eqBand2Gain,
  eqBand2Freq,
  eqBand2Q,
  eqBand3Gain,
  eqBand3Freq,
  eqBand3Q,
  eqBand4Gain,
  eqBand4Freq,
  eqBand4Q,
  kNumParams
};

using namespace iplug;
using namespace igraphics;

class ChannelStrip final : public Plugin
{
public:
  ChannelStrip(const InstanceInfo& info);

#if IPLUG_DSP // http://bit.ly/2S64BDd
  void ProcessBlock(sample** inputs, sample** outputs, int nFrames) override;
#endif
};
