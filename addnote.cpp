/*
  ZynAddSubFX - a software synthesizer
 
  ADnote.C - The "additive" synthesizer
  Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
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
#include "resonance.h"
#include "fft.h"
#include "oscillator.h"
#include "resonance.h"
#include "envelope_parameters.h"
#include "lfo_parameters.h"
#include "filter_parameters.h"
#include "Controller.h"
#include "lfo.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "filter.h"
#include "envelope.h"
#include "addsynth_internal.h"
#include "addnote.h"

ADnote::ADnote(
  struct zyn_addsynth * synth_ptr,
  Controller * ctl)
{
  m_tmpwave = new REALTYPE [SOUND_BUFFER_SIZE];
  m_bypassl = new REALTYPE [SOUND_BUFFER_SIZE];
  m_bypassr = new REALTYPE [SOUND_BUFFER_SIZE];

  m_ctl = ctl;

  m_stereo = synth_ptr->stereo;

  m_detune = getdetune(
    synth_ptr->GlobalPar.PDetuneType,
    synth_ptr->GlobalPar.PCoarseDetune,
    synth_ptr->GlobalPar.PDetune);

  /*
   * Get the Multiplier of the fine detunes of the voices
   */
  m_bandwidth_detune_multiplier = (synth_ptr->GlobalPar.PBandwidth - 64.0) / 64.0;
  m_bandwidth_detune_multiplier =
    pow(
      2.0,
      m_bandwidth_detune_multiplier * pow(fabs(m_bandwidth_detune_multiplier), 0.2) * 5.0);

  m_note_enabled = false;

  m_synth_ptr = synth_ptr;
}

