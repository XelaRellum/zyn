/* -*- Mode: C++ ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007,2008,2009 Nedko Arnaudov <nedko@arnaudov.name>
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
#include "sv_filter.h"
#include "formant_filter.h"
#include "filter.h"
#include "portamento.h"
#include "addsynth_internal.h"
#include "addnote.h"
#include "util.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

#define ZYN_DEFAULT_POLYPHONY 60

// (94.0 / 64.0 - 1) * 5.0 = 2.34375
#define ZYN_GLOBAL_FILTER_INITIAL_FREQUENCY 2.34375

// 40.0 / 127.0 = 0.31496062992125984
#define ZYN_GLOBAL_FILTER_INITIAL_Q 0.31496062992125984

struct note_channel
{
  int midinote;               // MIDI note, -1 when note "channel" is not playing
  zyn_addnote_handle note_handle;
};

bool
zyn_addsynth_create(
  float sample_rate,
  unsigned int voices_count,
  zyn_addsynth_handle * handle_ptr)
{
  struct zyn_addsynth * zyn_addsynth_ptr;
  unsigned int note_index;
  unsigned int voice_index;

//  printf("zyn_addsynth_create\n");
  zyn_addsynth_ptr = (struct zyn_addsynth *)malloc(sizeof(struct zyn_addsynth));
  if (zyn_addsynth_ptr == NULL)
  {
    goto fail;
  }

  zyn_addsynth_ptr->sample_rate = sample_rate;

  zyn_addsynth_ptr->temporary_samples_ptr = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * OSCIL_SIZE);
  zyn_fft_freqs_init(&zyn_addsynth_ptr->oscillator_fft_frequencies, OSCIL_SIZE / 2);

  zyn_addsynth_ptr->polyphony = ZYN_DEFAULT_POLYPHONY;
  zyn_addsynth_ptr->notes_array = (struct note_channel *)malloc(ZYN_DEFAULT_POLYPHONY * sizeof(struct note_channel));

  zyn_addsynth_ptr->all_sound_off = false;

  zyn_addsynth_ptr->velsns = 64;
  zyn_addsynth_ptr->fft = zyn_fft_create(OSCIL_SIZE);

  // ADnoteParameters temp begin

  zyn_addsynth_ptr->m_frequency_envelope_params.init_asr(0, false, 64, 50, 64, 60);
    
  zyn_addsynth_ptr->m_amplitude_envelope_params.init_adsr(64, true, 0, 40, 127, 25, false);

  zyn_addsynth_ptr->filter_type = ZYN_FILTER_TYPE_ANALOG;
  zyn_addsynth_ptr->m_filter_params.init(sample_rate, ZYN_FILTER_ANALOG_TYPE_LPF2, 94, 40);
  if (!zyn_filter_sv_create(sample_rate, ZYN_GLOBAL_FILTER_INITIAL_FREQUENCY, ZYN_GLOBAL_FILTER_INITIAL_Q, &zyn_addsynth_ptr->filter_sv))
  {
    goto fail_free_synth;
  }

  zyn_addsynth_ptr->m_filter_envelope_params.init_adsr_filter(0, true, 64, 40, 64, 70, 60, 64);

  zyn_resonance_init(&zyn_addsynth_ptr->resonance);

  zyn_addsynth_ptr->voices_count = voices_count;
  zyn_addsynth_ptr->voices_params_ptr = (struct zyn_addnote_voice_parameters *)malloc(sizeof(struct zyn_addnote_voice_parameters) * voices_count);

  for (voice_index = 0 ; voice_index < voices_count ; voice_index++)
  {
    zyn_oscillator_init(
      &zyn_addsynth_ptr->voices_params_ptr[voice_index].oscillator,
      sample_rate,
      zyn_addsynth_ptr->fft,
      &zyn_addsynth_ptr->resonance,
      zyn_addsynth_ptr->temporary_samples_ptr,
      &zyn_addsynth_ptr->oscillator_fft_frequencies);

    zyn_oscillator_init(
      &zyn_addsynth_ptr->voices_params_ptr[voice_index].modulator_oscillator,
      sample_rate,
      zyn_addsynth_ptr->fft,
      NULL,
      zyn_addsynth_ptr->temporary_samples_ptr,
      &zyn_addsynth_ptr->oscillator_fft_frequencies);

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

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_filter_params.init(sample_rate, ZYN_FILTER_ANALOG_TYPE_LPF2, 50, 60);
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
  zyn_addsynth_ptr->detune.fine = 0.0;
  zyn_addsynth_ptr->detune.coarse = 0;
  zyn_addsynth_ptr->detune.octave = 0;
  zyn_addsynth_ptr->detune.type = ZYN_DETUNE_TYPE_L35CENTS;
  zyn_addsynth_ptr->detune_bandwidth = 0.0;
    
  /* Amplitude Global Parameters */
  zyn_addsynth_ptr->PVolume=90;
  zyn_addsynth_ptr->PAmpVelocityScaleFunction=64;
  zyn_addsynth_ptr->PPunchStrength=0;
  zyn_addsynth_ptr->PPunchTime=60;
  zyn_addsynth_ptr->PPunchStretch=64;
  zyn_addsynth_ptr->PPunchVelocitySensing=72;
    
  /* Filter Global Parameters*/
  zyn_addsynth_ptr->m_filter_velocity_sensing_amount = 0.5;
  zyn_addsynth_ptr->m_filter_velocity_scale_function = 0;
  zyn_addsynth_ptr->m_filter_params.defaults();

  for (voice_index = 0 ; voice_index < voices_count ; voice_index++)
  {
    zyn_addsynth_ptr->voices_params_ptr[voice_index].enabled = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].white_noise = false;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fixed_detune.mode = ZYN_DETUNE_MODE_NORMAL;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fixed_detune.equal_temperate = 0;
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
    zyn_addsynth_ptr->voices_params_ptr[voice_index].detune.fine = 0.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].detune.coarse = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].detune.octave = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].detune.type = ZYN_DETUNE_TYPE_GLOBAL;
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
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fm_detune.fine = 0.0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fm_detune.coarse = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fm_detune.octave = 0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].fm_detune.type = ZYN_DETUNE_TYPE_GLOBAL;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled=0;
    zyn_addsynth_ptr->voices_params_ptr[voice_index].PFMVelocityScaleFunction=64; 

    zyn_addsynth_ptr->voices_params_ptr[voice_index].m_filter_params.defaults();
  }

  zyn_addsynth_ptr->voices_params_ptr[0].enabled = true;

  // ADnoteParameters temp end

  zyn_addsynth_ptr->bandwidth_depth = 64;
  zyn_addsynth_ptr->bandwidth_exponential = false;
  zyn_addsynth_ptr->modwheel_depth = 80;
  zyn_addsynth_ptr->modwheel_exponential = false;

  zyn_addsynth_set_bandwidth(zyn_addsynth_ptr, 64);
  zyn_addsynth_set_modwheel(zyn_addsynth_ptr, 64);

  zyn_portamento_init(&zyn_addsynth_ptr->portamento);

  zyn_addsynth_ptr->pitch_bend_range = 200.0; // 200 cents = 2 halftones
  zyn_addsynth_ptr->pitch_bend = 0; // center
  ZYN_UPDATE_PITCH_BEND(zyn_addsynth_ptr);

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
    if (!zyn_addnote_create(
          zyn_addsynth_ptr,
          &zyn_addsynth_ptr->notes_array[note_index].note_handle))
    {
    }

    zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
  }

  // init global components

  zyn_addsynth_component_init_amp_globals(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_AMP_GLOBALS,
    zyn_addsynth_ptr);

  zyn_addsynth_component_init_detune(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_DETUNE,
    &zyn_addsynth_ptr->detune);

  zyn_addsynth_component_init_amp_envelope(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_AMP_ENV,
    &zyn_addsynth_ptr->m_amplitude_envelope_params);

  zyn_addsynth_component_init_lfo(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_AMP_LFO,
    &zyn_addsynth_ptr->amplitude_lfo_params);

  zyn_addsynth_component_init_filter_globals(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FILTER_GLOBALS,
    zyn_addsynth_ptr);

  zyn_addsynth_component_init_filter_analog(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FILTER_ANALOG,
    zyn_addsynth_ptr);

  zyn_addsynth_component_init_filter_formant(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FILTER_FORMANT,
    zyn_addsynth_ptr);

  zyn_addsynth_component_init_filter_sv(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FILTER_SV,
    zyn_addsynth_ptr->filter_sv);

  zyn_addsynth_component_init_filter_envelope(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FILTER_ENV,
    &zyn_addsynth_ptr->m_filter_envelope_params);

  zyn_addsynth_component_init_lfo(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FILTER_LFO,
    &zyn_addsynth_ptr->filter_lfo_params);

  zyn_addsynth_component_init_frequency_globals(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FREQUENCY_GLOBALS);

  zyn_addsynth_component_init_frequency_envelope(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FREQUENCY_ENV,
    &zyn_addsynth_ptr->m_frequency_envelope_params);

  zyn_addsynth_component_init_lfo(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_FREQUENCY_LFO,
    &zyn_addsynth_ptr->frequency_lfo_params);

  zyn_addsynth_component_init_portamento(
    zyn_addsynth_ptr->global_components + ZYNADD_COMPONENT_PORTAMENTO,
    &zyn_addsynth_ptr->portamento);

  // init voices components

  zyn_addsynth_ptr->voices_components =
    (struct zyn_component_descriptor *)malloc(
      sizeof(struct zyn_component_descriptor) * voices_count * ZYNADD_VOICE_COMPONENTS_COUNT);

  for (voice_index = 0 ; voice_index < voices_count ; voice_index++)
  {
    zyn_addsynth_component_init_voice_globals(
      zyn_addsynth_ptr->voices_components + voice_index * ZYNADD_VOICE_COMPONENTS_COUNT + ZYNADD_COMPONENT_VOICE_GLOBALS,
      zyn_addsynth_ptr->voices_params_ptr + voice_index);

    zyn_addsynth_component_init_oscillator(
      zyn_addsynth_ptr->voices_components + voice_index * ZYNADD_VOICE_COMPONENTS_COUNT + ZYNADD_COMPONENT_VOICE_OSCILLATOR,
      &zyn_addsynth_ptr->voices_params_ptr[voice_index].oscillator);

    zyn_addsynth_component_init_detune(
      zyn_addsynth_ptr->voices_components + voice_index * ZYNADD_VOICE_COMPONENTS_COUNT + ZYNADD_COMPONENT_VOICE_DETUNE,
      &zyn_addsynth_ptr->voices_params_ptr[voice_index].detune);

    zyn_addsynth_component_init_fixed_detune(
      zyn_addsynth_ptr->voices_components + voice_index * ZYNADD_VOICE_COMPONENTS_COUNT + ZYNADD_COMPONENT_VOICE_FIXED_DETUNE,
      &zyn_addsynth_ptr->voices_params_ptr[voice_index].fixed_detune);

    zyn_addsynth_component_init_detune(
      zyn_addsynth_ptr->voices_components + voice_index * ZYNADD_VOICE_COMPONENTS_COUNT + ZYNADD_COMPONENT_VOICE_MODULATOR_DETUNE,
      &zyn_addsynth_ptr->voices_params_ptr[voice_index].fm_detune);
  }

  *handle_ptr = (zyn_addsynth_handle)zyn_addsynth_ptr;

