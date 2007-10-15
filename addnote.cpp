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
#include "lfo.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "filter.h"
#include "envelope.h"
#include "addsynth.h"
#include "portamento.h"
#include "addsynth_internal.h"
#include "addsynth_voice.h"
#include "addnote.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

ADnote::ADnote(
  struct zyn_addsynth * synth_ptr)
{
  m_tmpwave = new REALTYPE [SOUND_BUFFER_SIZE];
  m_bypassl = new REALTYPE [SOUND_BUFFER_SIZE];
  m_bypassr = new REALTYPE [SOUND_BUFFER_SIZE];

  m_voices_ptr = (struct addsynth_voice *)malloc(sizeof(struct addsynth_voice) * synth_ptr->voices_count);

  m_osc_pos_hi_ptr = (int *)malloc(sizeof(int) * synth_ptr->voices_count);
  m_osc_pos_lo_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  m_osc_pos_hi_FM_ptr = (unsigned short int *)malloc(sizeof(unsigned short int) * synth_ptr->voices_count);
  m_osc_pos_lo_FM_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  m_osc_freq_hi_ptr = (int *)malloc(sizeof(int) * synth_ptr->voices_count);
  m_osc_freq_lo_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  m_osc_freq_hi_FM_ptr = (unsigned short int *)malloc(sizeof(unsigned short int) * synth_ptr->voices_count);
  m_osc_freq_lo_FM_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  m_FM_old_smp_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  m_first_tick_ptr = (bool *)malloc(sizeof(bool) * synth_ptr->voices_count);

  m_old_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  m_new_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  m_FM_old_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  m_FM_new_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

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
  unsigned int voice_index;
  unsigned int i;
  int tmp[m_synth_ptr->voices_count];

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
    m_punch_duration = 1.0 / (time * m_synth_ptr->sample_rate * stretch);
  }
  else
  {
    m_punch_enabled = false;
  }

  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    zyn_oscillator_new_rand_seed(
      &m_synth_ptr->voices_params_ptr[voice_index].oscillator,
      rand());
    m_voices_ptr[voice_index].OscilSmp=NULL;
    m_voices_ptr[voice_index].FMSmp=NULL;
    m_voices_ptr[voice_index].VoiceOut=NULL;

    m_voices_ptr[voice_index].FMVoice=-1;

    if (!m_synth_ptr->voices_params_ptr[voice_index].enabled)
    {
      m_voices_ptr[voice_index].enabled = false;
      continue; //the voice is disabled
    }

    m_voices_ptr[voice_index].enabled = true;
    m_voices_ptr[voice_index].fixedfreq = m_synth_ptr->voices_params_ptr[voice_index].Pfixedfreq;
    m_voices_ptr[voice_index].fixedfreqET = m_synth_ptr->voices_params_ptr[voice_index].PfixedfreqET;

    // use the Globalpars.detunetype if the detunetype is 0
    if (m_synth_ptr->voices_params_ptr[voice_index].PDetuneType != 0)
    {
      // coarse detune
      m_voices_ptr[voice_index].Detune =
        getdetune(
          m_synth_ptr->voices_params_ptr[voice_index].PDetuneType,
          m_synth_ptr->voices_params_ptr[voice_index].PCoarseDetune,
          8192);

      // fine detune
      m_voices_ptr[voice_index].FineDetune =
        getdetune(
          m_synth_ptr->voices_params_ptr[voice_index].PDetuneType,
          0,
          m_synth_ptr->voices_params_ptr[voice_index].PDetune);
    }
    else
    {
      m_voices_ptr[voice_index].Detune=getdetune(m_synth_ptr->GlobalPar.PDetuneType
                                            ,m_synth_ptr->voices_params_ptr[voice_index].PCoarseDetune,8192);//coarse detune
      m_voices_ptr[voice_index].FineDetune=getdetune(m_synth_ptr->GlobalPar.PDetuneType
                                                ,0,m_synth_ptr->voices_params_ptr[voice_index].PDetune);//fine detune
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFMDetuneType!=0)
    {
      m_voices_ptr[voice_index].FMDetune = 
        getdetune(
          m_synth_ptr->voices_params_ptr[voice_index].PFMDetuneType,
          m_synth_ptr->voices_params_ptr[voice_index].PFMCoarseDetune,
          m_synth_ptr->voices_params_ptr[voice_index].PFMDetune);
    }
    else
    {
      m_voices_ptr[voice_index].FMDetune = getdetune(
        m_synth_ptr->GlobalPar.PDetuneType,
        m_synth_ptr->voices_params_ptr[voice_index].PFMCoarseDetune,
        m_synth_ptr->voices_params_ptr[voice_index].PFMDetune);
    }

    m_osc_pos_hi_ptr[voice_index] = 0;
    m_osc_pos_lo_ptr[voice_index] = 0.0;
    m_osc_pos_hi_FM_ptr[voice_index] = 0;
    m_osc_pos_lo_FM_ptr[voice_index] = 0.0;

    m_voices_ptr[voice_index].OscilSmp=new REALTYPE[OSCIL_SIZE+OSCIL_SMP_EXTRA_SAMPLES];//the extra points contains the first point

    //Get the voice's oscil or external's voice oscil
    int vc=voice_index;

    if (m_synth_ptr->voices_params_ptr[voice_index].Pextoscil != -1)
    {
      vc = m_synth_ptr->voices_params_ptr[voice_index].Pextoscil;
    }

    if (!random_grouping)
    {
      zyn_oscillator_new_rand_seed(
        &m_synth_ptr->voices_params_ptr[vc].oscillator,
        rand());
    }

    m_osc_pos_hi_ptr[voice_index] =
      zyn_oscillator_get(
        &m_synth_ptr->voices_params_ptr[vc].oscillator,
        m_voices_ptr[voice_index].OscilSmp,
        getvoicebasefreq(voice_index),
        m_synth_ptr->voices_params_ptr[voice_index].resonance);

    //I store the first elments to the last position for speedups
    for (i=0;i<OSCIL_SMP_EXTRA_SAMPLES;i++) m_voices_ptr[voice_index].OscilSmp[OSCIL_SIZE+i]=m_voices_ptr[voice_index].OscilSmp[i];

    m_osc_pos_hi_ptr[voice_index] += (int)((m_synth_ptr->voices_params_ptr[voice_index].Poscilphase - 64.0) / 128.0 * OSCIL_SIZE + OSCIL_SIZE * 4);
    m_osc_pos_hi_ptr[voice_index] %= OSCIL_SIZE;

    m_voices_ptr[voice_index].FilterCenterPitch=m_synth_ptr->voices_params_ptr[voice_index].m_filter_params.getfreq();
    m_voices_ptr[voice_index].filterbypass=m_synth_ptr->voices_params_ptr[voice_index].Pfilterbypass;

    m_voices_ptr[voice_index].fm_type = m_synth_ptr->voices_params_ptr[voice_index].fm_type;

    m_voices_ptr[voice_index].FMVoice=m_synth_ptr->voices_params_ptr[voice_index].PFMVoice;

    //Compute the Voice's modulator volume (incl. damping)
    REALTYPE fmvoldamp=pow(440.0/getvoicebasefreq(voice_index),m_synth_ptr->voices_params_ptr[voice_index].PFMVolumeDamp/64.0-1.0);
    switch (m_voices_ptr[voice_index].fm_type)
    {
    case ZYN_FM_TYPE_PHASE_MOD:
      fmvoldamp = pow(440.0/getvoicebasefreq(voice_index),m_synth_ptr->voices_params_ptr[voice_index].PFMVolumeDamp/64.0);
      m_voices_ptr[voice_index].FMVolume = (exp(m_synth_ptr->voices_params_ptr[voice_index].PFMVolume/127.0*FM_AMP_MULTIPLIER)-1.0)*fmvoldamp*4.0;
      break;
    case ZYN_FM_TYPE_FREQ_MOD:
      m_voices_ptr[voice_index].FMVolume = exp(m_synth_ptr->voices_params_ptr[voice_index].PFMVolume / 127.0 * FM_AMP_MULTIPLIER);
      m_voices_ptr[voice_index].FMVolume -= 1.0;
      m_voices_ptr[voice_index].FMVolume *= fmvoldamp * 4.0;
      break;
#if 0                           // ???????????
    case ZYN_FM_TYPE_PITCH_MOD:
      m_voices_ptr[voice_index].FMVolume=(m_synth_ptr->voices_params_ptr[voice_index].PFMVolume/127.0*8.0)*fmvoldamp;
      break;
#endif
    default:
      if (fmvoldamp > 1.0)
      {
        fmvoldamp = 1.0;
      }

      m_voices_ptr[voice_index].FMVolume = m_synth_ptr->voices_params_ptr[voice_index].PFMVolume / 127.0 * fmvoldamp;
    }

    //Voice's modulator velocity sensing
    m_voices_ptr[voice_index].FMVolume*=VelF(m_velocity,m_synth_ptr->voices_params_ptr[voice_index].PFMVelocityScaleFunction);

    m_FM_old_smp_ptr[voice_index] = 0.0; // this is for FM (integration)

    m_first_tick_ptr[voice_index] = true;
    m_voices_ptr[voice_index].DelayTicks=(int)((exp(m_synth_ptr->voices_params_ptr[voice_index].PDelay/127.0*log(50.0))-1.0)/SOUND_BUFFER_SIZE/10.0 * m_synth_ptr->sample_rate);
  }

  // Global Parameters
  m_frequency_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->m_frequency_envelope_params, m_basefreq);

  m_frequency_lfo.init(
    m_synth_ptr->sample_rate,
    m_basefreq,
    &m_synth_ptr->frequency_lfo_params,
    ZYN_LFO_TYPE_FREQUENCY);

  m_amplitude_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->m_amplitude_envelope_params, m_basefreq);

  m_amplitude_lfo.init(
    m_synth_ptr->sample_rate,
    m_basefreq,
    &m_synth_ptr->amplitude_lfo_params,
    ZYN_LFO_TYPE_AMPLITUDE);

  m_volume = 4.0*pow(0.1,3.0*(1.0-m_synth_ptr->GlobalPar.PVolume/96.0))//-60 dB .. 0 dB
    *VelF(m_velocity,m_synth_ptr->GlobalPar.PAmpVelocityScaleFunction);//velocity sensing

  m_amplitude_envelope.envout_dB(); // discard the first envelope output

  globalnewamplitude = m_volume * m_amplitude_envelope.envout_dB() * m_amplitude_lfo.amplfoout();

  m_filter_left.init(m_synth_ptr->sample_rate, &m_synth_ptr->m_filter_params);
  if (m_stereo)
  {
    m_filter_right.init(m_synth_ptr->sample_rate, &m_synth_ptr->m_filter_params);
  }

  m_filter_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->m_filter_envelope_params, m_basefreq);

  m_filter_lfo.init(
    m_synth_ptr->sample_rate,
    m_basefreq,
    &m_synth_ptr->filter_lfo_params,
    ZYN_LFO_TYPE_FILTER);

  m_filter_q_factor = m_synth_ptr->m_filter_params.getq();
  m_filter_frequency_tracking = m_synth_ptr->m_filter_params.getfreqtracking(m_basefreq);

  // Forbids the Modulation Voice to be greater or equal than voice
  for (i = 0 ; i < m_synth_ptr->voices_count ; i++)
  {
    if (m_voices_ptr[i].FMVoice >= (int)i)
    {
      m_voices_ptr[i].FMVoice = -1;
    }
  }

  // Voice Parameter init
  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    if (!m_voices_ptr[voice_index].enabled)
    {
      continue;
    }

    LOG_DEBUG("Starting %s voice (%u, %p)", m_synth_ptr->voices_params_ptr[voice_index].white_noise ? "white noise" : "signal", voice_index, m_synth_ptr->voices_params_ptr + voice_index);

    m_voices_ptr[voice_index].white_noise = m_synth_ptr->voices_params_ptr[voice_index].white_noise;
    /* Voice Amplitude Parameters Init */
    m_voices_ptr[voice_index].Volume=pow(0.1,3.0*(1.0-m_synth_ptr->voices_params_ptr[voice_index].PVolume/127.0)) // -60 dB .. 0 dB
      *VelF(m_velocity,m_synth_ptr->voices_params_ptr[voice_index].PAmpVelocityScaleFunction);//velocity

    if (m_synth_ptr->voices_params_ptr[voice_index].PVolumeminus!=0) m_voices_ptr[voice_index].Volume=-m_voices_ptr[voice_index].Volume;

    if (m_synth_ptr->voices_params_ptr[voice_index].PPanning==0)
    {
      m_voices_ptr[voice_index].Panning = zyn_random(); // random panning
    }
    else
    {
      m_voices_ptr[voice_index].Panning = m_synth_ptr->voices_params_ptr[voice_index].PPanning/128.0;
    }

    m_new_amplitude_ptr[voice_index] = 1.0;
    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled != 0)
    {
      m_voices_ptr[voice_index].m_amplitude_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->voices_params_ptr[voice_index].m_amplitude_envelope_params, m_basefreq);
      m_voices_ptr[voice_index].m_amplitude_envelope.envout_dB(); // discard the first envelope sample
      m_new_amplitude_ptr[voice_index] *= m_voices_ptr[voice_index].m_amplitude_envelope.envout_dB();
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpLfoEnabled != 0)
    {
      m_voices_ptr[voice_index].m_amplitude_lfo.init(
        m_synth_ptr->sample_rate,
        m_basefreq,
        &m_synth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params,
        ZYN_LFO_TYPE_AMPLITUDE);

      m_new_amplitude_ptr[voice_index] *= m_voices_ptr[voice_index].m_amplitude_lfo.amplfoout();
    }

    /* Voice Frequency Parameters Init */
    if (m_synth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled != 0)
    {
      m_voices_ptr[voice_index].m_frequency_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->voices_params_ptr[voice_index].m_frequency_envelope_params, m_basefreq);
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFreqLfoEnabled != 0)
    {
      m_voices_ptr[voice_index].m_frequency_lfo.init(
        m_synth_ptr->sample_rate,
        m_basefreq,
        &m_synth_ptr->voices_params_ptr[voice_index].frequency_lfo_params,
        ZYN_LFO_TYPE_FREQUENCY);
    }

    /* Voice Filter Parameters Init */
    if (m_synth_ptr->voices_params_ptr[voice_index].PFilterEnabled != 0)
    {
      m_voices_ptr[voice_index].m_voice_filter.init(m_synth_ptr->sample_rate, &m_synth_ptr->voices_params_ptr[voice_index].m_filter_params);
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled != 0)
    {
      m_voices_ptr[voice_index].m_filter_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->voices_params_ptr[voice_index].m_filter_envelope_params, m_basefreq);
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFilterLfoEnabled != 0)
    {
      m_voices_ptr[voice_index].m_filter_lfo.init(
        m_synth_ptr->sample_rate,
        m_basefreq,
        &m_synth_ptr->voices_params_ptr[voice_index].filter_lfo_params,
        ZYN_LFO_TYPE_FILTER);
    }

    m_voices_ptr[voice_index].FilterFreqTracking = m_synth_ptr->voices_params_ptr[voice_index].m_filter_params.getfreqtracking(m_basefreq);

    /* Voice Modulation Parameters Init */
    if (m_voices_ptr[voice_index].fm_type != ZYN_FM_TYPE_NONE && m_voices_ptr[voice_index].FMVoice < 0)
    {
      zyn_oscillator_new_rand_seed(
        &m_synth_ptr->voices_params_ptr[voice_index].modulator_oscillator,
        rand());

      m_voices_ptr[voice_index].FMSmp = new zyn_sample_type[OSCIL_SIZE + OSCIL_SMP_EXTRA_SAMPLES];

      //Perform Anti-aliasing only on MORPH or RING MODULATION

      int vc=voice_index;
      if (m_synth_ptr->voices_params_ptr[voice_index].PextFMoscil!=-1) vc=m_synth_ptr->voices_params_ptr[voice_index].PextFMoscil;

      REALTYPE tmp=1.0;
      if ((m_synth_ptr->voices_params_ptr[vc].modulator_oscillator.Padaptiveharmonics != 0) ||
          (m_voices_ptr[voice_index].fm_type == ZYN_FM_TYPE_MORPH) ||
          (m_voices_ptr[voice_index].fm_type == ZYN_FM_TYPE_RING_MOD))
      {
        tmp = getFMvoicebasefreq(voice_index);
      }

      if (!random_grouping)
      {
        zyn_oscillator_new_rand_seed(
          &m_synth_ptr->voices_params_ptr[vc].modulator_oscillator,
          rand());
      }

      m_osc_pos_hi_FM_ptr[voice_index] = m_osc_pos_hi_ptr[voice_index];
      m_osc_pos_hi_FM_ptr[voice_index] += zyn_oscillator_get(
        &m_synth_ptr->voices_params_ptr[vc].modulator_oscillator,
        m_voices_ptr[voice_index].FMSmp,
        tmp,
        false);
      m_osc_pos_hi_FM_ptr[voice_index] %= OSCIL_SIZE;

      for (int i=0;i<OSCIL_SMP_EXTRA_SAMPLES;i++) m_voices_ptr[voice_index].FMSmp[OSCIL_SIZE+i]=m_voices_ptr[voice_index].FMSmp[i];
      m_osc_pos_hi_FM_ptr[voice_index]+=(int)((m_synth_ptr->voices_params_ptr[voice_index].PFMoscilphase-64.0)/128.0*OSCIL_SIZE+OSCIL_SIZE*4);
      m_osc_pos_hi_FM_ptr[voice_index]%=OSCIL_SIZE;
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled != 0)
    {
      m_voices_ptr[voice_index].m_fm_frequency_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->voices_params_ptr[voice_index].m_fm_frequency_envelope_params, m_basefreq);
    }

    m_FM_new_amplitude_ptr[voice_index] = m_voices_ptr[voice_index].FMVolume;
    //m_FM_new_amplitude_ptr[voice_index] *= m_ctl->fmamp.relamp; // 0..1

    if (m_synth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled != 0)
    {
      m_voices_ptr[voice_index].m_fm_amplitude_envelope.init(m_synth_ptr->sample_rate, &m_synth_ptr->voices_params_ptr[voice_index].m_fm_amplitude_envelope_params, m_basefreq);
      m_FM_new_amplitude_ptr[voice_index] *= m_voices_ptr[voice_index].m_fm_amplitude_envelope.envout_dB();
    }
  }

  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    for (i = voice_index + 1 ; i < m_synth_ptr->voices_count ; i++)
    {
      tmp[i] = 0;
    }

    for (i = voice_index + 1 ; i < m_synth_ptr->voices_count ; i++)
    {
      if (m_voices_ptr[i].FMVoice == (int)voice_index && tmp[i] == 0)
      {
        m_voices_ptr[voice_index].VoiceOut = new REALTYPE[SOUND_BUFFER_SIZE];
        tmp[i] = 1;
      }
    }

    if (m_voices_ptr[voice_index].VoiceOut != NULL)
    {
      for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
      {
        m_voices_ptr[voice_index].VoiceOut[i] = 0.0;
      }
    }
  }
}

