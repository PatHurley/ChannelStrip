#include "ChannelStrip.h"
#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugPaths.h"

IColor ChStWhite = IColor(255, 224, 224, 224);
IColor ChStBlack = IColor(255, 32, 32, 32);

IText ChStTxt_White = IText(16.f, ChStWhite, "Roboto-Regular", EAlign::Center, EVAlign::Top);
IText ChStTxt_Black = IText(16.f, ChStBlack, "Roboto-Regular", EAlign::Center, EVAlign::Bottom);

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

  GetParam(kDyn1Thresh)->InitDouble("", -60.0, -70.0, 0.0, 0.1, "dB");
  GetParam(kDyn1Alt)->InitBool("", false);
  GetParam(kDyn1Attack)->InitDouble("", .01, .01, 1000.0, 0.01, "ms");
  GetParam(kDyn1Release)->InitDouble("", 150.0, 5.0, 5000.0, 0.1, "ms");
  GetParam(kDyn1Ratio)->InitInt("", 5, 0, 7);

  GetParam(kDyn2Thresh)->InitDouble("", -18.0, -70.0, 0.0, 0.1, "dB");
  GetParam(kDyn2Alt)->InitBool("", false);
  GetParam(kDyn2Attack)->InitDouble("", 15.0, .01, 1000.0, 0.01, "ms");
  GetParam(kDyn2Release)->InitDouble("", 100.0, 5.0, 5000.0, 0.1, "ms");
  GetParam(kDyn2Ratio)->InitInt("", 1, 0, 7);

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
    ISVG EQKnobFront = pGraphics->LoadSVG(SVG_EQKNOBFACE_FN);

    IBitmap DYNFacePlate = pGraphics->LoadBitmap(PNG_DYNBACKGROUND_FN);
    ISVG DYNKnobFront = pGraphics->LoadSVG(SVG_DYNKNOBFACE_FN);

    ISVG KnobBack = pGraphics->LoadSVG(SVG_KNOBBACK_FN);

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
    IRECT DYN1Bounds = DYNBounds.SubRectHorizontal(3, 0);
    IRECT DYN2Bounds = DYNBounds.SubRectHorizontal(3, 2);
    IRECT DYNDisplay = DYNBounds.SubRectHorizontal(3, 1).GetCentredInside(280, 230);

    IRECT D1Thresh = DYN1Bounds.SubRectHorizontal(2, 1).SubRectVertical(6, 2).GetCentredInside(110);
    IRECT D1Mode = DYN1Bounds.SubRectHorizontal(2, 1).SubRectVertical(7, 5).GetCentredInside(150, 30);
    IRECT D1Attack = DYN1Bounds.SubRectHorizontal(4, 1).SubRectVertical(2, 0).GetCentredInside(50);
    IRECT D1Release = DYN1Bounds.SubRectHorizontal(4, 1).SubRectVertical(2, 1).GetCentredInside(50);
    IRECT D1RatioSel = DYN1Bounds.SubRectHorizontal(4, 0).SubRectVertical(5, 0).GetCentredInside(60, 30);
    IRECT D1RatioInd = DYN1Bounds.SubRectHorizontal(4, 0).SubRectVertical(6, 3).GetCentredInside(60, 150);

    IRECT D2Thresh = DYN2Bounds.SubRectHorizontal(2, 0).SubRectVertical(6, 2).GetCentredInside(110);
    IRECT D2Mode = DYN2Bounds.SubRectHorizontal(2, 0).SubRectVertical(7, 5).GetCentredInside(150, 30);
    IRECT D2Attack = DYN2Bounds.SubRectHorizontal(4, 2).SubRectVertical(2, 0).GetCentredInside(50);
    IRECT D2Release = DYN2Bounds.SubRectHorizontal(4, 2).SubRectVertical(2, 1).GetCentredInside(50);
    IRECT D2RatioSel = DYN2Bounds.SubRectHorizontal(4, 3).SubRectVertical(5, 0).GetCentredInside(60, 30);
    IRECT D2RatioInd = DYN2Bounds.SubRectHorizontal(4, 3).SubRectVertical(6, 3).GetCentredInside(60, 150);

    // Style Definitions
    IVStyle ioMeterStyle = DEFAULT_STYLE
      .WithLabelText(ChStTxt_White)                // Custom label text
      .WithColor(kFG, IColor(255, 128, 255, 128))  // Signal Bars
      .WithColor(kBG, IColor(255, 64, 32, 32))     // Background
      .WithColor(kFR, ChStBlack)                   // Frame Lines
      .WithColor(kHL, ChStBlack)                   // Level Marker lines
      .WithColor(kX1, IColor(255, 232, 32, 32))    // Peak Marker line
      .WithColor(kON, IColor(255, 128, 128, 255)); // On color

    IVStyle ioKnobStyle = DEFAULT_STYLE
      .WithValueText(ChStTxt_White);

    IVStyle eqSwitchStyle = DEFAULT_STYLE;

    IVStyle dynKnobStyle = DEFAULT_STYLE
      .WithColor(kFG, ChStBlack)
      .WithColor(kFR, ChStWhite)
      .WithColor(kX1, ChStBlack)
      .WithValueText(ChStTxt_Black);

    IVStyle dynSwitchStyle = DEFAULT_STYLE;
    
    // Control attachment
    pGraphics->AttachControl(new IBitmapControl(EQBounds, EQFacePlate)); // EQ Background
    pGraphics->AttachControl(new IBitmapControl(DYNBounds, DYNFacePlate)); // DYN Background

    AttachBandControls(pGraphics, EQKnobFront, KnobBack, eqSwitchStyle, EQ1Bounds, kEqBand1Gain, kEqBand1Freq, kEqBand1Q, kEqBand1Alt, "HPF");
    AttachBandControls(pGraphics, EQKnobFront, KnobBack, eqSwitchStyle, EQ2Bounds, kEqBand2Gain, kEqBand2Freq, kEqBand2Q, kEqBand2Alt, "LO SHLF");
    AttachBandControls(pGraphics, EQKnobFront, KnobBack, eqSwitchStyle, EQ3Bounds, kEqBand3Gain, kEqBand3Freq, kEqBand3Q, kEqBand3Alt, "HI SHLF");
    AttachBandControls(pGraphics, EQKnobFront, KnobBack, eqSwitchStyle, EQ4Bounds, kEqBand4Gain, kEqBand4Freq, kEqBand4Q, kEqBand4Alt, "LPF");

    AttachDynControls(
      pGraphics, DYNKnobFront, KnobBack, dynSwitchStyle,
      D1Thresh, D1Mode, D1Attack, D1Release, D1RatioSel, D1RatioInd,
      kDyn1Thresh, kDyn1Alt, kDyn1Attack, kDyn1Release, kDyn1Ratio,
      "GATE", "EXPAND");

    AttachDynControls(
      pGraphics, DYNKnobFront, KnobBack, dynSwitchStyle,
      D2Thresh, D2Mode, D2Attack, D2Release, D2RatioSel, D2RatioInd,
      kDyn2Thresh, kDyn2Alt, kDyn2Attack, kDyn2Release, kDyn2Ratio,
      "COMP", "LIMIT");

    pGraphics->AttachControl(new IPanelControl(DYNDisplay, IColor(128, 32, 32, 32))); // DYN meter background

    IVKnobControl* inputKnob = new IVKnobControl(inputKnobBounds, kGainIn, "", ioKnobStyle, true, false, -135.0, 27.0, 0.0);
    IVPeakAvgMeterControl<2>* inputMeter = new IVPeakAvgMeterControl<2>(inputMeterBounds, "IN", ioMeterStyle);

    pGraphics->AttachControl(inputKnob);
    pGraphics->AttachControl(inputMeter, kCtrlTagInMeter);

    IVKnobControl* outputKnob = new IVKnobControl(outputKnobBounds, kGainOut, "", ioKnobStyle, true, false, -135.0, 27.0, 0.0);
    IVPeakAvgMeterControl<2>* outputMeter = new IVPeakAvgMeterControl<2>(outputMeterBounds, "OUT", ioMeterStyle);
    
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
  IGraphics* pGraphics, ISVG knobFront, ISVG knobBack, IVStyle switchStyle, IRECT bandRect, 
  int gainParamIndex, int freqParamIndex, int qParamIndex,
  int modeParamIndex, const char* altLabel)
{
  IRECT gainBounds = bandRect.SubRectVertical(2, 0).GetCentredInside(110);
  IRECT gainValue = gainBounds.GetCentredInside(50, 20).GetTranslated(0, 65);
  pGraphics->AttachControl(new ISVGControl(gainBounds.GetPadded(2), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(gainBounds, knobFront, gainParamIndex));
  pGraphics->AttachControl(new ICaptionControl(gainValue, gainParamIndex, ChStTxt_White, 0, true));

  IRECT freqBounds = bandRect.SubRectVertical(4, 2).SubRectHorizontal(2, 0).GetCentredInside(50);
  IRECT freqValue = freqBounds.GetCentredInside(65, 20).GetTranslated(0, 35);
  pGraphics->AttachControl(new ISVGControl(freqBounds.GetPadded(1.5), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(freqBounds, knobFront, freqParamIndex));
  pGraphics->AttachControl(new ICaptionControl(freqValue, freqParamIndex, ChStTxt_White, 0, true));

  IRECT qBounds = bandRect.SubRectVertical(4, 2).SubRectHorizontal(2, 1).GetCentredInside(50);
  IRECT qValue = qBounds.GetCentredInside(50, 20).GetTranslated(0, 35);
  pGraphics->AttachControl(new ISVGControl(qBounds.GetPadded(1.5), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(qBounds, knobFront, qParamIndex));
  pGraphics->AttachControl(new ICaptionControl(qValue, qParamIndex, ChStTxt_White, 0, true));

  IRECT modeBounds = bandRect.SubRectVertical(4, 3).GetCentredInside(150, 30);
  pGraphics->AttachControl(new IVTabSwitchControl(modeBounds, modeParamIndex, {"BELL", altLabel}, "", switchStyle));
}

void ChannelStrip::AttachDynControls(
  IGraphics* pGraphics, ISVG knobFront, ISVG knobBack, IVStyle buttonStyle,
  IRECT threshBounds, IRECT modeBounds, IRECT attackBounds, IRECT releaseBounds, IRECT ratioSelBounds, IRECT ratioIndBounds,
  int threshIndex, int modeIndex, int attackIndex, int releaseIndex, int ratioIndex,
  const char* normLabel, const char* altLabel)
{
  IRECT threshValue = threshBounds.GetCentredInside(50, 20).GetTranslated(0, 65);
  pGraphics->AttachControl(new ISVGControl(threshBounds.GetPadded(2), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(threshBounds, knobFront, threshIndex));
  pGraphics->AttachControl(new ICaptionControl(threshValue, threshIndex, ChStTxt_Black, 0, true));

  IRECT attackValue = attackBounds.GetCentredInside(60, 20).GetTranslated(0, 35);
  pGraphics->AttachControl(new ISVGControl(attackBounds.GetPadded(1.5), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(attackBounds, knobFront, attackIndex));
  pGraphics->AttachControl(new ICaptionControl(attackValue, attackIndex, ChStTxt_Black, 0, true));

  IRECT releaseValue = releaseBounds.GetCentredInside(60, 20).GetTranslated(0, 35);
  pGraphics->AttachControl(new ISVGControl(releaseBounds.GetPadded(1.5), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(releaseBounds, knobFront, releaseIndex));
  pGraphics->AttachControl(new ICaptionControl(releaseValue, releaseIndex, ChStTxt_Black, 0, true));

  pGraphics->AttachControl(new IVTabSwitchControl(modeBounds, modeIndex, {normLabel, altLabel}, "", buttonStyle));

  pGraphics->AttachControl(new IVRadioButtonControl(ratioIndBounds, ratioIndex, {"1:1", "2:1", "3:1", "4:1", "8:1", "10:1", "20:1", "INF"}, "", DEFAULT_STYLE));
}
