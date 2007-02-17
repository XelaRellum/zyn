/*
  ZynAddSubFX - a software synthesizer
 
  EnvelopeParams.h - Parameters for Envelope
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

#ifndef ENVELOPE_PARAMS_H
#define ENVELOPE_PARAMS_H

#define MAX_ENVELOPE_POINTS 40
#define MIN_ENVELOPE_DB -40

#define ZYN_ENVELOPE_MODE_ADSR         1 // ADSR parameters (linear amplitude)
#define ZYN_ENVELOPE_MODE_ADSR_DB      2 // ADSR_dB parameters (dB amplitude)
#define ZYN_ENVELOPE_MODE_ASR          3 // ASR parameters (frequency LFO)
#define ZYN_ENVELOPE_MODE_ADSR_FILTER  4 // ADSR_filter parameters (filter parameters)
#define ZYN_ENVELOPE_MODE_ASR_BW       5 // ASR_bw parameters (bandwidth parameters)

class EnvelopeParams
{
public:
  EnvelopeParams();
  ~EnvelopeParams();

  void
  init_adsr(
    unsigned char stretch,
    BOOL forced_release,
    char attack_duration,
    char decay_duration,
    char sustain_value,
    char release_duration,
    BOOL linear);

  void
  init_asr(
    unsigned char stretch,
    BOOL forced_release,
    char attack_value,
    char attack_duration,
    char release_value,
    char release_duration);

  void
  init_adsr_filter(
    unsigned char stretch,
    BOOL forced_release,
    char attack_value,
    char attack_duration,
    char decay_value,
    char decay_duration,
    char release_duration,
    char release_value);

  void
  init_asr_bw(
    unsigned char stretch,
    BOOL forced_release,
    char attack_value,
    char attack_duration,
    char release_value,
    char release_duration);

  unsigned char
  get_value(
    int index);

  void
  set_value(
    int index,
    unsigned char value);

  unsigned char
  get_duration(
    int index);

  void
  set_duration(
    int index,
    unsigned char duration);

  REALTYPE getdt(unsigned char i);
  void set_point_value(int i, unsigned char value);

  unsigned char Penvpoints;
  unsigned char Penvsustain;    // 127 pentru dezactivat
  unsigned char Penvdt[MAX_ENVELOPE_POINTS];
  float m_values[MAX_ENVELOPE_POINTS];
  unsigned char m_values_params[MAX_ENVELOPE_POINTS];

  // 0 = no stretch
  // 64 = normal stretch (piano-like)
  // 127 = 200% = envelope is stretched about 4 times/octave
  unsigned char m_stretch;

  BOOL m_forced_release;

  BOOL m_linear; // if the amplitude envelope is linear

  unsigned int m_mode;          // one of ZYN_ENVELOPE_MODE_XXX

  int m_attack_duration_index;
  int m_decay_duration_index;
  int m_release_duration_index;

  int m_attack_value_index;
  int m_decay_value_index;
  int m_sustain_value_index;
  int m_release_value_index;
};

#endif
