/* -*- Mode: C ; c-basic-offset: 2 -*- */
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

#ifndef ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED
#define ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED

#include "filter_common.h"
#include "filter_sv.h"

struct note_channel;

#define ZYN_FM_TYPE_NONE          0
#define ZYN_FM_TYPE_MORPH         1
#define ZYN_FM_TYPE_RING_MOD      2
#define ZYN_FM_TYPE_PHASE_MOD     3
#define ZYN_FM_TYPE_FREQ_MOD      4
#define ZYN_FM_TYPE_PITCH_MOD     5 /* code for this is disabled for some reason */

struct zyn_fixed_detune
{
  int mode;			/* One of ZYN_DETUNE_MODE_XXX */

  /* 0 .. 127 */
  /* at 64, 1 MIDI halftone -> 1 frequency halftone */
  unsigned char equal_temperate;
};

struct zyn_detune
{
  signed int type;       /* Detune type, one of ZYN_DETUNE_TYPE_XXX */
  signed int octave;     /* Octave, -8..7 */
  signed int coarse;     /* Coarse detune, -64 .. 63 */
  float fine;            /* Fine detune, -1..1 */
};

/***********************************************************/
/*                    VOICE PARAMETERS                     */
/***********************************************************/
struct zyn_addnote_voice_parameters
{
  /* whether voice is enabled */
  bool enabled;

  /* Type of the voice (false = Sound, true = White Noise)*/
  bool white_noise;

  /* Voice Delay */
  unsigned char PDelay;

  /* whether resonance is enabled for this voice */
  bool resonance;

  // What external oscil should I use, -1 for internal OscilSmp&FMSmp
  short int Pextoscil,PextFMoscil;
  // it is not allowed that the externoscil,externFMoscil => current voice

  // oscillator phases
  unsigned char Poscilphase,PFMoscilphase;

  // filter bypass
  unsigned char Pfilterbypass;

  /* Voice oscillator */
  struct zyn_oscillator oscillator;

  /**********************************
   *     FREQUENCY PARAMETERS        *
   **********************************/

  struct zyn_detune detune;
  struct zyn_fixed_detune fixed_detune;

  /* Frequency Envelope */
  unsigned char PFreqEnvelopeEnabled;
  EnvelopeParams m_frequency_envelope_params;

  /* Frequency LFO */
  unsigned char PFreqLfoEnabled;
  struct zyn_lfo_parameters frequency_lfo_params;

  /***************************
   *   AMPLITUDE PARAMETERS   *
   ***************************/

  /* Panning       0 - random
     1 - left
     64 - center
     127 - right
     The Panning is ignored if the instrument is mono */
  unsigned char PPanning;

  /* Voice Volume */
  unsigned char PVolume;

  /* If the Volume negative */
  unsigned char PVolumeminus;

  /* Velocity sensing */
  unsigned char PAmpVelocityScaleFunction;

  /* Amplitude Envelope */
  unsigned char PAmpEnvelopeEnabled;
  EnvelopeParams m_amplitude_envelope_params;

  /* Amplitude LFO */
  unsigned char PAmpLfoEnabled;
  struct zyn_lfo_parameters amplitude_lfo_params;

  /*************************
   *   FILTER PARAMETERS    *
   *************************/

  /* Voice Filter */
  unsigned char PFilterEnabled;
  FilterParams m_filter_params;

  /* Filter Envelope */
  unsigned char PFilterEnvelopeEnabled;
  EnvelopeParams m_filter_envelope_params;

  /* LFO Envelope */
  unsigned char PFilterLfoEnabled;
  struct zyn_lfo_parameters filter_lfo_params;

  /****************************
   *   MODULLATOR PARAMETERS   *
   ****************************/

  /* Modullator Parameters, one of ZYN_FM_TYPE_XXX */
  unsigned int fm_type;

  /* Voice that I use as modullator instead of FMSmp.
     It is -1 if I use FMSmp(default).
     It maynot be equal or bigger than current voice */
  short int PFMVoice;

  /* Modullator oscillator */
  struct zyn_oscillator modulator_oscillator;

  /* Modullator Volume */
  unsigned char PFMVolume;

  /* Modullator damping at higher frequencies */
  unsigned char PFMVolumeDamp;