/*
 * Kill a voice of ADnote
 */
void ADnote::KillVoice(unsigned int voice_index)
{
  int i;
  delete [] (m_voices_ptr[voice_index].OscilSmp);

  if ((m_voices_ptr[voice_index].fm_type != ZYN_FM_TYPE_NONE) && (m_voices_ptr[voice_index].FMVoice < 0))
  {
    delete m_voices_ptr[voice_index].FMSmp;
  }

  if (m_voices_ptr[voice_index].VoiceOut != NULL)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      m_voices_ptr[voice_index].VoiceOut[i] = 0.0;//do not delete, yet: perhaps is used by another voice
    }
  }

  m_voices_ptr[voice_index].enabled = false;
}

/*
 * Kill the note
 */
void
ADnote::KillNote()
{
  unsigned int voice_index;

  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    if (m_voices_ptr[voice_index].enabled)
    {
      KillVoice(voice_index);
    }

    // delete VoiceOut
    if (m_voices_ptr[voice_index].VoiceOut != NULL)
    {
      delete(m_voices_ptr[voice_index].VoiceOut);
      m_voices_ptr[voice_index].VoiceOut = NULL;
    }
  }

  m_note_enabled = false;
}

ADnote::~ADnote()
{
  if (m_note_enabled)
  {
    KillNote();
  }

  free(m_old_amplitude_ptr);
  free(m_new_amplitude_ptr);

  free(m_FM_old_amplitude_ptr);
  free(m_FM_new_amplitude_ptr);

  free(m_first_tick_ptr);

  free(m_FM_old_smp_ptr);

  free(m_osc_freq_hi_ptr);
  free(m_osc_freq_lo_ptr);
  free(m_osc_freq_hi_FM_ptr);
  free(m_osc_freq_lo_FM_ptr);

  free(m_osc_pos_hi_ptr);
  free(m_osc_pos_lo_ptr);
  free(m_osc_pos_hi_FM_ptr);
  free(m_osc_pos_lo_FM_ptr);

  free(m_voices_ptr);

  delete [] m_tmpwave;
  delete [] m_bypassl;
  delete [] m_bypassr;
}

