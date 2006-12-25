/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006 Nedko Arnaudov <nedko@arnaudov.name>
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

struct note_channel
{
  int midinote;               // MIDI note, -1 when note "channel" is not playing
  ADnote * note_ptr;
};

struct zyn_addsynth
{
  unsigned int polyphony;
  struct note_channel * notes_array;
  ADnoteParameters * params_ptr;
  FFTwrapper * fft_ptr;
  Controller * ctl_ptr;
  unsigned char velsns;         // velocity sensing (amplitude velocity scale)
  zyn_sample_type oldfreq;      // this is used for portamento

  BOOL random_panorama;         // whether panorama is random for each note
  float panorama;               // -1.0 for left, 0.0 for center, 1.0 for right

  BOOL stereo;                  // stereo or mono

  // How the Harmonic Amplitude is applied to voices that use the same oscillator
  BOOL random_grouping;

  // Amplitude LFO parameters
  float amplitude_lfo_frequency; // 0.0 .. 1.0
  float amplitude_lfo_depth;    // 0.0 .. 1.0
  BOOL amplitude_lfo_random_start_phase;
  float amplitude_lfo_start_phase; // 0.0 .. 1.0
  BOOL amplitude_lfo_depth_randomness_enabled;
  float amplitude_lfo_depth_randomness; // 0.0 .. 1.0
  BOOL amplitude_lfo_frequency_randomness_enabled;
  float amplitude_lfo_frequency_randomness; // 0.0 .. 1.0
  float amplitude_lfo_delay; // 0.0 .. 4.0, seconds
  float amplitude_lfo_stretch; // -1 .. 1, how the LFO is "stretched" according the note frequency (0=no stretch)
  unsigned int amplitude_lfo_shape;
};

#endif /* #ifndef ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED */
