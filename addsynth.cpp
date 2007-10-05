/* -*- Mode: C++ ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *****************************************************************************/

#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "common.h"
#include "addsynth.h"
#include "globals.h"
#include "resonance.h"
#include "fft.h"
#include "oscillator.h"
#include "envelope_parameters.h"
#include "envelope.h"
#include "lfo_parameters.h"
#include "lfo.h"
#include "filter_parameters.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "filter.h"
#include "Controller.h"
#include "addsynth_internal.h"
#include "addsynth_voice.h"
#include "addnote.h"
#include "util.h"
#include "log.h"

#define ZYN_DEFAULT_POLYPHONY 60

bool
zyn_addsynth_create(
  unsigned int voices_count,
  zyn_addsynth_handle * handle_ptr)
{
  struct zyn_addsynth * zyn_addsynth_ptr;
  unsigned int note_index;
  unsigned int voice_index;

//  printf("zyn_addsynth_create\n");
  zyn_addsynth_ptr = (struct zyn_addsynth *)malloc(sizeof(struct zyn_addsynth));

  zyn_addsynth_ptr->polyphony = ZYN_DEFAULT_POLYPHONY;
  zyn_addsynth_ptr->notes_array = (struct note_channel *)malloc(ZYN_DEFAULT_POLYPHONY * sizeof(struct note_channel));

  zyn_addsynth_ptr->velsns = 64;
  zyn_addsynth_ptr->fft = zyn_fft_create(OSCIL_SIZE);

  // ADnoteParameters temp begin

  zyn_addsynth_ptr->m_frequency_envelope_params.init_asr(0, false, 64, 50, 64, 60);
    
  zyn_addsynth_ptr->m_amplitude_envelope_params.init_adsr(64, true, 0, 40, 127, 25, false);

  zyn_addsynth_ptr->m_filter_params.init(2, 94, 40);
  zyn_addsynth_ptr->m_filter_envelope_params.init_adsr_filter(0, true, 64, 40, 64, 70, 60, 64);
  zyn_addsynth_ptr->GlobalPar.Reson=new Resonance();

  zyn_addsynth_ptr->voices_count = voices_count;
  zyn_addsynth_ptr->voices_params_ptr = (struct ADnoteVoiceParam *)malloc(sizeof(struct ADnoteVoiceParam) * voices_count);

  for (voice_index = 0 ; voice_index < voices_count ; voice_index++)
  {
    zyn_addsynth_ptr->voices_params_ptr[voice_index].OscilSmp =
      new OscilGen(zyn_addsynth_ptr->fft, zyn_addsynth_ptr->GlobalPar.Reson);
    zyn_addsynth_ptr->voices_params_ptr[voice_index].FMSmp =
      new OscilGen(zyn_addsynth_ptr->fft, NULL);

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_amplitude_envelope_params.init_adsr(64, true, 0, 100, 127, 100, false); 

    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.frequency = 90.0 / 127.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.depth = 32.0 / 127.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.random_start_phase = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.start_phase = 0.5;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.depth_randomness_enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.depth = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.frequency_randomness_enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.frequency_randomness = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.delay = 30.0 / 127.0 * 4.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.stretch = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params.shape = ZYN_LFO_SHAPE_TYPE_SINE;

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_frequency_envelope_params.init_asr(0, false, 30, 40, 64, 60);

    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.frequency = 50.0 / 127.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.depth = 40.0 / 127.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.random_start_phase = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.start_phase = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.depth_randomness_enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.depth = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.frequency_randomness_enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.frequency_randomness = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.delay = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.stretch = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].frequency_lfo_params.shape = ZYN_LFO_SHAPE_TYPE_SINE;

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_filter_params.init(2, 50, 60);
    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_filter_envelope_params.init_adsr_filter(0, false, 90, 70, 40, 70, 10, 40);

    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.frequency = 50.0 / 127.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.depth = 20.0 / 127.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.random_start_phase = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.start_phase = 0.5;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.depth_randomness_enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.depth = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.frequency_randomness_enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.frequency_randomness = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.delay = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.stretch = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].filter_lfo_params.shape = ZYN_LFO_SHAPE_TYPE_SINE;

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_fm_frequency_envelope_params.init_asr(0, false, 20, 90, 40, 80);
    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_fm_amplitude_envelope_params.init_adsr(64, true, 80, 90, 127, 100, false);
  }
    
  /* Frequency Global Parameters */
