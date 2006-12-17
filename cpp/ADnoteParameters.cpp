/*
  ZynAddSubFX - a software synthesizer
 
  ADnoteParameters.C - Parameters for ADnote (ADsynth)
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "ADnoteParameters.h"

ADnoteParameters::ADnoteParameters(FFTwrapper *fft_):Presets(){
  setpresettype("Padsyth");
  fft=fft_;

  GlobalPar.FreqEnvelope=new EnvelopeParams(0,0);
  GlobalPar.FreqEnvelope->ASRinit(64,50,64,60);
  GlobalPar.FreqLfo=new LFOParams(70,0,64,0,0,0,0,0);
    
  GlobalPar.AmpEnvelope=new EnvelopeParams(64,1);
  GlobalPar.AmpEnvelope->ADSRinit_dB(0,40,127,25);
  GlobalPar.AmpLfo=new LFOParams(80,0,64,0,0,0,0,1);

  GlobalPar.GlobalFilter=new FilterParams(2,94,40);
  GlobalPar.FilterEnvelope=new EnvelopeParams(0,1);
  GlobalPar.FilterEnvelope->ADSRinit_filter(64,40,64,70,60,64);
  GlobalPar.FilterLfo=new LFOParams(80,0,64,0,0,0,0,2);
  GlobalPar.Reson=new Resonance();

  for (int nvoice=0;nvoice<NUM_VOICES;nvoice++) EnableVoice(nvoice);
    
  defaults();
};

void ADnoteParameters::defaults(){
  //Default Parameters
  /* Frequency Global Parameters */
  GlobalPar.stereo = TRUE;      // Stereo
  GlobalPar.PDetune=8192;//zero
  GlobalPar.PCoarseDetune=0;
  GlobalPar.PDetuneType=1;
  GlobalPar.FreqEnvelope->defaults();
  GlobalPar.FreqLfo->defaults();
  GlobalPar.PBandwidth=64;
    
  /* Amplitude Global Parameters */
  GlobalPar.PVolume=90;
  GlobalPar.PAmpVelocityScaleFunction=64;
  GlobalPar.AmpEnvelope->defaults();
  GlobalPar.AmpLfo->defaults();
  GlobalPar.PPunchStrength=0;
  GlobalPar.PPunchTime=60;
  GlobalPar.PPunchStretch=64;
  GlobalPar.PPunchVelocitySensing=72;
    
  /* Filter Global Parameters*/
  GlobalPar.PFilterVelocityScale=64;
  GlobalPar.PFilterVelocityScaleFunction=64;
  GlobalPar.GlobalFilter->defaults();
  GlobalPar.FilterEnvelope->defaults();
  GlobalPar.FilterLfo->defaults();
  GlobalPar.Reson->defaults();


  for (int nvoice=0;nvoice<NUM_VOICES;nvoice++){
    defaults(nvoice);
  };
  VoicePar[0].Enabled=1;
};

/*
 * Defaults a voice
 */
void ADnoteParameters::defaults(int n){
  int nvoice=n;
  VoicePar[nvoice].Enabled=0;
  VoicePar[nvoice].Type=0;
  VoicePar[nvoice].Pfixedfreq=0;
  VoicePar[nvoice].PfixedfreqET=0;
  VoicePar[nvoice].Presonance=1;
  VoicePar[nvoice].Pfilterbypass=0;
  VoicePar[nvoice].Pextoscil=-1;
  VoicePar[nvoice].PextFMoscil=-1;
  VoicePar[nvoice].Poscilphase=64;
  VoicePar[nvoice].PFMoscilphase=64;
  VoicePar[nvoice].PDelay=0;
  VoicePar[nvoice].PVolume=100;
  VoicePar[nvoice].PVolumeminus=0;
  VoicePar[nvoice].PPanning=64;//center
  VoicePar[nvoice].PDetune=8192;//8192=0
  VoicePar[nvoice].PCoarseDetune=0;
  VoicePar[nvoice].PDetuneType=0;
  VoicePar[nvoice].PFreqLfoEnabled=0;
  VoicePar[nvoice].PFreqEnvelopeEnabled=0;
  VoicePar[nvoice].PAmpEnvelopeEnabled=0;
  VoicePar[nvoice].PAmpLfoEnabled=0;
  VoicePar[nvoice].PAmpVelocityScaleFunction=127;
  VoicePar[nvoice].PFilterEnabled=0;
  VoicePar[nvoice].PFilterEnvelopeEnabled=0;
  VoicePar[nvoice].PFilterLfoEnabled=0;
  VoicePar[nvoice].PFMEnabled=0;

  //I use the internal oscillator (-1)
  VoicePar[nvoice].PFMVoice=-1;

  VoicePar[nvoice].PFMVolume=90;
  VoicePar[nvoice].PFMVolumeDamp=64;
  VoicePar[nvoice].PFMDetune=8192;
  VoicePar[nvoice].PFMCoarseDetune=0;
  VoicePar[nvoice].PFMDetuneType=0;
  VoicePar[nvoice].PFMFreqEnvelopeEnabled=0;
  VoicePar[nvoice].PFMAmpEnvelopeEnabled=0;
  VoicePar[nvoice].PFMVelocityScaleFunction=64; 

  VoicePar[nvoice].OscilSmp->defaults();
  VoicePar[nvoice].FMSmp->defaults();

  VoicePar[nvoice].AmpEnvelope->defaults();
  VoicePar[nvoice].AmpLfo->defaults();

  VoicePar[nvoice].FreqEnvelope->defaults();
  VoicePar[nvoice].FreqLfo->defaults();

  VoicePar[nvoice].VoiceFilter->defaults();
  VoicePar[nvoice].FilterEnvelope->defaults();
  VoicePar[nvoice].FilterLfo->defaults();

  VoicePar[nvoice].FMFreqEnvelope->defaults();
  VoicePar[nvoice].FMAmpEnvelope->defaults();
};