/*
 * Computes the frequency of an oscillator
 */
void ADnote::setfreq(int nvoice,REALTYPE freq)
{
  REALTYPE speed;

  freq = fabs(freq);

  speed = freq * REALTYPE(OSCIL_SIZE) / m_synth_ptr->sample_rate;
  if (speed > OSCIL_SIZE)
  {
    speed = OSCIL_SIZE;
  }

  F2I(speed, m_osc_freq_hi_ptr[nvoice]);

  m_osc_freq_lo_ptr[nvoice] = speed - floor(speed);
}

/*
 * Computes the frequency of an modullator oscillator
 */
void ADnote::setfreqFM(int nvoice,REALTYPE freq)
{
  REALTYPE speed;

  freq = fabs(freq);

  speed = freq * REALTYPE(OSCIL_SIZE) / m_synth_ptr->sample_rate;
  if (speed > OSCIL_SIZE)
  {
    speed = OSCIL_SIZE;
  }

  F2I(speed, m_osc_freq_hi_FM_ptr[nvoice]);
  m_osc_freq_lo_FM_ptr[nvoice] = speed - floor(speed);
}

/*
 * Get Voice base frequency
 */
REALTYPE ADnote::getvoicebasefreq(int nvoice)
{
  REALTYPE detune;

  detune = m_voices_ptr[nvoice].Detune / 100.0;
  detune += m_voices_ptr[nvoice].FineDetune / 100.0 * m_synth_ptr->bandwidth_relbw * m_bandwidth_detune_multiplier;
  detune += m_detune / 100.0;

  if (m_voices_ptr[nvoice].fixedfreq == 0)
  {
    return m_basefreq * pow(2, detune / 12.0);
  }
  else
  {
    // the fixed freq is enabled
    REALTYPE fixedfreq = 440.0;
    int fixedfreqET = m_voices_ptr[nvoice].fixedfreqET;
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
  REALTYPE detune=m_voices_ptr[nvoice].FMDetune/100.0;
  return(getvoicebasefreq(nvoice)*pow(2,detune/12.0));
}

/*
 * Computes all the parameters for each tick
 */
void
ADnote::computecurrentparameters()
{
  unsigned int voice_index;
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
            m_frequency_lfo.lfoout() * m_synth_ptr->modwheel_relmod);

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
    //m_ctl->filtercutoff.relfreq + // (value-64.0)*filtercutoff.depth/4096.0*3.321928;//3.3219..=ln2(10)
    m_filter_frequency_tracking;

  temp_filter_frequency = m_filter_left.getrealfreq(temp_filter_frequency);

  global_filter_q = m_filter_q_factor;
  //global_filter_q *= m_ctl->filterq.relq; // filterq.relq=pow(30.0,(value-64.0)/64.0*(filterq.depth/64.0));

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
    portamentofreqrap = m_synth_ptr->portamento.freqrap;

    if (!m_synth_ptr->portamento.used)
    {
      // the portamento has finished
      m_portamento = false;     // this note is no longer "portamented"
    }
  }

  //compute parameters for all voices
  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    if (!m_voices_ptr[voice_index].enabled)
    {
      continue;
    }

    m_voices_ptr[voice_index].DelayTicks -= 1;

    if (m_voices_ptr[voice_index].DelayTicks > 0)
    {
      continue;
    }

    /*******************/
    /* Voice Amplitude */
    /*******************/
    m_old_amplitude_ptr[voice_index] = m_new_amplitude_ptr[voice_index];
    m_new_amplitude_ptr[voice_index] = 1.0;

    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      m_new_amplitude_ptr[voice_index] *= m_voices_ptr[voice_index].m_amplitude_envelope.envout_dB();
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpLfoEnabled)
    {
      m_new_amplitude_ptr[voice_index] *= m_voices_ptr[voice_index].m_amplitude_lfo.amplfoout();
    }

    /****************/
    /* Voice Filter */
    /****************/
    if (m_synth_ptr->voices_params_ptr[voice_index].PFilterEnabled)
    {
      filterpitch = m_voices_ptr[voice_index].FilterCenterPitch;

      if (m_synth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled)
      {
        filterpitch += m_voices_ptr[voice_index].m_filter_envelope.envout();
      }

      if (m_synth_ptr->voices_params_ptr[voice_index].PFilterLfoEnabled)
      {
        filterpitch += m_voices_ptr[voice_index].m_filter_lfo.lfoout();
      }

      filterfreq = filterpitch + m_voices_ptr[voice_index].FilterFreqTracking;
      filterfreq = m_voices_ptr[voice_index].m_voice_filter.getrealfreq(filterfreq);

      m_voices_ptr[voice_index].m_voice_filter.setfreq(filterfreq);
    }

    // compute only if the voice isn't noise
    if (!m_voices_ptr[voice_index].white_noise)
    {
      /*******************/
      /* Voice Frequency */
      /*******************/
      voicepitch=0.0;
      if (m_synth_ptr->voices_params_ptr[voice_index].PFreqLfoEnabled)
      {
        voicepitch += m_voices_ptr[voice_index].m_frequency_lfo.lfoout() / 100.0 * m_synth_ptr->bandwidth_relbw;
      }

      if (m_synth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled)
      {
        voicepitch += m_voices_ptr[voice_index].m_frequency_envelope.envout() / 100.0;
      }

      voicefreq = getvoicebasefreq(voice_index) * pow(2, (voicepitch + globalpitch) / 12.0); // Hz frequency
      voicefreq *= m_synth_ptr->pitch_bend_relative_frequency; // change the frequency by the controller
      setfreq(voice_index, voicefreq * portamentofreqrap);

      /***************/
      /*  Modulator */
      /***************/
      if (m_voices_ptr[voice_index].fm_type != ZYN_FM_TYPE_NONE)
      {
        FMrelativepitch = m_voices_ptr[voice_index].FMDetune / 100.0;
        if (m_synth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled)
        {
          FMrelativepitch += m_voices_ptr[voice_index].m_fm_frequency_envelope.envout() / 100;
        }

        FMfreq = pow(2.0, FMrelativepitch / 12.0) * voicefreq * portamentofreqrap;
        setfreqFM(voice_index, FMfreq);

        m_FM_old_amplitude_ptr[voice_index] = m_FM_new_amplitude_ptr[voice_index];
        m_FM_new_amplitude_ptr[voice_index] = m_voices_ptr[voice_index].FMVolume;
        //m_FM_new_amplitude_ptr[voice_index] *= m_ctl->fmamp.relamp; // 0..1

        if (m_synth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled)
        {
          m_FM_new_amplitude_ptr[voice_index] *= m_voices_ptr[voice_index].m_fm_amplitude_envelope.envout_dB();
        }
      }
    }
  }

  m_time += (REALTYPE)SOUND_BUFFER_SIZE / m_synth_ptr->sample_rate;
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
inline void ADnote::ComputeVoiceOscillator_LinearInterpolation(int voice_index){
  int i,poshi;
  REALTYPE poslo;

  poshi=m_osc_pos_hi_ptr[voice_index];
  poslo=m_osc_pos_lo_ptr[voice_index];
  REALTYPE *smps=m_voices_ptr[voice_index].OscilSmp;
  for (i=0;i<SOUND_BUFFER_SIZE;i++){
    m_tmpwave[i]=smps[poshi]*(1.0-poslo)+smps[poshi+1]*poslo;
    poslo+=m_osc_freq_lo_ptr[voice_index];
    if (poslo>=1.0) {
      poslo-=1.0;
      poshi++;
    }
    poshi+=m_osc_freq_hi_ptr[voice_index];
    poshi&=OSCIL_SIZE-1;
  }
  m_osc_pos_hi_ptr[voice_index]=poshi;
  m_osc_pos_lo_ptr[voice_index]=poslo;
}



