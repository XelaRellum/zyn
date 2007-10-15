/*
  ZynAddSubFX - a software synthesizer
 
  Resonance.h - Resonance 
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
#ifndef RESONANCE_H
#define RESONANCE_H

#define N_RES_POINTS 256

struct zyn_resonance
{
  bool enabled;                 // whether resonance is enabled  
  unsigned char points[N_RES_POINTS]; // how many points define the resonance function
  unsigned char maxdB;          // how many dB the signal may be amplified
  unsigned char centerfreq;     // the center frequency of the res. func.
  unsigned char octavesfreq;    // and the number of octaves
  unsigned char protectthefundamental; // the fundamental (1-st harmonic) is not damped, even it resonance function is low

  float center;                 // center frequency(relative)
  float bw;                     // bandwidth(relative)
};

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

void
zyn_resonance_init(
  struct zyn_resonance * resonance_ptr);

void
zyn_resonance_apply(
  struct zyn_resonance * resonance_ptr,
  int n,
  struct zyn_fft_freqs * fftdata_ptr,
  float freq);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#if 0
  void setpoint(int n,unsigned char p);
  void smooth();
  void interpolatepeaks(int type);
  void randomize(int type);


  REALTYPE getfreqpos(REALTYPE freq);
  REALTYPE getfreqx(REALTYPE x);
  REALTYPE getfreqresponse(REALTYPE freq);
  REALTYPE getcenterfreq();
  REALTYPE getoctavesfreq();

  void set_center(float center);
  void set_badnwidth(float bandwidth);
#endif

#endif
