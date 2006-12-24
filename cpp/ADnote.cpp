/*
  ZynAddSubFX - a software synthesizer
 
  ADnote.C - The "additive" synthesizer
  Copyright (C) 2006 Nedko Arnaudov <nedko@arnaudov.name>
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

#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "globals.h"
#include "Resonance.h"
#include "FFTwrapper.h"
#include "OscilGen.h"
#include "Resonance.h"
#include "ADnote.h"

ADnote::ADnote(
  ADnoteParameters * partparams,
  Controller * ctl)
{
  m_tmpwave = new REALTYPE [SOUND_BUFFER_SIZE];
  m_bypassl = new REALTYPE [SOUND_BUFFER_SIZE];
  m_bypassr = new REALTYPE [SOUND_BUFFER_SIZE];

  m_partparams = partparams;
  m_ctl = ctl;

  m_stereo = partparams->GlobalPar.stereo;

  m_note_global_parameters.Detune = getdetune(
    partparams->GlobalPar.PDetuneType,
    partparams->GlobalPar.PCoarseDetune,
    partparams->GlobalPar.PDetune);

  m_bandwidth_detune_multiplier = partparams->getBandwidthDetuneMultiplier();

  m_note_enabled = FALSE;
}

void
ADnote::note_on(
  float panorama,
  BOOL random_grouping,
  REALTYPE freq,
  REALTYPE velocity,
  BOOL portamento,
  int midinote)
{
  int voice_index;

  m_portamento = portamento;
  m_midinote = midinote;
  m_note_enabled = TRUE;
  m_basefreq = freq;

  if (velocity > 1.0)
  {
    m_velocity = 1.0;
  }
  else
  {
    m_velocity = velocity;
  }

  m_time = 0.0;

  m_note_global_parameters.Panning = (panorama + 1.0) / 2; // -1..1 -> 0 - 1

  m_note_global_parameters.FilterCenterPitch =
    m_partparams->GlobalPar.GlobalFilter->getfreq() + //center freq
    m_partparams->GlobalPar.PFilterVelocityScale / 127.0 * 6.0 * // velocity sensing
    (VelF(m_velocity,m_partparams->GlobalPar.PFilterVelocityScaleFunction) - 1);

  if (m_partparams->GlobalPar.PPunchStrength != 0)
  {
    m_note_global_parameters.punch.enabled = TRUE;
    m_note_global_parameters.punch.t = 1.0; //start from 1.0 and to 0.0
    m_note_global_parameters.punch.initialvalue =
      ( (pow(10,1.5*m_partparams->GlobalPar.PPunchStrength/127.0)-1.0)
        *VelF(m_velocity,m_partparams->GlobalPar.PPunchVelocitySensing) );
    REALTYPE time = pow(10,3.0*m_partparams->GlobalPar.PPunchTime/127.0)/10000.0; //0.1 .. 100 ms
    REALTYPE stretch = pow(440.0/freq,m_partparams->GlobalPar.PPunchStretch/64.0);
    m_note_global_parameters.punch.dt=1.0/(time*SAMPLE_RATE*stretch);
  }
  else
  {
    m_note_global_parameters.punch.enabled = FALSE;
  }

  for (voice_index=0;voice_index<NUM_VOICES;voice_index++)
  {
    m_partparams->VoicePar[voice_index].OscilSmp->newrandseed(rand());
    NoteVoicePar[voice_index].OscilSmp=NULL;
    NoteVoicePar[voice_index].FMSmp=NULL;
    NoteVoicePar[voice_index].VoiceOut=NULL;

    NoteVoicePar[voice_index].FMVoice=-1;

    if (m_partparams->VoicePar[voice_index].Enabled==0) {
      NoteVoicePar[voice_index].enabled = FALSE;
      continue; //the voice is disabled
    }

    NoteVoicePar[voice_index].enabled = TRUE;
    NoteVoicePar[voice_index].fixedfreq=m_partparams->VoicePar[voice_index].Pfixedfreq;
    NoteVoicePar[voice_index].fixedfreqET=m_partparams->VoicePar[voice_index].PfixedfreqET;

    //use the Globalpars.detunetype if the detunetype is 0
    if (m_partparams->VoicePar[voice_index].PDetuneType!=0){
      NoteVoicePar[voice_index].Detune=getdetune(m_partparams->VoicePar[voice_index].PDetuneType
                                            ,m_partparams->VoicePar[voice_index].PCoarseDetune,8192);//coarse detune
      NoteVoicePar[voice_index].FineDetune=getdetune(m_partparams->VoicePar[voice_index].PDetuneType
                                                ,0,m_partparams->VoicePar[voice_index].PDetune);//fine detune
    } else {
      NoteVoicePar[voice_index].Detune=getdetune(m_partparams->GlobalPar.PDetuneType
                                            ,m_partparams->VoicePar[voice_index].PCoarseDetune,8192);//coarse detune
      NoteVoicePar[voice_index].FineDetune=getdetune(m_partparams->GlobalPar.PDetuneType
                                                ,0,m_partparams->VoicePar[voice_index].PDetune);//fine detune
    }
    if (m_partparams->VoicePar[voice_index].PFMDetuneType!=0){
      NoteVoicePar[voice_index].FMDetune=getdetune(m_partparams->VoicePar[voice_index].PFMDetuneType
                                              ,m_partparams->VoicePar[voice_index].PFMCoarseDetune,m_partparams->VoicePar[voice_index].PFMDetune);
    } else {
      NoteVoicePar[voice_index].FMDetune=getdetune(m_partparams->GlobalPar.PDetuneType
                                              ,m_partparams->VoicePar[voice_index].PFMCoarseDetune,m_partparams->VoicePar[voice_index].PFMDetune);
    }

    oscposhi[voice_index]=0;oscposlo[voice_index]=0.0;
    oscposhiFM[voice_index]=0;oscposloFM[voice_index]=0.0;

    NoteVoicePar[voice_index].OscilSmp=new REALTYPE[OSCIL_SIZE+OSCIL_SMP_EXTRA_SAMPLES];//the extra points contains the first point

    //Get the voice's oscil or external's voice oscil
    int vc=voice_index;

    if (m_partparams->VoicePar[voice_index].Pextoscil != -1)
    {
      vc = m_partparams->VoicePar[voice_index].Pextoscil;
    }

    if (!random_grouping)
    {
      m_partparams->VoicePar[vc].OscilSmp->newrandseed(rand());
    }

    oscposhi[voice_index] = m_partparams->VoicePar[vc].OscilSmp->get(
      NoteVoicePar[voice_index].OscilSmp,
      getvoicebasefreq(voice_index),
      m_partparams->VoicePar[voice_index].Presonance);

    //I store the first elments to the last position for speedups
    for (int i=0;i<OSCIL_SMP_EXTRA_SAMPLES;i++) NoteVoicePar[voice_index].OscilSmp[OSCIL_SIZE+i]=NoteVoicePar[voice_index].OscilSmp[i];

    oscposhi[voice_index]+=(int)((m_partparams->VoicePar[voice_index].Poscilphase-64.0)/128.0*OSCIL_SIZE+OSCIL_SIZE*4);
    oscposhi[voice_index]%=OSCIL_SIZE;


    NoteVoicePar[voice_index].FreqLfo=NULL;
    NoteVoicePar[voice_index].FreqEnvelope=NULL;

    NoteVoicePar[voice_index].AmpLfo=NULL;
    NoteVoicePar[voice_index].AmpEnvelope=NULL;

    NoteVoicePar[voice_index].VoiceFilter=NULL;
    NoteVoicePar[voice_index].FilterEnvelope=NULL;
    NoteVoicePar[voice_index].FilterLfo=NULL;

    NoteVoicePar[voice_index].FilterCenterPitch=m_partparams->VoicePar[voice_index].VoiceFilter->getfreq();
    NoteVoicePar[voice_index].filterbypass=m_partparams->VoicePar[voice_index].Pfilterbypass;

    switch(m_partparams->VoicePar[voice_index].PFMEnabled){
    case 1:NoteVoicePar[voice_index].FMEnabled=MORPH;break;
    case 2:NoteVoicePar[voice_index].FMEnabled=RING_MOD;break;
    case 3:NoteVoicePar[voice_index].FMEnabled=PHASE_MOD;break;
    case 4:NoteVoicePar[voice_index].FMEnabled=FREQ_MOD;break;
    case 5:NoteVoicePar[voice_index].FMEnabled=PITCH_MOD;break;
    default:NoteVoicePar[voice_index].FMEnabled=NONE;
    }

    NoteVoicePar[voice_index].FMVoice=m_partparams->VoicePar[voice_index].PFMVoice;
    NoteVoicePar[voice_index].FMFreqEnvelope=NULL;
    NoteVoicePar[voice_index].FMAmpEnvelope=NULL;

    //Compute the Voice's modulator volume (incl. damping)
    REALTYPE fmvoldamp=pow(440.0/getvoicebasefreq(voice_index),m_partparams->VoicePar[voice_index].PFMVolumeDamp/64.0-1.0);
    switch (NoteVoicePar[voice_index].FMEnabled){
    case PHASE_MOD:fmvoldamp=pow(440.0/getvoicebasefreq(voice_index),m_partparams->VoicePar[voice_index].PFMVolumeDamp/64.0);
      NoteVoicePar[voice_index].FMVolume=(exp(m_partparams->VoicePar[voice_index].PFMVolume/127.0*FM_AMP_MULTIPLIER)-1.0)*fmvoldamp*4.0;
      break;
    case FREQ_MOD:NoteVoicePar[voice_index].FMVolume=(exp(m_partparams->VoicePar[voice_index].PFMVolume/127.0*FM_AMP_MULTIPLIER)-1.0)*fmvoldamp*4.0;
      break;
      //    case PITCH_MOD:NoteVoicePar[voice_index].FMVolume=(m_partparams->VoicePar[voice_index].PFMVolume/127.0*8.0)*fmvoldamp;//???????????
      //            break;
    default:if (fmvoldamp>1.0) fmvoldamp=1.0;
      NoteVoicePar[voice_index].FMVolume=m_partparams->VoicePar[voice_index].PFMVolume/127.0*fmvoldamp;
    }

    //Voice's modulator velocity sensing
    NoteVoicePar[voice_index].FMVolume*=VelF(m_velocity,m_partparams->VoicePar[voice_index].PFMVelocityScaleFunction);

    FMoldsmp[voice_index]=0.0;//this is for FM (integration)

    firsttick[voice_index]=1;
    NoteVoicePar[voice_index].DelayTicks=(int)((exp(m_partparams->VoicePar[voice_index].PDelay/127.0*log(50.0))-1.0)/SOUND_BUFFER_SIZE/10.0*SAMPLE_RATE);
  }

  initparameters(random_grouping);
}

/*
 * Kill a voice of ADnote
 */