void
ADnote::note_on(
  float panorama,
  bool random_grouping,
  REALTYPE freq,
  REALTYPE velocity,
  bool portamento,
  int midinote)
{
  int voice_index;
  int i;

  m_portamento = portamento;
  m_midinote = midinote;
  m_note_enabled = true;
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

  m_panning = (panorama + 1.0) / 2; // -1..1 -> 0 - 1

  m_filter_center_pitch =
    m_synth_ptr->m_filter_params.getfreq() + // center freq
    m_synth_ptr->m_filter_velocity_sensing_amount * 6.0 * // velocity sensing
    (zyn_velocity_scale(m_velocity, m_synth_ptr->m_filter_velocity_scale_function) - 1);

  if (m_synth_ptr->GlobalPar.PPunchStrength != 0)
  {
    m_punch_enabled = true;
    m_punch_t = 1.0; //start from 1.0 and to 0.0
    m_punch_initial_value =
      ( (pow(10,1.5*m_synth_ptr->GlobalPar.PPunchStrength/127.0)-1.0)
        *VelF(m_velocity,m_synth_ptr->GlobalPar.PPunchVelocitySensing) );
    REALTYPE time = pow(10,3.0*m_synth_ptr->GlobalPar.PPunchTime/127.0)/10000.0; //0.1 .. 100 ms
    REALTYPE stretch = pow(440.0/freq,m_synth_ptr->GlobalPar.PPunchStretch/64.0);
    m_punch_duration = 1.0 / (time * SAMPLE_RATE * stretch);
  }
  else
  {
    m_punch_enabled = false;
  }

  for (voice_index=0;voice_index<NUM_VOICES;voice_index++)
  {
    m_synth_ptr->voices_params[voice_index].OscilSmp->newrandseed(rand());
    m_voices[voice_index].OscilSmp=NULL;
    m_voices[voice_index].FMSmp=NULL;
    m_voices[voice_index].VoiceOut=NULL;

    m_voices[voice_index].FMVoice=-1;

    if (m_synth_ptr->voices_params[voice_index].Enabled==0) {
      m_voices[voice_index].enabled = false;
      continue; //the voice is disabled
    }

    m_voices[voice_index].enabled = true;
    m_voices[voice_index].fixedfreq=m_synth_ptr->voices_params[voice_index].Pfixedfreq;
    m_voices[voice_index].fixedfreqET=m_synth_ptr->voices_params[voice_index].PfixedfreqET;

    // use the Globalpars.detunetype if the detunetype is 0
    if (m_synth_ptr->voices_params[voice_index].PDetuneType != 0)
    {
      // coarse detune
      m_voices[voice_index].Detune =
        getdetune(
          m_synth_ptr->voices_params[voice_index].PDetuneType,
          m_synth_ptr->voices_params[voice_index].PCoarseDetune,
          8192);

      // fine detune
      m_voices[voice_index].FineDetune =
        getdetune(
          m_synth_ptr->voices_params[voice_index].PDetuneType,
          0,
          m_synth_ptr->voices_params[voice_index].PDetune);
    }
    else
    {
      m_voices[voice_index].Detune=getdetune(m_synth_ptr->GlobalPar.PDetuneType
                                            ,m_synth_ptr->voices_params[voice_index].PCoarseDetune,8192);//coarse detune
      m_voices[voice_index].FineDetune=getdetune(m_synth_ptr->GlobalPar.PDetuneType
                                                ,0,m_synth_ptr->voices_params[voice_index].PDetune);//fine detune
    }

    if (m_synth_ptr->voices_params[voice_index].PFMDetuneType!=0)
    {
      m_voices[voice_index].FMDetune = 
        getdetune(
          m_synth_ptr->voices_params[voice_index].PFMDetuneType,
          m_synth_ptr->voices_params[voice_index].PFMCoarseDetune,
          m_synth_ptr->voices_params[voice_index].PFMDetune);
    }
    else
    {
      m_voices[voice_index].FMDetune = getdetune(
        m_synth_ptr->GlobalPar.PDetuneType,
        m_synth_ptr->voices_params[voice_index].PFMCoarseDetune,
        m_synth_ptr->voices_params[voice_index].PFMDetune);
    }

    oscposhi[voice_index]=0;oscposlo[voice_index]=0.0;
    oscposhiFM[voice_index]=0;oscposloFM[voice_index]=0.0;

    m_voices[voice_index].OscilSmp=new REALTYPE[OSCIL_SIZE+OSCIL_SMP_EXTRA_SAMPLES];//the extra points contains the first point

    //Get the voice's oscil or external's voice oscil
    int vc=voice_index;

    if (m_synth_ptr->voices_params[voice_index].Pextoscil != -1)
    {
      vc = m_synth_ptr->voices_params[voice_index].Pextoscil;
    }

    if (!random_grouping)
    {
      m_synth_ptr->voices_params[vc].OscilSmp->newrandseed(rand());
    }

    oscposhi[voice_index] = m_synth_ptr->voices_params[vc].OscilSmp->get(
      m_voices[voice_index].OscilSmp,
      getvoicebasefreq(voice_index),
      m_synth_ptr->voices_params[voice_index].Presonance);

    //I store the first elments to the last position for speedups
    for (i=0;i<OSCIL_SMP_EXTRA_SAMPLES;i++) m_voices[voice_index].OscilSmp[OSCIL_SIZE+i]=m_voices[voice_index].OscilSmp[i];

    oscposhi[voice_index]+=(int)((m_synth_ptr->voices_params[voice_index].Poscilphase-64.0)/128.0*OSCIL_SIZE+OSCIL_SIZE*4);
    oscposhi[voice_index]%=OSCIL_SIZE;

    m_voices[voice_index].FilterCenterPitch=m_synth_ptr->voices_params[voice_index].m_filter_params.getfreq();
    m_voices[voice_index].filterbypass=m_synth_ptr->voices_params[voice_index].Pfilterbypass;

    m_voices[voice_index].fm_type = m_synth_ptr->voices_params[voice_index].fm_type;

    m_voices[voice_index].FMVoice=m_synth_ptr->voices_params[voice_index].PFMVoice;

    //Compute the Voice's modulator volume (incl. damping)
    REALTYPE fmvoldamp=pow(440.0/getvoicebasefreq(voice_index),m_synth_ptr->voices_params[voice_index].PFMVolumeDamp/64.0-1.0);
    switch (m_voices[voice_index].fm_type)
    {
    case ZYN_FM_TYPE_PHASE_MOD:
      fmvoldamp = pow(440.0/getvoicebasefreq(voice_index),m_synth_ptr->voices_params[voice_index].PFMVolumeDamp/64.0);
      m_voices[voice_index].FMVolume = (exp(m_synth_ptr->voices_params[voice_index].PFMVolume/127.0*FM_AMP_MULTIPLIER)-1.0)*fmvoldamp*4.0;
      break;
    case ZYN_FM_TYPE_FREQ_MOD:
      m_voices[voice_index].FMVolume = exp(m_synth_ptr->voices_params[voice_index].PFMVolume / 127.0 * FM_AMP_MULTIPLIER);
      m_voices[voice_index].FMVolume -= 1.0;
      m_voices[voice_index].FMVolume *= fmvoldamp * 4.0;
      break;
#if 0                           // ???????????
    case ZYN_FM_TYPE_PITCH_MOD:
      m_voices[voice_index].FMVolume=(m_synth_ptr->voices_params[voice_index].PFMVolume/127.0*8.0)*fmvoldamp;
      break;
#endif
    default:
      if (fmvoldamp > 1.0)
      {
        fmvoldamp = 1.0;
      }

      m_voices[voice_index].FMVolume = m_synth_ptr->voices_params[voice_index].PFMVolume / 127.0 * fmvoldamp;
    }

    //Voice's modulator velocity sensing
    m_voices[voice_index].FMVolume*=VelF(m_velocity,m_synth_ptr->voices_params[voice_index].PFMVelocityScaleFunction);

    FMoldsmp[voice_index]=0.0;//this is for FM (integration)

    firsttick[voice_index]=1;
    m_voices[voice_index].DelayTicks=(int)((exp(m_synth_ptr->voices_params[voice_index].PDelay/127.0*log(50.0))-1.0)/SOUND_BUFFER_SIZE/10.0*SAMPLE_RATE);
  }

  int nvoice,tmp[NUM_VOICES];

  // Global Parameters
  m_frequency_envelope.init(&m_synth_ptr->m_frequency_envelope_params, m_basefreq);

  m_frequency_lfo.init(
    m_basefreq,
    &m_synth_ptr->frequency_lfo_params,
    ZYN_LFO_TYPE_FREQUENCY);

  m_amplitude_envelope.init(&m_synth_ptr->m_amplitude_envelope_params, m_basefreq);

  m_amplitude_lfo.init(
    m_basefreq,
    &m_synth_ptr->amplitude_lfo_params,
    ZYN_LFO_TYPE_AMPLITUDE);

  m_volume = 4.0*pow(0.1,3.0*(1.0-m_synth_ptr->GlobalPar.PVolume/96.0))//-60 dB .. 0 dB
    *VelF(m_velocity,m_synth_ptr->GlobalPar.PAmpVelocityScaleFunction);//velocity sensing

  m_amplitude_envelope.envout_dB(); // discard the first envelope output

  globalnewamplitude = m_volume * m_amplitude_envelope.envout_dB() * m_amplitude_lfo.amplfoout();

  m_filter_left.init(&m_synth_ptr->m_filter_params);
  if (m_stereo)
  {
    m_filter_right.init(&m_synth_ptr->m_filter_params);
  }

  m_filter_envelope.init(&m_synth_ptr->m_filter_envelope_params, m_basefreq);

  m_filter_lfo.init(
    m_basefreq,
    &m_synth_ptr->filter_lfo_params,
    ZYN_LFO_TYPE_FILTER);

  m_filter_q_factor = m_synth_ptr->m_filter_params.getq();
  m_filter_frequency_tracking = m_synth_ptr->m_filter_params.getfreqtracking(m_basefreq);

  // Forbids the Modulation Voice to be greater or equal than voice
  for (i=0;i<NUM_VOICES;i++) if (m_voices[i].FMVoice>=i) m_voices[i].FMVoice=-1;

  // Voice Parameter init
  for (nvoice=0;nvoice<NUM_VOICES;nvoice++){
    if (!m_voices[nvoice].enabled) continue;

    m_voices[nvoice].noisetype=m_synth_ptr->voices_params[nvoice].Type;
    /* Voice Amplitude Parameters Init */
    m_voices[nvoice].Volume=pow(0.1,3.0*(1.0-m_synth_ptr->voices_params[nvoice].PVolume/127.0)) // -60 dB .. 0 dB
      *VelF(m_velocity,m_synth_ptr->voices_params[nvoice].PAmpVelocityScaleFunction);//velocity

    if (m_synth_ptr->voices_params[nvoice].PVolumeminus!=0) m_voices[nvoice].Volume=-m_voices[nvoice].Volume;

    if (m_synth_ptr->voices_params[nvoice].PPanning==0)
    {
      m_voices[nvoice].Panning = zyn_random(); // random panning
    }
    else
    {
      m_voices[nvoice].Panning = m_synth_ptr->voices_params[nvoice].PPanning/128.0;
    }

    newamplitude[nvoice]=1.0;
    if (m_synth_ptr->voices_params[nvoice].PAmpEnvelopeEnabled != 0)
    {
      m_voices[nvoice].m_amplitude_envelope.init(&m_synth_ptr->voices_params[nvoice].m_amplitude_envelope_params, m_basefreq);
      m_voices[nvoice].m_amplitude_envelope.envout_dB(); // discard the first envelope sample
      newamplitude[nvoice] *= m_voices[nvoice].m_amplitude_envelope.envout_dB();
    }

    if (m_synth_ptr->voices_params[nvoice].PAmpLfoEnabled != 0)
    {
      m_voices[nvoice].m_amplitude_lfo.init(
        m_basefreq,
        &m_synth_ptr->voices_params[nvoice].amplitude_lfo_params,
        ZYN_LFO_TYPE_AMPLITUDE);

      newamplitude[nvoice] *= m_voices[nvoice].m_amplitude_lfo.amplfoout();
    }

    /* Voice Frequency Parameters Init */
    if (m_synth_ptr->voices_params[nvoice].PFreqEnvelopeEnabled != 0)
    {
      m_voices[nvoice].m_frequency_envelope.init(&m_synth_ptr->voices_params[nvoice].m_frequency_envelope_params, m_basefreq);
    }

    if (m_synth_ptr->voices_params[nvoice].PFreqLfoEnabled != 0)
    {
      m_voices[nvoice].m_frequency_lfo.init(
        m_basefreq,
        &m_synth_ptr->voices_params[nvoice].frequency_lfo_params,
        ZYN_LFO_TYPE_FREQUENCY);
    }

    /* Voice Filter Parameters Init */
    if (m_synth_ptr->voices_params[nvoice].PFilterEnabled != 0)
    {
      m_voices[nvoice].m_voice_filter.init(&m_synth_ptr->voices_params[nvoice].m_filter_params);
    }

    if (m_synth_ptr->voices_params[nvoice].PFilterEnvelopeEnabled != 0)
    {
      m_voices[nvoice].m_filter_envelope.init(&m_synth_ptr->voices_params[nvoice].m_filter_envelope_params, m_basefreq);
    }

    if (m_synth_ptr->voices_params[nvoice].PFilterLfoEnabled != 0)
    {
      m_voices[nvoice].m_filter_lfo.init(
        m_basefreq,
        &m_synth_ptr->voices_params[nvoice].filter_lfo_params,
        ZYN_LFO_TYPE_FILTER);
    }

    m_voices[nvoice].FilterFreqTracking = m_synth_ptr->voices_params[nvoice].m_filter_params.getfreqtracking(m_basefreq);

    /* Voice Modulation Parameters Init */
    if (m_voices[nvoice].fm_type != ZYN_FM_TYPE_NONE && m_voices[nvoice].FMVoice < 0)
    {
      m_synth_ptr->voices_params[nvoice].FMSmp->newrandseed(rand());
      m_voices[nvoice].FMSmp=new REALTYPE[OSCIL_SIZE+OSCIL_SMP_EXTRA_SAMPLES];

      //Perform Anti-aliasing only on MORPH or RING MODULATION

      int vc=nvoice;
      if (m_synth_ptr->voices_params[nvoice].PextFMoscil!=-1) vc=m_synth_ptr->voices_params[nvoice].PextFMoscil;

      REALTYPE tmp=1.0;
      if ((m_synth_ptr->voices_params[vc].FMSmp->Padaptiveharmonics != 0) ||
          (m_voices[nvoice].fm_type == ZYN_FM_TYPE_MORPH) ||
          (m_voices[nvoice].fm_type == ZYN_FM_TYPE_RING_MOD))
      {
        tmp = getFMvoicebasefreq(nvoice);
      }

      if (!random_grouping)
      {
        m_synth_ptr->voices_params[vc].FMSmp->newrandseed(rand());
      }

      oscposhiFM[nvoice]=(oscposhi[nvoice]+m_synth_ptr->voices_params[vc].FMSmp->get(m_voices[nvoice].FMSmp,tmp)) % OSCIL_SIZE;
      for (int i=0;i<OSCIL_SMP_EXTRA_SAMPLES;i++) m_voices[nvoice].FMSmp[OSCIL_SIZE+i]=m_voices[nvoice].FMSmp[i];
      oscposhiFM[nvoice]+=(int)((m_synth_ptr->voices_params[nvoice].PFMoscilphase-64.0)/128.0*OSCIL_SIZE+OSCIL_SIZE*4);
      oscposhiFM[nvoice]%=OSCIL_SIZE;
    }

    if (m_synth_ptr->voices_params[nvoice].PFMFreqEnvelopeEnabled != 0)
    {
      m_voices[nvoice].m_fm_frequency_envelope.init(&m_synth_ptr->voices_params[nvoice].m_fm_frequency_envelope_params, m_basefreq);
    }

    FMnewamplitude[nvoice] = m_voices[nvoice].FMVolume*m_ctl->fmamp.relamp;

    if (m_synth_ptr->voices_params[nvoice].PFMAmpEnvelopeEnabled != 0)
    {
      m_voices[nvoice].m_fm_amplitude_envelope.init(&m_synth_ptr->voices_params[nvoice].m_fm_amplitude_envelope_params, m_basefreq);
      FMnewamplitude[nvoice] *= m_voices[nvoice].m_fm_amplitude_envelope.envout_dB();
    }
  }

  for (nvoice = 0 ; nvoice < NUM_VOICES ; nvoice++)
  {
    for (i = nvoice + 1 ; i < NUM_VOICES ; i++)
    {
      tmp[i] = 0;
    }

    for (i = nvoice + 1 ; i < NUM_VOICES ; i++)
    {
      if (m_voices[i].FMVoice == nvoice && tmp[i] == 0)
      {
        m_voices[nvoice].VoiceOut = new REALTYPE[SOUND_BUFFER_SIZE];
        tmp[i] = 1;
      }
    }

    if (m_voices[nvoice].VoiceOut != NULL)
    {
      for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
      {
        m_voices[nvoice].VoiceOut[i] = 0.0;
      }
    }
  }
}