/*
 * Computes the Oscillator (Without Modulation) - CubicInterpolation
 *
 The differences from the Linear are to little to deserve to be used. This is because I am using a large OSCIL_SIZE (>512)
 inline void ADnote::ComputeVoiceOscillator_CubicInterpolation(int voice_index){
 int i,poshi;
 REALTYPE poslo;

 poshi=m_osc_pos_hi_ptr[voice_index];
 poslo=m_osc_pos_lo_ptr[voice_index];
 REALTYPE *smps=m_voices_ptr[voice_index].OscilSmp;
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
 poslo+=m_osc_freq_lo_ptr[voice_index];
 if (poslo>=1.0) {
 poslo-=1.0;
 poshi++;
 }
 poshi+=m_osc_freq_hi_ptr[voice_index];
 poshi&=OSCIL_SIZE-1;
 }
 m_osc_pos_hi_ptr[voice_index]=poshi;
 m_osc_pos_lo_ptr[voice_index]=poslo;
 }
*/
/*
 * Computes the Oscillator (Morphing)
 */
inline void ADnote::ComputeVoiceOscillatorMorph(int voice_index){
  int i;
  REALTYPE amp;
  ComputeVoiceOscillator_LinearInterpolation(voice_index);
  if (m_FM_new_amplitude_ptr[voice_index]>1.0) m_FM_new_amplitude_ptr[voice_index]=1.0;
  if (m_FM_old_amplitude_ptr[voice_index]>1.0) m_FM_old_amplitude_ptr[voice_index]=1.0;

  if (m_voices_ptr[voice_index].FMVoice>=0){
    //if I use VoiceOut[] as modullator
    int FMVoice=m_voices_ptr[voice_index].FMVoice;
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {
      amp=INTERPOLATE_AMPLITUDE(m_FM_old_amplitude_ptr[voice_index]
                                ,m_FM_new_amplitude_ptr[voice_index],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]=m_tmpwave[i]*(1.0-amp)+amp*m_voices_ptr[FMVoice].VoiceOut[i];
    }
  } else {
    int poshiFM=m_osc_pos_hi_FM_ptr[voice_index];
    REALTYPE posloFM=m_osc_pos_lo_FM_ptr[voice_index];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      amp=INTERPOLATE_AMPLITUDE(m_FM_old_amplitude_ptr[voice_index]
                                ,m_FM_new_amplitude_ptr[voice_index],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]=m_tmpwave[i]*(1.0-amp)+amp
        *(m_voices_ptr[voice_index].FMSmp[poshiFM]*(1-posloFM)
          +m_voices_ptr[voice_index].FMSmp[poshiFM+1]*posloFM);
      posloFM+=m_osc_freq_lo_FM_ptr[voice_index];
      if (posloFM>=1.0) {
        posloFM-=1.0;
        poshiFM++;
      }
      poshiFM+=m_osc_freq_hi_FM_ptr[voice_index];
      poshiFM&=OSCIL_SIZE-1;
    }
    m_osc_pos_hi_FM_ptr[voice_index]=poshiFM;
    m_osc_pos_lo_FM_ptr[voice_index]=posloFM;
  }
}

