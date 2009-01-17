/*
  ZynAddSubFX - a software synthesizer
 
  LFO.cpp - LFO class implementation
  Copyright (C) 2006,2007,2008,2009 Nedko Arnaudov <nedko@arnaudov.name>
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
#include <assert.h>

#include "globals.h"
#include "lfo_parameters.h"
#include "lfo.h"

LFO::LFO()
{
}

LFO::~LFO()
{
}

void
LFO::init(
  float sample_rate,
  float base_frequency,       // note
  const struct zyn_lfo_parameters * parameters_ptr,
  unsigned int type)
{
  float lfostretch;
  float lfofreq;

  m_sample_rate = sample_rate;

  // max 2x/octave
  lfostretch = pow(base_frequency / 440.0, parameters_ptr->stretch);

  lfofreq = pow(2, parameters_ptr->frequency * 10.0);
  lfofreq -= 1.0;
  lfofreq /= 12.0;
  lfofreq *= lfostretch;

  m_incx = fabs(lfofreq) * (float)SOUND_BUFFER_SIZE / sample_rate;

  m_x = parameters_ptr->random_start_phase ? zyn_random() : parameters_ptr->start_phase;

  // Limit the Frequency(or else...)
  if (m_incx > 0.49999999)
  {
    m_incx = 0.499999999;
  }

  m_depth_randomness_enabled = parameters_ptr->depth_randomness_enabled;

  if (m_depth_randomness_enabled)
  {
    if (parameters_ptr->depth_randomness < 0.0)
    {
      assert(0);                  // this should be checked by caller
      m_depth_randomness = 0.0;
    }
    else if (parameters_ptr->depth_randomness > 1.0)
    {
      assert(0);                  // this should be checked by caller
      m_depth_randomness = 1.0;
    }
    else
    {
      m_depth_randomness = parameters_ptr->depth_randomness;
    }

    m_amp1 = (1 - m_depth_randomness) + m_depth_randomness * zyn_random();
    m_amp2 = (1 - m_depth_randomness) + m_depth_randomness * zyn_random();
  }
  else
  {
    m_amp1 = 1;
    m_amp2 = 1;
  }

  m_frequency_randomness_enabled = parameters_ptr->frequency_randomness_enabled;

  if (m_frequency_randomness_enabled)
  {
//    m_frequency_randomness = pow(parameters_ptr->frequency_randomness, 2.0) * 2.0 * 4.0;
    m_frequency_randomness = pow(parameters_ptr->frequency_randomness, 2.0) * 4.0;
  }

  switch (type)
  {
  case ZYN_LFO_TYPE_AMPLITUDE:
    m_lfointensity = parameters_ptr->depth;
    break;

  case ZYN_LFO_TYPE_FILTER:     // in octave
    m_lfointensity = parameters_ptr->depth * 4.0;
    break;

  case ZYN_LFO_TYPE_FREQUENCY:  // in centi
    m_lfointensity = pow(2, parameters_ptr->depth * 11.0) - 1.0;
    m_x -= 0.25;                // chance the starting phase
    break;

  default:
    assert(0);
  }

  m_shape = parameters_ptr->shape;
  m_delay = parameters_ptr->delay;
  m_incrnd = m_nextincrnd = 1.0;

  // twice because I want incrnd & nextincrnd to be random
  computenextincrnd();
  computenextincrnd();
}

/*
 * LFO out
 */
float
LFO::lfoout()
{
  float out;
  float tmp;

  switch (m_shape)
  {
  case ZYN_LFO_SHAPE_TYPE_SINE:
    out = cos(m_x * 2.0 * PI);
  case ZYN_LFO_SHAPE_TYPE_TRIANGLE:
    if ((m_x >= 0.0) && (m_x < 0.25))
    {
      out = 4.0 * m_x;
    }
    else if ((m_x > 0.25) && (m_x < 0.75))
    {
      out = 2 - 4 * m_x;
    }
    else
    {
      out = 4.0 * m_x - 4.0;
    }

    break;

  case ZYN_LFO_SHAPE_TYPE_SQUARE:
    if (m_x < 0.5)
    {
      out =- 1;
    }
    else
    {
      out = 1;
    }

    break;

  case ZYN_LFO_SHAPE_TYPE_RAMP_UP:
    out = (m_x - 0.5) * 2.0;
    break;

  case ZYN_LFO_SHAPE_TYPE_RAMP_DOWN:
    out = (0.5 - m_x) * 2.0;
    break;

  case ZYN_LFO_SHAPE_TYPE_EXP_DOWN_1:
    out = pow(0.05, m_x) * 2.0 - 1.0;
    break;

  case ZYN_LFO_SHAPE_TYPE_EXP_DOWN_2:
    out = pow(0.001, m_x) * 2.0 - 1.0;
    break;

  default:
    assert(0);
  };

  if ((m_shape == ZYN_LFO_SHAPE_TYPE_SINE) ||
      (m_shape == ZYN_LFO_SHAPE_TYPE_TRIANGLE))
  {
    out *= m_lfointensity * (m_amp1 + m_x * (m_amp2 - m_amp1));
  }
  else
  {
    out *= m_lfointensity * m_amp2;
  }

  if (m_delay < 0.00001)
  {
    if (m_frequency_randomness_enabled == 0)
    {
      m_x += m_incx;
    }
    else
    {
      tmp = (m_incrnd * (1.0 - m_x) + m_nextincrnd * m_x);
      if (tmp > 1.0)
      {
        tmp = 1.0;
      }
      else if (tmp < 0.0)
      {
        tmp = 0.0;
      }

      m_x += m_incx * tmp;
    }

    if (m_x >= 1)
    {
      m_x = fmod(m_x, 1.0);
      m_amp1 = m_amp2;

      if (m_depth_randomness_enabled)
      {
        m_amp2 = (1 - m_depth_randomness) + m_depth_randomness * zyn_random();
      }
      else
      {
        m_amp2 = 1;
      }

      computenextincrnd();
    }
  }
  else
  {
    m_delay -= (float)SOUND_BUFFER_SIZE / m_sample_rate;
  }

  return out;
}

/*
 * LFO out (for amplitude)
 */
float
LFO::amplfoout()
{
  REALTYPE out;

  out = 1.0 - m_lfointensity + lfoout();

  if (out < -1.0)
  {
    out = -1.0;
  }
  else if (out > 1.0)
  {
    out = 1.0;
  }

  return out;
}

void
LFO::computenextincrnd()
{
  if (!m_frequency_randomness_enabled)
  {
    return;
  }

  m_incrnd = m_nextincrnd;

  m_nextincrnd = pow(0.5, m_frequency_randomness) + zyn_random() * (pow(2.0, m_frequency_randomness) - 1.0);
}
