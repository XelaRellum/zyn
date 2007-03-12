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
Filter::init(FilterParams *pars)
{
  unsigned char Ftype=pars->Ptype;
  unsigned char Fstages=pars->Pstages;

  category=pars->Pcategory;

  switch (category)
  {
  case ZYN_FILTER_TYPE_FORMANT:
    filter = new FormantFilter(pars);
    break;
  case ZYN_FILTER_TYPE_STATE_VARIABLE:
    filter = new SVFilter(Ftype,1000.0,pars->getq(),Fstages);
    filter->outgain = dB2rap(pars->m_gain);
    if (filter->outgain>1.0)
    {
      filter->outgain = sqrt(filter->outgain);
    }
    break;
  case ZYN_FILTER_TYPE_ANALOG:
    m_analog_filter.init(Ftype, 1000.0, pars->getq(), Fstages);
    filter = &m_analog_filter;
    if (Ftype >= ZYN_FILTER_ANALOG_TYPE_PKF2 &&
        Ftype <= ZYN_FILTER_ANALOG_TYPE_HSH2)
    {
      filter->setgain(pars->m_gain);
    }
    else
    {
      filter->outgain = dB2rap(pars->m_gain);
    }
    break;
  default:
    assert(0);
  }
}

Filter::Filter()
{
  filter = NULL;
}

Filter::~Filter()
{
  if (filter != NULL)
  {
    delete filter;
  }
}

void Filter::filterout(REALTYPE *smp)
{
  filter->filterout(smp);
}

void Filter::setfreq(REALTYPE frequency)
{
  filter->setfreq(frequency);
}

void Filter::setfreq_and_q(REALTYPE frequency,REALTYPE q_)
{
  filter->setfreq_and_q(frequency,q_);
}

void Filter::setq(REALTYPE q_)
{
  filter->setq(q_);
}

REALTYPE
Filter::getrealfreq(REALTYPE freqpitch)
{
  if (category == ZYN_FILTER_TYPE_ANALOG ||
      category == ZYN_FILTER_TYPE_STATE_VARIABLE)
  {
    return pow(2.0,freqpitch+9.96578428); // log2(1000)=9.95748
  }
  else
  {
    return freqpitch;
  }
}
