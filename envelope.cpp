/*
  ZynAddSubFX - a software synthesizer
 
  Envelope.C - Envelope implementation
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

#include "globals.h"
#include "envelope_parameters.h"
#include "envelope.h"

void
Envelope::init(
  float sample_rate,
  EnvelopeParams * parameters_ptr,
  float basefreq)
{
  int i;
  float buffer_duration;

  envpoints = parameters_ptr->Penvpoints;

  if (envpoints > MAX_ENVELOPE_POINTS)
  {
    envpoints = MAX_ENVELOPE_POINTS;
  }

  envsustain = (parameters_ptr->Penvsustain == 0) ? -1 : parameters_ptr->Penvsustain;
  m_forced_release = parameters_ptr->m_forced_release;
  m_stretch = pow(440.0 / basefreq, parameters_ptr->m_stretch / 64.0);
  m_linear = parameters_ptr->m_linear;

  buffer_duration = SOUND_BUFFER_SIZE / sample_rate;

  for (i = 0 ; i < MAX_ENVELOPE_POINTS ; i++)
  {
    float tmp = parameters_ptr->getdt(i) / 1000.0 * m_stretch;
    if (tmp > buffer_duration)
    {
      envdt[i] = buffer_duration / tmp;
    }
    else
    {
      envdt[i] = 2.0;             // any value larger than 1
    }

    envval[i] = parameters_ptr->m_values[i];
  }

  envdt[0] = 1.0;

  currentpoint = 1; // the envelope starts from 1
  m_key_released = false;
  t = 0.0;
  m_finished = false;
  inct = envdt[1];
  envoutval = 0.0;
}

Envelope::Envelope()
{
}

Envelope::~Envelope()
{
}

/*
 * Relase the key (note envelope)
 */
void
Envelope::relasekey()
{
  if (m_key_released)
  {
    return;
  }

  m_key_released = true;

  if (m_forced_release)
  {
    t = 0.0;
  }
}

/*
 * Envelope Output
 */
float
Envelope::envout()
{
  float out;

  if (m_finished)               // if the envelope is finished
  {
    envoutval = envval[envpoints - 1];
    return envoutval;
  }

  if ((currentpoint == envsustain + 1) && !m_key_released) // if it is sustaining now
  {
    envoutval = envval[envsustain];
    return envoutval;
  }

  if (m_key_released && m_forced_release) // do the forced release
  {
    int tmp = (envsustain < 0) ? (envpoints - 1) : (envsustain + 1); // if there is no sustain point, use the last point for release

    if (envdt[tmp] < 0.00000001)
    {
      out = envval[tmp];
    }
    else
    {
      out = envoutval + (envval[tmp] - envoutval) * t;
    }

    t += envdt[tmp] * m_stretch;

    if (t >= 1.0)
    {
      currentpoint = envsustain + 2;
      m_forced_release = false;
      t = 0.0;
      inct = envdt[currentpoint];
      if (currentpoint >= envpoints || envsustain < 0)
      {
        m_finished = true;
      }
    }

    return out;
  }

  if (inct >= 1.0)
  {
    out = envval[currentpoint];
  }
  else
  {
    out = envval[currentpoint - 1] + (envval[currentpoint] - envval[currentpoint - 1]) * t;
  }

  t += inct;

  if (t >= 1.0)
  {
    if (currentpoint >= envpoints - 1)
    {
      m_finished = true;
    }
    else
    {
      currentpoint++;
    }

    t = 0.0;

    inct = envdt[currentpoint];
  }

  envoutval = out;

  return out;
}

/*
 * Envelope Output (dB)
 */
float
Envelope::envout_dB()
{
  float out;

  if (m_linear)
  {
    return envout();
  }

  // first point is always lineary interpolated
  if (currentpoint == 1 &&
      (!m_key_released || !m_forced_release))
  {
    float v1 = dB2rap(envval[0]);
    float v2 = dB2rap(envval[1]);
    out = v1 + (v2 - v1) * t;

    t += inct;

    if (t >= 1.0)
    {
      t = 0.0;
      inct = envdt[2];
      currentpoint++;
      out = v2;
    }

    if (out > 0.001)
    {
      envoutval=rap2dB(out);
    }
    else
    {
      envoutval=-40.0;
    }
  }
  else
  {
    out = dB2rap(envout());
  }

  return out;
}

bool
Envelope::finished()
{
  return m_finished;
}