void ADnote::KillVoice(int nvoice){

  delete [] (NoteVoicePar[nvoice].OscilSmp);

  if (NoteVoicePar[nvoice].FreqEnvelope!=NULL) delete(NoteVoicePar[nvoice].FreqEnvelope);
  NoteVoicePar[nvoice].FreqEnvelope=NULL;

  if (NoteVoicePar[nvoice].FreqLfo!=NULL) delete(NoteVoicePar[nvoice].FreqLfo);
  NoteVoicePar[nvoice].FreqLfo=NULL;

  if (NoteVoicePar[nvoice].AmpEnvelope!=NULL) delete (NoteVoicePar[nvoice].AmpEnvelope);
  NoteVoicePar[nvoice].AmpEnvelope=NULL;

  if (NoteVoicePar[nvoice].AmpLfo!=NULL) delete (NoteVoicePar[nvoice].AmpLfo);
  NoteVoicePar[nvoice].AmpLfo=NULL;

  if (NoteVoicePar[nvoice].VoiceFilter!=NULL) delete (NoteVoicePar[nvoice].VoiceFilter);
  NoteVoicePar[nvoice].VoiceFilter=NULL;

  if (NoteVoicePar[nvoice].FilterEnvelope!=NULL) delete (NoteVoicePar[nvoice].FilterEnvelope);
  NoteVoicePar[nvoice].FilterEnvelope=NULL;

  if (NoteVoicePar[nvoice].FilterLfo!=NULL) delete (NoteVoicePar[nvoice].FilterLfo);
  NoteVoicePar[nvoice].FilterLfo=NULL;

  if (NoteVoicePar[nvoice].FMFreqEnvelope!=NULL) delete (NoteVoicePar[nvoice].FMFreqEnvelope);
  NoteVoicePar[nvoice].FMFreqEnvelope=NULL;

  if (NoteVoicePar[nvoice].FMAmpEnvelope!=NULL) delete (NoteVoicePar[nvoice].FMAmpEnvelope);
  NoteVoicePar[nvoice].FMAmpEnvelope=NULL;

  if ((NoteVoicePar[nvoice].FMEnabled!=NONE)&&(NoteVoicePar[nvoice].FMVoice<0)) delete NoteVoicePar[nvoice].FMSmp;

  if (NoteVoicePar[nvoice].VoiceOut!=NULL)
    for (int i=0;i<SOUND_BUFFER_SIZE;i++) NoteVoicePar[nvoice].VoiceOut[i]=0.0;//do not delete, yet: perhaps is used by another voice

  NoteVoicePar[nvoice].enabled = FALSE;
}