//  zyn_addsynth_ptr->GlobalPar.stereo = true;      // Stereo
  zyn_addsynth_ptr->GlobalPar.PDetune=8192;//zero
  zyn_addsynth_ptr->GlobalPar.PCoarseDetune=0;
  zyn_addsynth_ptr->GlobalPar.PDetuneType=1;
  zyn_addsynth_ptr->GlobalPar.PBandwidth=64;
    
  /* Amplitude Global Parameters */
  zyn_addsynth_ptr->GlobalPar.PVolume=90;
  zyn_addsynth_ptr->GlobalPar.PAmpVelocityScaleFunction=64;
  zyn_addsynth_ptr->GlobalPar.PPunchStrength=0;
  zyn_addsynth_ptr->GlobalPar.PPunchTime=60;
  zyn_addsynth_ptr->GlobalPar.PPunchStretch=64;
  zyn_addsynth_ptr->GlobalPar.PPunchVelocitySensing=72;
    
  /* Filter Global Parameters*/
  zyn_addsynth_ptr->m_filter_velocity_sensing_amount = 0.5;
  zyn_addsynth_ptr->m_filter_velocity_scale_function = 0;
  zyn_addsynth_ptr->m_filter_params.defaults();
  zyn_addsynth_ptr->GlobalPar.Reson->defaults();

  for (voice_index=0 ; voice_index < voices_count ; voice_index++)
  {
    zyn_addsynth_ptr->voices_params_ptr[voice_index].enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].Type=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].Pfixedfreq=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PfixedfreqET=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].resonance = true;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].Pfilterbypass=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].Pextoscil=-1;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PextFMoscil=-1;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].Poscilphase=64;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMoscilphase=64;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PDelay=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PVolume=100;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PVolumeminus=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PPanning=64;//center
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PDetune=8192;//8192=0
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PCoarseDetune=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PDetuneType=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFreqLfoEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PAmpLfoEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PAmpVelocityScaleFunction=127;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFilterEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFilterLfoEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fm_type = ZYN_FM_TYPE_NONE;

    //I use the internal oscillator (-1)
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMVoice=-1;

    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMVolume=90;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMVolumeDamp=64;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMDetune=8192;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMCoarseDetune=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMDetuneType=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMVelocityScaleFunction=64; 

    zyn_addsynth_ptr->voices_params_ptr[voice_index].OscilSmp->defaults();
    zyn_addsynth_ptr->voices_params_ptr[voice_index].FMSmp->defaults();

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_filter_params.defaults();
  }

  zyn_addsynth_ptr->voices_params_ptr[0].enabled = true;

  // ADnoteParameters temp end

  zyn_addsynth_ptr->ctl_ptr = new Controller;
  zyn_addsynth_ptr->ctl_ptr->defaults();
  zyn_addsynth_ptr->oldfreq = -1.0;

  zyn_addsynth_ptr->random_panorama = false;
  zyn_addsynth_ptr->panorama = 0.0;

  zyn_addsynth_ptr->stereo = true;

  zyn_addsynth_ptr->random_grouping = false;

  zyn_addsynth_ptr->amplitude_lfo_params.frequency = 80.0 / 127.0;
  zyn_addsynth_ptr->amplitude_lfo_params.depth = 0;
  zyn_addsynth_ptr->amplitude_lfo_params.random_start_phase = false;
  zyn_addsynth_ptr->amplitude_lfo_params.start_phase = 0.5;
  zyn_addsynth_ptr->amplitude_lfo_params.depth_randomness_enabled = false;
  zyn_addsynth_ptr->amplitude_lfo_params.depth_randomness = 0.5;
  zyn_addsynth_ptr->amplitude_lfo_params.frequency_randomness_enabled = false;
  zyn_addsynth_ptr->amplitude_lfo_params.frequency_randomness = 0.5;
  zyn_addsynth_ptr->amplitude_lfo_params.delay = 0;
  zyn_addsynth_ptr->amplitude_lfo_params.stretch = 0;
  zyn_addsynth_ptr->amplitude_lfo_params.shape = ZYN_LFO_SHAPE_TYPE_SINE;

  zyn_addsynth_ptr->filter_lfo_params.frequency = 80.0 / 127.0;
  zyn_addsynth_ptr->filter_lfo_params.depth = 0;
  zyn_addsynth_ptr->filter_lfo_params.random_start_phase = false;
  zyn_addsynth_ptr->filter_lfo_params.start_phase = 0.5;
  zyn_addsynth_ptr->filter_lfo_params.depth_randomness_enabled = false;
  zyn_addsynth_ptr->filter_lfo_params.depth_randomness = 0.5;
  zyn_addsynth_ptr->filter_lfo_params.frequency_randomness_enabled = false;
  zyn_addsynth_ptr->filter_lfo_params.frequency_randomness = 0.5;
  zyn_addsynth_ptr->filter_lfo_params.delay = 0;
  zyn_addsynth_ptr->filter_lfo_params.stretch = 0;
  zyn_addsynth_ptr->filter_lfo_params.shape = ZYN_LFO_SHAPE_TYPE_SINE;

  zyn_addsynth_ptr->frequency_lfo_params.frequency = 70.0 / 127.0;
  zyn_addsynth_ptr->frequency_lfo_params.depth = 0;
  zyn_addsynth_ptr->frequency_lfo_params.random_start_phase = false;
  zyn_addsynth_ptr->frequency_lfo_params.start_phase = 0.5;
  zyn_addsynth_ptr->frequency_lfo_params.depth_randomness_enabled = false;
  zyn_addsynth_ptr->frequency_lfo_params.depth_randomness = 0.5;
  zyn_addsynth_ptr->frequency_lfo_params.frequency_randomness_enabled = false;
  zyn_addsynth_ptr->frequency_lfo_params.frequency_randomness = 0.5;
  zyn_addsynth_ptr->frequency_lfo_params.delay = 0;
  zyn_addsynth_ptr->frequency_lfo_params.stretch = 0;
  zyn_addsynth_ptr->frequency_lfo_params.shape = ZYN_LFO_SHAPE_TYPE_SINE;

  for (note_index = 0 ; note_index < ZYN_DEFAULT_POLYPHONY ; note_index++)
  {
    zyn_addsynth_ptr->notes_array[note_index].note_ptr = new ADnote(zyn_addsynth_ptr, zyn_addsynth_ptr->ctl_ptr);
    zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
  }

  *handle_ptr = (zyn_addsynth_handle)zyn_addsynth_ptr;

  OscilGen::tmpsmps = new REALTYPE[OSCIL_SIZE];
  zyn_fft_freqs_init(&OscilGen::outoscilFFTfreqs, OSCIL_SIZE / 2);

