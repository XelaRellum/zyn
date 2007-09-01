/*
  ZynAddSubFX - a software synthesizer
 
  FFTwrapper.h  -  A wrapper for Fast Fourier Transforms
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

#ifndef FFT_WRAPPER_H
#define FFT_WRAPPER_H

#include "globals.h"

struct zyn_fft_freqs
{
  zyn_sample_type *s,*c;               /* sine and cosine components */
};

typedef void * zyn_fft_handle;

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

void
zyn_fft_freqs_init(
  struct zyn_fft_freqs * f,
  int size);

void
zyn_fft_freqs_uninit(
  struct zyn_fft_freqs * f);

zyn_fft_handle
zyn_fft_create(
  int fftsize);

void
zyn_fft_destroy(
  zyn_fft_handle handle);

void
zyn_fft_smps2freqs(
  zyn_fft_handle handle,
  REALTYPE * smps,
  struct zyn_fft_freqs freqs);

void
zyn_fft_freqs2smps(
  zyn_fft_handle handle,
  struct zyn_fft_freqs freqs,
  REALTYPE * smps);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