/*
 * Kill the note
 */
void ADnote::KillNote(){
  int nvoice;
  for (nvoice=0;nvoice<NUM_VOICES;nvoice++){
    if (NoteVoicePar[nvoice].enabled) KillVoice(nvoice);

    //delete VoiceOut
    if (NoteVoicePar[nvoice].VoiceOut!=NULL) delete(NoteVoicePar[nvoice].VoiceOut);
    NoteVoicePar[nvoice].VoiceOut=NULL;
  }

  delete (m_note_global_parameters.FreqEnvelope);
  delete (m_note_global_parameters.FreqLfo);
  delete (m_note_global_parameters.AmpEnvelope);
  delete (m_note_global_parameters.AmpLfo);
  delete (m_note_global_parameters.GlobalFilterL);
  if (m_stereo) delete (m_note_global_parameters.GlobalFilterR);
  delete (m_note_global_parameters.FilterEnvelope);
  delete (m_note_global_parameters.FilterLfo);

  m_note_enabled = FALSE;
}

ADnote::~ADnote(){
  if (m_note_enabled) KillNote();
  delete [] m_tmpwave;
  delete [] m_bypassl;
  delete [] m_bypassr;
}

/*
 * Init the parameters
 */
void
ADnote::initparameters(
  BOOL random_grouping)
{
  int nvoice,i,tmp[NUM_VOICES];

  // Global Parameters
  m_note_global_parameters.FreqEnvelope=new Envelope(m_partparams->GlobalPar.FreqEnvelope,m_basefreq);
  m_note_global_parameters.FreqLfo=new LFO(m_partparams->GlobalPar.FreqLfo,m_basefreq);

  m_note_global_parameters.AmpEnvelope=new Envelope(m_partparams->GlobalPar.AmpEnvelope,m_basefreq);
  m_note_global_parameters.AmpLfo=new LFO(m_partparams->GlobalPar.AmpLfo,m_basefreq);

  m_note_global_parameters.Volume=4.0*pow(0.1,3.0*(1.0-m_partparams->GlobalPar.PVolume/96.0))//-60 dB .. 0 dB
    *VelF(m_velocity,m_partparams->GlobalPar.PAmpVelocityScaleFunction);//velocity sensing

  m_note_global_parameters.AmpEnvelope->envout_dB();//discard the first envelope output
  globalnewamplitude=m_note_global_parameters.Volume*m_note_global_parameters.AmpEnvelope->envout_dB()*m_note_global_parameters.AmpLfo->amplfoout();

  m_note_global_parameters.GlobalFilterL=new Filter(m_partparams->GlobalPar.GlobalFilter);
  if (m_stereo) m_note_global_parameters.GlobalFilterR=new Filter(m_partparams->GlobalPar.GlobalFilter);

  m_note_global_parameters.FilterEnvelope=new Envelope(m_partparams->GlobalPar.FilterEnvelope,m_basefreq);
  m_note_global_parameters.FilterLfo=new LFO(m_partparams->GlobalPar.FilterLfo,m_basefreq);
  m_note_global_parameters.FilterQ=m_partparams->GlobalPar.GlobalFilter->getq();
  m_note_global_parameters.FilterFreqTracking=m_partparams->GlobalPar.GlobalFilter->getfreqtracking(m_basefreq);

  // Forbids the Modulation Voice to be greater or equal than voice
  for (i=0;i<NUM_VOICES;i++) if (NoteVoicePar[i].FMVoice>=i) NoteVoicePar[i].FMVoice=-1;

  // Voice Parameter init
  for (nvoice=0;nvoice<NUM_VOICES;nvoice++){
    if (!NoteVoicePar[nvoice].enabled) continue;

    NoteVoicePar[nvoice].noisetype=m_partparams->VoicePar[nvoice].Type;
    /* Voice Amplitude Parameters Init */
    NoteVoicePar[nvoice].Volume=pow(0.1,3.0*(1.0-m_partparams->VoicePar[nvoice].PVolume/127.0)) // -60 dB .. 0 dB
      *VelF(m_velocity,m_partparams->VoicePar[nvoice].PAmpVelocityScaleFunction);//velocity

    if (m_partparams->VoicePar[nvoice].PVolumeminus!=0) NoteVoicePar[nvoice].Volume=-NoteVoicePar[nvoice].Volume;

    if (m_partparams->VoicePar[nvoice].PPanning==0)
    {
      NoteVoicePar[nvoice].Panning = zyn_random(); // random panning
    }
    else
    {
      NoteVoicePar[nvoice].Panning = m_partparams->VoicePar[nvoice].PPanning/128.0;
    }

    newamplitude[nvoice]=1.0;
    if (m_partparams->VoicePar[nvoice].PAmpEnvelopeEnabled!=0) {
      NoteVoicePar[nvoice].AmpEnvelope=new Envelope(m_partparams->VoicePar[nvoice].AmpEnvelope,m_basefreq);
      NoteVoicePar[nvoice].AmpEnvelope->envout_dB();//discard the first envelope sample
      newamplitude[nvoice]*=NoteVoicePar[nvoice].AmpEnvelope->envout_dB();
    }

    if (m_partparams->VoicePar[nvoice].PAmpLfoEnabled!=0){
      NoteVoicePar[nvoice].AmpLfo=new LFO(m_partparams->VoicePar[nvoice].AmpLfo,m_basefreq);
      newamplitude[nvoice]*=NoteVoicePar[nvoice].AmpLfo->amplfoout();
    }

    /* Voice Frequency Parameters Init */
    if (m_partparams->VoicePar[nvoice].PFreqEnvelopeEnabled!=0)
      NoteVoicePar[nvoice].FreqEnvelope=new Envelope(m_partparams->VoicePar[nvoice].FreqEnvelope,m_basefreq);

    if (m_partparams->VoicePar[nvoice].PFreqLfoEnabled!=0) NoteVoicePar[nvoice].FreqLfo=new LFO(m_partparams->VoicePar[nvoice].FreqLfo,m_basefreq);

    /* Voice Filter Parameters Init */
    if (m_partparams->VoicePar[nvoice].PFilterEnabled!=0){
      NoteVoicePar[nvoice].VoiceFilter=new Filter(m_partparams->VoicePar[nvoice].VoiceFilter);
    }

    if (m_partparams->VoicePar[nvoice].PFilterEnvelopeEnabled!=0)
      NoteVoicePar[nvoice].FilterEnvelope=new Envelope(m_partparams->VoicePar[nvoice].FilterEnvelope,m_basefreq);

    if (m_partparams->VoicePar[nvoice].PFilterLfoEnabled!=0)
      NoteVoicePar[nvoice].FilterLfo=new LFO(m_partparams->VoicePar[nvoice].FilterLfo,m_basefreq);

    NoteVoicePar[nvoice].FilterFreqTracking=m_partparams->VoicePar[nvoice].VoiceFilter->getfreqtracking(m_basefreq);

    /* Voice Modulation Parameters Init */
    if ((NoteVoicePar[nvoice].FMEnabled!=NONE)&&(NoteVoicePar[nvoice].FMVoice<0)){
      m_partparams->VoicePar[nvoice].FMSmp->newrandseed(rand());
      NoteVoicePar[nvoice].FMSmp=new REALTYPE[OSCIL_SIZE+OSCIL_SMP_EXTRA_SAMPLES];

      //Perform Anti-aliasing only on MORPH or RING MODULATION

      int vc=nvoice;
      if (m_partparams->VoicePar[nvoice].PextFMoscil!=-1) vc=m_partparams->VoicePar[nvoice].PextFMoscil;

      REALTYPE tmp=1.0;
      if ((m_partparams->VoicePar[vc].FMSmp->Padaptiveharmonics!=0)||
          (NoteVoicePar[nvoice].FMEnabled==MORPH)||
          (NoteVoicePar[nvoice].FMEnabled==RING_MOD)){
        tmp=getFMvoicebasefreq(nvoice);
      }

      if (!random_grouping)
      {
        m_partparams->VoicePar[vc].FMSmp->newrandseed(rand());
      }

      oscposhiFM[nvoice]=(oscposhi[nvoice]+m_partparams->VoicePar[vc].FMSmp->get(NoteVoicePar[nvoice].FMSmp,tmp)) % OSCIL_SIZE;
      for (int i=0;i<OSCIL_SMP_EXTRA_SAMPLES;i++) NoteVoicePar[nvoice].FMSmp[OSCIL_SIZE+i]=NoteVoicePar[nvoice].FMSmp[i];
      oscposhiFM[nvoice]+=(int)((m_partparams->VoicePar[nvoice].PFMoscilphase-64.0)/128.0*OSCIL_SIZE+OSCIL_SIZE*4);
      oscposhiFM[nvoice]%=OSCIL_SIZE;
    }

    if (m_partparams->VoicePar[nvoice].PFMFreqEnvelopeEnabled!=0)
      NoteVoicePar[nvoice].FMFreqEnvelope=new Envelope(m_partparams->VoicePar[nvoice].FMFreqEnvelope,m_basefreq);

    FMnewamplitude[nvoice]=NoteVoicePar[nvoice].FMVolume*m_ctl->fmamp.relamp;

    if (m_partparams->VoicePar[nvoice].PFMAmpEnvelopeEnabled!=0){
      NoteVoicePar[nvoice].FMAmpEnvelope=new Envelope(m_partparams->VoicePar[nvoice].FMAmpEnvelope,m_basefreq);
      FMnewamplitude[nvoice]*=NoteVoicePar[nvoice].FMAmpEnvelope->envout_dB();
    }
  }

  for (nvoice=0;nvoice<NUM_VOICES;nvoice++){
    for (i=nvoice+1;i<NUM_VOICES;i++) tmp[i]=0;
    for (i=nvoice+1;i<NUM_VOICES;i++)
      if ((NoteVoicePar[i].FMVoice==nvoice)&&(tmp[i]==0)){
        NoteVoicePar[nvoice].VoiceOut=new REALTYPE[SOUND_BUFFER_SIZE];
        tmp[i]=1;
      }
    if (NoteVoicePar[nvoice].VoiceOut!=NULL) for (i=0;i<SOUND_BUFFER_SIZE;i++) NoteVoicePar[nvoice].VoiceOut[i]=0.0;
  }
}