/*
 * Init the voice parameters
 */
void ADnoteParameters::EnableVoice(int nvoice){
  VoicePar[nvoice].OscilSmp=new OscilGen(fft,GlobalPar.Reson);
  VoicePar[nvoice].FMSmp=new OscilGen(fft,NULL);

  VoicePar[nvoice].AmpEnvelope=new EnvelopeParams(64,1);
  VoicePar[nvoice].AmpEnvelope->ADSRinit_dB(0,100,127,100); 
  VoicePar[nvoice].AmpLfo=new LFOParams(90,32,64,0,0,30,0,1);

  VoicePar[nvoice].FreqEnvelope=new EnvelopeParams(0,0);
  VoicePar[nvoice].FreqEnvelope->ASRinit(30,40,64,60);
  VoicePar[nvoice].FreqLfo=new LFOParams(50,40,0,0,0,0,0,0);

  VoicePar[nvoice].VoiceFilter=new FilterParams(2,50,60);
  VoicePar[nvoice].FilterEnvelope=new EnvelopeParams(0,0);
  VoicePar[nvoice].FilterEnvelope->ADSRinit_filter(90,70,40,70,10,40);
  VoicePar[nvoice].FilterLfo=new LFOParams(50,20,64,0,0,0,0,2);

  VoicePar[nvoice].FMFreqEnvelope=new EnvelopeParams(0,0);
  VoicePar[nvoice].FMFreqEnvelope->ASRinit(20,90,40,80);
  VoicePar[nvoice].FMAmpEnvelope=new EnvelopeParams(64,1);
  VoicePar[nvoice].FMAmpEnvelope->ADSRinit(80,90,127,100);
};

/*
 * Get the Multiplier of the fine detunes of the voices
 */
REALTYPE ADnoteParameters::getBandwidthDetuneMultiplier(){
  REALTYPE bw=(GlobalPar.PBandwidth-64.0)/64.0;
  bw=pow(2.0,bw*pow(fabs(bw),0.2)*5.0);
    
  return(bw);
};


/*
 * Kill the voice
 */
void ADnoteParameters::KillVoice(int nvoice){
  delete (VoicePar[nvoice].OscilSmp);
  delete (VoicePar[nvoice].FMSmp);

  delete (VoicePar[nvoice].AmpEnvelope);
  delete (VoicePar[nvoice].AmpLfo);

  delete (VoicePar[nvoice].FreqEnvelope);
  delete (VoicePar[nvoice].FreqLfo);

  delete (VoicePar[nvoice].VoiceFilter);
  delete (VoicePar[nvoice].FilterEnvelope);
  delete (VoicePar[nvoice].FilterLfo);

  delete (VoicePar[nvoice].FMFreqEnvelope);
  delete (VoicePar[nvoice].FMAmpEnvelope);
};

ADnoteParameters::~ADnoteParameters(){
  delete(GlobalPar.FreqEnvelope);
  delete(GlobalPar.FreqLfo);
  delete(GlobalPar.AmpEnvelope);
  delete(GlobalPar.AmpLfo);
  delete(GlobalPar.GlobalFilter);
  delete(GlobalPar.FilterEnvelope);
  delete(GlobalPar.FilterLfo);
  delete(GlobalPar.Reson);

  for (int nvoice=0;nvoice<NUM_VOICES;nvoice++){
    KillVoice(nvoice);
  };
};