//  printf("zyn_addsynth_create(%08X)\n", (unsigned int)*handle_ptr);

  return true;

fail_free_synth:

fail:
  return false;
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
  bool note_active;

  silence_two_buffers(buffer_left, buffer_right, SOUND_BUFFER_SIZE);

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote != -1)
    {
      //printf("mixing note channel %u\n", note_index);

      note_active = zyn_addnote_noteout(
        zyn_addsynth_ptr->notes_array[note_index].note_handle,
        note_buffer_left,
        note_buffer_right);

      mix_add_two_buffers(
        buffer_left,
        buffer_right,
        note_buffer_left,
        note_buffer_right,
        SOUND_BUFFER_SIZE);

      if (!note_active)
      {
        zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
      }
    }
  }

  if (zyn_addsynth_ptr->all_sound_off)
  {
    fadeout_two_buffers(buffer_left, buffer_right, SOUND_BUFFER_SIZE);

    for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
    {
      if (zyn_addsynth_ptr->notes_array[note_index].midinote != -1)
      {
        zyn_addnote_force_disable(zyn_addsynth_ptr->notes_array[note_index].note_handle);
        zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
      }
    }

    zyn_addsynth_ptr->all_sound_off = false;
  }

  zyn_portamento_update(&zyn_addsynth_ptr->portamento);
}