//  printf("zyn_addsynth_create(%08X)\n", (unsigned int)*handle_ptr);

  return true;
}

#define zyn_addsynth_ptr ((struct zyn_addsynth *)handle)

void
zyn_addsynth_get_audio_output(
  zyn_addsynth_handle handle,
  zyn_sample_type * buffer_left,
  zyn_sample_type * buffer_right)
{
  unsigned int note_index;
  zyn_sample_type note_buffer_left[SOUND_BUFFER_SIZE];
  zyn_sample_type note_buffer_right[SOUND_BUFFER_SIZE];

  silence_two_buffers(buffer_left, buffer_right, SOUND_BUFFER_SIZE);

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote != -1)
    {
      //printf("mixing note channel %u\n", note_index);

      zyn_addsynth_ptr->notes_array[note_index].note_ptr->noteout(note_buffer_left, note_buffer_right);

      mix_add_two_buffers(
        buffer_left,
        buffer_right,
        note_buffer_left,
        note_buffer_right,
        SOUND_BUFFER_SIZE);

      if (zyn_addsynth_ptr->notes_array[note_index].note_ptr->finished())
      {
        zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
      }
    }
  }
}

void
zyn_addsynth_note_on(
  zyn_addsynth_handle handle,
  unsigned int note)
{
  unsigned int note_index;
  unsigned char velocity;
  int masterkeyshift;
  float vel;
  zyn_sample_type notebasefreq;

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote == -1)
    {
      goto unused_note_channel_found;
    }
  }

  //printf("note on %u - ignored\n", note);

  return;

