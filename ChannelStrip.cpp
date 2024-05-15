#include "ChannelStrip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugPaths.h"

IColor ChStWhite = IColor(255, 224, 224, 224);
IColor ChStBlack = IColor(255, 32, 32, 32);

ChannelStrip::ChannelStrip(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  //Initialize Parameters
  GetParam(kGainIn)->InitDouble("", 0.0, -50.0, 10.0, 0.01, "dB");
  GetParam(kGainOut)->InitDouble("", 0.0, -50.0, 10.0, 0.01, "dB");

  GetParam(kEqBand1Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand1Freq)->InitDouble("", 100.0, 20.0, 20000.0, 0.1, "Hz");
  GetParam(kEqBand1Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q");
  GetParam(kEqBand1Alt)->InitBool("", false);

  GetParam(kEqBand2Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand2Freq)->InitDouble("", 1000.0, 20.0, 20000.0, 0.1, "Hz");
  GetParam(kEqBand2Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q");
  GetParam(kEqBand2Alt)->InitBool("", false);

  GetParam(kEqBand3Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand3Freq)->InitDouble("", 5000.0, 20.0, 20000.0, 0.1, "Hz");
  GetParam(kEqBand3Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q");
  GetParam(kEqBand3Alt)->InitBool("", false);

  GetParam(kEqBand4Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand4Freq)->InitDouble("", 10000.0, 20.0, 20000.0, 0.1, "Hz");
  GetParam(kEqBand4Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q");
  GetParam(kEqBand4Alt)->InitBool("", false);

  GetParam(kDyn1Thresh)->InitDouble("", 0.0, -50.0, 0.0, 0.01, "dB");
  GetParam(kDyn1Attack)->InitDouble("", 15.0, .01, 1000.0, 0.01, "ms");
  GetParam(kDyn1Release)->InitDouble("", 100.0, 5.0, 5000.0, 0.1, "ms");
  GetParam(kDyn1Ratio)->InitInt("", 0, 0, 7);

  GetParam(kDyn2Thresh)->InitDouble("", 0.0, -50.0, 0.0, 0.01, "dB");
  GetParam(kDyn2Attack)->InitDouble("", 15.0, .01, 1000.0, 0.01, "ms");
  GetParam(kDyn2Release)->InitDouble("", 100.0, 5.0, 5000.0, 0.1, "ms");
  GetParam(kDyn2Ratio)->InitInt("", 0, 0, 7);

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(IColor(255, 128, 128, 128));
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();

    //EQ
    IRECT EQBounds = IRECT(50, 0, 950, 250);
    IRECT EQ1Bounds = EQBounds.SubRectHorizontal(4, 0).GetReducedFromTop(5);
    IRECT EQ2Bounds = EQBounds.SubRectHorizontal(4, 1).GetReducedFromTop(5);
    IRECT EQ3Bounds = EQBounds.SubRectHorizontal(4, 2).GetReducedFromTop(5);
    IRECT EQ4Bounds = EQBounds.SubRectHorizontal(4, 3).GetReducedFromTop(5);

    pGraphics->AttachControl(new IPanelControl(EQBounds, ChStBlack)); // EQ Background

    AttachBandControls(pGraphics, EQ1Bounds, kEqBand1Gain, kEqBand1Freq, kEqBand1Q, kEqBand1Alt, "HPF");
    AttachBandControls(pGraphics, EQ2Bounds, kEqBand2Gain, kEqBand2Freq, kEqBand2Q, kEqBand2Alt, "LO SHLF");
    AttachBandControls(pGraphics, EQ3Bounds, kEqBand3Gain, kEqBand3Freq, kEqBand3Q, kEqBand3Alt, "HI SHLF");
    AttachBandControls(pGraphics, EQ4Bounds, kEqBand4Gain, kEqBand4Freq, kEqBand4Q, kEqBand4Alt, "LPF");

    //DYN
    IRECT DYNBounds = IRECT(50, 250, 950, 500);
    IRECT DYN1Bounds = DYNBounds.SubRectHorizontal(2, 0);
    IRECT DYN2Bounds = DYNBounds.SubRectHorizontal(2, 1);

    pGraphics->AttachControl(new IPanelControl(DYNBounds, ChStWhite)); // DYN Background

    // Input gain & meter
    IRECT inputKnobBounds = IRECT(0, 460, 50, 510);
    IRECT inputMeterBounds = IRECT(5, 5, 45, 455);

    IVKnobControl* inputKnob = new IVKnobControl(inputKnobBounds, kGainIn, "", DEFAULT_STYLE, true, false, -135.0, 27.0, 0.0);
    IVPeakAvgMeterControl<2>* inputMeter = new IVPeakAvgMeterControl<2>(inputMeterBounds, "IN", DEFAULT_STYLE, EDirection::Vertical);

    pGraphics->AttachControl(inputKnob);
    pGraphics->AttachControl(inputMeter, kCtrlTagInMeter);

    // Output gain & meter
    IRECT outputKnobBounds = IRECT(950, 460, 1000, 510);
    IRECT outputMeterBounds = IRECT(955, 5, 995, 455);

    IVKnobControl* outputKnob = new IVKnobControl(outputKnobBounds, kGainOut, "", DEFAULT_STYLE, true, false, -135.0, 27.0, 0.0);
    IVPeakAvgMeterControl<2>* outputMeter = new IVPeakAvgMeterControl<2>(outputMeterBounds, "OUT", DEFAULT_STYLE, EDirection::Vertical);

    pGraphics->AttachControl(outputKnob);
    pGraphics->AttachControl(outputMeter, kCtrlTagOutMeter);
  };
#endif
}

void ChannelStrip::AttachBandControls(IGraphics* pGraphics, IRECT bandRect,
                                      int gainParamIndex, int freqParamIndex, int qParamIndex,
                                      int modeParamIndex, const char* altLabel)
{
  IRECT gainBounds = bandRect.SubRectVertical(2, 0).GetCentredInside(125, 125);
  IRECT freqBounds = bandRect.SubRectVertical(4, 2).SubRectHorizontal(2, 0).GetCentredInside(66, 66);
  IRECT qBounds = bandRect.SubRectVertical(4, 2).SubRectHorizontal(2, 1).GetCentredInside(66, 66);
  IRECT modeBounds = bandRect.SubRectVertical(4, 3).GetCentredInside(150, 30);

  IVStyle knobStyle = IVStyle().WithColor(kFG, ChStWhite);
  knobStyle.valueText = IText(16.f, ChStWhite, "Roboto-Regular", EAlign::Center, EVAlign::Top);
  IVStyle switchStyle = DEFAULT_STYLE;

  pGraphics->AttachControl(new IVKnobControl(gainBounds, gainParamIndex, "", knobStyle, true, false, -135, 135, 0));
  pGraphics->AttachControl(new IVKnobControl(freqBounds, freqParamIndex, "", knobStyle, true, false, -135, 135));
  pGraphics->AttachControl(new IVKnobControl(qBounds, qParamIndex, "", knobStyle, true, false, -135, 135, 0));
  pGraphics->AttachControl(new IVTabSwitchControl(modeBounds, modeParamIndex, {"BELL", altLabel}, "", switchStyle));
}

#if IPLUG_DSP
void ChannelStrip::OnIdle()
{
  mPeakAvgInMeterSender.TransmitData(*this);
  mPeakAvgOutMeterSender.TransmitData(*this);
}

void ChannelStrip::OnReset()
{
  mPeakAvgInMeterSender.Reset(GetSampleRate());
  mPeakAvgOutMeterSender.Reset(GetSampleRate());
}

void ChannelStrip::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gainIn = GetParam(kGainIn)->Value();
  const double gainOut = GetParam(kGainOut)->Value();

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

  mPeakAvgInMeterSender.ProcessBlock(inputs, nFrames, kCtrlTagInMeter);
  mPeakAvgOutMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagOutMeter);
}
#endif
