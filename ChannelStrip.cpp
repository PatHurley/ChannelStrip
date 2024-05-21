#include "ChannelStrip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugPaths.h"

IColor ChStWhite = IColor(255, 224, 224, 224);
IColor ChStGray  = IColor(255, 128, 128, 128);
IColor ChStBlack = IColor(255, 32, 32, 32);
IText  ChStText  = IText(16.f, ChStWhite, "Roboto-Regular", EAlign::Center, EVAlign::Top);

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
  
  mLayoutFunc = [&](IGraphics* pGraphics) {  // GUI
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->AttachPanelBackground(ChStBlack);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    const IRECT b = pGraphics->GetBounds();

    // Asset Loading
    IBitmap EQFacePlate = pGraphics->LoadBitmap(PNG_EQBACKGROUND_FN);
    IBitmap DYNFacePlate = pGraphics->LoadBitmap(PNG_DYNBACKGROUND_FN);

    // Control Bounds
    IRECT inputKnobBounds = IRECT(0, 460, 56, 510); // l, t, r, b
    IRECT inputMeterBounds = IRECT(6, 6, 50, 454);

    IRECT outputKnobBounds = IRECT(956, 460, 1012, 510);
    IRECT outputMeterBounds = IRECT(962, 6, 1006, 454);

    IRECT EQBounds = IRECT(56, 0, 956, 250);
    IRECT EQ1Bounds = EQBounds.SubRectHorizontal(4, 0).GetReducedFromTop(5);
    IRECT EQ2Bounds = EQBounds.SubRectHorizontal(4, 1).GetReducedFromTop(5);
    IRECT EQ3Bounds = EQBounds.SubRectHorizontal(4, 2).GetReducedFromTop(5);
    IRECT EQ4Bounds = EQBounds.SubRectHorizontal(4, 3).GetReducedFromTop(5);

    IRECT DYNBounds = IRECT(56, 250, 956, 500);
    IRECT DYN1Bounds = DYNBounds.SubRectHorizontal(2, 0);
    IRECT DYN2Bounds = DYNBounds.SubRectHorizontal(2, 1);

    // Style Definitions
    IVStyle ioMeterStyle = DEFAULT_STYLE
      .WithLabelText(ChStText)                     // Custom label text
      .WithColor(kFG, IColor(255, 128, 255, 128))  // Signal Bars
      .WithColor(kBG, IColor(255, 64, 32, 32))     // Background
      .WithColor(kFR, ChStBlack)                   // Frame Lines
      .WithColor(kHL, ChStBlack)                   // Level Marker lines
      .WithColor(kX1, IColor(255, 232, 32, 32))    // Peak Marker line
      .WithColor(kON, IColor(255, 128, 128, 255)); // On color

    IVStyle ioKnobStyle = DEFAULT_STYLE
      .WithValueText(ChStText);

    IVStyle eqKnobStyle = DEFAULT_STYLE
      .WithColor(kFG, ChStWhite)
      .WithValueText(ChStText);

    IVStyle eqSwitchStyle = DEFAULT_STYLE;
    
    // Control attachment
    pGraphics->AttachControl(new IBitmapControl(EQBounds, EQFacePlate)); // EQ Background
    pGraphics->AttachControl(new IBitmapControl(DYNBounds, DYNFacePlate)); // DYN Background

    AttachBandControls(pGraphics, EQ1Bounds, eqKnobStyle, eqSwitchStyle, kEqBand1Gain, kEqBand1Freq, kEqBand1Q, kEqBand1Alt, "HPF");
    AttachBandControls(pGraphics, EQ2Bounds, eqKnobStyle, eqSwitchStyle, kEqBand2Gain, kEqBand2Freq, kEqBand2Q, kEqBand2Alt, "LO SHLF");
    AttachBandControls(pGraphics, EQ3Bounds, eqKnobStyle, eqSwitchStyle, kEqBand3Gain, kEqBand3Freq, kEqBand3Q, kEqBand3Alt, "HI SHLF");
    AttachBandControls(pGraphics, EQ4Bounds, eqKnobStyle, eqSwitchStyle, kEqBand4Gain, kEqBand4Freq, kEqBand4Q, kEqBand4Alt, "LPF");

    IVKnobControl* inputKnob = new IVKnobControl(inputKnobBounds, kGainIn, "", ioKnobStyle, true, false, -135.0, 27.0, 0.0);
    IVPeakAvgMeterControl<2>* inputMeter = new IVPeakAvgMeterControl<2>(inputMeterBounds, "IN", ioMeterStyle);

    IVKnobControl* outputKnob = new IVKnobControl(outputKnobBounds, kGainOut, "", ioKnobStyle, true, false, -135.0, 27.0, 0.0);
    IVPeakAvgMeterControl<2>* outputMeter = new IVPeakAvgMeterControl<2>(outputMeterBounds, "OUT", ioMeterStyle);

    pGraphics->AttachControl(inputKnob);
    pGraphics->AttachControl(inputMeter, kCtrlTagInMeter);
    
    pGraphics->AttachControl(outputKnob);
    pGraphics->AttachControl(outputMeter, kCtrlTagOutMeter);
  };
#endif
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

  std::vector<double> signalLevelsIn(nChans, 0.0);
  std::vector<double> signalLevelsOut(nChans, 0.0);

  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      // Apply input gain
      outputs[c][s] = inputs[c][s] * pow(10.0, gainIn / 20.00);
      signalLevelsIn[c] += outputs[c][s] * outputs[c][s];

      // TODO: Apply EQ

      // TODO: Apply DYN

      // Apply output gain
      outputs[c][s] *= pow(10.0, gainOut / 20.00);
      signalLevelsOut[c] += outputs[c][s] * outputs[c][s];
    }
  }

  mPeakAvgInMeterSender.ProcessBlock(inputs, nFrames, kCtrlTagInMeter);
  mPeakAvgOutMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagOutMeter);
}
#endif

void ChannelStrip::AttachBandControls(
  IGraphics* pGraphics, IRECT bandRect, IVStyle knobStyle, IVStyle switchStyle, int gainParamIndex, int freqParamIndex, int qParamIndex, int modeParamIndex, const char* altLabel)
{
  IRECT gainBounds = bandRect.SubRectVertical(2, 0).GetCentredInside(125, 125);
  IRECT freqBounds = bandRect.SubRectVertical(4, 2).SubRectHorizontal(2, 0).GetCentredInside(66, 66);
  IRECT qBounds = bandRect.SubRectVertical(4, 2).SubRectHorizontal(2, 1).GetCentredInside(66, 66);
  IRECT modeBounds = bandRect.SubRectVertical(4, 3).GetCentredInside(150, 30);

  pGraphics->AttachControl(new IVKnobControl(gainBounds, gainParamIndex, "", knobStyle, true, false, -135, 135, 0));
  pGraphics->AttachControl(new IVKnobControl(freqBounds, freqParamIndex, "", knobStyle, true, false, -135, 135));
  pGraphics->AttachControl(new IVKnobControl(qBounds, qParamIndex, "", knobStyle, true, false, -135, 135, 0));
  pGraphics->AttachControl(new IVTabSwitchControl(modeBounds, modeParamIndex, {"BELL", altLabel}, "", switchStyle));
}