/*
 * Computes the frequency of an oscillator
 */
void ADnote::setfreq(int nvoice,REALTYPE freq){
  REALTYPE speed;
  freq=fabs(freq);
  speed=freq*REALTYPE(OSCIL_SIZE)/(REALTYPE) SAMPLE_RATE;
  if (speed>OSCIL_SIZE) speed=OSCIL_SIZE;

  F2I(speed,oscfreqhi[nvoice]);
  oscfreqlo[nvoice]=speed-floor(speed);
}

/*
 * Computes the frequency of an modullator oscillator
 */
void ADnote::setfreqFM(int nvoice,REALTYPE freq){
  REALTYPE speed;
  freq=fabs(freq);
  speed=freq*REALTYPE(OSCIL_SIZE)/(REALTYPE) SAMPLE_RATE;
  if (speed>OSCIL_SIZE) speed=OSCIL_SIZE;

  F2I(speed,oscfreqhiFM[nvoice]);
  oscfreqloFM[nvoice]=speed-floor(speed);
}

/*
 * Get Voice base frequency
 */
REALTYPE ADnote::getvoicebasefreq(int nvoice)
{
  REALTYPE detune;

  detune =
    NoteVoicePar[nvoice].Detune/100.0 +
    NoteVoicePar[nvoice].FineDetune/100.0 * m_ctl->bandwidth.relbw * m_bandwidth_detune_multiplier +
    m_note_global_parameters.Detune/100.0;

  if (NoteVoicePar[nvoice].fixedfreq == 0)
  {
    return m_basefreq * pow(2, detune / 12.0);
  }
  else
  {
    // the fixed freq is enabled
    REALTYPE fixedfreq = 440.0;
    int fixedfreqET = NoteVoicePar[nvoice].fixedfreqET;
    if (fixedfreqET!=0)
    {
      // if the frequency varies according the keyboard note
      REALTYPE tmp = (m_midinote - 69.0) / 12.0 * (pow(2.0,(fixedfreqET-1)/63.0) - 1.0);
      if (fixedfreqET <= 64)
      {
        fixedfreq *= pow(2.0,tmp);
      }
      else
      {
        fixedfreq *= pow(3.0,tmp);
      }
    }

    return fixedfreq * pow(2.0, detune / 12.0);
  }
}