/*
 * Kill a voice of ADnote
 */
void ADnote::KillVoice(int nvoice)
{
  int i;
  delete [] (m_voices[nvoice].OscilSmp);

  if ((m_voices[nvoice].fm_type != ZYN_FM_TYPE_NONE) && (m_voices[nvoice].FMVoice < 0))
  {
    delete m_voices[nvoice].FMSmp;
  }

  if (m_voices[nvoice].VoiceOut != NULL)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      m_voices[nvoice].VoiceOut[i] = 0.0;//do not delete, yet: perhaps is used by another voice
    }
  }

  m_voices[nvoice].enabled = false;
}

/*
 * Kill the note
 */
void
ADnote::KillNote()
{
  int nvoice;

  for (nvoice = 0 ; nvoice < NUM_VOICES ; nvoice++)
  {
    if (m_voices[nvoice].enabled)
    {
      KillVoice(nvoice);
    }

    // delete VoiceOut
    if (m_voices[nvoice].VoiceOut != NULL)
    {
      delete(m_voices[nvoice].VoiceOut);
      m_voices[nvoice].VoiceOut = NULL;
    }
  }

  m_note_enabled = false;
}

ADnote::~ADnote(){
  if (m_note_enabled) KillNote();
  delete [] m_tmpwave;
  delete [] m_bypassl;
  delete [] m_bypassr;
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
    m_voices[nvoice].Detune / 100.0 +
    m_voices[nvoice].FineDetune / 100.0 * m_ctl->bandwidth.relbw * m_bandwidth_detune_multiplier +
    m_detune / 100.0;

  if (m_voices[nvoice].fixedfreq == 0)
  {
    return m_basefreq * pow(2, detune / 12.0);
  }
  else
  {
    // the fixed freq is enabled
    REALTYPE fixedfreq = 440.0;
    int fixedfreqET = m_voices[nvoice].fixedfreqET;
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
  REALTYPE detune=m_voices[nvoice].FMDetune/100.0;
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
    0.01 * (m_frequency_envelope.envout() +
            m_frequency_lfo.lfoout() * m_ctl->modwheel.relmod);

  globaloldamplitude = globalnewamplitude;

  globalnewamplitude =
    m_volume *
    m_amplitude_envelope.envout_dB() *
    m_amplitude_lfo.amplfoout();

  globalfilterpitch =
    m_filter_envelope.envout() +
    m_filter_lfo.lfoout() +
    m_filter_center_pitch;

  temp_filter_frequency =
    globalfilterpitch +
    m_ctl->filtercutoff.relfreq +
    m_filter_frequency_tracking;

  temp_filter_frequency = m_filter_left.getrealfreq(temp_filter_frequency);

  global_filter_q = m_filter_q_factor * m_ctl->filterq.relq;

  m_filter_left.setfreq_and_q(temp_filter_frequency, global_filter_q);
  if (m_stereo)
  {
    m_filter_right.setfreq_and_q(temp_filter_frequency, global_filter_q);
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
      m_portamento = false;     // this note is no longer "portamented"
    }
  }

  //compute parameters for all voices
  for (nvoice = 0 ; nvoice < NUM_VOICES ; nvoice++)
  {
    if (!m_voices[nvoice].enabled)
    {
      continue;
    }

    m_voices[nvoice].DelayTicks -= 1;

    if (m_voices[nvoice].DelayTicks > 0)
    {
      continue;
    }

    /*******************/
    /* Voice Amplitude */
    /*******************/
    oldamplitude[nvoice] = newamplitude[nvoice];
    newamplitude[nvoice] = 1.0;

    if (m_synth_ptr->voices_params[nvoice].PAmpEnvelopeEnabled)
    {
      newamplitude[nvoice] *= m_voices[nvoice].m_amplitude_envelope.envout_dB();
    }

    if (m_synth_ptr->voices_params[nvoice].PAmpLfoEnabled)
    {
      newamplitude[nvoice] *= m_voices[nvoice].m_amplitude_lfo.amplfoout();
    }

    /****************/
    /* Voice Filter */
    /****************/
    if (m_synth_ptr->voices_params[nvoice].PFilterEnabled)
    {
      filterpitch = m_voices[nvoice].FilterCenterPitch;

      if (m_synth_ptr->voices_params[nvoice].PFilterEnvelopeEnabled)
      {
        filterpitch += m_voices[nvoice].m_filter_envelope.envout();
      }

      if (m_synth_ptr->voices_params[nvoice].PFilterLfoEnabled)
      {
        filterpitch += m_voices[nvoice].m_filter_lfo.lfoout();
      }

      filterfreq = filterpitch + m_voices[nvoice].FilterFreqTracking;
      filterfreq = m_voices[nvoice].m_voice_filter.getrealfreq(filterfreq);

      m_voices[nvoice].m_voice_filter.setfreq(filterfreq);
    }

    // compute only if the voice isn't noise
    if (m_voices[nvoice].noisetype == 0)
    {
      /*******************/
      /* Voice Frequency */
      /*******************/
      voicepitch=0.0;
      if (m_synth_ptr->voices_params[nvoice].PFreqLfoEnabled)
      {
        voicepitch += m_voices[nvoice].m_frequency_lfo.lfoout() / 100.0 * m_ctl->bandwidth.relbw;
      }

      if (m_synth_ptr->voices_params[nvoice].PFreqEnvelopeEnabled)
      {
        voicepitch += m_voices[nvoice].m_frequency_envelope.envout() / 100.0;
      }

      voicefreq = getvoicebasefreq(nvoice) * pow(2, (voicepitch + globalpitch) / 12.0); // Hz frequency
      voicefreq *= m_ctl->pitchwheel.relfreq; // change the frequency by the controller
      setfreq(nvoice, voicefreq * portamentofreqrap);

      /***************/
      /*  Modulator */
      /***************/
      if (m_voices[nvoice].fm_type != ZYN_FM_TYPE_NONE)
      {
        FMrelativepitch = m_voices[nvoice].FMDetune / 100.0;
        if (m_synth_ptr->voices_params[nvoice].PFMFreqEnvelopeEnabled)
        {
          FMrelativepitch += m_voices[nvoice].m_fm_frequency_envelope.envout() / 100;
        }

        FMfreq = pow(2.0, FMrelativepitch / 12.0) * voicefreq * portamentofreqrap;
        setfreqFM(nvoice, FMfreq);

        FMoldamplitude[nvoice] = FMnewamplitude[nvoice];
        FMnewamplitude[nvoice] = m_voices[nvoice].FMVolume * m_ctl->fmamp.relamp;
        if (m_synth_ptr->voices_params[nvoice].PFMAmpEnvelopeEnabled)
        {
          FMnewamplitude[nvoice] *= m_voices[nvoice].m_fm_amplitude_envelope.envout_dB();
        }
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
  REALTYPE *smps=m_voices[nvoice].OscilSmp;
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
 REALTYPE *smps=m_voices[nvoice].OscilSmp;
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

  if (m_voices[nvoice].FMVoice>=0){
    //if I use VoiceOut[] as modullator
    int FMVoice=m_voices[nvoice].FMVoice;
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]=m_tmpwave[i]*(1.0-amp)+amp*m_voices[FMVoice].VoiceOut[i];
    }
  } else {
    int poshiFM=oscposhiFM[nvoice];
    REALTYPE posloFM=oscposloFM[nvoice];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]=m_tmpwave[i]*(1.0-amp)+amp
        *(m_voices[nvoice].FMSmp[poshiFM]*(1-posloFM)
          +m_voices[nvoice].FMSmp[poshiFM+1]*posloFM);
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
  if (m_voices[nvoice].FMVoice>=0){
    // if I use VoiceOut[] as modullator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      int FMVoice=m_voices[nvoice].FMVoice;
      for (i=0;i<SOUND_BUFFER_SIZE;i++)
        m_tmpwave[i]*=(1.0-amp)+amp*m_voices[FMVoice].VoiceOut[i];
    }
  } else {
    int poshiFM=oscposhiFM[nvoice];
    REALTYPE posloFM=oscposloFM[nvoice];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      amp=INTERPOLATE_AMPLITUDE(FMoldamplitude[nvoice]
                                ,FMnewamplitude[nvoice],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]*=( m_voices[nvoice].FMSmp[poshiFM]*(1.0-posloFM)
                    +m_voices[nvoice].FMSmp[poshiFM+1]*posloFM)*amp
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

  if (m_voices[nvoice].FMVoice>=0){
    //if I use VoiceOut[] as modulator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]=m_voices[m_voices[nvoice].FMVoice].VoiceOut[i];
  } else {
    //Compute the modulator and store it in m_tmpwave[]
    int poshiFM=oscposhiFM[nvoice];
    REALTYPE posloFM=oscposloFM[nvoice];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      m_tmpwave[i]=(m_voices[nvoice].FMSmp[poshiFM]*(1.0-posloFM)
                  +m_voices[nvoice].FMSmp[poshiFM+1]*posloFM);
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

    m_tmpwave[i]=m_voices[nvoice].OscilSmp[carposhi]*(1.0-carposlo)
      +m_voices[nvoice].OscilSmp[carposhi+1]*carposlo;

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
 */
void
ADnote::noteout(
  REALTYPE * outl,
  REALTYPE * outr)
{
  int i,nvoice;

  silence_two_buffers(outl, outr, SOUND_BUFFER_SIZE);

  if (!m_note_enabled)
  {
    return;
  }

  silence_two_buffers(m_bypassl, m_bypassr, SOUND_BUFFER_SIZE);

  computecurrentparameters();

  for (nvoice = 0 ; nvoice < NUM_VOICES ; nvoice++)
  {
    if (!m_voices[nvoice].enabled || m_voices[nvoice].DelayTicks > 0)
    {
      continue;
    }

    if (m_voices[nvoice].noisetype == 0) //voice mode = sound
    {
      switch (m_voices[nvoice].fm_type)
      {
      case ZYN_FM_TYPE_MORPH:
        ComputeVoiceOscillatorMorph(nvoice);
        break;
      case ZYN_FM_TYPE_RING_MOD:
        ComputeVoiceOscillatorRingModulation(nvoice);
        break;
      case ZYN_FM_TYPE_PHASE_MOD:
        ComputeVoiceOscillatorFrequencyModulation(nvoice,0);
        break;
      case ZYN_FM_TYPE_FREQ_MOD:
        ComputeVoiceOscillatorFrequencyModulation(nvoice,1);
        break;
#if 0
      case ZYN_FM_TYPE_PITCH_MOD:
        ComputeVoiceOscillatorPitchModulation(nvoice);
        break;
#endif
      default:
        ComputeVoiceOscillator_LinearInterpolation(nvoice);
        //if (config.cfg.Interpolation) ComputeVoiceOscillator_CubicInterpolation(nvoice);
      }
    }
    else
    {
      ComputeVoiceNoise(nvoice);
    }

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
    if (m_synth_ptr->voices_params[nvoice].PFilterEnabled)
    {
      m_voices[nvoice].m_voice_filter.filterout(&m_tmpwave[0]);
    }

    //check if the amplitude envelope is finished, if yes, the voice will be fadeout
    if (m_synth_ptr->voices_params[nvoice].PAmpEnvelopeEnabled)
    {
      if (m_voices[nvoice].m_amplitude_envelope.finished())
      {
        for (i=0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          m_tmpwave[i] *= 1.0 - (REALTYPE)i / (REALTYPE)SOUND_BUFFER_SIZE;
        }
      }

      // the voice is killed later
    }


    // Put the ADnote samples in VoiceOut (without appling Global volume, because I wish to use this voice as a modullator)
    if (m_voices[nvoice].VoiceOut!=NULL)
    {
      for (i=0;i<SOUND_BUFFER_SIZE;i++)
      {
        m_voices[nvoice].VoiceOut[i] = m_tmpwave[i];
      }
    }

    // Add the voice that do not bypass the filter to out
    if (m_voices[nvoice].filterbypass==0)
    {
      // no bypass

      if (m_stereo)
      {
        // stereo
        for (i=0;i<SOUND_BUFFER_SIZE;i++)
        {
          outl[i]+=m_tmpwave[i]*m_voices[nvoice].Volume*m_voices[nvoice].Panning*2.0;
          outr[i]+=m_tmpwave[i]*m_voices[nvoice].Volume*(1.0-m_voices[nvoice].Panning)*2.0;
        }
      }
      else
      {
        // mono
        for (i=0;i<SOUND_BUFFER_SIZE;i++) outl[i]+=m_tmpwave[i]*m_voices[nvoice].Volume;
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
          m_bypassl[i]+=m_tmpwave[i]*m_voices[nvoice].Volume*m_voices[nvoice].Panning*2.0;
          m_bypassr[i]+=m_tmpwave[i]*m_voices[nvoice].Volume*(1.0-m_voices[nvoice].Panning)*2.0;
        }
      }
      else
      {
        // mono
        for (i=0;i<SOUND_BUFFER_SIZE;i++) m_bypassl[i]+=m_tmpwave[i]*m_voices[nvoice].Volume;
      }
    }
    // check if there is necesary to proces the voice longer (if the Amplitude envelope isn't finished)
    if (m_synth_ptr->voices_params[nvoice].PAmpEnvelopeEnabled)
    {
      if (m_voices[nvoice].m_amplitude_envelope.finished())
      {
        KillVoice(nvoice);
      }
    }
  }


  // Processing Global parameters
  m_filter_left.filterout(&outl[0]);

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
    m_filter_right.filterout(&outr[0]);
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
      outl[i] *= tmpvol * (1.0 - m_panning);
      outr[i] *= tmpvol * m_panning;
    }
  }
  else
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      outl[i] *= globalnewamplitude * (1.0 - m_panning);
      outr[i] *= globalnewamplitude * m_panning;
    }
  }

  // Apply the punch
  if (m_punch_enabled)
  {
    for (i=0;i<SOUND_BUFFER_SIZE;i++)
    {
      REALTYPE punchamp = m_punch_initial_value * m_punch_t + 1.0;
      outl[i] *= punchamp;
      outr[i] *= punchamp;
      m_punch_t -= m_punch_duration;
      if (m_punch_t < 0.0)
      {
        m_punch_enabled = false;
        break;
      }
    }
  }

  // Check if the global amplitude is finished.
  // If it does, disable the note
  if (m_amplitude_envelope.finished())
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      // fade-out

      REALTYPE tmp = 1.0 - (REALTYPE)i / (REALTYPE)SOUND_BUFFER_SIZE;

      outl[i] *= tmp;
      outr[i] *= tmp;
    }

    KillNote();
  }
}

/*
 * Relase the key (NoteOff)
 */
void ADnote::relasekey()
{
  int nvoice;

  for (nvoice = 0 ; nvoice < NUM_VOICES ; nvoice++)
  {
    if (!m_voices[nvoice].enabled)
    {
      continue;
    }

    if (m_synth_ptr->voices_params[nvoice].PAmpEnvelopeEnabled)
    {
      m_voices[nvoice].m_amplitude_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params[nvoice].PFreqEnvelopeEnabled)
    {
      m_voices[nvoice].m_frequency_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params[nvoice].PFilterEnvelopeEnabled)
    {
      m_voices[nvoice].m_filter_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params[nvoice].PFMFreqEnvelopeEnabled)
    {
      m_voices[nvoice].m_fm_frequency_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params[nvoice].PFMAmpEnvelopeEnabled)
    {
      m_voices[nvoice].m_fm_amplitude_envelope.relasekey();
    }
  }

  m_frequency_envelope.relasekey();
  m_filter_envelope.relasekey();
  m_amplitude_envelope.relasekey();
}

/*
 * Check if the note is finished
 */
bool
ADnote::finished()
{
  return !m_note_enabled;
}