unused_note_channel_found:
  //printf("note on %u - channel %u\n", note, note_index);
  velocity = 100;
  masterkeyshift = 0;

  vel = VelF(velocity/127.0, zyn_addsynth_ptr->velsns);
  notebasefreq = 440.0*pow(2.0,(note-69.0)/12.0);

  // Portamento

  if (zyn_addsynth_ptr->oldfreq < 1.0) /* only when the first note is played */
  {
    zyn_addsynth_ptr->oldfreq = notebasefreq;
  }
      
  bool portamento = zyn_addsynth_ptr->ctl_ptr->initportamento(zyn_addsynth_ptr->oldfreq, notebasefreq);
      
  zyn_addsynth_ptr->oldfreq = notebasefreq;

  zyn_addsynth_ptr->notes_array[note_index].midinote = note;
  zyn_addsynth_ptr->notes_array[note_index].note_ptr->note_on(
    zyn_addsynth_ptr->random_panorama ? RND : zyn_addsynth_ptr->panorama,
    zyn_addsynth_ptr->random_grouping,
    notebasefreq,
    vel,
    portamento,
    note);
}

void
zyn_addsynth_note_off(
  zyn_addsynth_handle handle,
  unsigned int note)
{
  unsigned int note_index;

  //printf("note off %u\n", note);

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote == (char)note)
    {
      zyn_addsynth_ptr->notes_array[note_index].note_ptr->relasekey();
    }
  }
}

void
zyn_addsynth_destroy(
  zyn_addsynth_handle handle)
{
  unsigned int voice_index;

//  printf("zyn_addsynth_destroy(%08X)\n", (unsigned int)handle);
  zyn_fft_destroy(zyn_addsynth_ptr->fft);

  // ADnoteParameters temp begin

  delete(zyn_addsynth_ptr->GlobalPar.Reson);

  for (voice_index = 0 ; voice_index < zyn_addsynth_ptr->voices_count ; voice_index++)
  {
    delete (zyn_addsynth_ptr->voices_params_ptr[voice_index].OscilSmp);
    delete (zyn_addsynth_ptr->voices_params_ptr[voice_index].FMSmp);
  }

  // ADnoteParameters temp end

  delete zyn_addsynth_ptr->ctl_ptr;

  free(zyn_addsynth_ptr->voices_params_ptr);

  free(zyn_addsynth_ptr->notes_array);

  delete zyn_addsynth_ptr;
}

float percent_from_0_127(unsigned char value)
{
  return ((float)(value)/127.0)*100.0; // 0-127 -> percent
}

unsigned char percent_to_0_127(float value)
{
  return (unsigned char)roundf(value / 100.0 * 127.0);
}

