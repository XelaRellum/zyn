/*
  ZynAddSubFX - a software synthesizer

  Analog Filter.h - Several analog filters (lowpass, highpass...)
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

#ifndef ANALOG_FILTER_H
#define ANALOG_FILTER_H

struct analog_filter_stage
{
  float c1;
  float c2;
};

class AnalogFilter: public Filter_
{
public:
  AnalogFilter() {};
  ~AnalogFilter() {};

  void init(float sample_rate, unsigned char type, float freq, float q_factor, unsigned char stages);
  void filterout(float *smp);
  void setfreq(float frequency);
  void setfreq_and_q(float frequency,float q_);
  void setq(float q_);

  void settype(int type_);
  void setgain(float dBgain);
  void setstages(int stages_);
  void cleanup();

  // Obtains the response for a given frequency
  float H(float freq);

private:
  float m_sample_rate;

  struct analog_filter_stage m_x[MAX_FILTER_STAGES + 1];
  struct analog_filter_stage m_y[MAX_FILTER_STAGES + 1];
  struct analog_filter_stage m_x_old[MAX_FILTER_STAGES + 1];
  struct analog_filter_stage m_y_old[MAX_FILTER_STAGES + 1];

  void
  singlefilterout(
    float * smp,
    struct analog_filter_stage * x,
    struct analog_filter_stage * y,
    float * c,
    float * d);

  void computefiltercoefs();

  int m_type;                   // The type of the filter, one of ZYN_FILTER_ANALOG_TYPE_XXX
  int m_additional_stages;      // how many times the filter is applied (0->1, 1->2, etc.)
  float m_frequency;            // Frequency given in Hz
  float m_q_factor;             // Q factor (resonance or Q factor)
  float m_gain;                 // the gain of the filter (if are shelf/peak) filters

  int m_order;                    // the order of the filter (number of poles)

  // coefficients
  float m_c[3];
  float m_d[3];

  // old coefficients(used only if some filter paremeters changes very fast, and it needs interpolation)
  float m_c_old[3];
  float m_d_old[3];

  bool m_needs_interpolation;
  bool m_first_time;
  bool m_above_nq;              // whether the frequency is above the nyquist
  bool m_old_above_nq;          // whether last time was above nyquist (used to see if it needs interpolation)

  float m_interpolation_buffer[SOUND_BUFFER_SIZE];
};

#endif
