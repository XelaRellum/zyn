/*
  ZynAddSubFX - a software synthesizer
 
  LFO.cpp - LFO class implementation
  Copyright (C) 2006 Nedko Arnaudov <nedko@arnaudov.name>
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

#include "globals.h"
#include "lfo_parameters.h"
#include "lfo.h"

LFO::LFO(
  LFOParams *lfopars,
  float basefreq)
{
  float lfostretch;
  float lfofreq;
  float tmp;

  if (lfopars->Pstretch == 0)
  {
    lfopars->Pstretch = 1;
  }

  // max 2x/octave
  lfostretch = pow(
    basefreq / 440.0,
    (lfopars->Pstretch - 64.0) / 63.0);

  lfofreq = pow(2, lfopars->Pfreq * 10.0);
  lfofreq -= 1.0;
  lfofreq /= 12.0;
  lfofreq *= lfostretch;

  m_incx = fabs(lfofreq) * (float)SOUND_BUFFER_SIZE / (float)SAMPLE_RATE;

  if (lfopars->Pcontinous==0)
  {
    if (lfopars->Pstartphase == 0)
    {
      m_x = zyn_random();
    }
    else
    {
      m_x = fmod((lfopars->Pstartphase - 64.0) / 127.0 + 1.0, 1.0);
    }
  }
  else
  {
    tmp = fmod(lfopars->time * m_incx, 1.0);

    m_x = fmod((lfopars->Pstartphase - 64.0) / 127.0 + 1.0 + tmp, 1.0);
  }

  // Limit the Frequency(or else...)
  if (m_incx > 0.49999999)
  {
    m_incx = 0.499999999;
  }

  m_lfornd = lfopars->Prandomness / 127.0;

  if (m_lfornd < 0.0)
  {
    m_lfornd = 0.0;
  }
  else if (m_lfornd > 1.0)
  {
    m_lfornd = 1.0;
  }

//    lfofreqrnd = pow(lfopars->Pfreqrand/127.0, 2.0) * 2.0 * 4.0;
  m_lfofreqrnd = pow(lfopars->Pfreqrand/127.0, 2.0) * 4.0;

  switch (lfopars->fel)
  {
  case 1:
    m_lfointensity = lfopars->Pintensity / 127.0;
    break;

  case 2:                       // in octave
    m_lfointensity = lfopars->Pintensity / 127.0 * 4.0;
    break;

  default:                      // in centi
    m_lfointensity = pow(2, lfopars->Pintensity / 127.0 * 11.0) - 1.0;
    m_x -= 0.25;                // chance the starting phase
  }

  m_amp1 = (1 - m_lfornd) + m_lfornd * zyn_random();
  m_amp2 = (1 - m_lfornd) + m_lfornd * zyn_random();
  m_lfotype = lfopars->PLFOtype;
  m_lfodelay = lfopars->Pdelay / 127.0 * 4.0; // 0..4 sec
  m_incrnd = m_nextincrnd = 1.0;
  m_freqrndenabled = lfopars->Pfreqrand != 0;

  // twice because I want incrnd & nextincrnd to be random
  computenextincrnd();
  computenextincrnd();
}

LFO::~LFO()
{
}

/*
 * LFO out
 */
float
LFO::lfoout()
{
  float out;
  float tmp;

  switch (m_lfotype)
  {
  case 1:                       // LFO_TRIANGLE
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

  case 2:                       // LFO_SQUARE
    if (m_x < 0.5)
    {
      out =- 1;
    }
    else
    {
      out = 1;
    }

    break;

  case 3:                       // LFO_RAMPUP
    out = (m_x - 0.5) * 2.0;
    break;

  case 4:                       // LFO_RAMPDOWN
    out = (0.5 - m_x) * 2.0;
    break;

  case 5:                       // LFO_EXP_DOWN 1
    out = pow(0.05, m_x) * 2.0 - 1.0;
    break;

  case 6:                       // LFO_EXP_DOWN 2
    out = pow(0.001, m_x) * 2.0 - 1.0;
    break;

  default:                      // LFO_SINE
    out = cos(m_x * 2.0 * PI);
  };

  if ((m_lfotype == 0) || (m_lfotype == 1))
  {
    out *= m_lfointensity * (m_amp1 + m_x * (m_amp2 - m_amp1));
  }
  else
  {
    out *= m_lfointensity * m_amp2;
  }

  if (m_lfodelay < 0.00001)
  {
    if (m_freqrndenabled == 0)
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
      m_amp2 = (1 - m_lfornd) + m_lfornd * zyn_random();

      computenextincrnd();
    }
  }
  else
  {
    m_lfodelay -= (float)SOUND_BUFFER_SIZE / (float)SAMPLE_RATE;
  }

  return out;
};

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
};


void
LFO::computenextincrnd()
{
  if (m_freqrndenabled == 0)
  {
    return;
  }

  m_incrnd = m_nextincrnd;

  m_nextincrnd = pow(0.5, m_lfofreqrnd) + zyn_random() * (pow(2.0, m_lfofreqrnd) - 1.0);
};