/*
 * Get Voice's Modullator base frequency
 */
REALTYPE ADnote::getFMvoicebasefreq(int nvoice){
  REALTYPE detune=NoteVoicePar[nvoice].FMDetune/100.0;
  return(getvoicebasefreq(nvoice)*pow(2,detune/12.0));
}

/*
 * Computes all the parameters for each tick
 */
void
ADnote::computecurrentparameters()
{
  int nvoice;
  float voicefreq;
  float voicepitch;
  float filterpitch;
  float filterfreq;
  float FMfreq;
  float FMrelativepitch;
  float globalpitch;
  float globalfilterpitch;
  float temp_filter_frequency;
  float global_filter_q;

  globalpitch =
    0.01 * (m_note_global_parameters.FreqEnvelope->envout() +
            m_note_global_parameters.FreqLfo->lfoout() * m_ctl->modwheel.relmod);

  globaloldamplitude = globalnewamplitude;

  globalnewamplitude =
    m_note_global_parameters.Volume *
    m_note_global_parameters.AmpEnvelope->envout_dB() *
    m_note_global_parameters.AmpLfo->amplfoout();

  globalfilterpitch =
    m_note_global_parameters.FilterEnvelope->envout() +
    m_note_global_parameters.FilterLfo->lfoout() +
    m_note_global_parameters.FilterCenterPitch;

  temp_filter_frequency =
    globalfilterpitch +
    m_ctl->filtercutoff.relfreq +
    m_note_global_parameters.FilterFreqTracking;

  temp_filter_frequency = m_note_global_parameters.GlobalFilterL->getrealfreq(temp_filter_frequency);

  global_filter_q = m_note_global_parameters.FilterQ * m_ctl->filterq.relq;

  m_note_global_parameters.GlobalFilterL->setfreq_and_q(temp_filter_frequency, global_filter_q);
  if (m_stereo)
  {
    m_note_global_parameters.GlobalFilterR->setfreq_and_q(temp_filter_frequency, global_filter_q);
  }

  // compute the portamento, if it is used by this note
  REALTYPE portamentofreqrap=1.0;
  if (m_portamento)
  {
    // this voice use portamento
    portamentofreqrap = m_ctl->portamento.freqrap;
    if (m_ctl->portamento.used == 0)
    {
      // the portamento has finished
      m_portamento = FALSE;     // this note is no longer "portamented"
    }
  }

  //compute parameters for all voices
  for (nvoice = 0 ; nvoice < NUM_VOICES ; nvoice++)
  {
    if (!NoteVoicePar[nvoice].enabled)
    {
      continue;
    }

    NoteVoicePar[nvoice].DelayTicks -= 1;

    if (NoteVoicePar[nvoice].DelayTicks > 0)
    {
      continue;
    }

    /*******************/
    /* Voice Amplitude */
    /*******************/
    oldamplitude[nvoice] = newamplitude[nvoice];
    newamplitude[nvoice] = 1.0;

    if (NoteVoicePar[nvoice].AmpEnvelope != NULL)
    {
      newamplitude[nvoice]*=NoteVoicePar[nvoice].AmpEnvelope->envout_dB();
    }

    if (NoteVoicePar[nvoice].AmpLfo!=NULL)
      newamplitude[nvoice]*=NoteVoicePar[nvoice].AmpLfo->amplfoout();

    /****************/
    /* Voice Filter */
    /****************/
    if (NoteVoicePar[nvoice].VoiceFilter!=NULL){
      filterpitch=NoteVoicePar[nvoice].FilterCenterPitch;

      if (NoteVoicePar[nvoice].FilterEnvelope!=NULL)
        filterpitch+=NoteVoicePar[nvoice].FilterEnvelope->envout();

      if (NoteVoicePar[nvoice].FilterLfo!=NULL)
        filterpitch+=NoteVoicePar[nvoice].FilterLfo->lfoout();

      filterfreq=filterpitch+NoteVoicePar[nvoice].FilterFreqTracking;
      filterfreq=NoteVoicePar[nvoice].VoiceFilter->getrealfreq(filterfreq);

      NoteVoicePar[nvoice].VoiceFilter->setfreq(filterfreq);
    }

    if (NoteVoicePar[nvoice].noisetype==0){//compute only if the voice isn't noise

      /*******************/
      /* Voice Frequency */
      /*******************/
      voicepitch=0.0;
      if (NoteVoicePar[nvoice].FreqLfo!=NULL)
        voicepitch+=NoteVoicePar[nvoice].FreqLfo->lfoout()/100.0
          *m_ctl->bandwidth.relbw;

      if (NoteVoicePar[nvoice].FreqEnvelope!=NULL) voicepitch+=NoteVoicePar[nvoice].FreqEnvelope->envout()/100.0;
      voicefreq=getvoicebasefreq(nvoice)*pow(2,(voicepitch+globalpitch)/12.0);//Hz frequency
      voicefreq*=m_ctl->pitchwheel.relfreq;//change the frequency by the controller
      setfreq(nvoice,voicefreq*portamentofreqrap);

      /***************/
      /*  Modulator */
      /***************/
      if (NoteVoicePar[nvoice].FMEnabled!=NONE){
        FMrelativepitch=NoteVoicePar[nvoice].FMDetune/100.0;
        if (NoteVoicePar[nvoice].FMFreqEnvelope!=NULL) FMrelativepitch+=NoteVoicePar[nvoice].FMFreqEnvelope->envout()/100;
        FMfreq=pow(2.0,FMrelativepitch/12.0)*voicefreq*portamentofreqrap;
        setfreqFM(nvoice,FMfreq);

        FMoldamplitude[nvoice]=FMnewamplitude[nvoice];
        FMnewamplitude[nvoice]=NoteVoicePar[nvoice].FMVolume*m_ctl->fmamp.relamp;
        if (NoteVoicePar[nvoice].FMAmpEnvelope!=NULL)
          FMnewamplitude[nvoice]*=NoteVoicePar[nvoice].FMAmpEnvelope->envout_dB();
      }
    }

  }
  m_time += (REALTYPE)SOUND_BUFFER_SIZE/(REALTYPE)SAMPLE_RATE;
}


/*
 * Fadein in a way that removes clicks but keep sound "punchy"
 */
