#include "ChannelStrip.h"
#include "UI.h"

#include "IPlug_include_in_plug_src.h"
#include "IControls.h"
#include "IPlugPaths.h"

auto ChStWhite     = IColor(255, 224, 224, 224);
auto ChStBlack     = IColor(255, 32, 32, 32);
auto ChStTxt_White = IText(16.f, ChStWhite, "Roboto-Regular", EAlign::Center, EVAlign::Top);
auto ChStTxt_Black = IText(16.f, ChStBlack, "Roboto-Regular", EAlign::Center, EVAlign::Bottom);

ChannelStrip::ChannelStrip(const InstanceInfo& info)
: Plugin(info, MakeConfig(kNumParams, kNumPresets))
{
  //Initialize Parameters
  mSampleRate = GetSampleRate();

  GetParam(kGainIn)->InitDouble("", 0.0, -50.0, 10.0, 0.01, "dB");
  GetParam(kGainOut)->InitDouble("", 0.0, -50.0, 10.0, 0.01, "dB");

  GetParam(kEqBand1Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand1Freq)->InitDouble("", 100.0, 20.0, 20000.0, 0.1, "Hz", 0, "", IParam::ShapePowCurve(4.3));
  GetParam(kEqBand1Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q", 0, "", IParam::ShapePowCurve(3.3));
  GetParam(kEqBand1Alt)->InitBool("", false);

  GetParam(kEqBand2Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand2Freq)->InitDouble("", 1000.0, 20.0, 20000.0, 0.1, "Hz", 0, "", IParam::ShapePowCurve(4.3));
  GetParam(kEqBand2Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q", 0, "", IParam::ShapePowCurve(3.3));
  GetParam(kEqBand2Alt)->InitBool("", false);

  GetParam(kEqBand3Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand3Freq)->InitDouble("", 5000.0, 20.0, 20000.0, 0.1, "Hz", 0, "", IParam::ShapePowCurve(4.3));
  GetParam(kEqBand3Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q", 0, "", IParam::ShapePowCurve(3.3));
  GetParam(kEqBand3Alt)->InitBool("", false);

  GetParam(kEqBand4Gain)->InitDouble("", 0.0, -12.0, 12.0, 0.01, "dB");
  GetParam(kEqBand4Freq)->InitDouble("", 10000.0, 20.0, 20000.0, 0.1, "Hz", 0, "", IParam::ShapePowCurve(4.3));
  GetParam(kEqBand4Q)->InitDouble("", 1.0, 0.01, 10.0, 0.01, "Q", 0, "", IParam::ShapePowCurve(3.3));
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

  GetParam(kDynMeter)->InitDouble("", 0.0, 0.0, 72.0, 0.01, "dB");

#if IPLUG_EDITOR // http://bit.ly/2S64BDd
  mMakeGraphicsFunc = [&]() {
    return MakeGraphics(*this, PLUG_WIDTH, PLUG_HEIGHT, PLUG_FPS, GetScaleForScreen(PLUG_WIDTH, PLUG_HEIGHT));
  };
  
  mLayoutFunc = [&](IGraphics* pGraphics) {
    pGraphics->AttachCornerResizer(EUIResizerMode::Scale, false);
    pGraphics->LoadFont("Roboto-Regular", ROBOTO_FN);
    pGraphics->AttachPanelBackground(ChStBlack);

    // Asset Loading
    IBitmap EQ_BG_PNG = pGraphics->LoadBitmap(PNG_EQBACKGROUND);
    IBitmap DYN_BG_PNG = pGraphics->LoadBitmap(PNG_DYNBACKGROUND);

    ISVG KnobBack_SVG = pGraphics->LoadSVG(SVG_KNOBBACK);
    ISVG EQKnobFace_SVG = pGraphics->LoadSVG(SVG_EQKNOBFACE);
    ISVG DYNKnobFace_SVG = pGraphics->LoadSVG(SVG_DYNKNOBFACE);
    ISVG DYNDisplayFace = pGraphics->LoadSVG(SVG_DYNDISPLAYFACE);
    ISVG DYNDisplayNeedle = pGraphics->LoadSVG(SVG_DYNDISPLAYNEEDLE);

    // Control Bounds
    IRECT inputKnob_Bounds = IRECT(0, 460, 56, 510); // l, t, r, b
    IRECT inputMeter_Bounds = IRECT(6, 6, 50, 454);

    IRECT outputKnob_Bounds = IRECT(956, 460, 1012, 510);
    IRECT outputMeter_Bounds = IRECT(962, 6, 1006, 454);

    IRECT EQ_Bounds = IRECT(56, 0, 956, 250);
    IRECT EQ1_Bounds = EQ_Bounds.SubRectHorizontal(4, 0).GetReducedFromTop(5);
    IRECT EQ2_Bounds = EQ_Bounds.SubRectHorizontal(4, 1).GetReducedFromTop(5);
    IRECT EQ3_Bounds = EQ_Bounds.SubRectHorizontal(4, 2).GetReducedFromTop(5);
    IRECT EQ4_Bounds = EQ_Bounds.SubRectHorizontal(4, 3).GetReducedFromTop(5);

    IRECT DYN_Bounds = IRECT(56, 250, 956, 500);
    IRECT DYN1_Bounds = DYN_Bounds.SubRectHorizontal(3, 0);
    IRECT DYN2_Bounds = DYN_Bounds.SubRectHorizontal(3, 2);
    IRECT DYND_Bounds = DYN_Bounds.SubRectHorizontal(3, 1).GetPadded(-5);

    IRECT D1Thresh = DYN1_Bounds.SubRectHorizontal(2, 1).SubRectVertical(6, 2).GetCentredInside(110);
    IRECT D1Mode = DYN1_Bounds.SubRectHorizontal(2, 1).SubRectVertical(7, 5).GetCentredInside(150, 30);
    IRECT D1Attack = DYN1_Bounds.SubRectHorizontal(4, 1).SubRectVertical(2, 0).GetCentredInside(50);
    IRECT D1Release = DYN1_Bounds.SubRectHorizontal(4, 1).SubRectVertical(2, 1).GetCentredInside(50);
    IRECT D1RatioSel = DYN1_Bounds.SubRectHorizontal(4, 0).SubRectVertical(8, 1).GetCentredInside(60, 30);
    IRECT D1RatioInd = DYN1_Bounds.SubRectHorizontal(4, 0).SubRectVertical(6, 3).GetCentredInside(60, 150);

    IRECT D2Thresh = DYN2_Bounds.SubRectHorizontal(2, 0).SubRectVertical(6, 2).GetCentredInside(110);
    IRECT D2Mode = DYN2_Bounds.SubRectHorizontal(2, 0).SubRectVertical(7, 5).GetCentredInside(150, 30);
    IRECT D2Attack = DYN2_Bounds.SubRectHorizontal(4, 2).SubRectVertical(2, 0).GetCentredInside(50);
    IRECT D2Release = DYN2_Bounds.SubRectHorizontal(4, 2).SubRectVertical(2, 1).GetCentredInside(50);
    IRECT D2RatioSel = DYN2_Bounds.SubRectHorizontal(4, 3).SubRectVertical(8, 1).GetCentredInside(60, 30);
    IRECT D2RatioInd = DYN2_Bounds.SubRectHorizontal(4, 3).SubRectVertical(6, 3).GetCentredInside(60, 150);

    // Style Definitions
    IVStyle ioMeterStyle = DEFAULT_STYLE
      .WithLabelText(ChStTxt_White)              // Custom label text
      .WithColor(kFG, IColor(64, 172, 192, 208)) // Signal Bars
      .WithColor(kBG, 0)                         // Background
      .WithColor(kFR, COLOR_BLACK)               // Frame Lines
      .WithColor(kHL, COLOR_BLACK)               // Level Marker lines
      .WithColor(kX1, IColor(255, 232, 32, 32)); // Peak Marker line

    IVStyle ioKnobStyle = DEFAULT_STYLE
      .WithValueText(ChStTxt_White);

    IVStyle eqSwitchStyle = DEFAULT_STYLE
      .WithShowValue(false)
      .WithDrawShadows(false)
      .WithColor(kFG, IColor(255, 255, 255, 255))
      .WithColor(kPR, IColor(255, 180, 180, 192))
      .WithValueText(ChStTxt_Black)
      .WithEmboss(true);

    IVStyle dynKnobStyle = DEFAULT_STYLE
      .WithColor(kFG, ChStBlack)
      .WithColor(kFR, ChStWhite)
      .WithColor(kX1, ChStBlack)
      .WithValueText(ChStTxt_Black);

    IVStyle dynSwitchStyle = DEFAULT_STYLE
      .WithShowValue(false)
      .WithDrawShadows(false)
      .WithColor(kFG, IColor(255, 32, 32, 32))
      .WithColor(kPR, IColor(255, 0, 0, 0))
      .WithValueText(ChStTxt_White)
      .WithEmboss(true);

    IVStyle ratioIndStyle = DEFAULT_STYLE
      .WithColor(kFG, IColor(255, 96, 32, 32))
      .WithColor(kFR, IColor(255, 64, 32, 32))
      .WithColor(kON, IColor(255, 255, 0, 0))
      .WithDrawShadows(false)
      .WithValueText(ChStTxt_Black);

    // Control attachment
    pGraphics->AttachControl(new IBitmapControl(EQ_Bounds, EQ_BG_PNG));                // EQ Background
    pGraphics->AttachControl(new IBitmapControl(DYN_Bounds, DYN_BG_PNG));              // DYN Background
    pGraphics->AttachControl(new IPanelControl(DYND_Bounds, IColor(255, 32, 36, 36))); // DYN Display background

    AttachMeterControls(
      pGraphics, inputKnob_Bounds, inputMeter_Bounds,
      kGainIn, kCtrlTagInMeter, ioKnobStyle, ioMeterStyle, "IN");
    AttachMeterControls(
      pGraphics, outputKnob_Bounds, outputMeter_Bounds,
      kGainOut, kCtrlTagOutMeter, ioKnobStyle, ioMeterStyle, "OUT");

    AttachBandControls(
      pGraphics, EQKnobFace_SVG, KnobBack_SVG, eqSwitchStyle, EQ1_Bounds,
      kEqBand1Gain, kEqBand1Freq, kEqBand1Q, kEqBand1Alt, "HPF");
    AttachBandControls(
      pGraphics, EQKnobFace_SVG, KnobBack_SVG, eqSwitchStyle, EQ2_Bounds,
      kEqBand2Gain, kEqBand2Freq, kEqBand2Q, kEqBand2Alt, "LO SHLF");
    AttachBandControls(
      pGraphics, EQKnobFace_SVG, KnobBack_SVG, eqSwitchStyle, EQ3_Bounds,
      kEqBand3Gain, kEqBand3Freq, kEqBand3Q, kEqBand3Alt, "HI SHLF");
    AttachBandControls(
      pGraphics, EQKnobFace_SVG, KnobBack_SVG, eqSwitchStyle, EQ4_Bounds,
      kEqBand4Gain, kEqBand4Freq, kEqBand4Q, kEqBand4Alt, "LPF");

    AttachDynControls(
      pGraphics, DYNKnobFace_SVG, KnobBack_SVG, dynSwitchStyle, ratioIndStyle,
      D1Thresh, D1Mode, D1Attack, D1Release, D1RatioSel, D1RatioInd,
      kDyn1Thresh, kDyn1Alt, kDyn1Attack, kDyn1Release, kDyn1Ratio, "GATE", "EXPAND");
    AttachDynControls(
      pGraphics, DYNKnobFace_SVG, KnobBack_SVG, dynSwitchStyle, ratioIndStyle,
      D2Thresh, D2Mode, D2Attack, D2Release, D2RatioSel, D2RatioInd,
      kDyn2Thresh, kDyn2Alt, kDyn2Attack, kDyn2Release, kDyn2Ratio, "COMP", "LIMIT");

    //// Dynamics Display
    //pGraphics->AttachControl(new IPanelControl(DYND_Bounds.GetHPadded(-20).GetVPadded(-10), IColor(255, 72, 80, 64)));
    //pGraphics->AttachControl(new ISVGControl(DYND_Bounds.GetHPadded(-25).GetVPadded(-15), DYNDisplayFace));
    //auto meterNeedle  = new ISVGKnobControl(DYND_Bounds.GetVShifted(55).GetCentredInside(280), DYNDisplayNeedle, kDynMeter);
    //auto meterReadout = new ICaptionControl(DYND_Bounds.GetCentredInside(50, 20).GetTranslated(0, -20), kDynMeter, ChStTxt_Black, 0, true);
    //meterNeedle->SetIgnoreMouse(true);
    //meterReadout->SetIgnoreMouse(true);
    //pGraphics->AttachControl(meterNeedle);
    //pGraphics->AttachControl(meterReadout);
  };
#endif
}

#if IPLUG_DSP
void ChannelStrip::ProcessBlock(sample** inputs, sample** outputs, int nFrames)
{
  const double gainIn = GetParam(kGainIn)->Value();
  const double gainOut = GetParam(kGainOut)->Value();

  const int nChans = NOutChansConnected();

  std::vector<double> signalLevelsIn(nChans, 0.0);
  std::vector<double> signalLevelsOut(nChans, 0.0);

  std::vector<std::vector<sample>> inSignalBuffer(nChans, std::vector<sample>(nFrames, 0.0));

  for (int s = 0; s < nFrames; s++) {
    for (int c = 0; c < nChans; c++) {
      double inSample = inputs[c][s];

      // Apply input gain
      double gainedInput = inSample * pow(10.0, gainIn / 20.00);
      outputs[c][s] = inSignalBuffer[c][s] = gainedInput; // apply the gain to the output signal, and store the signal level to the buffer
      signalLevelsIn[c] += gainedInput * gainedInput;

      // TODO: Apply EQ


      // TODO: Apply DYN

      // Apply output gain
      outputs[c][s] *= pow(10.0, gainOut / 20.00);
      signalLevelsOut[c] += outputs[c][s] * outputs[c][s];
    }
  }

  std::vector<sample*> inPostSignal(nChans);
  for (int c = 0; c < nChans; c++) {
    inPostSignal[c] = inSignalBuffer[c].data();
  }
  mPeakAvgInMeterSender.ProcessBlock(inPostSignal.data(), nFrames, kCtrlTagInMeter);
  mPeakAvgOutMeterSender.ProcessBlock(outputs, nFrames, kCtrlTagOutMeter);
  //mPeakAvgDynMeterSender.ProcessBlock(, nFrames, kCtrlTagDynMeter);
}

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
#endif

void ChannelStrip::AttachMeterControls(
  IGraphics* pGraphics, IRECT gainBounds, IRECT meterBounds,
  int gainIndex, int meterCtrlTag,  IVStyle knobStyle, IVStyle meterStyle,
  const char* label)
{
  auto zone1 = IColor(255, 16, 40, 16);
  auto zone2 = IColor(255, 40, 40, 16);
  auto zone3 = IColor(255, 40, 16, 16);

  pGraphics->AttachControl(new IPanelControl(meterBounds.GetFromBottom(271), zone1));
  pGraphics->AttachControl(new IPanelControl(meterBounds.GetReducedFromBottom(271).GetReducedFromTop(74), zone2));
  pGraphics->AttachControl(new IPanelControl(meterBounds.GetFromTop(74).GetReducedFromTop(16), zone3));

  pGraphics->AttachControl(new IVKnobControl(gainBounds, gainIndex, "", knobStyle, true, false, -135.0, 27.0, 0.0));
  pGraphics->AttachControl(new IVPeakAvgMeterControl<2>(meterBounds, label, meterStyle, EDirection::Vertical, {"", ""}, 0, -65.F, 10.F, {0, -18, -42, -60}), meterCtrlTag);
}

void ChannelStrip::AttachBandControls(
  IGraphics* pGraphics, ISVG knobFront, ISVG knobBack, IVStyle switchStyle, IRECT bandRect, 
  int gainParamIndex, int freqParamIndex, int qParamIndex,
  int modeParamIndex, const char* altLabel)
{
  IRECT gainBounds = bandRect.SubRectVertical(2, 0).GetCentredInside(110);
  IRECT gainValue = gainBounds.GetCentredInside(50, 20).GetTranslated(0, 65);
  pGraphics->AttachControl(new ISVGControl(gainBounds.GetPadded(2.5), knobBack));
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
  IGraphics* pGraphics, ISVG knobFront, ISVG knobBack, IVStyle buttonStyle, IVStyle ratioIndStyle,
  IRECT threshBounds, IRECT modeBounds, IRECT attackBounds, IRECT releaseBounds, IRECT ratioSelBounds, IRECT ratioIndBounds,
  int threshIndex, int modeIndex, int attackIndex, int releaseIndex, int ratioIndex,
  const char* normLabel, const char* altLabel)
{
  IRECT threshLabel = threshBounds.GetCentredInside(50, 20).GetTranslated(0, -65);
  IRECT threshValue = threshBounds.GetCentredInside(50, 20).GetTranslated(0, 65);
  pGraphics->AttachControl(new ITextControl(threshLabel, "THRESHOLD", ChStTxt_Black, 0, true));
  pGraphics->AttachControl(new ISVGControl(threshBounds.GetPadded(2), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(threshBounds, knobFront, threshIndex));
  pGraphics->AttachControl(new ICaptionControl(threshValue, threshIndex, ChStTxt_Black, 0, true));

  IRECT attackLabel = attackBounds.GetCentredInside(60, 20).GetTranslated(0, -35);
  IRECT attackValue = attackBounds.GetCentredInside(60, 20).GetTranslated(0, 35);
  pGraphics->AttachControl(new ITextControl(attackLabel, "ATK", ChStTxt_Black, 0, true));
  pGraphics->AttachControl(new ISVGControl(attackBounds.GetPadded(1.5), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(attackBounds, knobFront, attackIndex));
  pGraphics->AttachControl(new ICaptionControl(attackValue, attackIndex, ChStTxt_Black, 0, true));

  IRECT releaseLabel = releaseBounds.GetCentredInside(60, 20).GetTranslated(0, -35);
  IRECT releaseValue = releaseBounds.GetCentredInside(60, 20).GetTranslated(0, 35);
  pGraphics->AttachControl(new ITextControl(releaseLabel, "RLS", ChStTxt_Black, 0, true));
  pGraphics->AttachControl(new ISVGControl(releaseBounds.GetPadded(1.5), knobBack));
  pGraphics->AttachControl(new ISVGKnobControl(releaseBounds, knobFront, releaseIndex));
  pGraphics->AttachControl(new ICaptionControl(releaseValue, releaseIndex, ChStTxt_Black, 0, true));

  pGraphics->AttachControl(new IVTabSwitchControl(modeBounds, modeIndex, {normLabel, altLabel}, "", buttonStyle));

  pGraphics->AttachControl(new IVSwitchControl(ratioSelBounds, ratioIndex, "", buttonStyle, true));
  pGraphics->AttachControl(new IVRadioButtonControl(ratioIndBounds, ratioIndex, {"1:1", "2:1", "3:1", "4:1", "8:1", "10:1", "20:1", "INF"}, "", ratioIndStyle));
  auto buttonLabel = new ITextControl(ratioSelBounds, "RATIO", ChStTxt_White, 0, true);
  buttonLabel->SetIgnoreMouse(true);
  pGraphics->AttachControl(buttonLabel);
}