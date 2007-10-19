/*
  ZynAddSubFX - a software synthesizer
 
  OscilGen.C - Waveform generator for ADnote
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

#include <stdbool.h>
#include <stdlib.h>
#include <math.h>
#include <assert.h>

#include "globals.h"
#include "fft.h"
#include "resonance.h"
#include "oscillator.h"
#include "log.h"

static
zyn_sample_type
zyn_oscillator_base_function_pulse(float x, float a)
{
  return (fmod(x, 1.0) < a) ? -1.0 : 1.0;
}

static
zyn_sample_type
zyn_oscillator_base_function_saw(float x, float a)
{
  if (a < 0.00001)
  {
    a = 0.00001;
  }
  else if (a > 0.99999)
  {
    a = 0.99999;
  }

  x = fmod(x, 1);
  if (x < a)
  {
    return x / a * 2.0 - 1.0;
  }
  else
  {
    return (1.0 - x) / (1.0 - a) * 2.0 - 1.0;
  }
}

static
zyn_sample_type
zyn_oscillator_base_function_triangle(float x, float a)
{
  x = fmod(x + 0.25, 1);
  a = 1 - a;

  if (a < 0.00001)
  {
    a = 0.00001;
  }

  if (x < 0.5)
  {
    x = x * 4 - 1.0;
  }
  else
  {
    x = (1.0 - x) * 4 - 1.0;
  }

  x /= -a;

  if (x <- 1.0)
  {
    x = -1.0;
  }

  if (x > 1.0)
  {
    x = 1.0;
  }

  return x;
}

static
zyn_sample_type
zyn_oscillator_base_function_power(float x, float a)
{
  x=fmod(x,1);
  if (a<0.00001) a=0.00001;
  else if (a>0.99999) a=0.99999;
  return(pow(x,exp((a-0.5)*10.0))*2.0-1.0);
}

static
zyn_sample_type
zyn_oscillator_base_function_gauss(float x, float a)
{
  x = fmod(x, 1) * 2.0 - 1.0;

  if (a < 0.00001)
  {
    a = 0.00001;
  }

  return exp(-x * x * (exp(a * 8) + 5.0)) * 2.0 - 1.0;
}

static
zyn_sample_type
zyn_oscillator_base_function_diode(float x, float a)
{
  if (a < 0.00001)
  {
    a = 0.00001;
  }
  else if (a > 0.99999)
  {
    a = 0.99999;
  }

  a = a * 2.0 - 1.0;
  x = cos((x + 0.5) * 2.0 * PI) - a;

  if (x < 0.0)
  {
    x = 0.0;
  }

  return x / (1.0 - a) * 2 - 1.0;
}

static
zyn_sample_type
zyn_oscillator_base_function_abssine(float x, float a)
{
  x = fmod(x, 1);
  if (a < 0.00001)
  {
    a = 0.00001;
  }
  else if (a > 0.99999)
  {
    a = 0.99999;
  }

  return sin(pow(x, exp((a - 0.5) * 5.0)) * PI) * 2.0 - 1.0;
}

static
zyn_sample_type
zyn_oscillator_base_function_pulsesine(float x, float a)
{
  if (a < 0.00001)
  {
    a = 0.00001;
  }

  x = (fmod(x, 1) - 0.5) * exp((a - 0.5) * log(128));

  if (x < -0.5)
  {
    x = -0.5;
  }
  else if (x > 0.5)
  {
    x = 0.5;
  }

  x = sin(x * PI * 2.0);

  return x;
}

static
zyn_sample_type
zyn_oscillator_base_function_stretchsine(float x, float a)
{
  float b;

  x = fmod(x + 0.5, 1) * 2.0 - 1.0;

  a = (a - 0.5) * 4;

  if (a > 0.0)
  {
    a *= 2;
  }

  a = pow(3.0, a);

  b = pow(fabs(x), a);

  if (x < 0)
  {
    b = -b;
  }

  return -sin(b * PI);
}

static
zyn_sample_type
zyn_oscillator_base_function_chirp(float x, float a)
{
  x = fmod(x, 1.0) * 2.0 * PI;

  a = (a - 0.5) * 4;

  if (a < 0.0)
  {
    a *= 2.0;
  }

  a = pow(3.0, a);

  return sin(x / 2.0) * sin(a * x * x);
}

static
zyn_sample_type
zyn_oscillator_base_function_absstretchsine(float x, float a)
{
  float b;

  x = fmod(x + 0.5,1) * 2.0 - 1.0;
  a = (a - 0.5) * 9;
  a = pow(3.0, a);

  b = pow(fabs(x), a);

  if (x < 0)
  {
    b = -b;
  }

  return -pow(sin(b * PI), 2);
}

static
zyn_sample_type
zyn_oscillator_base_function_chebyshev(float x, float a)
{
  a = a * a * a * 30.0 + 1.0;

  return cos(acos(x * 2.0 - 1.0) * a);
}

static
zyn_sample_type
zyn_oscillator_base_function_sqr(float x, float a)
{
  a = a * a * a  * a * 160.0 + 0.001;

  return -atan(sin(x * 2.0 * PI) * a);
}

/* 
 * Get the base function
 */