float
zyn_addsynth_get_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter)
{
  struct zyn_lfo_parameters * lfo_params_ptr;

  if (component == ZYNADD_COMPONENT_AMP_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_PANORAMA:
      return zyn_addsynth_ptr->panorama;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PVolume);
    case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING:
      return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PAmpVelocityScaleFunction);
    case ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH:
      return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchStrength);
    case ZYNADD_PARAMETER_FLOAT_PUNCH_TIME:
      return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchTime);
    case ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH:
      return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchStretch);
    case ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING:
      return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchVelocitySensing);
    default:
      LOG_ERROR("Unknown float amplitude global parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_amplitude_envelope_params.get_duration(
          zyn_addsynth_ptr->m_amplitude_envelope_params.m_attack_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_amplitude_envelope_params.get_duration(
          zyn_addsynth_ptr->m_amplitude_envelope_params.m_decay_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_SUSTAIN_VALUE:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_amplitude_envelope_params.get_value(
          zyn_addsynth_ptr->m_amplitude_envelope_params.m_sustain_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_amplitude_envelope_params.get_duration(
          zyn_addsynth_ptr->m_amplitude_envelope_params.m_release_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_amplitude_envelope_params.m_stretch) * 2;
    default:
      LOG_ERROR("Unknown amplitude envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_LFO ||
           component == ZYNADD_COMPONENT_FILTER_LFO ||
           component == ZYNADD_COMPONENT_FREQUENCY_LFO)
  {
    switch (component)
    {
    case ZYNADD_COMPONENT_AMP_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->amplitude_lfo_params;
      break;
    case ZYNADD_COMPONENT_FILTER_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->filter_lfo_params;
      break;
    case ZYNADD_COMPONENT_FREQUENCY_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->frequency_lfo_params;
      break;
    default:
      assert(0);
    }

    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY:
      return lfo_params_ptr->frequency;
    case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH:
      return lfo_params_ptr->depth * 100;
    case ZYNADD_PARAMETER_FLOAT_LFO_START_PHASE:
      return lfo_params_ptr->start_phase;
    case ZYNADD_PARAMETER_FLOAT_LFO_DELAY:
      return lfo_params_ptr->delay;
    case ZYNADD_PARAMETER_FLOAT_LFO_STRETCH:
      return lfo_params_ptr->stretch;
    case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH_RANDOMNESS:
      return lfo_params_ptr->depth_randomness * 100;
    case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY_RANDOMNESS:
      return lfo_params_ptr->frequency_randomness * 100;
    default:
      LOG_ERROR("Unknown LFO parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FILTER_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_filter_envelope_params.get_value(
          zyn_addsynth_ptr->m_filter_envelope_params.m_attack_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_filter_envelope_params.get_duration(
          zyn_addsynth_ptr->m_filter_envelope_params.m_attack_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_VALUE:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_filter_envelope_params.get_value(
          zyn_addsynth_ptr->m_filter_envelope_params.m_decay_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_filter_envelope_params.get_duration(
          zyn_addsynth_ptr->m_filter_envelope_params.m_decay_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_filter_envelope_params.get_value(
          zyn_addsynth_ptr->m_filter_envelope_params.m_release_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_filter_envelope_params.get_duration(
          zyn_addsynth_ptr->m_filter_envelope_params.m_release_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      return percent_from_0_127(zyn_addsynth_ptr->m_filter_envelope_params.m_stretch) * 2;
    default:
      LOG_ERROR("Unknown filter envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FREQUENCY_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_frequency_envelope_params.get_value(
          zyn_addsynth_ptr->m_frequency_envelope_params.m_attack_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_frequency_envelope_params.get_duration(
          zyn_addsynth_ptr->m_frequency_envelope_params.m_attack_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_frequency_envelope_params.get_value(
          zyn_addsynth_ptr->m_frequency_envelope_params.m_release_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      return percent_from_0_127(
        zyn_addsynth_ptr->m_frequency_envelope_params.get_duration(
          zyn_addsynth_ptr->m_frequency_envelope_params.m_release_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      return percent_from_0_127(zyn_addsynth_ptr->m_frequency_envelope_params.m_stretch) * 2;
    default:
      LOG_ERROR("Unknown frequency envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FILTER_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_FREQUNECY:
      return percent_from_0_127(zyn_addsynth_ptr->m_filter_params.Pfreq) / 100;
    case ZYNADD_PARAMETER_FLOAT_Q_FACTOR:
      return percent_from_0_127(zyn_addsynth_ptr->m_filter_params.Pq) / 100;
    case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_AMOUNT:
      return zyn_addsynth_ptr->m_filter_velocity_sensing_amount;
    case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_FUNCTION:
      return zyn_addsynth_ptr->m_filter_velocity_scale_function;
    case ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING:
      return zyn_addsynth_ptr->m_filter_params.m_frequency_tracking;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      return zyn_addsynth_ptr->m_filter_params.m_gain;
    default:
      LOG_ERROR("Unknown filter parameter %u", parameter);
      assert(0);
    }
  }
  else
  {
    LOG_ERROR("Unknown component %u", component);
    assert(0);
  }
}

void
zyn_addsynth_set_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter,
  float value)
{
  struct zyn_lfo_parameters * lfo_params_ptr;

  if (component == ZYNADD_COMPONENT_AMP_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_PANORAMA:
      zyn_addsynth_ptr->panorama = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      zyn_addsynth_ptr->GlobalPar.PVolume = percent_to_0_127(value);
      return;
    case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING:
      zyn_addsynth_ptr->GlobalPar.PAmpVelocityScaleFunction = percent_to_0_127(value);
      return;
    case ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH:
      zyn_addsynth_ptr->GlobalPar.PPunchStrength = percent_to_0_127(value);
      return;
    case ZYNADD_PARAMETER_FLOAT_PUNCH_TIME:
      zyn_addsynth_ptr->GlobalPar.PPunchTime = percent_to_0_127(value);
      return;
    case ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH:
      zyn_addsynth_ptr->GlobalPar.PPunchStretch = percent_to_0_127(value);
      return;
    case ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING:
      zyn_addsynth_ptr->GlobalPar.PPunchVelocitySensing = percent_to_0_127(value);
      return;
    default:
      LOG_ERROR("Unknown float amplitude global parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      zyn_addsynth_ptr->m_amplitude_envelope_params.set_duration(
        zyn_addsynth_ptr->m_amplitude_envelope_params.m_attack_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION:
      zyn_addsynth_ptr->m_amplitude_envelope_params.set_duration(
        zyn_addsynth_ptr->m_amplitude_envelope_params.m_decay_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_SUSTAIN_VALUE:
      zyn_addsynth_ptr->m_amplitude_envelope_params.set_value(
        zyn_addsynth_ptr->m_amplitude_envelope_params.m_sustain_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      zyn_addsynth_ptr->m_amplitude_envelope_params.set_duration(
        zyn_addsynth_ptr->m_amplitude_envelope_params.m_release_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      zyn_addsynth_ptr->m_amplitude_envelope_params.m_stretch = percent_to_0_127(value/2);
      return;
    default:
      LOG_ERROR("Unknown amplitude envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_LFO ||
           component == ZYNADD_COMPONENT_FILTER_LFO ||
           component == ZYNADD_COMPONENT_FREQUENCY_LFO)
  {
    switch (component)
    {
    case ZYNADD_COMPONENT_AMP_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->amplitude_lfo_params;
      break;
    case ZYNADD_COMPONENT_FILTER_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->filter_lfo_params;
      break;
    case ZYNADD_COMPONENT_FREQUENCY_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->frequency_lfo_params;
      break;
    default:
      assert(0);
    }

    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY:
      lfo_params_ptr->frequency = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH:
      lfo_params_ptr->depth = value / 100;
      return;
    case ZYNADD_PARAMETER_FLOAT_LFO_START_PHASE:
      lfo_params_ptr->start_phase = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_LFO_DELAY:
      lfo_params_ptr->delay = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_LFO_STRETCH:
      lfo_params_ptr->stretch = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH_RANDOMNESS:
      lfo_params_ptr->depth_randomness = value / 100;
      return;
    case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY_RANDOMNESS:
      lfo_params_ptr->frequency_randomness = value / 100;
      return;
    default:
      LOG_ERROR("Unknown LFO parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FILTER_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE:
      zyn_addsynth_ptr->m_filter_envelope_params.set_value(
        zyn_addsynth_ptr->m_filter_envelope_params.m_attack_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      zyn_addsynth_ptr->m_filter_envelope_params.set_duration(
        zyn_addsynth_ptr->m_filter_envelope_params.m_attack_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_VALUE:
      zyn_addsynth_ptr->m_filter_envelope_params.set_value(
        zyn_addsynth_ptr->m_filter_envelope_params.m_decay_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION:
      zyn_addsynth_ptr->m_filter_envelope_params.set_duration(
        zyn_addsynth_ptr->m_filter_envelope_params.m_decay_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE:
      zyn_addsynth_ptr->m_filter_envelope_params.set_value(
        zyn_addsynth_ptr->m_filter_envelope_params.m_release_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      zyn_addsynth_ptr->m_filter_envelope_params.set_duration(
        zyn_addsynth_ptr->m_filter_envelope_params.m_release_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      zyn_addsynth_ptr->m_filter_envelope_params.m_stretch = percent_to_0_127(value/2);
      return;
    default:
      LOG_ERROR("Unknown filter envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FREQUENCY_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE:
      zyn_addsynth_ptr->m_frequency_envelope_params.set_value(
        zyn_addsynth_ptr->m_frequency_envelope_params.m_attack_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      zyn_addsynth_ptr->m_frequency_envelope_params.set_duration(
        zyn_addsynth_ptr->m_frequency_envelope_params.m_attack_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE:
      zyn_addsynth_ptr->m_frequency_envelope_params.set_value(
        zyn_addsynth_ptr->m_frequency_envelope_params.m_release_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      zyn_addsynth_ptr->m_frequency_envelope_params.set_duration(
        zyn_addsynth_ptr->m_frequency_envelope_params.m_release_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      zyn_addsynth_ptr->m_frequency_envelope_params.m_stretch = percent_to_0_127(value/2);
      return;
    default:
      LOG_ERROR("Unknown frequency envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FILTER_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_FREQUNECY:
      zyn_addsynth_ptr->m_filter_params.Pfreq = percent_to_0_127(value * 100);
      return;
    case ZYNADD_PARAMETER_FLOAT_Q_FACTOR:
      zyn_addsynth_ptr->m_filter_params.Pq = percent_to_0_127(value * 100);
      return;
    case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_AMOUNT:
      zyn_addsynth_ptr->m_filter_velocity_sensing_amount = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_FUNCTION:
      zyn_addsynth_ptr->m_filter_velocity_scale_function = -value;
      return;
    case ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING:
      zyn_addsynth_ptr->m_filter_params.m_frequency_tracking = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      zyn_addsynth_ptr->m_filter_params.m_gain = value;
      return;
    default:
      LOG_ERROR("Unknown filter parameter %u", parameter);
      assert(0);
    }
  }
  else
  {
    LOG_ERROR("Unknown component %u", component);
    assert(0);
  }
}

bool
zyn_addsynth_get_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter)
{
  struct zyn_lfo_parameters * lfo_params_ptr;

  if (component == ZYNADD_COMPONENT_AMP_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
      return zyn_addsynth_ptr->random_panorama;
    case ZYNADD_PARAMETER_BOOL_STEREO:
      return zyn_addsynth_ptr->stereo;
    case ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING:
      return zyn_addsynth_ptr->random_grouping;
    default:
      LOG_ERROR("Unknown bool amplitude global parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      return zyn_addsynth_ptr->m_amplitude_envelope_params.m_forced_release;
    case ZYNADD_PARAMETER_BOOL_ENV_LINEAR:
      return zyn_addsynth_ptr->m_amplitude_envelope_params.m_linear;
    default:
      LOG_ERROR("Unknown bool amplitude envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_LFO ||
           component == ZYNADD_COMPONENT_FILTER_LFO ||
           component == ZYNADD_COMPONENT_FREQUENCY_LFO)
  {
    switch (component)
    {
    case ZYNADD_COMPONENT_AMP_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->amplitude_lfo_params;
      break;
    case ZYNADD_COMPONENT_FILTER_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->filter_lfo_params;
      break;
    case ZYNADD_COMPONENT_FREQUENCY_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->frequency_lfo_params;
      break;
    default:
      assert(0);
    }

    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_START_PHASE:
      return lfo_params_ptr->random_start_phase;
    case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_DEPTH:
      return lfo_params_ptr->depth_randomness_enabled;
    case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_FREQUENCY:
      return lfo_params_ptr->frequency_randomness_enabled;
    default:
      LOG_ERROR("Unknown bool LFO parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FILTER_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      return zyn_addsynth_ptr->m_filter_envelope_params.m_forced_release;
    default:
      LOG_ERROR("Unknown bool filter envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FREQUENCY_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      return zyn_addsynth_ptr->m_frequency_envelope_params.m_forced_release;
    default:
      LOG_ERROR("Unknown bool frequency envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_VOICE_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_VOICE_RESONANCE:
      return true;
    default:
      LOG_ERROR("Unknown bool voice global parameter %u", parameter);
      assert(0);
    }
  }
  else
  {
    LOG_ERROR("Unknown component %u", component);
    assert(0);
  }
}

void
zyn_addsynth_set_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter,
  bool value)
{
  struct zyn_lfo_parameters * lfo_params_ptr;

  if (component == ZYNADD_COMPONENT_AMP_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
      zyn_addsynth_ptr->random_panorama = value;
      return;
    case ZYNADD_PARAMETER_BOOL_STEREO:
      zyn_addsynth_ptr->stereo = value;
      return;
    case ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING:
      zyn_addsynth_ptr->random_grouping = value;
      return;
    default:
      LOG_ERROR("Unknown bool amplitude global parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      zyn_addsynth_ptr->m_amplitude_envelope_params.m_forced_release = value;
      return;
    case ZYNADD_PARAMETER_BOOL_ENV_LINEAR:
      zyn_addsynth_ptr->m_amplitude_envelope_params.m_linear = value;
      return;
    default:
      LOG_ERROR("Unknown bool amplitude envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_AMP_LFO ||
           component == ZYNADD_COMPONENT_FILTER_LFO ||
           component == ZYNADD_COMPONENT_FREQUENCY_LFO)
  {
    switch (component)
    {
    case ZYNADD_COMPONENT_AMP_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->amplitude_lfo_params;
      break;
    case ZYNADD_COMPONENT_FILTER_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->filter_lfo_params;
      break;
    case ZYNADD_COMPONENT_FREQUENCY_LFO:
      lfo_params_ptr = &zyn_addsynth_ptr->frequency_lfo_params;
      break;
    default:
      assert(0);
    }

    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_START_PHASE:
      lfo_params_ptr->random_start_phase = value;
      return;
    case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_DEPTH:
      lfo_params_ptr->depth_randomness_enabled = value;
      return;
    case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_FREQUENCY:
      lfo_params_ptr->frequency_randomness_enabled = value;
      return;
    default:
      LOG_ERROR("Unknown bool LFO parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FILTER_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      zyn_addsynth_ptr->m_filter_envelope_params.m_forced_release = value;
      return;
    default:
      LOG_ERROR("Unknown bool filter envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_FREQUENCY_ENV)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      zyn_addsynth_ptr->m_frequency_envelope_params.m_forced_release = value;
      return;
    default:
      LOG_ERROR("Unknown bool frequency envelope parameter %u", parameter);
      assert(0);
    }
  }
  else if (component == ZYNADD_COMPONENT_VOICE_GLOBALS)
  {
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_VOICE_RESONANCE:
      return;
    default:
      LOG_ERROR("Unknown bool voice global parameter %u", parameter);
      assert(0);
    }
  }
  else
  {
    LOG_ERROR("Unknown component %u", component);
    assert(0);
  }
}

unsigned int
zyn_addsynth_get_shape_parameter(
  zyn_addsynth_handle handle,
  unsigned int component)
{
  switch (component)
  {
  case ZYNADD_COMPONENT_AMP_LFO:
    return zyn_addsynth_ptr->amplitude_lfo_params.shape;
  case ZYNADD_COMPONENT_FILTER_LFO:
    return zyn_addsynth_ptr->filter_lfo_params.shape;
  case ZYNADD_COMPONENT_FREQUENCY_LFO:
    return zyn_addsynth_ptr->frequency_lfo_params.shape;
  default:
    LOG_ERROR("Unknown component %u", component);
    assert(0);
  }
}

void
zyn_addsynth_set_shape_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int value)
{
  switch (component)
  {
  case ZYNADD_COMPONENT_AMP_LFO:
    zyn_addsynth_ptr->amplitude_lfo_params.shape = value;
    break;
  case ZYNADD_COMPONENT_FILTER_LFO:
    zyn_addsynth_ptr->filter_lfo_params.shape = value;
    break;
  case ZYNADD_COMPONENT_FREQUENCY_LFO:
    zyn_addsynth_ptr->frequency_lfo_params.shape = value;
    break;
  default:
    LOG_ERROR("Unknown component %u", component);
    assert(0);
  }
}

unsigned int
zyn_addsynth_get_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component)
{
  return ZYN_FILTER_TYPE_ANALOG;
}

void
zyn_addsynth_set_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int value)
{
}

unsigned int
zyn_addsynth_get_analog_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component)
{
  return ZYN_FILTER_ANALOG_TYPE_LPF1;
}

void
zyn_addsynth_set_analog_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int value)
{
}

signed int
zyn_addsynth_get_int_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter)
{
  return -1;
}

void
zyn_addsynth_set_int_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter,
  signed int value)
{
}
