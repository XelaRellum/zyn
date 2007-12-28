/*
  ZynAddSubFX - a software synthesizer
 
  FormantFilter.h - formant filter
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

#ifndef FORMANT_FILTER_H
#define FORMANT_FILTER_H

struct zyn_formant
{
  float frequency;
  float amplitude;
  float q_factor;
};

class FormantFilter : public Filter_
{
public:
  FormantFilter() {};
  ~FormantFilter() {};

  void init(float sample_rate, FilterParams *pars);
  void filterout(float *smp);
  void setfreq(float frequency);
  void setfreq_and_q(float frequency,float q_);
  void setq(float q_);

  void cleanup();
private:
  AnalogFilter m_formants[FF_MAX_FORMANTS];
  float m_inbuffer[SOUND_BUFFER_SIZE];
  float m_tmpbuf[SOUND_BUFFER_SIZE];

  struct zyn_formant m_formantpar[FF_MAX_VOWELS][FF_MAX_FORMANTS];
  struct zyn_formant m_currentformants[FF_MAX_FORMANTS];

  struct {
    unsigned char nvowel;
  } m_sequence[FF_MAX_SEQUENCE];
    
  float m_oldformantamp[FF_MAX_FORMANTS];
    
  int m_sequencesize;
  int m_numformants;
  int m_firsttime;
  float m_oldinput;
  float m_slowinput;
  float m_Qfactor;
  float m_formantslowness;
  float m_oldQfactor;
  float m_vowelclearness;
  float m_sequencestretch;
    
  void setpos(float input);
};

#endif
