#include "ChannelStrip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"

ChannelStrip::ChannelStrip(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  //Initialize Parameters
  GetParam(kGainIn)->InitDouble("", 0.0, -50.0, 10.0, 0.01, "dB");
  GetParam(kGainOut)->InitDouble("", 0.0, -50.0, 10.0, 0.01, "dB");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(COLOR_GRAY);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();

    IVStyle meterStyle = DEFAULT_STYLE;
    IVStyle knobStyle = DEFAULT_STYLE;

    //Input
    IRECT inputKnobBounds = IRECT(0, 450, 50, 500);
    IVKnobControl* inputKnob = new IVKnobControl(inputKnobBounds, kGainIn, "", knobStyle, true, false, -135.0, 27.0, 0.0);
    pGraphics->AttachControl(inputKnob);

    IRECT inputMeterBounds = IRECT(5.0, 5.0, 45, 445);
    IVMeterControl<2>* inputMeter = new IVMeterControl<2>(inputMeterBounds, "IN", meterStyle, EDirection::Vertical);
    pGraphics->AttachControl(inputMeter);
    inputMeter->GetControl(0)->As<IVMeterControl<2>>()->Set(0, signalLevelsIn[0]);
    inputMeter->GetControl(1)->As<IVMeterControl<2>>()->Set(1, signalLevelsIn[1]);

    //Output
    pGraphics->AttachControl(new IVKnobControl(IRECT(950, 450, 1000, 500), kGainOut, "", knobStyle, true, false, -135.0, 27.0, 0.0));
    pGraphics->AttachControl(new IVMeterControl<2>(IRECT(955, 5.0, 995, 445), "OUT", meterStyle, EDirection::Vertical));
  };
#endif
}

#if IPLUG_DSP
void ChannelStrip::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gainIn = GetParam(kGainIn)->Value();
  const double gainOut = GetParam(kGainOut)->Value();

  /*
  const double eq1gain = GetParam(eqBand1Gain)->Value();
  const double eq1freq = GetParam(eqBand1Freq)->Value();
  const double eq1q = GetParam(eqBand1Q)->Value();
  */

  const int nChans = NOutChansConnected();

  // Buffer to store the signal level for each channel
  std::vector<double> signalLevelsIn(nChans, 0.0);
  std::vector<double> signalLevelsOut(nChans, 0.0);

  //Process Audio Samples
  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      //Apply input gain
      outputs[c][s] = inputs[c][s] * pow(10.0, gainIn / 20.00);
      signalLevelsIn[c] += outputs[c][s] * outputs[c][s];

      // TODO: Apply EQ

      // Apply output gain
      outputs[c][s] *= pow(10.0, gainOut / 20.00);
      signalLevelsOut[c] += outputs[c][s] * outputs[c][s];
    }
  }
}
#endif
