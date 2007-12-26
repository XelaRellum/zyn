/*
  ZynAddSubFX - a software synthesizer
 
  Filter.C - Filters, uses analog,formant,etc. filters
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
#include "filter_parameters.h"
#include "analog_filter.h"
#include "filter.h"
#include "analog_filter.h"
#include "formant_filter.h"
#include "sv_filter.h"

void
Filter::init(float sample_rate, FilterParams *pars)
{
  unsigned char Ftype=pars->Ptype;

  m_category = pars->m_category;

  switch (m_category)
  {
#if 0
  case ZYN_FILTER_TYPE_FORMANT:
    m_filter = new FormantFilter(sample_rate, pars);
    break;
  case ZYN_FILTER_TYPE_STATE_VARIABLE:
    m_filter = new SVFilter(sample_rate, Ftype, 1000.0, pars->getq(), pars->m_additional_stages);
    m_filter->outgain = dB2rap(pars->m_gain);
    if (m_filter->outgain > 1.0)
    {
      m_filter->outgain = sqrt(m_filter->outgain);
    }
    break;
#endif
  case ZYN_FILTER_TYPE_ANALOG:
    m_analog_filter.init(sample_rate, Ftype, 1000.0, pars->getq(), pars->m_additional_stages);
    m_filter = &m_analog_filter;
    if (Ftype >= ZYN_FILTER_ANALOG_TYPE_PKF2 &&
        Ftype <= ZYN_FILTER_ANALOG_TYPE_HSH2)
    {
      m_filter->setgain(pars->m_gain);
    }
    else
    {
      m_filter->outgain = dB2rap(pars->m_gain);
    }
    break;
  default:
    assert(0);
  }
}

Filter::Filter()
{
  m_filter = NULL;
}

Filter::~Filter()
{
  if (m_filter != NULL)
  {
//    delete m_filter;
  }
}

void Filter::filterout(REALTYPE *smp)
{
  m_filter->filterout(smp);
}

void Filter::setfreq(REALTYPE frequency)
{
  m_filter->setfreq(frequency);
}

void Filter::setfreq_and_q(REALTYPE frequency,REALTYPE q_)
{
  m_filter->setfreq_and_q(frequency,q_);
}

void Filter::setq(REALTYPE q_)
{
  m_filter->setq(q_);
}

REALTYPE
Filter::getrealfreq(REALTYPE freqpitch)
{
  if (m_category == ZYN_FILTER_TYPE_ANALOG ||
      m_category == ZYN_FILTER_TYPE_STATE_VARIABLE)
  {
    return pow(2.0,freqpitch+9.96578428); // log2(1000)=9.95748
  }
  else
  {
    return freqpitch;
  }
}
