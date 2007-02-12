/*
  ZynAddSubFX - a software synthesizer
 
  LFO.h - LFO implementation
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

#ifndef LFO_H
#define LFO_H

#define ZYN_LFO_TYPE_FREQUENCY     0
#define ZYN_LFO_TYPE_AMPLITUDE     1
#define ZYN_LFO_TYPE_FILTER        2

class LFO
{
public:
  LFO();

  void
  init(
    float base_frequency,       // note
    const struct zyn_lfo_parameters * parameters_ptr,
    unsigned int type);         // one of ZYN_LFO_TYPE_XXX

  ~LFO();

  float lfoout();

  float amplfoout();

private:
  void computenextincrnd();

  float m_x;

  float m_incx;
  float m_incrnd;
  float m_nextincrnd;

  // used for randomness
  float m_amp1;
  float m_amp2;

  float m_lfointensity;

  BOOL m_depth_randomness_enabled;
  float m_depth_randomness;
  BOOL m_frequency_randomness_enabled;
  float m_frequency_randomness;

  float m_delay;

  char m_shape;
};

#endif