void
zyn_oscillator_get_base_function(
  struct zyn_oscillator * oscillator_ptr,
  zyn_sample_type * samples)
{
  int i;    
  float par;
  float basefuncmodulationpar1;
  float basefuncmodulationpar2;
  float basefuncmodulationpar3;
  float t;

  par = oscillator_ptr->base_function_adjust;
    
  basefuncmodulationpar1 = oscillator_ptr->Pbasefuncmodulationpar1 / 127.0;
  basefuncmodulationpar2 = oscillator_ptr->Pbasefuncmodulationpar2 / 127.0;
  basefuncmodulationpar3 = oscillator_ptr->Pbasefuncmodulationpar3 / 127.0;

  switch(oscillator_ptr->Pbasefuncmodulation)
  {
  case 1:
    basefuncmodulationpar1 = (pow(2, basefuncmodulationpar1 * 5.0) - 1.0) / 10.0;
    basefuncmodulationpar3 = floor(pow(2, basefuncmodulationpar3 * 5.0) - 1.0);
    if (basefuncmodulationpar3 < 0.9999)
    {
      basefuncmodulationpar3 = -1.0;
    }
    break;
  case 2:
    basefuncmodulationpar1 = (pow(2, basefuncmodulationpar1 * 5.0) - 1.0) / 10.0;
    basefuncmodulationpar3 = 1.0 + floor((pow(2, basefuncmodulationpar3 * 5.0) - 1.0));
    break;
  case 3:
    basefuncmodulationpar1 = (pow(2,basefuncmodulationpar1*7.0)-1.0)/10.0;
    basefuncmodulationpar3 = 0.01+(pow(2,basefuncmodulationpar3*16.0)-1.0)/10.0;
    break;
  }

  LOG_DEBUG("%.5f %.5f", basefuncmodulationpar1, basefuncmodulationpar3);

  for (i = 0 ; i < OSCIL_SIZE ; i++)
  {
    t = i * 1.0 / OSCIL_SIZE;

    switch (oscillator_ptr->Pbasefuncmodulation)
    {
    case 1:
      // rev
      t = t * basefuncmodulationpar3 + sin((t + basefuncmodulationpar2) * 2.0 * PI) * basefuncmodulationpar1;
      break;
    case 2:
      // sine
      t = t + sin((t * basefuncmodulationpar3 + basefuncmodulationpar2) * 2.0 * PI) * basefuncmodulationpar1;
      break;
    case 3:
      // power
      t = t + pow((1.0 - cos((t + basefuncmodulationpar2) * 2.0 * PI)) * 0.5, basefuncmodulationpar3) * basefuncmodulationpar1;
      break;
    };
  
    t = t - floor(t);
  
    switch (oscillator_ptr->base_function)
    {
    case ZYN_OSCILLATOR_BASE_FUNCTION_SINE:
      samples[i] = -sin(2.0 * PI * i / OSCIL_SIZE);
    case ZYN_OSCILLATOR_BASE_FUNCTION_TRIANGLE:
      samples[i] = zyn_oscillator_base_function_triangle(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_PULSE:
      samples[i] = zyn_oscillator_base_function_pulse(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_SAW:
      samples[i] = zyn_oscillator_base_function_saw(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_POWER:
      samples[i] = zyn_oscillator_base_function_power(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_GAUSS:
      samples[i] = zyn_oscillator_base_function_gauss(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_DIODE:
      samples[i] = zyn_oscillator_base_function_diode(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_ABS_SINE:
      samples[i] = zyn_oscillator_base_function_abssine(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_PULSE_SINE:
      samples[i] = zyn_oscillator_base_function_pulsesine(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_STRETCH_SINE:
      samples[i] = zyn_oscillator_base_function_stretchsine(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_CHIRP:
      samples[i] = zyn_oscillator_base_function_chirp(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_ABS_STRETCH_SINE:
      samples[i] = zyn_oscillator_base_function_absstretchsine(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_CHEBYSHEV:
      samples[i] = zyn_oscillator_base_function_chebyshev(t, par);
      break;
    case ZYN_OSCILLATOR_BASE_FUNCTION_SQRT:
      samples[i] = zyn_oscillator_base_function_sqr(t, par);
      break;
    default:
      assert(0);
    }
  }
}

/* Shift the harmonics */
static
void
zyn_oscillator_shift_harmonics(
  struct zyn_oscillator * oscillator_ptr)
{
  int i;
  REALTYPE hc,hs;
  int harmonicshift;
  int oldh;

  if (oscillator_ptr->Pharmonicshift == 0)
  {
    return;
  }
    
  harmonicshift = -oscillator_ptr->Pharmonicshift;
    
  if (harmonicshift>0)
  {
    for (i=OSCIL_SIZE/2-2;i>=0;i--)
    {
      oldh = i - harmonicshift;
      if (oldh < 0)
      {
        hc = 0.0;
        hs = 0.0;
      }
      else
      {
        hc = oscillator_ptr->oscilFFTfreqs.c[oldh + 1];
        hs = oscillator_ptr->oscilFFTfreqs.s[oldh + 1];
      }

      oscillator_ptr->oscilFFTfreqs.c[i + 1] = hc;
      oscillator_ptr->oscilFFTfreqs.s[i + 1] = hs;
    }
  }
  else
  {
    for (i = 0 ; i < OSCIL_SIZE / 2 - 1 ; i++)
    {
      oldh = i + abs(harmonicshift);
      if (oldh >= (OSCIL_SIZE / 2 - 1))
      {
        hc = 0.0;
        hs = 0.0;
      }
      else
      {
        hc = oscillator_ptr->oscilFFTfreqs.c[oldh + 1];
        hs = oscillator_ptr->oscilFFTfreqs.s[oldh + 1];

        if (fabs(hc) < 0.000001)
        {
          hc = 0.0;
        }

        if (fabs(hs) < 0.000001)
        {
          hs = 0.0;
        }
      }
      
      oscillator_ptr->oscilFFTfreqs.c[i + 1] = hc;
      oscillator_ptr->oscilFFTfreqs.s[i + 1] = hs;
    }
  }
    
  oscillator_ptr->oscilFFTfreqs.c[0] = 0.0;
}

/* 
 * Change the base function
 */
static
void
zyn_oscillator_change_base_function(
  struct zyn_oscillator * oscillator_ptr)
{
  int i;

  if (oscillator_ptr->base_function != ZYN_OSCILLATOR_BASE_FUNCTION_SINE)
  {
    zyn_oscillator_get_base_function(oscillator_ptr, oscillator_ptr->temporary_samples_ptr);
    zyn_fft_smps2freqs(oscillator_ptr->fft, oscillator_ptr->temporary_samples_ptr, &oscillator_ptr->basefuncFFTfreqs);
    oscillator_ptr->basefuncFFTfreqs.c[0] = 0.0;
  }
  else
  {
    for (i = 0 ; i < OSCIL_SIZE / 2 ; i++)
    {
      oscillator_ptr->basefuncFFTfreqs.s[i] = 0.0;
      oscillator_ptr->basefuncFFTfreqs.c[i] = 0.0;
    }
    //in this case basefuncFFTfreqs_ are not used
  }

  oscillator_ptr->prepared = false;
  oscillator_ptr->base_function_needs_prepare = false;
  oscillator_ptr->oldbasefuncmodulation = oscillator_ptr->Pbasefuncmodulation;
  oscillator_ptr->oldbasefuncmodulationpar1 = oscillator_ptr->Pbasefuncmodulationpar1;
  oscillator_ptr->oldbasefuncmodulationpar2 = oscillator_ptr->Pbasefuncmodulationpar2;
  oscillator_ptr->oldbasefuncmodulationpar3 = oscillator_ptr->Pbasefuncmodulationpar3;
}

void
zyn_oscillator_waveshape_samples(
  int n,
  zyn_sample_type *smps,
  unsigned int type,
  float drive)
{
  int i;
  float ws;
  float tmpv;
  zyn_sample_type tmp;

  ws = drive / 100.0;

  switch (type)
  {
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_ATAN:
    // Arctangent
    ws = pow(10, ws * ws * 3.0) - 1.0 + 0.001;
    for (i = 0 ; i < n ; i++)
    {
      smps[i] = atan(smps[i] * ws) / atan(ws);
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_ASYM1:
    // Asymmetric
    ws = ws * ws * 32.0 + 0.0001;

    if (ws<1.0)
    {
      tmpv = sin(ws) + 0.1;
    }
    else
    {
      tmpv = 1.1;
    }

    for (i = 0 ; i < n ; i++)
    {
      smps[i] = sin(smps[i] * (0.1 + ws - ws * smps[i])) / tmpv;
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_POW:
    // Pow
    ws = ws * ws * ws * 20.0 + 0.0001;
    for (i = 0 ; i < n ; i++)
    {
      smps[i] *= ws;
      if (fabs(smps[i])<1.0)
      {
        smps[i]=(smps[i]-pow(smps[i],3.0))*3.0;
        if (ws<1.0) smps[i]/=ws;
      }
      else
      {
        smps[i] = 0.0;
      }
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_SINE:
    // Sine
    ws = ws * ws * ws * 32.0 + 0.0001;
    if (ws < 1.57)
    {
      tmpv = sin(ws);
    }
    else
    {
      tmpv = 1.0;
    }

    for (i = 0 ; i < n ; i++)
    {
      smps[i] = sin(smps[i] * ws) / tmpv;
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_QUANTISIZE:
    // Quantisize
    ws = ws * ws + 0.000001;
    for (i = 0 ; i < n ; i++) 
    {
      smps[i] = floor(smps[i] / ws + 0.5) * ws;
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_ZIGZAG:
    // Zigzag
    ws = ws * ws * ws * 32 + 0.0001;
    if (ws < 1.0)
    {
      tmpv = sin(ws);
    }
    else
    {
      tmpv = 1.0;
    }

    for (i = 0 ; i < n ; i++) 
    {
      smps[i] = asin(sin(smps[i] * ws)) / tmpv;
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_LIMITER:
    // Limiter
    ws = pow(2.0, -ws * ws * 8.0);
    for (i=0;i<n;i++)
    {
      tmp = smps[i];
      if (fabs(tmp) > ws)
      {
        if (tmp >= 0.0)
        {
          smps[i] = 1.0;
        }
        else
        {
          smps[i] = -1.0;
        }
      }
      else
      {
        smps[i] /= ws;
      }
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_UPPER_LIMITER:
    // Upper Limiter
    ws = pow(2.0, -ws * ws * 8.0);
    for (i = 0 ; i < n ; i++)
    {
      tmp = smps[i];
      if (tmp>ws) smps[i]=ws;
      smps[i]*=2.0;
    };
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_LOWER_LIMITER:
    // Lower Limiter
    ws=pow(2.0,-ws*ws*8.0);
    for (i=0;i<n;i++)
    {
      tmp = smps[i];
      if (tmp<-ws) smps[i]=-ws;
      smps[i]*=2.0;
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_INVERSE_LIMITER:
    // Inverse Limiter
    ws = (pow(2.0, ws * 6.0) - 1.0) / pow(2.0, 6.0);
    for (i = 0 ; i < n ; i++)
    {
      tmp = smps[i];
      if (fabs(tmp) > ws)
      {
        if (tmp >= 0.0)
        {
          smps[i] = tmp - ws;
        }
        else
        {
          smps[i] = tmp + ws;
        }
      }
      else
      {
        smps[i] = 0;
      }
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_CLIP:
    // Clip
    ws = pow(5, ws * ws * 1.0) - 1.0;
    for (i = 0 ; i < n ; i++)
    {
      smps[i] = smps[i] * (ws + 0.5) * 0.9999 - floor(0.5 + smps[i] * (ws + 0.5) * 0.9999);
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_ASYM2:
    // Asym2
    ws = ws * ws * ws * 30 + 0.001;
    if (ws < 0.3)
    {
      tmpv = ws;
    }
    else
    {
      tmpv=1.0;
    }

    for (i = 0 ; i < n ; i++)
    {
      tmp = smps[i] * ws;
      if (tmp > -2.0 && tmp < 1.0)
      {
        smps[i] = tmp * (1.0 - tmp) * (tmp + 2.0) / tmpv;
      }
      else
      {
        smps[i] = 0.0;
      }
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_POW2:
    // Pow2
    ws = ws * ws * ws * 32.0 + 0.0001;
    if (ws < 1.0)
    {
      tmpv = ws * (1 + ws) / 2.0;
    }
    else
    {
      tmpv = 1.0;
    }

    for (i = 0 ; i < n ; i++)
    {
      tmp = smps[i] * ws;
      if (tmp > -1.0 && tmp < 1.618034)
      {
        smps[i] = tmp * (1.0 - tmp) / tmpv;
      }
      else if (tmp > 0.0)
      {
        smps[i] = -1.0;
      }
      else
      {
        smps[i] = -2.0;
      }
    }
    break;
  case ZYN_OSCILLATOR_WAVESHAPE_TYPE_SIGMOID:
    // sigmoid
    ws = pow(ws, 5.0) * 80.0 + 0.0001;
    if (ws > 10.0)
    {
      tmpv = 0.5;
    }
    else
    {
      tmpv = 0.5 - 1.0 / (exp(ws) + 1.0);
    }

    for (i = 0 ; i < n ; i++)
    {
      tmp = smps[i] * ws;
      if (tmp < -10.0)
      {
        tmp = -10.0;
      }
      else if (tmp>10.0)
      {
        tmp = 10.0;
      }

      tmp = 0.5 - 1.0 / (exp(tmp) + 1.0);

      smps[i] = tmp / tmpv;
    }
    break;
    //update to Distorsion::changepar (Ptype max) if there is added more waveshapings functions
  }
}

/* 
 * Waveshape
 */
static
void
zyn_oscillator_waveshape(
  struct zyn_oscillator * oscillator_ptr)
{
  int i;
  float tmp;
  float max;

  if (oscillator_ptr->waveshaping_function == ZYN_OSCILLATOR_WAVESHAPE_TYPE_NONE)
  {
    return;
  }

  // remove the DC
  oscillator_ptr->oscilFFTfreqs.c[0] = 0.0;

  // reduce the amplitude of the freqs near the nyquist
  for (i = 1 ; i < OSCIL_SIZE / 8 ; i++)
  {
    tmp = i / (OSCIL_SIZE / 8.0);
    oscillator_ptr->oscilFFTfreqs.s[OSCIL_SIZE / 2 - i] *= tmp;
    oscillator_ptr->oscilFFTfreqs.c[OSCIL_SIZE / 2 - i] *= tmp;
  }

  zyn_fft_freqs2smps(oscillator_ptr->fft, &oscillator_ptr->oscilFFTfreqs, oscillator_ptr->temporary_samples_ptr);

  // Normalize

  max = 0.0;

  for (i = 0 ; i < OSCIL_SIZE ; i++) 
  {
    if (max < fabs(oscillator_ptr->temporary_samples_ptr[i]))
    {
      max = fabs(oscillator_ptr->temporary_samples_ptr[i]);
    }
  }

  if (max < 0.00001)
  {
    max=1.0;
  }

  max = 1.0 / max;

  for (i = 0 ; i < OSCIL_SIZE ; i++)
  {
    oscillator_ptr->temporary_samples_ptr[i] *= max;
  }

  // Do the waveshaping
  zyn_oscillator_waveshape_samples(
    OSCIL_SIZE,
    oscillator_ptr->temporary_samples_ptr,
    oscillator_ptr->waveshaping_function,
    oscillator_ptr->waveshaping_drive);

  // perform FFT
  zyn_fft_smps2freqs(oscillator_ptr->fft, oscillator_ptr->temporary_samples_ptr, &oscillator_ptr->oscilFFTfreqs);
}

/* 
 * Filter the oscillator
 */
//Filter the oscillator accotding to Pfiltertype and Pfilterpar
static
void
zyn_oscillator_filter(
  struct zyn_oscillator * oscillator_ptr)
{
  REALTYPE par;
  REALTYPE par2;
  REALTYPE max;
  REALTYPE tmp;
  REALTYPE p2;
  REALTYPE x;
  int i;
  REALTYPE gain;
  REALTYPE imax;

  if (oscillator_ptr->Pfiltertype == 0)
  {
    return;
  }

  par = 1.0 - oscillator_ptr->Pfilterpar1 / 128.0;
  par2 = oscillator_ptr->Pfilterpar2 / 127.0;
  max = 0.0;
  tmp = 0.0;

  for (i = 1 ; i < OSCIL_SIZE / 2 ; i++)
  {
    gain = 1.0;
    switch(oscillator_ptr->Pfiltertype)
    {
    case 1:
      // lp
      gain = pow(1.0 - par * par * par * 0.99, i);
      tmp = par2 * par2 * par2 * par2 * 0.5 + 0.0001;
      if (gain < tmp)
      {
        gain = pow(gain, 10.0) / pow(tmp, 9.0);
      }
      break;
    case 2:
      // hp1
      gain = 1.0 - pow(1.0 - par * par, i + 1);
      gain = pow(gain, par2 * 2.0 + 0.1);
      break;
    case 3:
      // hp1b
      if (par < 0.2)
      {
        par = par * 0.25 + 0.15;
      }

      gain = 1.0 - pow(1.0 - par * par * 0.999 + 0.001, i * 0.05 * i + 1.0);
      tmp = pow(5.0, par2 * 2.0);
      gain = pow(gain, tmp);
      break;
    case 4:
      // bp1
      gain = i + 1 - pow(2, (1.0 - par) * 7.5);
      gain = 1.0 / (1.0 + gain * gain / (i + 1.0));
      tmp = pow(5.0, par2 * 2.0);
      gain = pow(gain, tmp);
      if (gain < 1e-5)
      {
        gain = 1e-5;
      }
      break;
    case 5:
      // bs1
      gain = i + 1 - pow(2, (1.0 - par) * 7.5);
      gain = pow(atan(gain / (i / 10.0 + 1)) / 1.57, 6);
      gain = pow(gain, par2 * par2 * 3.9 + 0.1);
      break;
    case 6:
      // lp2
      tmp = pow(par2, 0.33);
      gain = (i + 1 > pow(2, (1.0 - par) * 10) ? 0.0 : 1.0) * par2 + (1.0 - par2);
      break;
    case 7:
      // hp2
      tmp = pow(par2,0.33);
      //tmp = 1.0 - (1.0 - par2) * (1.0 - par2);
      gain = (i + 1 > pow(2, (1.0 - par) * 7) ? 1.0 : 0.0) * par2 + (1.0 - par2);
      if (oscillator_ptr->Pfilterpar1 == 0)
      {
        gain = 1.0;
      }
      break;
    case 8:
      // bp2
      tmp = pow(par2, 0.33);
      //tmp = 1.0 - (1.0 - par2) * (1.0 - par2);
      gain = (fabs(pow(2, (1.0 - par) * 7) - i) > i / 2 + 1 ? 0.0 : 1.0) * par2 + (1.0 - par2);
      break;
    case 9:
      // bs2
      tmp = pow(par2, 0.33);
      gain = (fabs(pow(2, (1.0 - par) * 7) - i) < i / 2 + 1 ? 0.0 : 1.0) * par2 + (1.0 - par2);
      break;
    case 10:
      tmp = pow(5.0, par2 * 2.0 - 1.0);
      tmp = pow(i / 32.0, tmp) * 32.0;
      if (oscillator_ptr->Pfilterpar2 == 64)
      {
        tmp = i;
      }
      gain = cos(par * par * PI / 2.0 * tmp); // cos
      gain *= gain;
      break;
    case 11:tmp=pow(5.0,par2*2.0-1.0);
      tmp=pow(i/32.0,tmp)*32.0;
      if (oscillator_ptr->Pfilterpar2 == 64)
      {
        tmp = i;
      }
      gain = sin(par * par * PI / 2.0 * tmp); // sin
      gain *= gain;
      break;
    case 12:p2=1.0-par+0.2;
      x=i/(64.0*p2*p2); 
      if (x<0.0) x=0.0;
      else if (x>1.0) x=1.0;
      tmp=pow(1.0-par2,2.0);
      gain=cos(x*PI)*(1.0-tmp)+1.01+tmp;//low shelf
      break;
    case 13:
      tmp = (int)pow(2.0, (1.0 - par) * 7.2);
      gain = 1.0;
      if (i == (int)tmp)
      {
        gain = pow(2.0, par2 * par2 * 8.0);
      }
      break;
    }
  
  
    oscillator_ptr->oscilFFTfreqs.s[i] *= gain;
    oscillator_ptr->oscilFFTfreqs.c[i] *= gain;
    tmp = oscillator_ptr->oscilFFTfreqs.s[i] * oscillator_ptr->oscilFFTfreqs.s[i] + oscillator_ptr->oscilFFTfreqs.c[i] * oscillator_ptr->oscilFFTfreqs.c[i];
    if (max < tmp)
    {
      max = tmp;
    }
  }

  max = sqrt(max);
  if (max < 1e-10)
  {
    max = 1.0;
  }

  imax = 1.0 / max;
  for (i = 1 ; i < OSCIL_SIZE / 2 ; i++)
  {
    oscillator_ptr->oscilFFTfreqs.s[i] *= imax; 
    oscillator_ptr->oscilFFTfreqs.c[i] *= imax; 
  }
}

/* 
 * Do the Frequency Modulation of the Oscil
 */
static
void
zyn_oscillator_modulation(
  struct zyn_oscillator * oscillator_ptr)
{
  int i;
  float modulationpar1;
  float modulationpar2;
  float modulationpar3;
  float tmp;
  float max;
  float t;
  int poshi;
  float poslo;

  oscillator_ptr->oldmodulation = oscillator_ptr->Pmodulation;
  oscillator_ptr->oldmodulationpar1 = oscillator_ptr->Pmodulationpar1;
  oscillator_ptr->oldmodulationpar2 = oscillator_ptr->Pmodulationpar2;
  oscillator_ptr->oldmodulationpar3 = oscillator_ptr->Pmodulationpar3;

  if (oscillator_ptr->Pmodulation==0)
  {
    return;
  }

  modulationpar1 = oscillator_ptr->Pmodulationpar1 / 127.0;
  modulationpar2 = 0.5 - oscillator_ptr->Pmodulationpar2 / 127.0;
  modulationpar3 = oscillator_ptr->Pmodulationpar3 / 127.0;

  switch(oscillator_ptr->Pmodulation)
  {
  case 1:
    modulationpar1 = (pow(2, modulationpar1 * 7.0) - 1.0) / 100.0;
    modulationpar3 = floor((pow(2, modulationpar3 * 5.0) - 1.0));
    if (modulationpar3 < 0.9999)
    {
      modulationpar3 = -1.0;
    }
    break;
  case 2:
    modulationpar1 = (pow(2, modulationpar1 * 7.0) - 1.0) / 100.0;
    modulationpar3 = 1.0 + floor(pow(2, modulationpar3 * 5.0) - 1.0);
    break;
  case 3:
    modulationpar1 = (pow(2, modulationpar1 * 9.0) - 1.0) / 100.0;
    modulationpar3 = 0.01 + (pow(2, modulationpar3 * 16.0) - 1.0) / 10.0;
    break;
  }

  oscillator_ptr->oscilFFTfreqs.c[0] = 0.0; // remove the DC

  // reduce the amplitude of the freqs near the nyquist
  for (i = 1 ; i < OSCIL_SIZE / 8 ; i++)
  {
    tmp = i / (OSCIL_SIZE / 8.0);
    oscillator_ptr->oscilFFTfreqs.s[OSCIL_SIZE / 2 - i] *= tmp;
    oscillator_ptr->oscilFFTfreqs.c[OSCIL_SIZE / 2 - i] *= tmp;
  }

  zyn_fft_freqs2smps(oscillator_ptr->fft, &oscillator_ptr->oscilFFTfreqs, oscillator_ptr->temporary_samples_ptr);

  // Normalize

  max = 0.0;

  for (i = 0 ; i < OSCIL_SIZE ; i++)
  {
    if (max < fabs(oscillator_ptr->temporary_samples_ptr[i]))
    {
      max = fabs(oscillator_ptr->temporary_samples_ptr[i]);
    }
  }

  if (max < 0.00001)
  {
    max = 1.0;
  }

  max = 1.0 / max;
  
  for (i=0 ; i < OSCIL_SIZE ; i++)
  {
    oscillator_ptr->modulation_temp[i] = oscillator_ptr->temporary_samples_ptr[i] * max;
  }

  for (i = 0 ; i < ZYN_OSCILLATOR_EXTRA_POINTS ; i++)
  {
    oscillator_ptr->modulation_temp[i + OSCIL_SIZE] = oscillator_ptr->temporary_samples_ptr[i] * max;
  }
    
  // Do the modulation
  for (i = 0 ; i < OSCIL_SIZE ; i++)
  {
    t = i * 1.0 / OSCIL_SIZE;

    switch(oscillator_ptr->Pmodulation)
    {
    case 1:
      // rev
      t = t * modulationpar3 + sin((t + modulationpar2) * 2.0 * PI) * modulationpar1;
      break;
    case 2:
      //sine
      t = t + sin((t * modulationpar3 + modulationpar2) * 2.0 * PI) * modulationpar1;
      break;
    case 3:
      // power
      t = t + pow((1.0 - cos((t + modulationpar2) * 2.0 * PI)) * 0.5, modulationpar3) * modulationpar1;
      break;
    }
  
    t = (t - floor(t)) * OSCIL_SIZE;
  
    poshi = (int)t;
    poslo = t - floor(t);

    oscillator_ptr->temporary_samples_ptr[i] = oscillator_ptr->modulation_temp[poshi] * (1.0 - poslo) + oscillator_ptr->modulation_temp[poshi + 1] * poslo;
  }

  // perform FFT
  zyn_fft_smps2freqs(oscillator_ptr->fft, oscillator_ptr->temporary_samples_ptr, &oscillator_ptr->oscilFFTfreqs);
}

/* 
 * Adjust the spectrum
 */
static
void
zyn_oscillator_spectrum_adjust(
  struct zyn_oscillator * oscillator_ptr)
{
  float par;
  int i;
  float max;
  float tmp;
  float mag;
  float phase;

  if (oscillator_ptr->Psatype == 0)
  {
    return;
  }

  par = oscillator_ptr->Psapar / 127.0;
  switch (oscillator_ptr->Psatype)
  {
  case 1:
    par = 1.0 - par * 2.0;
    if (par >= 0.0)
    {
      par = pow(5.0, par);
    }
    else
    {
      par = pow(8.0, par);
    }
    break;
  case 2:
    par = pow(10.0, (1.0 - par) * 3.0) * 0.25;
    break;
  case 3:
    par = pow(10.0, (1.0 - par) * 3.0) * 0.25;
    break;
  }

  max = 0.0;
  for (i = 0 ; i < OSCIL_SIZE / 2 ; i++)
  {
    tmp = pow(oscillator_ptr->oscilFFTfreqs.c[i], 2) + pow(oscillator_ptr->oscilFFTfreqs.s[i], 2.0);
    if (max < tmp)
    {
      max = tmp;
    }
  }

  max = sqrt(max) / OSCIL_SIZE * 2.0;
  if (max < 1e-8)
  {
    max = 1.0;
  }
    
  for (i = 0 ; i < OSCIL_SIZE / 2 ; i++)
  {
    mag = sqrt(pow(oscillator_ptr->oscilFFTfreqs.s[i], 2) + pow(oscillator_ptr->oscilFFTfreqs.c[i], 2.0)) / max;
    phase = atan2(oscillator_ptr->oscilFFTfreqs.s[i], oscillator_ptr->oscilFFTfreqs.c[i]);
  
    switch (oscillator_ptr->Psatype)
    {
    case 1:
      mag = pow(mag, par);
      break;
    case 2:
      if (mag < par)
      {
        mag = 0.0;
      }
      break;
    case 3:
      mag /= par;
      if (mag > 1.0)
      {
        mag = 1.0;
      }
      break;
    }
    oscillator_ptr->oscilFFTfreqs.c[i] = mag * cos(phase);
    oscillator_ptr->oscilFFTfreqs.s[i] = mag * sin(phase);
  }  
}

/* 
 * Prepare the Oscillator
 */
static
void
zyn_oscillator_prepare(
  struct zyn_oscillator * oscillator_ptr)
{
  int i, j, k;
  REALTYPE a, b, c, d, hmagnew;
  
  if (oscillator_ptr->base_function_needs_prepare ||
      (oscillator_ptr->oldbasefuncmodulation != oscillator_ptr->Pbasefuncmodulation) ||
      (oscillator_ptr->oldbasefuncmodulationpar1 != oscillator_ptr->Pbasefuncmodulationpar1) ||
      (oscillator_ptr->oldbasefuncmodulationpar2 != oscillator_ptr->Pbasefuncmodulationpar2) ||
      (oscillator_ptr->oldbasefuncmodulationpar3 != oscillator_ptr->Pbasefuncmodulationpar3))
  { 
    zyn_oscillator_change_base_function(oscillator_ptr);
  }

  for (i = 0 ; i < MAX_AD_HARMONICS ; i++)
  {
    oscillator_ptr->hphase[i] = (oscillator_ptr->Phphase[i] - 64.0) / 64.0 * PI / (i + 1);
  }

  for (i = 0 ; i < MAX_AD_HARMONICS ; i++)
  {
    hmagnew = 1.0 - fabs(oscillator_ptr->Phmag[i] / 64.0 - 1.0);
    switch(oscillator_ptr->Phmagtype)
    {
    case 1:
      oscillator_ptr->hmag[i] = exp(hmagnew * log(0.01));
      break;
    case 2:
      oscillator_ptr->hmag[i] = exp(hmagnew * log(0.001));
      break;
    case 3:
      oscillator_ptr->hmag[i] = exp(hmagnew * log(0.0001));
      break;
    case 4:
      oscillator_ptr->hmag[i] = exp(hmagnew * log(0.00001));
      break;
    default:
      oscillator_ptr->hmag[i] = 1.0 - hmagnew;
      break; 
    }

    if (oscillator_ptr->Phmag[i] < 64)
    {
      oscillator_ptr->hmag[i] =- oscillator_ptr->hmag[i];
    }
  }
    
  // remove the harmonics where Phmag[i] == 64
  for (i = 0 ; i < MAX_AD_HARMONICS ; i++)
  {
    if (oscillator_ptr->Phmag[i]==64)
    {
      oscillator_ptr->hmag[i] = 0.0;
    }
  }

  for (i=0 ; i < OSCIL_SIZE / 2 ; i++)
  {
    oscillator_ptr->oscilFFTfreqs.c[i] = 0.0;
    oscillator_ptr->oscilFFTfreqs.s[i] = 0.0;
  }

  if (oscillator_ptr->base_function == ZYN_OSCILLATOR_BASE_FUNCTION_SINE)
  {
    // the sine case
    for (i = 0 ; i < MAX_AD_HARMONICS ; i++)
    {
      oscillator_ptr->oscilFFTfreqs.c[i + 1] = -oscillator_ptr->hmag[i] * sin(oscillator_ptr->hphase[i] * (i + 1)) / 2.0;
      oscillator_ptr->oscilFFTfreqs.s[i + 1] = oscillator_ptr->hmag[i] * cos(oscillator_ptr->hphase[i] * (i + 1)) / 2.0;
    }
  }
  else
  {
    for (j = 0 ; j < MAX_AD_HARMONICS ; j++)
    {
      if (oscillator_ptr->Phmag[j] == 64)
      {
        continue;
      }

      for (i = 1 ; i < OSCIL_SIZE / 2 ; i++)
      {
        k = i * (j + 1);

        if (k >= OSCIL_SIZE / 2)
        {
          break;
        }

        a = oscillator_ptr->basefuncFFTfreqs.c[i];
        b = oscillator_ptr->basefuncFFTfreqs.s[i];
        c = oscillator_ptr->hmag[j] * cos(oscillator_ptr->hphase[j] * k);
        d = oscillator_ptr->hmag[j] * sin(oscillator_ptr->hphase[j] * k);
        oscillator_ptr->oscilFFTfreqs.c[k] += a * c - b * d;
        oscillator_ptr->oscilFFTfreqs.s[k] += a * d + b * c;
      }
    }
  }

  if (oscillator_ptr->Pharmonicshiftfirst != 0)
  {
    zyn_oscillator_shift_harmonics(oscillator_ptr);
  }

  if (oscillator_ptr->Pfilterbeforews == 0)
  {
    zyn_oscillator_waveshape(oscillator_ptr);
    zyn_oscillator_filter(oscillator_ptr);
  }
  else
  {
    zyn_oscillator_filter(oscillator_ptr);
    zyn_oscillator_waveshape(oscillator_ptr);
  }

  zyn_oscillator_modulation(oscillator_ptr);
  zyn_oscillator_spectrum_adjust(oscillator_ptr);

  if (oscillator_ptr->Pharmonicshiftfirst == 0)
  {
    zyn_oscillator_shift_harmonics(oscillator_ptr);
  }

  oscillator_ptr->oscilFFTfreqs.c[0] = 0.0;

  oscillator_ptr->oldhmagtype = oscillator_ptr->Phmagtype;
  oscillator_ptr->oldharmonicshift = oscillator_ptr->Pharmonicshift + oscillator_ptr->Pharmonicshiftfirst * 256;

  oscillator_ptr->prepared = true;
}

static
void
zyn_oscillator_defaults(
  struct zyn_oscillator * oscillator_ptr)
{
  int i;

  oscillator_ptr->oldhmagtype = 0;
  oscillator_ptr->oldbasefuncmodulation = 0;
  oscillator_ptr->oldharmonicshift = 0;
  oscillator_ptr->oldbasefuncmodulationpar1 = 0;
  oscillator_ptr->oldbasefuncmodulationpar2 = 0;
  oscillator_ptr->oldbasefuncmodulationpar3 = 0;
  oscillator_ptr->oldmodulation = 0;
  oscillator_ptr->oldmodulationpar1 = 0;
  oscillator_ptr->oldmodulationpar2 = 0;
  oscillator_ptr->oldmodulationpar3 = 0;

  for (i = 0 ; i < MAX_AD_HARMONICS ; i++)
  {
    oscillator_ptr->hmag[i] = 0.0;
    oscillator_ptr->hphase[i] = 0.0;
    oscillator_ptr->Phmag[i] = 64;
    oscillator_ptr->Phphase[i] = 64;
  }

  oscillator_ptr->Phmag[0] = 127;
  oscillator_ptr->Phmagtype = 0;
  if (oscillator_ptr->ADvsPAD)
  {
    oscillator_ptr->Prand = 127; // max phase randomness (usefull if the oscil will be imported to a ADsynth from a PADsynth
  }
  else
  {
    oscillator_ptr->Prand = 64; // no randomness
  }

  oscillator_ptr->base_function = ZYN_OSCILLATOR_BASE_FUNCTION_SINE;
  oscillator_ptr->base_function_adjust = 0.5;

  oscillator_ptr->Pbasefuncmodulation = 0;
  oscillator_ptr->Pbasefuncmodulationpar1 = 64;
  oscillator_ptr->Pbasefuncmodulationpar2 = 64;
  oscillator_ptr->Pbasefuncmodulationpar3 = 32;

  oscillator_ptr->Pmodulation = 0;
  oscillator_ptr->Pmodulationpar1 = 64;
  oscillator_ptr->Pmodulationpar2 = 64;
  oscillator_ptr->Pmodulationpar3 = 32;

  oscillator_ptr->waveshaping_function = ZYN_OSCILLATOR_WAVESHAPE_TYPE_NONE;
  oscillator_ptr->waveshaping_drive = 50;
  oscillator_ptr->Pfiltertype = 0;
  oscillator_ptr->Pfilterpar1 = 64;
  oscillator_ptr->Pfilterpar2 = 64;
  oscillator_ptr->Pfilterbeforews = 0;
  oscillator_ptr->Psatype = 0;
  oscillator_ptr->Psapar = 64;

  oscillator_ptr->Pamprandpower = 64;
  oscillator_ptr->Pamprandtype = 0;
    
  oscillator_ptr->Pharmonicshift = 0;
  oscillator_ptr->Pharmonicshiftfirst = 0;

  oscillator_ptr->Padaptiveharmonics = 0;
  oscillator_ptr->Padaptiveharmonicspower = 100;
  oscillator_ptr->Padaptiveharmonicsbasefreq = 128;
  oscillator_ptr->Padaptiveharmonicspar = 50;
    
  for (i = 0 ; i < OSCIL_SIZE / 2 ; i++)
  {
    oscillator_ptr->oscilFFTfreqs.s[i] = 0.0;
    oscillator_ptr->oscilFFTfreqs.c[i] = 0.0;
    oscillator_ptr->basefuncFFTfreqs.s[i] = 0.0;
    oscillator_ptr->basefuncFFTfreqs.c[i] = 0.0;
  }

  oscillator_ptr->prepared = false;
  oscillator_ptr->base_function_needs_prepare = true;
  oscillator_ptr->oldfilterpars = 0;
  oscillator_ptr->oldsapars = 0;

  zyn_oscillator_prepare(oscillator_ptr);
}

void
zyn_oscillator_init(
  struct zyn_oscillator * oscillator_ptr,
  float sample_rate,
  zyn_fft_handle fft,
  struct zyn_resonance * resonance_ptr,
  zyn_sample_type * temporary_samples_ptr,
  struct zyn_fft_freqs * oscillator_fft_frequencies_ptr)
{
  oscillator_ptr->sample_rate = sample_rate;

  oscillator_ptr->fft = fft;
  oscillator_ptr->resonance_ptr = resonance_ptr;

  oscillator_ptr->temporary_samples_ptr = temporary_samples_ptr;
  oscillator_ptr->oscillator_fft_frequencies_ptr = oscillator_fft_frequencies_ptr;

  zyn_fft_freqs_init(&oscillator_ptr->oscilFFTfreqs, OSCIL_SIZE / 2);
  zyn_fft_freqs_init(&oscillator_ptr->basefuncFFTfreqs, OSCIL_SIZE / 2);

  oscillator_ptr->randseed = 1;
  oscillator_ptr->ADvsPAD = false;

  zyn_oscillator_defaults(oscillator_ptr);
}

void
zyn_oscillator_uninit(
  struct zyn_oscillator * oscillator_ptr)
{
  zyn_fft_freqs_uninit(&oscillator_ptr->basefuncFFTfreqs);
  zyn_fft_freqs_uninit(&oscillator_ptr->oscilFFTfreqs);
}

void
zyn_oscillator_new_rand_seed(
  struct zyn_oscillator * oscillator_ptr,
  unsigned int randseed)
{
  oscillator_ptr->randseed = randseed;
}

static
void
zyn_oscillator_adaptive_harmonic(
  struct zyn_oscillator * oscillator_ptr,
  struct zyn_fft_freqs * freqs_ptr,
  float freq)
{
  struct zyn_fft_freqs inf;
  int i;
  float hc;
  float hs;
  float basefreq;
  float power;
  float rap;
  bool down;
  float h;
  int high;
  float low;

  if (oscillator_ptr->Padaptiveharmonics == 0 /* || freq < 1.0*/)
  {
    return;
  }

  if (freq < 1.0)
  {
    freq = 440.0;
  }

  zyn_fft_freqs_init(&inf, OSCIL_SIZE / 2);

  for (i = 0 ; i < OSCIL_SIZE / 2 ; i++)
  {
    inf.s[i] = freqs_ptr->s[i];
    inf.c[i] = freqs_ptr->c[i];
    freqs_ptr->s[i] = 0.0;
    freqs_ptr->c[i] = 0.0;
  }

  inf.c[0]=0.0;inf.s[0]=0.0;    
    
  hc = 0.0;
  hs = 0.0;
  basefreq = 30.0 * pow(10.0, oscillator_ptr->Padaptiveharmonicsbasefreq / 128.0);
  power = (oscillator_ptr->Padaptiveharmonicspower + 1.0) / 101.0;
    
  rap = freq / basefreq;

  rap = pow(rap, power);

  if (rap > 1.0)
  {
    rap = 1.0 / rap;
    down = true;
  }
  else
  {
    down = false;
  }
    
  for (i = 0 ; i < OSCIL_SIZE / 2 - 2 ; i++)
  {
    h = i * rap;
    high = (int)(i * rap);
    low = fmod(h, 1.0);

    if (high >= OSCIL_SIZE / 2 - 2)
    {
      break;
    }
    else
    {
      if (down)
      {
        freqs_ptr->c[high] += inf.c[i] * (1.0 - low);
        freqs_ptr->s[high] += inf.s[i] * (1.0 - low);
        freqs_ptr->c[high + 1] += inf.c[i] * low;
        freqs_ptr->s[high + 1] += inf.s[i] * low;
      }
      else
      {
        hc = inf.c[high] * (1.0 - low) + inf.c[high + 1] * low;
        hs = inf.s[high] * (1.0 - low) + inf.s[high + 1] * low;
      }

      if (fabs(hc) < 0.000001)
      {
        hc = 0.0;
      }

      if (fabs(hs) < 0.000001)
      {
        hs = 0.0;
      }
    }
  
    if (!down)
    {
      if (i == 0)
      {
        // corect the aplitude of the first harmonic
        hc *= rap;
        hs *= rap;
      }

      freqs_ptr->c[i] = hc;
      freqs_ptr->s[i] = hs;
    }
  }
    
  freqs_ptr->c[1] += freqs_ptr->c[0];
  freqs_ptr->s[1] += freqs_ptr->s[0];
  freqs_ptr->c[0] = 0.0;
  freqs_ptr->s[0] = 0.0;    

  zyn_fft_freqs_uninit(&inf);
}

static
void
zyn_oscillator_adaptive_harmonic_post_process(
  struct zyn_oscillator * oscillator_ptr,
  float *f,
  int size)
{
  float inf[size];
  int i;
  float par;
  int nh;
  int sub_vs_add;

  if (oscillator_ptr->Padaptiveharmonics <= 1)
  {
    return;
  }

  par = oscillator_ptr->Padaptiveharmonicspar * 0.01;
  par = 1.0 - pow(1.0 - par, 1.5);
    
  for (i = 0 ; i < size ; i++)
  {
    inf[i] = f[i] * par;
    f[i] = f[i] * (1.0 - par);
  }
    
  if (oscillator_ptr->Padaptiveharmonics == 2)
  { 
    // 2n+1
    for (i = 0 ; i < size ; i++)
    {
      // i=0 pt prima armonica,etc.
      if (i % 2 == 0)
      {
        f[i] += inf[i];
      }
    }
  }
  else
  {
    // celelalte moduri
    nh = (oscillator_ptr->Padaptiveharmonics - 3) / 2 + 2;
    sub_vs_add = (oscillator_ptr->Padaptiveharmonics - 3) % 2;
    if (sub_vs_add==0)
    {
      for (i = 0 ; i < size ; i++)
      {
        if ((i + 1) % nh == 0)
        {
          f[i] += inf[i];
        }
      }
    }
    else
    {
      for (i = 0 ; i < size / nh - 1 ; i++)
      {
        f[(i + 1) * nh - 1] += inf[i];
      }
    }
  }
}

/* 
 * Get the oscillator function
 */
short
zyn_oscillator_get(
  struct zyn_oscillator * oscillator_ptr,
  zyn_sample_type *smps,
  float freqHz,
  bool resonance)
{
  int i;
  int nyquist;
  int outpos;
  int newpars;
  int realnyquist;
  float rnd;
  float angle;
  float a;
  float b;
  float c;
  float d;
  unsigned int realrnd;
  float power;
  float normalize;
  float amp;
  float rndfreq; 
  float sum;
  int j;
   
  if (oscillator_ptr->oldhmagtype != oscillator_ptr->Phmagtype)
  {
    oscillator_ptr->prepared = false;
  }

  newpars = oscillator_ptr->Pfiltertype * 256;
  newpars += oscillator_ptr->Pfilterpar1;
  newpars += oscillator_ptr->Pfilterpar2 * 65536;
  newpars += oscillator_ptr->Pfilterbeforews * 16777216;

  if (oscillator_ptr->oldfilterpars != newpars)
  {
    oscillator_ptr->prepared = false;
    oscillator_ptr->oldfilterpars = newpars;
  }

  newpars = oscillator_ptr->Psatype * 256 + oscillator_ptr->Psapar;

  if (oscillator_ptr->oldsapars != newpars)
  {
    oscillator_ptr->prepared = false;
    oscillator_ptr->oldsapars = newpars;
  }

  if (oscillator_ptr->oldbasefuncmodulation != oscillator_ptr->Pbasefuncmodulation ||
      oscillator_ptr->oldbasefuncmodulationpar1 != oscillator_ptr->Pbasefuncmodulationpar1 ||
      oscillator_ptr->oldbasefuncmodulationpar2 != oscillator_ptr->Pbasefuncmodulationpar2 ||
      oscillator_ptr->oldbasefuncmodulationpar3 != oscillator_ptr->Pbasefuncmodulationpar3)
  {
    oscillator_ptr->prepared = false;
  }

  if (oscillator_ptr->oldmodulation != oscillator_ptr->Pmodulation ||
      oscillator_ptr->oldmodulationpar1 != oscillator_ptr->Pmodulationpar1 ||
      oscillator_ptr->oldmodulationpar2 != oscillator_ptr->Pmodulationpar2 ||
      oscillator_ptr->oldmodulationpar3 != oscillator_ptr->Pmodulationpar3)
  {
    oscillator_ptr->prepared = false;
  }

  if (oscillator_ptr->oldharmonicshift != oscillator_ptr->Pharmonicshift + oscillator_ptr->Pharmonicshiftfirst * 256)
  {
    oscillator_ptr->prepared = false;
  }
    
  if (!oscillator_ptr->prepared)
  {
    zyn_oscillator_prepare(oscillator_ptr);
  }

  outpos = (int)((RND * 2.0 - 1.0) * (float)OSCIL_SIZE * (oscillator_ptr->Prand - 64.0) / 64.0);
  outpos = (outpos + 2 * OSCIL_SIZE) % OSCIL_SIZE;


  for (i = 0 ; i < OSCIL_SIZE / 2 ; i++)
  {
    oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] = 0.0;
    oscillator_ptr->oscillator_fft_frequencies_ptr->s[i] = 0.0;
  }

  nyquist = (int)(0.5 * oscillator_ptr->sample_rate / fabs(freqHz)) + 2;

  if (oscillator_ptr->ADvsPAD)
  {
    nyquist = (int)(OSCIL_SIZE / 2);
  }

  if (nyquist > OSCIL_SIZE / 2)
  {
    nyquist = OSCIL_SIZE / 2;
  }

  realnyquist = nyquist;
    
  if (oscillator_ptr->Padaptiveharmonics != 0)
  {
    nyquist = OSCIL_SIZE / 2;
  }

  for (i=1;i<nyquist-1;i++)
  {
    oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] = oscillator_ptr->oscilFFTfreqs.c[i];
    oscillator_ptr->oscillator_fft_frequencies_ptr->s[i] = oscillator_ptr->oscilFFTfreqs.s[i];
  }

  zyn_oscillator_adaptive_harmonic(oscillator_ptr, oscillator_ptr->oscillator_fft_frequencies_ptr, freqHz);
  zyn_oscillator_adaptive_harmonic_post_process(oscillator_ptr, &oscillator_ptr->oscillator_fft_frequencies_ptr->c[1], OSCIL_SIZE / 2 - 1);
  zyn_oscillator_adaptive_harmonic_post_process(oscillator_ptr, &oscillator_ptr->oscillator_fft_frequencies_ptr->s[1], OSCIL_SIZE / 2 - 1);

  nyquist = realnyquist;

  // do the antialiasing in the case of adaptive harmonics
  if (oscillator_ptr->Padaptiveharmonics)
  {
    for (i = nyquist ; i < OSCIL_SIZE / 2 ; i++)
    {
      oscillator_ptr->oscillator_fft_frequencies_ptr->s[i]=0;
      oscillator_ptr->oscillator_fft_frequencies_ptr->c[i]=0;
    }
  }

  // Randomness (each harmonic), the block type is computed 
  // in ADnote by setting start position according to this setting
  if (oscillator_ptr->Prand > 64 &&
      freqHz >= 0.0 &&
      !oscillator_ptr->ADvsPAD)
  {
    rnd = PI * pow((oscillator_ptr->Prand - 64.0) / 64.0, 2.0);

    // to Nyquist only for AntiAliasing
    for (i = 1 ; i < nyquist - 1 ; i++)
    {
      angle = rnd * i * RND;
      a = oscillator_ptr->oscillator_fft_frequencies_ptr->c[i];
      b = oscillator_ptr->oscillator_fft_frequencies_ptr->s[i];
      c = cos(angle);
      d = sin(angle);
      oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] = a * c - b * d;
      oscillator_ptr->oscillator_fft_frequencies_ptr->s[i] = a * d + b * c;
    }
  }

  // Harmonic Amplitude Randomness
  if (freqHz > 0.1 && !oscillator_ptr->ADvsPAD)
  {
    realrnd = rand();
    srand(oscillator_ptr->randseed);
    power = oscillator_ptr->Pamprandpower / 127.0;
    normalize = 1.0 / (1.2 - power);

    switch (oscillator_ptr->Pamprandtype)
    {
    case 1:
      power = power * 2.0 - 0.5;
      power = pow(15.0, power);
      for (i=1;i<nyquist-1;i++)
      {
        amp = pow(RND, power) * normalize;
        oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] *= amp;
        oscillator_ptr->oscillator_fft_frequencies_ptr->s[i] *= amp;
      }
      break;
    case 2:
      power = power * 2.0 - 0.5;
      power = pow(15.0, power) * 2.0;
      rndfreq = 2 * PI * RND;

      for (i = 1 ; i < nyquist - 1 ; i++)
      {
        amp = pow(fabs(sin(i * rndfreq)), power) * normalize;
        oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] *= amp;
        oscillator_ptr->oscillator_fft_frequencies_ptr->s[i] *= amp;
      }
      break;
    }

    srand(realrnd + 1);
  }

  if (freqHz > 0.1 && resonance)
  {
    zyn_resonance_apply(oscillator_ptr->resonance_ptr, nyquist - 1, oscillator_ptr->oscillator_fft_frequencies_ptr, freqHz);
  }

  // Full RMS normalize
  sum = 0;

  for (j = 1 ; j < OSCIL_SIZE / 2 ; j++)
  {
    sum += oscillator_ptr->oscillator_fft_frequencies_ptr->c[j] * oscillator_ptr->oscillator_fft_frequencies_ptr->c[j];
    sum += oscillator_ptr->oscillator_fft_frequencies_ptr->s[j] * oscillator_ptr->oscillator_fft_frequencies_ptr->s[j];
  }

  if (sum < 0.000001)
  {
    sum=1.0;
  }

  sum = 1.0 / sqrt(sum);

  for (j = 1 ; j < OSCIL_SIZE / 2 ; j++)
  {
    oscillator_ptr->oscillator_fft_frequencies_ptr->c[j]*=sum; 
    oscillator_ptr->oscillator_fft_frequencies_ptr->s[j]*=sum; 
  }   

  if (oscillator_ptr->ADvsPAD && freqHz > 0.1)
  {
    // in this case the smps will contain the freqs
    for (i=1;i<OSCIL_SIZE/2;i++)
    {
      smps[i - 1] = sqrt(oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] * oscillator_ptr->oscillator_fft_frequencies_ptr->c[i] + oscillator_ptr->oscillator_fft_frequencies_ptr->s[i] * oscillator_ptr->oscillator_fft_frequencies_ptr->s[i]);
    }
  }
  else
  {
    zyn_fft_freqs2smps(oscillator_ptr->fft, oscillator_ptr->oscillator_fft_frequencies_ptr, smps);

    // correct the amplitude
    for (i = 0 ; i < OSCIL_SIZE ; i++)
    {
      smps[i] *= 0.25;
    }
  }

  if (oscillator_ptr->Prand < 64)
  {
    return outpos;
  }

  return 0;
}

#if 0
//computes the full spectrum of oscil from harmonics,phases and basefunc
void prepare();

void getbasefunction(REALTYPE *smps);

//called by UI
void getspectrum(int n,REALTYPE *spc,int what);//what=0 pt. oscil,1 pt. basefunc
void getcurrentbasefunction(REALTYPE *smps);
void useasbase();//convert oscil to base function

void defaults();

void convert2sine(int magtype);
  
//computes the basefunction and make the FFT; newbasefunc<0  = same basefunc
void changebasefunction();
//Waveshaping
void waveshape();

//Filter the oscillator accotding to Pfiltertype and Pfilterpar
void oscilfilter();

//Adjust the spectrum
void spectrumadjust();
  
//Shift the harmonics
void shiftharmonics();
    
//Do the oscil modulation stuff
void modulation();

//Do the adaptive harmonic stuff
void adaptiveharmonic(struct zyn_fft_freqs f, REALTYPE freq);
  
//Do the adaptive harmonic postprocessing (2n+1,2xS,2xA,etc..)
//this function is called even for the user interface
//this can be called for the sine and components, and for the spectrum 
//(that's why the sine and cosine components should be processed with a separate call)
void adaptiveharmonicpostprocess(REALTYPE *f, int size);
    
void
OscilGen::convert2sine(int magtype)
{
  REALTYPE mag[MAX_AD_HARMONICS],phase[MAX_AD_HARMONICS];
  REALTYPE oscil[OSCIL_SIZE];
  struct zyn_fft_freqs freqs;
  zyn_fft_handle fft;

  zyn_fft_freqs_init(&freqs, OSCIL_SIZE / 2);

  get(oscil,-1.0);
  fft = zyn_fft_create(OSCIL_SIZE);
  zyn_fft_smps2freqs(fft, oscil,freqs);
  zyn_fft_destroy(fft);

  REALTYPE max=0.0;
        
  mag[0]=0;
  phase[0]=0;
  for (int i=0;i<MAX_AD_HARMONICS;i++){
    mag[i]=sqrt(pow(freqs.s[i+1],2)+pow(freqs.c[i+1],2.0));
    phase[i]=atan2(freqs.c[i+1],freqs.s[i+1]);
    if (max<mag[i]) max=mag[i];
  };
  if (max<0.00001) max=1.0;
    
  defaults();
        
  for (int i=0;i<MAX_AD_HARMONICS-1;i++){
    REALTYPE newmag=mag[i]/max;
    REALTYPE newphase=phase[i];

    Phmag[i]=(int) ((newmag)*64.0)+64;
  
    Phphase[i]=64-(int) (64.0*newphase/PI);
    if (Phphase[i]>127) Phphase[i]=127;
  
    if (Phmag[i]==64) Phphase[i]=64;
  };

  zyn_fft_freqs_uninit(&freqs);

  prepare();
}

/* 
 * Get the spectrum of the oscillator for the UI
 */
void OscilGen::getspectrum(int n, REALTYPE *spc,int what){
  if (n>OSCIL_SIZE/2) n=OSCIL_SIZE/2;

  for (int i=1;i<n;i++){
    if (what==0){
      spc[i-1]=sqrt(oscilFFTfreqs.c[i]*oscilFFTfreqs.c[i]
                    +oscilFFTfreqs.s[i]*oscilFFTfreqs.s[i]);
    } else {
      if (Pcurrentbasefunc==0) spc[i-1]=((i==1)?(1.0):(0.0));
      else spc[i-1]=sqrt(basefuncFFTfreqs.c[i]*basefuncFFTfreqs.c[i]+
                         basefuncFFTfreqs.s[i]*basefuncFFTfreqs.s[i]);
    };
  };
    
  if (what==0) {
    for (int i=0;i<n;i++) outoscilFFTfreqs.s[i]=outoscilFFTfreqs.c[i]=spc[i+1];
    for (int i=n;i<OSCIL_SIZE/2;i++) outoscilFFTfreqs.s[i]=outoscilFFTfreqs.c[i]=0.0;
    adaptiveharmonic(outoscilFFTfreqs,0.0);
    for (int i=1;i<n;i++) spc[i-1]=outoscilFFTfreqs.s[i];
    adaptiveharmonicpostprocess(spc,n-1);
  };
};


/* 
 * Convert the oscillator as base function
 */
void OscilGen::useasbase(){
  int i;

  for (i=0;i<OSCIL_SIZE/2;i++) {
    basefuncFFTfreqs.c[i]=oscilFFTfreqs.c[i];
    basefuncFFTfreqs.s[i]=oscilFFTfreqs.s[i];
  };

  oldbasefunc=Pcurrentbasefunc=127;

  prepare();
};


/* 
 * Get the base function for UI
 */
void
OscilGen::getcurrentbasefunction(REALTYPE *smps)
{
  if (Pcurrentbasefunc!=0)
  {
    zyn_fft_freqs2smps(m_fft, basefuncFFTfreqs, smps);
  }
  else
  {
    // the sine case
    getbasefunction(smps);
  }
}
#endif