/*
 * Computes the Oscillator (Ring Modulation)
 */
inline void ADnote::ComputeVoiceOscillatorRingModulation(int voice_index){
  int i;
  REALTYPE amp;
  ComputeVoiceOscillator_LinearInterpolation(voice_index);
  if (m_FM_new_amplitude_ptr[voice_index]>1.0) m_FM_new_amplitude_ptr[voice_index]=1.0;
  if (m_FM_old_amplitude_ptr[voice_index]>1.0) m_FM_old_amplitude_ptr[voice_index]=1.0;
  if (m_voices_ptr[voice_index].FMVoice>=0){
    // if I use VoiceOut[] as modullator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) {
      amp=INTERPOLATE_AMPLITUDE(m_FM_old_amplitude_ptr[voice_index]
                                ,m_FM_new_amplitude_ptr[voice_index],i,SOUND_BUFFER_SIZE);
      int FMVoice=m_voices_ptr[voice_index].FMVoice;
      for (i=0;i<SOUND_BUFFER_SIZE;i++)
        m_tmpwave[i]*=(1.0-amp)+amp*m_voices_ptr[FMVoice].VoiceOut[i];
    }
  } else {
    int poshiFM=m_osc_pos_hi_FM_ptr[voice_index];
    REALTYPE posloFM=m_osc_pos_lo_FM_ptr[voice_index];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      amp=INTERPOLATE_AMPLITUDE(m_FM_old_amplitude_ptr[voice_index]
                                ,m_FM_new_amplitude_ptr[voice_index],i,SOUND_BUFFER_SIZE);
      m_tmpwave[i]*=( m_voices_ptr[voice_index].FMSmp[poshiFM]*(1.0-posloFM)
                    +m_voices_ptr[voice_index].FMSmp[poshiFM+1]*posloFM)*amp
        +(1.0-amp);
      posloFM+=m_osc_freq_lo_FM_ptr[voice_index];
      if (posloFM>=1.0) {
        posloFM-=1.0;
        poshiFM++;
      }
      poshiFM+=m_osc_freq_hi_FM_ptr[voice_index];
      poshiFM&=OSCIL_SIZE-1;
    }
    m_osc_pos_hi_FM_ptr[voice_index]=poshiFM;
    m_osc_pos_lo_FM_ptr[voice_index]=posloFM;
  }
}



