/*
  ZynAddSubFX - a software synthesizer
 
  FormantFilter.C - formant filters
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

#include "globals.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "filter_parameters.h"
#include "formant_filter.h"

void FormantFilter::init(float sample_rate, FilterParams *pars)
{
  int i, j;

  m_numformants = pars->Pnumformants;

  for (i = 0 ; i < m_numformants ; i++)
  {
    m_formants[i].init(sample_rate, ZYN_FILTER_ANALOG_TYPE_BPF2, 1000.0, 10.0, pars->m_additional_stages, 0.0);
  }

  cleanup();

  for (j = 0 ; j < FF_MAX_VOWELS ; j++)
  {
    for (i = 0 ; i < m_numformants ; i++)
    {
	    m_formantpar[j][i].frequency = pars->getformantfreq(pars->Pvowels[j].formants[i].freq);
	    m_formantpar[j][i].amplitude = pars->getformantamp(pars->Pvowels[j].formants[i].amp);
	    m_formantpar[j][i].q_factor = pars->getformantq(pars->Pvowels[j].formants[i].q);
    }
  }

  for (i = 0 ; i < FF_MAX_FORMANTS ; i++)
  {
    m_oldformantamp[i] = 1.0;
  }

  for (i = 0 ; i < m_numformants ; i++)
  {
    m_currentformants[i].frequency = 1000.0;
    m_currentformants[i].amplitude = 1.0;
    m_currentformants[i].q_factor = 2.0;
  }

  m_formantslowness = pow(1.0 - pars->Pformantslowness / 128.0, 3.0);

  m_sequencesize = pars->Psequencesize;
  if (m_sequencesize == 0)
  {
    m_sequencesize = 1;
  }

  for (i = 0 ; i < m_sequencesize ; i++)
  {
    m_sequence[i].nvowel = pars->Psequence[i].nvowel;
  }
    
  m_vowelclearness = pow(10.0, (pars->Pvowelclearness - 32.0) / 48.0);

  m_sequencestretch = pow(0.1, (pars->Psequencestretch - 32.0) / 48.0);
  if (pars->Psequencereversed)
  {
    m_sequencestretch *= -1.0;
  }

  m_outgain = dB2rap(pars->m_gain);

  m_oldinput = -1.0;

  m_Qfactor = 1.0;
  m_oldQfactor = m_Qfactor;

  m_firsttime = 1;
}

void FormantFilter::cleanup()
{
  int i;

  for (i = 0 ; i < m_numformants ; i++)
  {
    m_formants[i].cleanup();
  }
}

void FormantFilter::setpos(float input){
  int p1,p2;
    
  if (m_firsttime!=0) m_slowinput=input;
	else m_slowinput=m_slowinput*(1.0-m_formantslowness)+input*m_formantslowness;

  if ((fabs(m_oldinput-input)<0.001)&&(fabs(m_slowinput-input)<0.001)&&
      (fabs(m_Qfactor-m_oldQfactor)<0.001)) {
//	m_oldinput=input; daca setez asta, o sa faca probleme la schimbari foarte lente
    m_firsttime=0;
    return;
  } else m_oldinput=input;


  float pos=fmod(input*m_sequencestretch,1.0);if (pos<0.0) pos+=1.0;
    
  F2I(pos*m_sequencesize,p2);
  p1=p2-1;if (p1<0) p1+=m_sequencesize;

  pos=fmod(pos*m_sequencesize,1.0);    
  if (pos<0.0) pos=0.0; else if (pos>1.0) pos=1.0;
  pos=(atan((pos*2.0-1.0)*m_vowelclearness)/atan(m_vowelclearness)+1.0)*0.5;

  p1=m_sequence[p1].nvowel;
  p2=m_sequence[p2].nvowel;
    
  if (m_firsttime!=0) {
    for (int i=0;i<m_numformants;i++){
	    m_currentformants[i].frequency=m_formantpar[p1][i].frequency*(1.0-pos)+m_formantpar[p2][i].frequency*pos;
	    m_currentformants[i].amplitude=m_formantpar[p1][i].amplitude*(1.0-pos)+m_formantpar[p2][i].amplitude*pos;
	    m_currentformants[i].q_factor=m_formantpar[p1][i].q_factor*(1.0-pos)+m_formantpar[p2][i].q_factor*pos;	
	    m_formants[i].setfreq_and_q(m_currentformants[i].frequency,m_currentformants[i].q_factor*m_Qfactor);
	    m_oldformantamp[i]=m_currentformants[i].amplitude;
    };
    m_firsttime=0;
  } else {
    for (int i=0;i<m_numformants;i++){
	    m_currentformants[i].frequency=m_currentformants[i].frequency*(1.0-m_formantslowness)
        +(m_formantpar[p1][i].frequency*(1.0-pos)+m_formantpar[p2][i].frequency*pos)*m_formantslowness;

	    m_currentformants[i].amplitude=m_currentformants[i].amplitude*(1.0-m_formantslowness)
        +(m_formantpar[p1][i].amplitude*(1.0-pos)+m_formantpar[p2][i].amplitude*pos)*m_formantslowness;

	    m_currentformants[i].q_factor=m_currentformants[i].q_factor*(1.0-m_formantslowness)
        +(m_formantpar[p1][i].q_factor*(1.0-pos)+m_formantpar[p2][i].q_factor*pos)*m_formantslowness;

	    m_formants[i].setfreq_and_q(m_currentformants[i].frequency,m_currentformants[i].q_factor*m_Qfactor);
    };
  };
    
  m_oldQfactor=m_Qfactor;
};

void FormantFilter::setfreq(float frequency){
  setpos(frequency);
};

void FormantFilter::setq(float q_){
  m_Qfactor=q_;
  for (int i=0;i<m_numformants;i++) m_formants[i].setq(m_Qfactor*m_currentformants[i].q_factor);
};

void FormantFilter::setfreq_and_q(float frequency,float q_){    
  m_Qfactor=q_;    
  setpos(frequency);
};


void FormantFilter::filterout(float *smp){
  int i,j;
  for (i=0;i<SOUND_BUFFER_SIZE;i++) {
    m_inbuffer[i]=smp[i];
    smp[i]=0.0;
  };
    
  for (j=0;j<m_numformants;j++) {
    for (i=0;i<SOUND_BUFFER_SIZE;i++) m_tmpbuf[i]=m_inbuffer[i]*m_outgain;
    m_formants[j].filterout(m_tmpbuf);

    if (ABOVE_AMPLITUDE_THRESHOLD(m_oldformantamp[j],m_currentformants[j].amplitude))
      for (i=0;i<SOUND_BUFFER_SIZE;i++) smp[i]+=m_tmpbuf[i]*
        INTERPOLATE_AMPLITUDE(m_oldformantamp[j],m_currentformants[j].amplitude,i,SOUND_BUFFER_SIZE);
    else for (i=0;i<SOUND_BUFFER_SIZE;i++) smp[i]+=m_tmpbuf[i]*m_currentformants[j].amplitude;
    m_oldformantamp[j]=m_currentformants[j].amplitude;
  };
};