inline void ADnote::fadein(REALTYPE *smps){
  int zerocrossings=0;
  for (int i=1;i<SOUND_BUFFER_SIZE;i++)
    if ((smps[i-1]<0.0) && (smps[i]>0.0)) zerocrossings++;//this is only the possitive crossings

  REALTYPE tmp=(SOUND_BUFFER_SIZE-1.0)/(zerocrossings+1)/3.0;
  if (tmp<8.0) tmp=8.0;

  int n;
  F2I(tmp,n);//how many samples is the fade-in
  if (n>SOUND_BUFFER_SIZE) n=SOUND_BUFFER_SIZE;
  for (int i=0;i<n;i++) {//fade-in
    REALTYPE tmp=0.5-cos((REALTYPE)i/(REALTYPE) n*PI)*0.5;
    smps[i]*=tmp;
  }
}

/*
 * Computes the Oscillator (Without Modulation) - LinearInterpolation
 */
inline void ADnote::ComputeVoiceOscillator_LinearInterpolation(int nvoice){
  int i,poshi;
  REALTYPE poslo;

  poshi=oscposhi[nvoice];
  poslo=oscposlo[nvoice];
  REALTYPE *smps=NoteVoicePar[nvoice].OscilSmp;
  for (i=0;i<SOUND_BUFFER_SIZE;i++){
    m_tmpwave[i]=smps[poshi]*(1.0-poslo)+smps[poshi+1]*poslo;
    poslo+=oscfreqlo[nvoice];
    if (poslo>=1.0) {
      poslo-=1.0;
      poshi++;
    }
    poshi+=oscfreqhi[nvoice];
    poshi&=OSCIL_SIZE-1;
  }
  oscposhi[nvoice]=poshi;
  oscposlo[nvoice]=poslo;
}



/*
 * Computes the Oscillator (Without Modulation) - CubicInterpolation
 *
 The differences from the Linear are to little to deserve to be used. This is because I am using a large OSCIL_SIZE (>512)
 inline void ADnote::ComputeVoiceOscillator_CubicInterpolation(int nvoice){
 int i,poshi;
 REALTYPE poslo;

 poshi=oscposhi[nvoice];
 poslo=oscposlo[nvoice];
 REALTYPE *smps=NoteVoicePar[nvoice].OscilSmp;
 REALTYPE xm1,x0,x1,x2,a,b,c;
 for (i=0;i<SOUND_BUFFER_SIZE;i++){
 xm1=smps[poshi];
 x0=smps[poshi+1];
 x1=smps[poshi+2];
 x2=smps[poshi+3];
 a=(3.0 * (x0-x1) - xm1 + x2) / 2.0;
 b = 2.0*x1 + xm1 - (5.0*x0 + x2) / 2.0;
 c = (x1 - xm1) / 2.0;
 m_tmpwave[i]=(((a * poslo) + b) * poslo + c) * poslo + x0;
 printf("a\n");
 //m_tmpwave[i]=smps[poshi]*(1.0-poslo)+smps[poshi+1]*poslo;
 poslo+=oscfreqlo[nvoice];
 if (poslo>=1.0) {
 poslo-=1.0;
 poshi++;
 }
 poshi+=oscfreqhi[nvoice];
 poshi&=OSCIL_SIZE-1;
 }
 oscposhi[nvoice]=poshi;
 oscposlo[nvoice]=poslo;
 }
*/
/*
 * Computes the Oscillator (Morphing)
 */
inline void ADnote::ComputeVoiceOscillatorMorph(int nvoice){
  int i;
  REALTYPE amp;
  ComputeVoiceOscillator_LinearInterpolation(nvoice);
  if (FMnewamplitude[nvoice]>1.0) FMnewamplitude[nvoice]=1.0;
  if (FMoldamplitude[nvoice]>1.0) FMoldamplitude[nvoice]=1.0;

  if (NoteVoicePar[nvoice].FMVoice>=0){
    //if I use VoiceOut[] as modullator
    int FMVoice=NoteVoicePar[nvoice].FMVoice;
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]=m_tmpwave[i]*(1.0-amp)+amp*NoteVoicePar[FMVoice].VoiceOut[i];
    }
  } else {
    int poshiFM=oscposhiFM[nvoice];
    REALTYPE posloFM=oscposloFM[nvoice];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]=m_tmpwave[i]*(1.0-amp)+amp
        *(NoteVoicePar[nvoice].FMSmp[poshiFM]*(1-posloFM)
          +NoteVoicePar[nvoice].FMSmp[poshiFM+1]*posloFM);
      posloFM+=oscfreqloFM[nvoice];
      if (posloFM>=1.0) {
        posloFM-=1.0;
        poshiFM++;
      }
      poshiFM+=oscfreqhiFM[nvoice];
      poshiFM&=OSCIL_SIZE-1;
    }
    oscposhiFM[nvoice]=poshiFM;
    oscposloFM[nvoice]=posloFM;
  }
}

/*
 * Computes the Oscillator (Ring Modulation)
 */
inline void ADnote::ComputeVoiceOscillatorRingModulation(int nvoice){
  int i;
  REALTYPE amp;
  ComputeVoiceOscillator_LinearInterpolation(nvoice);
  if (FMnewamplitude[nvoice]>1.0) FMnewamplitude[nvoice]=1.0;
  if (FMoldamplitude[nvoice]>1.0) FMoldamplitude[nvoice]=1.0;
  if (NoteVoicePar[nvoice].FMVoice>=0){
    // if I use VoiceOut[] as modullator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      int FMVoice=NoteVoicePar[nvoice].FMVoice;
      for (i=0;i<SOUND_BUFFER_SIZE;i++)
        m_tmpwave[i]*=(1.0-amp)+amp*NoteVoicePar[FMVoice].VoiceOut[i];
    }
  } else {
    int poshiFM=oscposhiFM[nvoice];
    REALTYPE posloFM=oscposloFM[nvoice];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]*=( NoteVoicePar[nvoice].FMSmp[poshiFM]*(1.0-posloFM)
                    +NoteVoicePar[nvoice].FMSmp[poshiFM+1]*posloFM)*amp
        +(1.0-amp);
      posloFM+=oscfreqloFM[nvoice];
      if (posloFM>=1.0) {
        posloFM-=1.0;
        poshiFM++;
      }
      poshiFM+=oscfreqhiFM[nvoice];
      poshiFM&=OSCIL_SIZE-1;
    }
    oscposhiFM[nvoice]=poshiFM;
    oscposloFM[nvoice]=posloFM;
  }
}



/*
 * Computes the Oscillator (Phase Modulation or Frequency Modulation)
 */