/*
 * Computes the Oscillator (Phase Modulation or Frequency Modulation)
 */
inline void ADnote::ComputeVoiceOscillatorFrequencyModulation(int voice_index,int FMmode){
  int carposhi;
  int i,FMmodfreqhi;
  REALTYPE FMmodfreqlo,carposlo;

  if (m_voices_ptr[voice_index].FMVoice>=0){
    //if I use VoiceOut[] as modulator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]=m_voices_ptr[m_voices_ptr[voice_index].FMVoice].VoiceOut[i];
  } else {
    //Compute the modulator and store it in m_tmpwave[]
    int poshiFM=m_osc_pos_hi_FM_ptr[voice_index];
    REALTYPE posloFM=m_osc_pos_lo_FM_ptr[voice_index];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      m_tmpwave[i]=(m_voices_ptr[voice_index].FMSmp[poshiFM]*(1.0-posloFM)
                  +m_voices_ptr[voice_index].FMSmp[poshiFM+1]*posloFM);
      posloFM+=m_osc_freq_lo_FM_ptr[voice_index];
      if (posloFM>=1.0) {
        posloFM=fmod(posloFM,1.0);
        poshiFM++;
      }
      poshiFM+=m_osc_freq_hi_FM_ptr[voice_index];
      poshiFM&=OSCIL_SIZE-1;
    }
    m_osc_pos_hi_FM_ptr[voice_index]=poshiFM;
    m_osc_pos_lo_FM_ptr[voice_index]=posloFM;
  }
  // Amplitude interpolation
  if (ABOVE_AMPLITUDE_THRESHOLD(m_FM_old_amplitude_ptr[voice_index],m_FM_new_amplitude_ptr[voice_index])){
    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      m_tmpwave[i]*=INTERPOLATE_AMPLITUDE(m_FM_old_amplitude_ptr[voice_index]
                                        ,m_FM_new_amplitude_ptr[voice_index],i,SOUND_BUFFER_SIZE);
    }
  } else for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]*=m_FM_new_amplitude_ptr[voice_index];


  //normalize makes all sample-rates, oscil_sizes toproduce same sound
  if (FMmode!=0){//Frequency modulation
    REALTYPE normalize = OSCIL_SIZE / 262144.0 * 44100.0 / m_synth_ptr->sample_rate;
    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      m_FM_old_smp_ptr[voice_index]=fmod(m_FM_old_smp_ptr[voice_index]+m_tmpwave[i]*normalize,OSCIL_SIZE);
      m_tmpwave[i]=m_FM_old_smp_ptr[voice_index];
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
    carposhi=m_osc_pos_hi_ptr[voice_index]+FMmodfreqhi;
    carposlo=m_osc_pos_lo_ptr[voice_index]+FMmodfreqlo;

    if (carposlo>=1.0) {
      carposhi++;
      carposlo=fmod(carposlo,1.0);
    }
    carposhi&=(OSCIL_SIZE-1);

    m_tmpwave[i]=m_voices_ptr[voice_index].OscilSmp[carposhi]*(1.0-carposlo)
      +m_voices_ptr[voice_index].OscilSmp[carposhi+1]*carposlo;

    m_osc_pos_lo_ptr[voice_index]+=m_osc_freq_lo_ptr[voice_index];
    if (m_osc_pos_lo_ptr[voice_index]>=1.0) {
      m_osc_pos_lo_ptr[voice_index]=fmod(m_osc_pos_lo_ptr[voice_index],1.0);
      m_osc_pos_hi_ptr[voice_index]++;
    }

    m_osc_pos_hi_ptr[voice_index]+=m_osc_freq_hi_ptr[voice_index];
    m_osc_pos_hi_ptr[voice_index]&=OSCIL_SIZE-1;
  }
}


/*Calculeaza Oscilatorul cu PITCH MODULATION*/
inline void ADnote::ComputeVoiceOscillatorPitchModulation(int voice_index){
//TODO
}

/*
 * Computes the Noise
 */