void
zyn_addsynth_note_on(
  zyn_addsynth_handle handle,
  unsigned int note,
  unsigned int velocity)
{
  unsigned int note_index;
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
  masterkeyshift = 0;

  vel = VelF(velocity/127.0, zyn_addsynth_ptr->velsns);
  notebasefreq = 440.0*pow(2.0,(note-69.0)/12.0);

  // Portamento

  if (zyn_addsynth_ptr->oldfreq < 1.0) /* only when the first note is played */
  {
    zyn_addsynth_ptr->oldfreq = notebasefreq;
  }
      
  bool portamento = zyn_portamento_start(zyn_addsynth_ptr->sample_rate, &zyn_addsynth_ptr->portamento, zyn_addsynth_ptr->oldfreq, notebasefreq);
      
  zyn_addsynth_ptr->oldfreq = notebasefreq;

  zyn_addsynth_ptr->notes_array[note_index].midinote = note;

  zyn_addnote_note_on(
    zyn_addsynth_ptr->notes_array[note_index].note_handle,
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
      zyn_addnote_note_off(zyn_addsynth_ptr->notes_array[note_index].note_handle);
    }
  }
}

void
zyn_addsynth_all_notes_off(
  zyn_addsynth_handle handle)
{
  unsigned int note_index;

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote != -1)
    {
      zyn_addnote_note_off(zyn_addsynth_ptr->notes_array[note_index].note_handle);
    }
  }
}