inline void ADnote::ComputeVoiceOscillatorFrequencyModulation(int nvoice,int FMmode){
  int carposhi;
  int i,FMmodfreqhi;
  REALTYPE FMmodfreqlo,carposlo;

  if (NoteVoicePar[nvoice].FMVoice>=0){
    //if I use VoiceOut[] as modulator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]=NoteVoicePar[NoteVoicePar[nvoice].FMVoice].VoiceOut[i];
  } else {
    //Compute the modulator and store it in m_tmpwave[]
    int poshiFM=oscposhiFM[nvoice];
    REALTYPE posloFM=oscposloFM[nvoice];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      m_tmpwave[i]=(NoteVoicePar[nvoice].FMSmp[poshiFM]*(1.0-posloFM)
                  +NoteVoicePar[nvoice].FMSmp[poshiFM+1]*posloFM);
      posloFM+=oscfreqloFM[nvoice];
      if (posloFM>=1.0) {
        posloFM=fmod(posloFM,1.0);
        poshiFM++;
      }
      poshiFM+=oscfreqhiFM[nvoice];
      poshiFM&=OSCIL_SIZE-1;
    }
    oscposhiFM[nvoice]=poshiFM;
    oscposloFM[nvoice]=posloFM;
  }
  // Amplitude interpolation
  if (ABOVE_AMPLITUDE_THRESHOLD(FMoldamplitude[nvoice],FMnewamplitude[nvoice])){
    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      m_tmpwave[i]*=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                        ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
    }
  } else for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]*=FMnewamplitude[nvoice];


  //normalize makes all sample-rates, oscil_sizes toproduce same sound
  if (FMmode!=0){//Frequency modulation
    REALTYPE normalize=OSCIL_SIZE/262144.0*44100.0/(REALTYPE)SAMPLE_RATE;
    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      FMoldsmp[nvoice]=fmod(FMoldsmp[nvoice]+m_tmpwave[i]*normalize,OSCIL_SIZE);
      m_tmpwave[i]=FMoldsmp[nvoice];
    }
  } else {//Phase modulation
    REALTYPE normalize=OSCIL_SIZE/262144.0;
    for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]*=normalize;
  }

  for (i=0;i<SOUND_BUFFER_SIZE;i++){
    F2I(m_tmpwave[i],FMmodfreqhi);
    FMmodfreqlo=fmod(m_tmpwave[i]+0.0000000001,1.0);
    if (FMmodfreqhi<0) FMmodfreqlo++;

    //carrier
    carposhi=oscposhi[nvoice]+FMmodfreqhi;
    carposlo=oscposlo[nvoice]+FMmodfreqlo;

    if (carposlo>=1.0) {
      carposhi++;
      carposlo=fmod(carposlo,1.0);
    }
    carposhi&=(OSCIL_SIZE-1);

    m_tmpwave[i]=NoteVoicePar[nvoice].OscilSmp[carposhi]*(1.0-carposlo)
      +NoteVoicePar[nvoice].OscilSmp[carposhi+1]*carposlo;

    oscposlo[nvoice]+=oscfreqlo[nvoice];
    if (oscposlo[nvoice]>=1.0) {
      oscposlo[nvoice]=fmod(oscposlo[nvoice],1.0);
      oscposhi[nvoice]++;
    }

    oscposhi[nvoice]+=oscfreqhi[nvoice];
    oscposhi[nvoice]&=OSCIL_SIZE-1;
  }
}


/*Calculeaza Oscilatorul cu PITCH MODULATION*/
inline void ADnote::ComputeVoiceOscillatorPitchModulation(int nvoice){
//TODO
}

/*
 * Computes the Noise
 */
inline void ADnote::ComputeVoiceNoise(int nvoice){

  for (int i=0;i<SOUND_BUFFER_SIZE;i++)
  {
    m_tmpwave[i] = zyn_random() * 2.0 - 1.0;
  }
}

/*
 * Compute the ADnote samples
 * Returns 0 if the note is finished
 */