inline void ADnote::ComputeVoiceNoise(int voice_index){

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
  int i;
  unsigned int voice_index;

  silence_two_buffers(outl, outr, SOUND_BUFFER_SIZE);

  if (!m_note_enabled)
  {
    return;
  }

  silence_two_buffers(m_bypassl, m_bypassr, SOUND_BUFFER_SIZE);

  computecurrentparameters();

  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    if (!m_voices_ptr[voice_index].enabled || m_voices_ptr[voice_index].DelayTicks > 0)
    {
      continue;
    }

    if (!m_voices_ptr[voice_index].white_noise) //voice mode = sound
    {
      switch (m_voices_ptr[voice_index].fm_type)
      {
      case ZYN_FM_TYPE_MORPH:
        ComputeVoiceOscillatorMorph(voice_index);
        break;
      case ZYN_FM_TYPE_RING_MOD:
        ComputeVoiceOscillatorRingModulation(voice_index);
        break;
      case ZYN_FM_TYPE_PHASE_MOD:
        ComputeVoiceOscillatorFrequencyModulation(voice_index,0);
        break;
      case ZYN_FM_TYPE_FREQ_MOD:
        ComputeVoiceOscillatorFrequencyModulation(voice_index,1);
        break;
#if 0
      case ZYN_FM_TYPE_PITCH_MOD:
        ComputeVoiceOscillatorPitchModulation(voice_index);
        break;
#endif
      default:
        ComputeVoiceOscillator_LinearInterpolation(voice_index);
        //if (config.cfg.Interpolation) ComputeVoiceOscillator_CubicInterpolation(voice_index);
      }
    }
    else
    {
      ComputeVoiceNoise(voice_index);
    }

    // Voice Processing

    // Amplitude
    if (ABOVE_AMPLITUDE_THRESHOLD(m_old_amplitude_ptr[voice_index],m_new_amplitude_ptr[voice_index])){
      int rest=SOUND_BUFFER_SIZE;
      //test if the amplitude if raising and the difference is high
      if ((m_new_amplitude_ptr[voice_index]>m_old_amplitude_ptr[voice_index])&&((m_new_amplitude_ptr[voice_index]-m_old_amplitude_ptr[voice_index])>0.25)){
        rest=10;
        if (rest>SOUND_BUFFER_SIZE) rest=SOUND_BUFFER_SIZE;
        for (int i=0;i<SOUND_BUFFER_SIZE-rest;i++) m_tmpwave[i]*=m_old_amplitude_ptr[voice_index];
      }
      // Amplitude interpolation
      for (i=0;i<rest;i++){
        m_tmpwave[i+(SOUND_BUFFER_SIZE-rest)]*=INTERPOLATE_AMPLITUDE(m_old_amplitude_ptr[voice_index]
                                                                   ,m_new_amplitude_ptr[voice_index],i,rest);
      }
    } else for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpwave[i]*=m_new_amplitude_ptr[voice_index];

    // Fade in
    if (m_first_tick_ptr[voice_index])
    {
      fadein(&m_tmpwave[0]);
      m_first_tick_ptr[voice_index] = false;
    }

    // Filter
    if (m_synth_ptr->voices_params_ptr[voice_index].PFilterEnabled)
    {
      m_voices_ptr[voice_index].m_voice_filter.filterout(&m_tmpwave[0]);
    }

    //check if the amplitude envelope is finished, if yes, the voice will be fadeout
    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      if (m_voices_ptr[voice_index].m_amplitude_envelope.finished())
      {
        for (i=0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          m_tmpwave[i] *= 1.0 - (REALTYPE)i / (REALTYPE)SOUND_BUFFER_SIZE;
        }
      }

      // the voice is killed later
    }


    // Put the ADnote samples in VoiceOut (without appling Global volume, because I wish to use this voice as a modullator)
    if (m_voices_ptr[voice_index].VoiceOut!=NULL)
    {
      for (i=0;i<SOUND_BUFFER_SIZE;i++)
      {
        m_voices_ptr[voice_index].VoiceOut[i] = m_tmpwave[i];
      }
    }

    // Add the voice that do not bypass the filter to out
    if (m_voices_ptr[voice_index].filterbypass==0)
    {
      // no bypass

      if (m_stereo)
      {
        // stereo
        for (i=0;i<SOUND_BUFFER_SIZE;i++)
        {
          outl[i]+=m_tmpwave[i]*m_voices_ptr[voice_index].Volume*m_voices_ptr[voice_index].Panning*2.0;
          outr[i]+=m_tmpwave[i]*m_voices_ptr[voice_index].Volume*(1.0-m_voices_ptr[voice_index].Panning)*2.0;
        }
      }
      else
      {
        // mono
        for (i=0;i<SOUND_BUFFER_SIZE;i++) outl[i]+=m_tmpwave[i]*m_voices_ptr[voice_index].Volume;
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
          m_bypassl[i]+=m_tmpwave[i]*m_voices_ptr[voice_index].Volume*m_voices_ptr[voice_index].Panning*2.0;
          m_bypassr[i]+=m_tmpwave[i]*m_voices_ptr[voice_index].Volume*(1.0-m_voices_ptr[voice_index].Panning)*2.0;
        }
      }
      else
      {
        // mono
        for (i=0;i<SOUND_BUFFER_SIZE;i++) m_bypassl[i]+=m_tmpwave[i]*m_voices_ptr[voice_index].Volume;
      }
    }
    // check if there is necesary to proces the voice longer (if the Amplitude envelope isn't finished)
    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      if (m_voices_ptr[voice_index].m_amplitude_envelope.finished())
      {
        KillVoice(voice_index);
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
  unsigned int voice_index;

  for (voice_index = 0 ; voice_index < m_synth_ptr->voices_count ; voice_index++)
  {
    if (!m_voices_ptr[voice_index].enabled)
    {
      continue;
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      m_voices_ptr[voice_index].m_amplitude_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled)
    {
      m_voices_ptr[voice_index].m_frequency_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled)
    {
      m_voices_ptr[voice_index].m_filter_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled)
    {
      m_voices_ptr[voice_index].m_fm_frequency_envelope.relasekey();
    }

    if (m_synth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled)
    {
      m_voices_ptr[voice_index].m_fm_amplitude_envelope.relasekey();
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