void
zyn_addsynth_all_sound_off(
  zyn_addsynth_handle handle)
{
  zyn_addsynth_ptr->all_sound_off = true;
}

void
zyn_addsynth_destroy(
  zyn_addsynth_handle handle)
{
  unsigned int voice_index;

  free(zyn_addsynth_ptr->voices_components);

//  printf("zyn_addsynth_destroy(%08X)\n", (unsigned int)handle);
  zyn_fft_destroy(zyn_addsynth_ptr->fft);

  // ADnoteParameters temp begin

  for (voice_index = 0 ; voice_index < zyn_addsynth_ptr->voices_count ; voice_index++)
  {
    zyn_oscillator_uninit(&zyn_addsynth_ptr->voices_params_ptr[voice_index].oscillator);
    zyn_oscillator_uninit(&zyn_addsynth_ptr->voices_params_ptr[voice_index].modulator_oscillator);
  }

  zyn_filter_sv_destroy(zyn_addsynth_ptr->filter_sv);

  // ADnoteParameters temp end

  free(zyn_addsynth_ptr->voices_params_ptr);

  free(zyn_addsynth_ptr->notes_array);

  free(zyn_addsynth_ptr->temporary_samples_ptr);

  delete zyn_addsynth_ptr;
}

zyn_addsynth_component
zyn_addsynth_get_global_component(
  zyn_addsynth_handle handle,
  unsigned int component)
{
  if (component >= ZYNADD_GLOBAL_COMPONENTS_COUNT)
  {
    assert(0);
    return NULL;
  }

  return zyn_addsynth_ptr->global_components + component;
}