int ADnote::noteout(REALTYPE * outl,REALTYPE *outr){
  int i,nvoice;

  silence_two_buffers(outl, outr, SOUND_BUFFER_SIZE);

  if (!m_note_enabled) return(0);

  silence_two_buffers(m_bypassl, m_bypassr, SOUND_BUFFER_SIZE);

  computecurrentparameters();

  for (nvoice=0;nvoice<NUM_VOICES;nvoice++){
    if ((!NoteVoicePar[nvoice].enabled) || (NoteVoicePar[nvoice].DelayTicks>0)) continue;
    if (NoteVoicePar[nvoice].noisetype==0){//voice mode=sound
      switch (NoteVoicePar[nvoice].FMEnabled){
      case MORPH:ComputeVoiceOscillatorMorph(nvoice);break;
      case RING_MOD:ComputeVoiceOscillatorRingModulation(nvoice);break;
      case PHASE_MOD:ComputeVoiceOscillatorFrequencyModulation(nvoice,0);break;
      case FREQ_MOD:ComputeVoiceOscillatorFrequencyModulation(nvoice,1);break;
        //case PITCH_MOD:ComputeVoiceOscillatorPitchModulation(nvoice);break;
      default:ComputeVoiceOscillator_LinearInterpolation(nvoice);
        //if (config.cfg.Interpolation) ComputeVoiceOscillator_CubicInterpolation(nvoice);

      }
    } else ComputeVoiceNoise(nvoice);
    // Voice Processing

    // Amplitude
    if (ABOVE_AMPLITUDE_THRESHOLD(oldamplitude[nvoice],newamplitude[nvoice])){
      int rest=SOUND_BUFFER_SIZE;
      //test if the amplitude if raising and the difference is high
      if ((newamplitude[nvoice]>oldamplitude[nvoice])&&((newamplitude[nvoice]-oldamplitude[nvoice])>0.25)){
        rest=10;
        if (rest>SOUND_BUFFER_SIZE) rest=SOUND_BUFFER_SIZE;
        for (int i=0;i<SOUND_BUFFER_SIZE-rest;i++) m_tmpwave[i]*=oldamplitude[nvoice];
      }
      // Amplitude interpolation
      for (i=0;i<rest;i++){
        m_tmpwave[i+(SOUND_BUFFER_SIZE-rest)]*=INTERPOLATE_AMPLITUDE(oldamplitude[nvoice]
                                                                   ,newamplitude[nvoice],i,rest);
      }
    } else for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]*=newamplitude[nvoice];

    // Fade in
    if (firsttick[nvoice]!=0){
      fadein(&m_tmpwave[0]);
      firsttick[nvoice]=0;
    }


    // Filter
    if (NoteVoicePar[nvoice].VoiceFilter!=NULL) NoteVoicePar[nvoice].VoiceFilter->filterout(&m_tmpwave[0]);

    //check if the amplitude envelope is finished, if yes, the voice will be fadeout
    if (NoteVoicePar[nvoice].AmpEnvelope!=NULL) {
      if (NoteVoicePar[nvoice].AmpEnvelope->finished()!=0)
        for (i=0;i<SOUND_BUFFER_SIZE;i++)
          m_tmpwave[i]*=1.0-(REALTYPE)i/(REALTYPE)SOUND_BUFFER_SIZE;
      //the voice is killed later
    }


    // Put the ADnote samples in VoiceOut (without appling Global volume, because I wish to use this voice as a modullator)
    if (NoteVoicePar[nvoice].VoiceOut!=NULL)
      for (i=0;i<SOUND_BUFFER_SIZE;i++) NoteVoicePar[nvoice].VoiceOut[i]=m_tmpwave[i];


    // Add the voice that do not bypass the filter to out
    if (NoteVoicePar[nvoice].filterbypass==0)
    {
      // no bypass

      if (m_stereo)
      {
        // stereo
        for (i=0;i<SOUND_BUFFER_SIZE;i++)
        {
          outl[i]+=m_tmpwave[i]*NoteVoicePar[nvoice].Volume*NoteVoicePar[nvoice].Panning*2.0;
          outr[i]+=m_tmpwave[i]*NoteVoicePar[nvoice].Volume*(1.0-NoteVoicePar[nvoice].Panning)*2.0;
        }
      }
      else
      {
        // mono
        for (i=0;i<SOUND_BUFFER_SIZE;i++) outl[i]+=m_tmpwave[i]*NoteVoicePar[nvoice].Volume;
      }
    }
    else
    {
      // bypass the filter

      if (m_stereo)
      {
        // stereo
        for (i=0;i<SOUND_BUFFER_SIZE;i++)
        {
          m_bypassl[i]+=m_tmpwave[i]*NoteVoicePar[nvoice].Volume*NoteVoicePar[nvoice].Panning*2.0;
          m_bypassr[i]+=m_tmpwave[i]*NoteVoicePar[nvoice].Volume*(1.0-NoteVoicePar[nvoice].Panning)*2.0;
        }
      }
      else
      {
        // mono
        for (i=0;i<SOUND_BUFFER_SIZE;i++) m_bypassl[i]+=m_tmpwave[i]*NoteVoicePar[nvoice].Volume;
      }
    }
    // chech if there is necesary to proces the voice longer (if the Amplitude envelope isn't finished)
    if (NoteVoicePar[nvoice].AmpEnvelope!=NULL) {
      if (NoteVoicePar[nvoice].AmpEnvelope->finished()!=0) KillVoice(nvoice);
    }
  }


  // Processing Global parameters
  m_note_global_parameters.GlobalFilterL->filterout(&outl[0]);

  if (!m_stereo)
  {
    // set the right channel=left channel
    for (i=0;i<SOUND_BUFFER_SIZE;i++)
    {
      outr[i]=outl[i];
      m_bypassr[i]=m_bypassl[i];
    }
  }
  else
  {
    m_note_global_parameters.GlobalFilterR->filterout(&outr[0]);
  }

  for (i=0;i<SOUND_BUFFER_SIZE;i++)
  {
//    outl[i]+=m_bypassl[i];
//    outr[i]+=m_bypassr[i];
  }

  if (ABOVE_AMPLITUDE_THRESHOLD(globaloldamplitude,globalnewamplitude))
  {
    // Amplitude Interpolation
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      REALTYPE tmpvol = INTERPOLATE_AMPLITUDE(globaloldamplitude, globalnewamplitude, i, SOUND_BUFFER_SIZE);
      outl[i] *= tmpvol * (1.0 - m_note_global_parameters.Panning);
      outr[i] *= tmpvol * m_note_global_parameters.Panning;
    }
  }
  else
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      outl[i] *= globalnewamplitude * (1.0 - m_note_global_parameters.Panning);
      outr[i] *= globalnewamplitude * m_note_global_parameters.Panning;
    }
  }

  // Apply the punch
  if (m_note_global_parameters.punch.enabled)
  {
    for (i=0;i<SOUND_BUFFER_SIZE;i++)
    {
      REALTYPE punchamp = m_note_global_parameters.punch.initialvalue*m_note_global_parameters.punch.t+1.0;
      outl[i] *= punchamp;
      outr[i] *= punchamp;
      m_note_global_parameters.punch.t -= m_note_global_parameters.punch.dt;
      if (m_note_global_parameters.punch.t < 0.0)
      {
        m_note_global_parameters.punch.enabled = FALSE;
        break;
      }
    }
  }

  // Check if the global amplitude is finished.
  // If it does, disable the note
  if (m_note_global_parameters.AmpEnvelope->finished()!=0) {
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {//fade-out
      REALTYPE tmp=1.0-(REALTYPE)i/(REALTYPE)SOUND_BUFFER_SIZE;
      outl[i]*=tmp;
      outr[i]*=tmp;
    }
    KillNote();
  }
  return(1);
}

/*
 * Relase the key (NoteOff)
 */
void ADnote::relasekey()
{
  int nvoice;

  for (nvoice=0;nvoice<NUM_VOICES;nvoice++)
  {
    if (!NoteVoicePar[nvoice].enabled) continue;
    if (NoteVoicePar[nvoice].AmpEnvelope!=NULL) NoteVoicePar[nvoice].AmpEnvelope->relasekey();
    if (NoteVoicePar[nvoice].FreqEnvelope!=NULL) NoteVoicePar[nvoice].FreqEnvelope->relasekey();
    if (NoteVoicePar[nvoice].FilterEnvelope!=NULL) NoteVoicePar[nvoice].FilterEnvelope->relasekey();
    if (NoteVoicePar[nvoice].FMFreqEnvelope!=NULL) NoteVoicePar[nvoice].FMFreqEnvelope->relasekey();
    if (NoteVoicePar[nvoice].FMAmpEnvelope!=NULL) NoteVoicePar[nvoice].FMAmpEnvelope->relasekey();
  }
  m_note_global_parameters.FreqEnvelope->relasekey();
  m_note_global_parameters.FilterEnvelope->relasekey();
  m_note_global_parameters.AmpEnvelope->relasekey();
}

/*
 * Check if the note is finished
 */
BOOL
ADnote::finished()
{
  return !m_note_enabled;
}