  /* Modullator Velocity Sensing */
  unsigned char PFMVelocityScaleFunction;

  /* Detune of the Modullator */
  struct zyn_detune fm_detune;

  /* Frequency Envelope of the Modullator */
  unsigned char PFMFreqEnvelopeEnabled;
  EnvelopeParams m_fm_frequency_envelope_params;

  /* Frequency Envelope of the Modullator */
  unsigned char PFMAmpEnvelopeEnabled;
  EnvelopeParams m_fm_amplitude_envelope_params;
};

#include "addsynth_component.h"

struct zyn_addsynth
{
  float sample_rate;
  unsigned int polyphony;
  struct note_channel * notes_array;
  bool all_sound_off;
  zyn_fft_handle fft;
  unsigned char velsns;         // velocity sensing (amplitude velocity scale)
  zyn_sample_type oldfreq;      // this is used for portamento

  bool random_panorama;         // whether panorama is random for each note
  float panorama;               // -1.0 for left, 0.0 for center, 1.0 for right

  /* The instrument type - MONO/STEREO
     If the mode is MONO, the panning of voices are not used
     Stereo=true, Mono=false. */
  bool stereo;                  // stereo or mono

  // How the Harmonic Amplitude is applied to voices that use the same oscillator
  bool random_grouping;

  // RESONANCE
  struct zyn_resonance resonance;

  // 0-127, Master volume
  unsigned char PVolume;

  // 0-127, velocity sensing
  unsigned char PAmpVelocityScaleFunction;

  // 0-127
  unsigned char PPunchStrength;

  // 0-127
  unsigned char PPunchTime;

  // 0-127
  unsigned char PPunchStretch;

  // 0-127
  unsigned char PPunchVelocitySensing;

  // Amplitude LFO parameters
  struct zyn_lfo_parameters amplitude_lfo_params;

  EnvelopeParams m_amplitude_envelope_params;

  int filter_type;              /* filter category/supertype, one of ZYN_FILTER_TYPE_XXX */
  FilterParams m_filter_params;
  zyn_filter_sv_handle filter_sv;

  // Velocity sensing amount of the Filter, 0 .. 1
  float m_filter_velocity_sensing_amount;

  // Velocity sensing function of the Filter, 1 .. -1
  float m_filter_velocity_scale_function;

  // Filter LFO parameters
  struct zyn_lfo_parameters filter_lfo_params;

  EnvelopeParams m_filter_envelope_params;

  struct zyn_detune detune;

  /* how much the relative fine detunes of the voices are changed */
  /* -1.0 .. 1.0 */
  float detune_bandwidth;

  // Frequency LFO parameters
  struct zyn_lfo_parameters frequency_lfo_params;

  EnvelopeParams m_frequency_envelope_params;

  zyn_portamento portamento;

  float pitch_bend_range;
  float pitch_bend;
  float pitch_bend_relative_frequency;

  int bandwidth_depth;
  int bandwidth_exponential;
  float bandwidth_relbw;

  int modwheel_depth;
  int modwheel_exponential;
  float modwheel_relmod;

  zyn_sample_type * temporary_samples_ptr; // this array stores some termporary data and it has OSCIL_SIZE elements
  struct zyn_fft_freqs oscillator_fft_frequencies;

  unsigned int voices_count;

  struct zyn_addnote_voice_parameters * voices_params_ptr; /* array with one entry per voice */

  struct zyn_component_descriptor global_components[ZYNADD_GLOBAL_COMPONENTS_COUNT];

  struct zyn_component_descriptor * voices_components;
};

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

float percent_from_0_127(unsigned char value);
unsigned char percent_to_0_127(float value);

void zyn_addsynth_set_bandwidth(struct zyn_addsynth * zyn_addsynth_ptr, int value);
void zyn_addsynth_set_modwheel(struct zyn_addsynth * zyn_addsynth_ptr, int value);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#define ZYN_UPDATE_PITCH_BEND(zyn_addsynth_ptr)       \
  (zyn_addsynth_ptr)->pitch_bend_relative_frequency = \
    pow(2, (zyn_addsynth_ptr)->pitch_bend *           \
        (zyn_addsynth_ptr)->pitch_bend_range /        \
        1200.0);

#endif /* #ifndef ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED */
