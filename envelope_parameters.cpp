/*
  ZynAddSubFX - a software synthesizer
 
  EnvelopeParams.C - Parameters for Envelope
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

#include <math.h>               // pow()
#include <assert.h>

#include "globals.h"
#include "envelope_parameters.h"

EnvelopeParams::EnvelopeParams()
{
  int i;
    
  for (i = 0 ; i < MAX_ENVELOPE_POINTS ; i++)
  {
    Penvdt[i] = 32;
    m_values[i] = 64;
  }

  Penvdt[0] = 0;                // no used
  Penvsustain = 1;
  Penvpoints = 1;
  m_stretch = 64;
  m_forced_release = TRUE;
  m_linear = FALSE;

  m_attack_duration_index = -1;
  m_decay_duration_index = -1;
  m_release_duration_index = -1;

  m_attack_value_index = -1;
  m_decay_value_index = -1;
  m_sustain_value_index = -1;
  m_release_value_index = -1;
}

EnvelopeParams::~EnvelopeParams()
{
}

REALTYPE EnvelopeParams::getdt(unsigned char i)
{
  return (pow(2.0 , Penvdt[i] / 127.0 * 12.0) - 1.0) * 10.0; // miliseconds
}

void EnvelopeParams::set_point_value(int i, unsigned char value)
{
  m_values_params[i] = value;

  switch (m_mode)
  {
  case ZYN_ENVELOPE_MODE_ADSR:
    if (m_linear)
    {
      m_values[i] = value / 127.0;
    }
    else
    {
      m_values[i] = (1.0 - value / 127.0) * MIN_ENVELOPE_DB;
    }
    break;
  case ZYN_ENVELOPE_MODE_ASR:
    m_values[i] = (pow(2,6.0 * fabs(value - 64.0) / 64.0) - 1.0) * 100.0;
    if (value < 64)
    {
      m_values[i] = -m_values[i];
    }
    break;
  case ZYN_ENVELOPE_MODE_ADSR_FILTER:
    m_values[i] = (value - 64.0) / 64.0 * 6.0; // 6 octaves (filtru)
    break;
  case ZYN_ENVELOPE_MODE_ASR_BW:
    m_values[i] = (value - 64.0) / 64.0 * 10;
    break;
  default:
    assert(0);
  }
}

void
EnvelopeParams::init_adsr(
  unsigned char stretch,
  BOOL forced_release,
  char attack_duration,
  char decay_duration,
  char sustain_value,
  char release_duration,
  BOOL linear)
{
  m_stretch = stretch;
  m_forced_release = forced_release;        
  m_linear = linear;
  m_mode = ZYN_ENVELOPE_MODE_ADSR;

  Penvpoints = 4;
  Penvsustain = 2;

  set_point_value(0, 0);

  m_attack_duration_index = 1;
  Penvdt[1] = attack_duration;

  set_point_value(1, 127);

  Penvdt[2] = decay_duration;
  m_decay_duration_index = 2;

  set_point_value(2, sustain_value);
  m_sustain_value_index = 2;

  Penvdt[3] = release_duration;
  m_release_duration_index = 3;

  set_point_value(3, 0);
}

void
EnvelopeParams::init_asr(
  unsigned char stretch,
  BOOL forced_release,
  char attack_value,
  char attack_duration,
  char release_value,
  char release_duration)
{
  m_stretch = stretch;
  m_forced_release = forced_release;        
  m_mode = ZYN_ENVELOPE_MODE_ASR;

  Penvpoints = 3;
  Penvsustain = 1;

  set_point_value(0, attack_value);
  m_attack_value_index = 0;

  Penvdt[1] = attack_duration;
  m_attack_duration_index = 1;

  set_point_value(1, 64);

  set_point_value(2, release_value);
  m_release_value_index = 2;

  Penvdt[2] = release_duration;
  m_release_duration_index = 2;
}

void
EnvelopeParams::init_adsr_filter(
  unsigned char stretch,
  BOOL forced_release,
  char attack_value,
  char attack_duration,
  char decay_value,
  char decay_duration,
  char release_value,
  char release_duration)
{
  m_stretch = stretch;
  m_forced_release = forced_release;        
  m_mode = ZYN_ENVELOPE_MODE_ADSR_FILTER;

  Penvpoints = 4;
  Penvsustain = 2;

  set_point_value(0, attack_value);
  m_attack_value_index = 0;

  Penvdt[1] = attack_duration;
  m_attack_duration_index = 1;

  set_point_value(1, decay_value);
  m_decay_value_index = 1;

  Penvdt[2] = decay_duration;
  m_decay_duration_index = 2;

  set_point_value(2, 64);

  Penvdt[3] = release_duration;
  m_release_duration_index = 3;

  set_point_value(3, release_value);
  m_release_value_index = 3;
}

void
EnvelopeParams::init_asr_bw(
  unsigned char stretch,
  BOOL forced_release,
  char attack_value,
  char attack_duration,
  char release_value,
  char release_duration)
{
  m_stretch = stretch;
  m_forced_release = forced_release;        
  m_mode = ZYN_ENVELOPE_MODE_ASR_BW;

  Penvpoints = 3;
  Penvsustain = 1;

  set_point_value(0, attack_value);
  m_attack_value_index = 0;

  set_point_value(1, 64);

  Penvdt[1] = attack_duration;
  m_attack_duration_index = 1;

  Penvdt[2] = release_duration;
  m_release_duration_index = 2;

  set_point_value(2, release_value);
  m_release_value_index = 2;
}

unsigned char
EnvelopeParams::get_value(
  int index)
{
  assert(index >= 0);
  assert(index < MAX_ENVELOPE_POINTS);
  return m_values_params[index];
}

void
EnvelopeParams::set_value(
  int index,
  unsigned char value)
{
  assert(index >= 0);
  assert(index < MAX_ENVELOPE_POINTS);
  set_point_value(index, value);
}

unsigned char
EnvelopeParams::get_duration(
  int index)
{
  assert(index >= 0);
  assert(index < MAX_ENVELOPE_POINTS);
  return Penvdt[index];
}

void
EnvelopeParams::set_duration(
  int index,
  unsigned char duration)
{
  assert(index >= 0);
  assert(index < MAX_ENVELOPE_POINTS);
  Penvdt[index] = duration;
}
