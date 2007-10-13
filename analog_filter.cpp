/*
  ZynAddSubFX - a software synthesizer

  AnalogFilter.C - Several analog filters (lowpass, highpass...)
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
#include "analog_filter.h"

void
AnalogFilter::init(
  float sample_rate,
  unsigned char Ftype,
  float Ffreq,
  float Fq,
  unsigned char Fstages)
{
  int i;

  m_sample_rate = sample_rate;

  m_additional_stages = Fstages;

  for (i = 0 ; i < 3 ; i++)
  {
    m_c_old[i] = 0.0;
    m_d_old[i] = 0.0;
    m_c[i]  =0.0;
    m_d[i] = 0.0;
  }

  m_type = Ftype;

  m_frequency = Ffreq;

  m_q_factor = Fq;

  m_gain = 1.0;

  if (m_additional_stages >= MAX_FILTER_STAGES)
  {
    m_additional_stages = MAX_FILTER_STAGES;
  }

  cleanup();

  m_first_time = false;

  m_above_nq = false;
  m_old_above_nq = false;

  setfreq_and_q(Ffreq, Fq);

  m_first_time = true;

  m_d[0] = 0;                   // this is not used
  outgain = 1.0;
}

void
AnalogFilter::cleanup()
{
  int i;

  for (i=0 ; i < MAX_FILTER_STAGES + 1 ; i++)
  {
    m_x[i].c1 = 0.0;
    m_x[i].c2 = 0.0;

    m_y[i].c1 = 0.0;
    m_y[i].c2 = 0.0;

    m_x_old[i] = m_x[i];
    m_y_old[i] = m_y[i];
  };

  m_needs_interpolation = false;
}

void
AnalogFilter::computefiltercoefs()
{
  float tmp;
  float omega, sn, cs, alpha, beta;
  int zerocoefs = 0;            // this is used if the freq is too high
  float freq;
  float tmpq, tmpgain;

  // do not allow frequencies bigger than samplerate/2
  freq = m_frequency;

  if (freq > m_sample_rate / 2 - 500.0)
  {
    freq = m_sample_rate / 2 - 500.0;
    zerocoefs = 1;
  }

  if (freq < 0.1)
  {
    freq = 0.1;
  }

  // do not allow bogus Q
  if (m_q_factor < 0.0)
  {
    m_q_factor = 0.0;
  }

  if (m_additional_stages == 0)
  {
    tmpq = m_q_factor;
    tmpgain = m_gain;
  }
  else
  {
    tmpq = (m_q_factor > 1.0 ? pow(m_q_factor, 1.0 / (m_additional_stages + 1)) : m_q_factor);
    tmpgain = pow(m_gain, 1.0 / (m_additional_stages + 1));
  }

  // most of theese are implementations of
  // the "Cookbook formulae for audio EQ" by Robert Bristow-Johnson
  // The original location of the Cookbook is:
  // http://www.harmony-central.com/Computer/Programming/Audio-EQ-Cookbook.txt
  switch(m_type)
  {
  case ZYN_FILTER_ANALOG_TYPE_LPF1:
    if (zerocoefs == 0)
    {
      tmp = exp(-2.0 * PI * freq / m_sample_rate);
    }
    else
    {
      tmp = 0.0;
    }

    m_c[0] = 1.0-tmp;
    m_c[1] = 0.0;
    m_c[2] = 0.0;

    m_d[1] = tmp;
    m_d[2] = 0.0;

    m_order = 1;

    break;

  case ZYN_FILTER_ANALOG_TYPE_HPF1:
    if (zerocoefs == 0)
    {
      tmp = exp(-2.0 * PI * freq / m_sample_rate);
    }
    else
    {
      tmp = 0.0;
    }

    m_c[0] = (1.0 + tmp) / 2.0;
    m_c[1] = -(1.0 + tmp) / 2.0;
    m_c[2] = 0.0;

    m_d[1] = tmp;
    m_d[2] = 0.0;

    m_order = 1;

    break;

  case ZYN_FILTER_ANALOG_TYPE_LPF2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      alpha = sn / (2 * tmpq);
      tmp = 1 + alpha;
      m_c[0] = (1.0 - cs) / 2.0 / tmp;
      m_c[1] = (1.0 - cs) / tmp;
      m_c[2] = (1.0 - cs) / 2.0 / tmp;
      m_d[1] = -2 * cs / tmp * (-1);
      m_d[2] = (1 - alpha) / tmp * (-1);
    }
    else
    {
      m_c[0] = 1.0;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;

  case ZYN_FILTER_ANALOG_TYPE_HPF2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      alpha = sn / (2 * tmpq);
      tmp = 1 + alpha;
      m_c[0] = (1.0 + cs) / 2.0 / tmp;
      m_c[1] = -(1.0 + cs) / tmp;
      m_c[2] = (1.0 + cs) / 2.0 / tmp;
      m_d[1] = -2 * cs / tmp * (-1);
      m_d[2] = (1 - alpha) / tmp * (-1);
    }
    else
    {
      m_c[0] = 0.0;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;

  case ZYN_FILTER_ANALOG_TYPE_BPF2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      alpha = sn / (2 * tmpq);
      tmp = 1 + alpha;
      m_c[0] = alpha / tmp * sqrt(tmpq + 1);
      m_c[1] = 0;
      m_c[2] = -alpha / tmp * sqrt(tmpq + 1);
      m_d[1] = -2 * cs / tmp * (-1);
      m_d[2] = (1 - alpha) / tmp * (-1);
    }
    else
    {
      m_c[0] = 0.0;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;

  case ZYN_FILTER_ANALOG_TYPE_NF2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      alpha = sn / (2 * sqrt(tmpq));
      tmp = 1 + alpha;
      m_c[0] = 1 / tmp;
      m_c[1] = -2 * cs / tmp;
      m_c[2] = 1 / tmp;
      m_d[1] = -2 * cs / tmp * (-1);
      m_d[2] = (1 - alpha) / tmp * (-1);
    }
    else
    {
      m_c[0] = 1.0;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;
  case ZYN_FILTER_ANALOG_TYPE_PKF2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      tmpq *= 3.0;
      alpha = sn / (2 * tmpq);
      tmp = 1 + alpha / tmpgain;
      m_c[0] = (1.0 + alpha * tmpgain) / tmp;
      m_c[1] = (-2.0 * cs) / tmp;
      m_c[2] = (1.0 - alpha * tmpgain) / tmp;
      m_d[1] = -2 * cs / tmp * (-1);
      m_d[2] = (1 - alpha / tmpgain) / tmp * (-1);
    }
    else
    {
      m_c[0] = 1.0;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;

  case ZYN_FILTER_ANALOG_TYPE_LSH2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      tmpq = sqrt(tmpq);
      alpha = sn / (2 * tmpq);
      beta = sqrt(tmpgain) / tmpq;
      tmp = (tmpgain + 1.0) + (tmpgain - 1.0) * cs + beta * sn;

      m_c[0] = tmpgain * ((tmpgain + 1.0) - (tmpgain - 1.0) * cs + beta * sn) / tmp;
      m_c[1] = 2.0 * tmpgain * ((tmpgain - 1.0) - (tmpgain + 1.0) * cs) / tmp;
      m_c[2] = tmpgain * ((tmpgain + 1.0) - (tmpgain - 1.0) * cs - beta * sn) / tmp;
      m_d[1] = -2.0 * ((tmpgain - 1.0) + (tmpgain + 1.0) * cs) / tmp * (-1);
      m_d[2] = ((tmpgain + 1.0) + (tmpgain - 1.0) * cs - beta * sn) / tmp * (-1);
    }
    else
    {
      m_c[0] = tmpgain;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;

  case ZYN_FILTER_ANALOG_TYPE_HSH2:
    if (zerocoefs == 0)
    {
      omega = 2 * PI * freq / m_sample_rate;
      sn = sin(omega);
      cs = cos(omega);
      tmpq = sqrt(tmpq);
      alpha = sn / (2 * tmpq);
      beta = sqrt(tmpgain) / tmpq;
      tmp = (tmpgain + 1.0) - (tmpgain - 1.0) * cs + beta * sn;

      m_c[0] = tmpgain * ((tmpgain + 1.0) + (tmpgain - 1.0) * cs + beta * sn) / tmp;
      m_c[1] = -2.0 * tmpgain * ((tmpgain - 1.0) + (tmpgain + 1.0) * cs) / tmp;
      m_c[2] = tmpgain * ((tmpgain + 1.0) + (tmpgain - 1.0) * cs - beta * sn) / tmp;
      m_d[1] = 2.0 * ((tmpgain - 1.0) - (tmpgain + 1.0) * cs) / tmp * (-1);
      m_d[2] = ((tmpgain + 1.0) - (tmpgain - 1.0) * cs - beta * sn) / tmp * (-1);
    }
    else
    {
      m_c[0] = 1.0;
      m_c[1] = 0.0;
      m_c[2] = 0.0;

      m_d[1] = 0.0;
      m_d[2] = 0.0;
    }

    m_order = 2;

    break;

  default:
    assert(0);                  // wrong type
  }
}


void
AnalogFilter::setfreq(float frequency)
{
  float rap;
  bool nyquist_thresh;
  int i;

  if (frequency < 0.1)
  {
    frequency = 0.1;
  }

  rap = m_frequency / frequency;
  if (rap < 1.0)
  {
    rap = 1.0 / rap;
  }

  m_old_above_nq = m_above_nq;
  m_above_nq = frequency > (m_sample_rate / 2 - 500.0);

  nyquist_thresh = ZYN_BOOL_XOR(m_above_nq, m_old_above_nq);

  // if the frequency is changed fast, it needs interpolation (now, filter and coeficients backup)
  if (rap > 3.0 || nyquist_thresh)
  {
    for (i = 0 ; i < 3 ; i++)
    {
      m_c_old[i] = m_c[i];
      m_d_old[i] = m_d[i];
    }

    for (i = 0 ; i < MAX_FILTER_STAGES + 1 ; i++)
    {
      m_x_old[i] = m_x[i];
      m_y_old[i] = m_y[i];
    }

    if (!m_first_time)
    {
      m_needs_interpolation = true;
    }
  }

  m_frequency = frequency;

  computefiltercoefs();

  m_first_time = false;
}

void AnalogFilter::setfreq_and_q(float frequency, float q_factor)
{
  m_q_factor = q_factor;

  setfreq(frequency);
}

void AnalogFilter::setq(float q_factor)
{
  m_q_factor = q_factor;

  computefiltercoefs();
}

void AnalogFilter::settype(int type)
{
  m_type = type;

  computefiltercoefs();
}

void AnalogFilter::setgain(float dBgain)
{
  m_gain = dB2rap(dBgain);

  computefiltercoefs();
}

void AnalogFilter::setstages(int additional_stages)
{
  if (additional_stages >= MAX_FILTER_STAGES)
  {
    m_additional_stages = MAX_FILTER_STAGES - 1;
  }
  else
  {
    m_additional_stages = additional_stages;
  }

  cleanup();

  computefiltercoefs();
}

void
AnalogFilter::singlefilterout(
  float * smp,
  struct analog_filter_stage * x,
  struct analog_filter_stage * y,
  float * c,
  float * d)
{
  int i;
  float y0;

  if (m_order == 1)             // First order filter
  {
    for (i=0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      y0 = smp[i] * c[0] + x->c1 * c[1] + y->c1 * d[1];
      y->c1 = y0;
      x->c1 = smp[i];
      // output
      smp[i] = y0;
    }
  }

  if (m_order == 2)             // Second order filter
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      y0 = smp[i] * c[0] + x->c1 * c[1] + x->c2 * c[2] + y->c1 * d[1] + y->c2 * d[2];
      y->c2 = y->c1;
      y->c1 = y0;
      x->c2 = x->c1;
      x->c1 = smp[i];
      //output
      smp[i] = y0;
    }
  }
}

void AnalogFilter::filterout(float *smp)
{
  int i;
  float x;

  if (m_needs_interpolation)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      m_interpolation_buffer[i] = smp[i];
    }

    for (i = 0 ; i < 1 + m_additional_stages ; i++)
    {
      singlefilterout(m_interpolation_buffer, m_x_old + i , m_y_old + i , m_c_old , m_d_old);
    }
  }

  for (i = 0 ; i < 1 + m_additional_stages ; i++)
  {
    singlefilterout(smp, m_x + i, m_y + i, m_c, m_d);
  }

  if (m_needs_interpolation)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      x = i / (float)SOUND_BUFFER_SIZE;
      smp[i] = m_interpolation_buffer[i] * (1.0 - x) + smp[i] * x;
    }

    m_needs_interpolation = false;
  }

  for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    smp[i] *= outgain;
  }
}

float
AnalogFilter::H(float freq)
{
  float fr;
  float x;
  float y;
  float h;
  int n;

  fr = freq / m_sample_rate * PI * 2.0;
  x = m_c[0];
  y = 0.0;

  for (n = 1 ; n < 3 ; n++)
  {
    x += cos(n * fr) * m_c[n];
    y -= sin(n * fr) * m_c[n];
  }

  h = x * x + y * y;

  x = 1.0;
  y = 0.0;

  for (n = 1 ; n < 3 ; n++)
  {
    x -= cos(n * fr) * m_d[n];
    y += sin(n * fr) * m_d[n];
  }

  h = h / (x * x + y * y);

  return pow(h, (m_additional_stages + 1.0) / 2.0);
}
