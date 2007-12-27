/*
  ZynAddSubFX - a software synthesizer

  SVFilter.C - Several state-variable filters
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
#include <stdio.h>
#include <assert.h>

#include "globals.h"
#include "filter_base.h"
#include "sv_filter.h"

void SVFilter::init(float sample_rate, int type, REALTYPE Ffreq, REALTYPE Fq, unsigned char Fstages, float gain)
{
  m_sample_rate = sample_rate;
  stages = Fstages;
  m_type = type;
  freq = Ffreq;
  q = Fq;
  m_gain = 1.0;
  m_outgain = 1.0;
  needsinterpolation = 0;
  firsttime = 1;

  if (stages >= MAX_FILTER_STAGES)
  {
    stages = MAX_FILTER_STAGES;
  }

  cleanup();
  setfreq_and_q(Ffreq,Fq);

  m_outgain = dB2rap(gain);
  if (m_outgain > 1.0)
  {
    m_outgain = sqrt(m_outgain);
  }
}

void SVFilter::cleanup()
{
  int i;

  for (i = 0 ; i < MAX_FILTER_STAGES + 1 ; i++)
  {
    st[i].low = 0.0;
    st[i].high = 0.0;
    st[i].band = 0.0;
    st[i].notch = 0.0;
  }

  oldabovenq = 0;

  abovenq = 0;
}

void SVFilter::computefiltercoefs()
{
  par.f = freq / m_sample_rate * 4.0;

  if (par.f > 0.99999)
  {
    par.f = 0.99999;
  }

  par.q = 1.0 - atan(sqrt(q)) * 2.0 / PI;
  par.q = pow(par.q, 1.0 / (stages + 1));
  par.q_sqrt = sqrt(par.q);
}

void SVFilter::setfreq(REALTYPE frequency)
{
  REALTYPE rap;
  int nyquistthresh;

  if (frequency < 0.1)
  {
    frequency = 0.1;
  }

  rap = freq / frequency;
  if (rap < 1.0)
  {
    rap = 1.0 / rap;
  }

  oldabovenq = abovenq;
  abovenq = frequency > (m_sample_rate / 2 - 500.0);

  nyquistthresh = (abovenq ^ oldabovenq);

  if (rap > 3.0 || nyquistthresh != 0)
  {
    // if the frequency is changed fast, it needs interpolation (now, filter and coeficients backup)
    if (firsttime == 0)
    {
      needsinterpolation = 1;
    }

    ipar = par;
  }

  freq = frequency;

  computefiltercoefs();

  firsttime = 0;
}

void SVFilter::setfreq_and_q(REALTYPE frequency,REALTYPE q_)
{
  q = q_;

  setfreq(frequency);
}

void SVFilter::setq(REALTYPE q_)
{
  q = q_;
  computefiltercoefs();
}

void SVFilter::settype(int type)
{
  m_type = type;

  computefiltercoefs();
}

void SVFilter::setgain(REALTYPE dBgain)
{
  m_gain = dB2rap(dBgain);
  computefiltercoefs();
}

void SVFilter::setstages(int stages_)
{
  if (stages_ >= MAX_FILTER_STAGES)
  {
    stages_ = MAX_FILTER_STAGES - 1;
  }

  stages=stages_;

  cleanup();

  computefiltercoefs();
}

void SVFilter::singlefilterout(REALTYPE *smp,fstage &x,parameters &par)
{
  int i;
  REALTYPE *out=NULL;

  switch(m_type)
  {
  case ZYN_FILTER_SV_TYPE_LOWPASS:
    out = &x.low;
    break;
  case ZYN_FILTER_SV_TYPE_HIGHPASS:
    out = &x.high;
    break;
  case ZYN_FILTER_SV_TYPE_BANDPASS:
    out = &x.band;
    break;
  case ZYN_FILTER_SV_TYPE_NOTCH:
    out = &x.notch;
    break;
  default:
    assert(0);
    return;
  }

  for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    x.low = x.low + par.f * x.band;
    x.high = par.q_sqrt * smp[i] - x.low - par.q * x.band;
    x.band = par.f * x.high + x.band;
    x.notch = x.high + x.low;

    smp[i] = *out;
  }
}

void SVFilter::filterout(REALTYPE *smp)
{
  int i;
  REALTYPE x;

  if (needsinterpolation != 0)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      m_ismp[i] = smp[i];
    }

    for (i=0 ; i < stages + 1 ; i++)
    {
      singlefilterout(m_ismp, st[i], ipar);
    }
  }

  for (i = 0 ; i < stages + 1 ; i++)
  {
    singlefilterout(smp, st[i], par);
  }

  if (needsinterpolation != 0)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      x = i / (REALTYPE)SOUND_BUFFER_SIZE;
      smp[i] = m_ismp[i] * (1.0 - x) + smp[i] * x;
    }

    needsinterpolation = 0;
  }

  for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    smp[i] *= m_outgain;
  }
}