zyn_addsynth_component
zyn_addsynth_get_voice_component(
  zyn_addsynth_handle handle,
  unsigned int voice,
  unsigned int component)
{
  if (voice >= zyn_addsynth_ptr->voices_count)
  {
    assert(0);
    return NULL;
  }

  if (component >= ZYNADD_VOICE_COMPONENTS_COUNT)
  {
    assert(0);
    return NULL;
  }

  return zyn_addsynth_ptr->voices_components + voice * ZYNADD_VOICE_COMPONENTS_COUNT + component;
}

float percent_from_0_127(unsigned char value)
{
  return ((float)(value)/127.0)*100.0; // 0-127 -> percent
}

unsigned char percent_to_0_127(float value)
{
  return (unsigned char)roundf(value / 100.0 * 127.0);
}

#define component_ptr ((struct zyn_component_descriptor *)component)

float
zyn_addsynth_get_float_parameter(
  zyn_addsynth_component component,
  unsigned int parameter)
{
  return component_ptr->get_float(component_ptr->context, parameter);
}

void
zyn_addsynth_set_float_parameter(
  zyn_addsynth_component component,
  unsigned int parameter,
  float value)
{
  return component_ptr->set_float(component_ptr->context, parameter, value);
}

bool
zyn_addsynth_get_bool_parameter(
  zyn_addsynth_component component,
  unsigned int parameter)
{
  //LOG_DEBUG("component %p, context %p", component_ptr, component_ptr->context);
  return component_ptr->get_bool(component_ptr->context, parameter);
}

void
zyn_addsynth_set_bool_parameter(
  zyn_addsynth_component component,
  unsigned int parameter,
  bool value)
{
  return component_ptr->set_bool(component_ptr->context, parameter, value);
}

signed int
zyn_addsynth_get_int_parameter(
  zyn_addsynth_component component,
  unsigned int parameter)
{
  return component_ptr->get_int(component_ptr->context, parameter);
}

void
zyn_addsynth_set_int_parameter(
  zyn_addsynth_component component,
  unsigned int parameter,
  signed int value)
{
  return component_ptr->set_int(component_ptr->context, parameter, value);
}

#undef zyn_addsynth_ptr

void
zyn_addsynth_set_bandwidth(struct zyn_addsynth * zyn_addsynth_ptr, int value)
{
  float tmp;

  if (!zyn_addsynth_ptr->bandwidth_exponential)
  {
    if (value < 64 && zyn_addsynth_ptr->bandwidth_depth >= 64)
    {
      tmp = 1.0;
    }
    else
    {
      tmp = pow(25.0, pow(zyn_addsynth_ptr->bandwidth_depth / 127.0, 1.5)) - 1.0;
    }

    zyn_addsynth_ptr->bandwidth_relbw = (value / 64.0 - 1.0) * tmp + 1.0;
    if (zyn_addsynth_ptr->bandwidth_relbw < 0.01)
    {
      zyn_addsynth_ptr->bandwidth_relbw = 0.01;
    }
  }
  else
  {
    zyn_addsynth_ptr->bandwidth_relbw = pow(25.0, (value - 64.0) / 64.0 * (zyn_addsynth_ptr->bandwidth_depth / 64.0));
  }
}

void
zyn_addsynth_set_modwheel(struct zyn_addsynth * zyn_addsynth_ptr, int value)
{
  float tmp;

  if (!zyn_addsynth_ptr->modwheel_exponential)
  {
    if ((value < 64) && (zyn_addsynth_ptr->modwheel_depth >= 64))
    {
      tmp = 1.0;
    }
    else
    {
      tmp = pow(25.0, pow(zyn_addsynth_ptr->modwheel_depth / 127.0, 1.5) * 2.0) / 25.0;
    }

    zyn_addsynth_ptr->modwheel_relmod = (value / 64.0 - 1.0) * tmp + 1.0;
    if (zyn_addsynth_ptr->modwheel_relmod < 0.0)
    {
      zyn_addsynth_ptr->modwheel_relmod = 0.0;
    }
  }
  else
  {
    zyn_addsynth_ptr->modwheel_relmod = pow(25.0 , (value - 64.0) / 64.0 * (zyn_addsynth_ptr->modwheel_depth / 80.0));
  }
}
