/*
  ZynAddSubFX - a software synthesizer
 
  FFTwrapper.c  -  A wrapper for Fast Fourier Transforms
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
#include <stdlib.h>

#ifdef FFTW_VERSION_2
# include <fftw.h>
/* If you got error messages about rfftw.h, replace the next include  line with "#include <srfftw.h>"
   or with "#include <drfftw.h> (if one doesn't work try the other). It may be necessary to replace
   the <fftw.h> with <dfftw.h> or <sfftw.h>. If the neither one doesn't work, 
   please install latest version of fftw(recomanded from the sources) from www.fftw.org.
   If you'll install fftw3 you need to change the Makefile.inc
   Hope all goes right." */
# include <rfftw.h>
#else
# include <fftw3.h>
# define fftw_real double
# define rfftw_plan fftw_plan
#endif

#include "fft_wrapper.h"

struct zyn_fft
{
  int size;
  fftw_real * tmp_data1;
  fftw_real * tmp_data2;
  rfftw_plan plan;
  rfftw_plan plan_inv;
};

zyn_fft_handle
zyn_fft_create(
  int fftsize)
{
  struct zyn_fft * fft_ptr;

  fft_ptr = malloc(sizeof(struct zyn_fft));

  fft_ptr->size = fftsize;

  fft_ptr->tmp_data1 = malloc(sizeof(fftw_real) * fftsize);
  fft_ptr->tmp_data2 = malloc(sizeof(fftw_real) * fftsize);

#ifdef FFTW_VERSION_2
  fft_ptr->plan = rfftw_create_plan(fftsize, FFTW_REAL_TO_COMPLEX, FFTW_ESTIMATE | FFTW_IN_PLACE);
  fft_ptr->plan_inv = rfftw_create_plan(fftsize, FFTW_COMPLEX_TO_REAL, FFTW_ESTIMATE | FFTW_IN_PLACE);
#else
  fft_ptr->plan = fftw_plan_r2r_1d(fftsize, fft_ptr->tmp_data1, fft_ptr->tmp_data1, FFTW_R2HC, FFTW_ESTIMATE);
  fft_ptr->plan_inv = fftw_plan_r2r_1d(fftsize, fft_ptr->tmp_data2, fft_ptr->tmp_data2, FFTW_HC2R, FFTW_ESTIMATE);
#endif

  return (zyn_fft_handle)fft_ptr;
}

#define fft_ptr ((struct zyn_fft *)handle)

void
zyn_fft_destroy(
  zyn_fft_handle handle)
{
#ifdef FFTW_VERSION_2
  rfftw_destroy_plan(fft_ptr->plan);
  rfftw_destroy_plan(fft_ptr->plan_inv);
#else 
  fftw_destroy_plan(fft_ptr->plan);
  fftw_destroy_plan(fft_ptr->plan_inv);
#endif

  free(fft_ptr->tmp_data1);
  free(fft_ptr->tmp_data2);
}

/*
 * do the Fast Fourier Transform
 */
void
zyn_fft_smps2freqs(
  zyn_fft_handle handle,
  REALTYPE * smps,
  struct zyn_fft_freqs freqs)
{
  int i;
  fftw_real * tmp_data_ptr;

  for (i = 0 ; i < fft_ptr->size ; i++)
  {
    fft_ptr->tmp_data1[i] = smps[i];
  }

#ifdef FFTW_VERSION_2
  rfftw_one(fft_ptr->plan, fft_ptr->tmp_data1, fft_ptr->tmp_data2);
  tmp_data_ptr = fft_ptr->tmp_data2;
#else
  fftw_execute(fft_ptr->plan);
  tmp_data_ptr = fft_ptr->tmp_data1;
#endif

  for (i = 0 ; i < fft_ptr->size / 2 ; i++)
  {
    freqs.c[i] = tmp_data_ptr[i];
    if (i != 0)
    {
      freqs.s[i] = tmp_data_ptr[fft_ptr->size - i];
    }
  }

  fft_ptr->tmp_data2[fft_ptr->size / 2] = 0.0;
}

/*
 * do the Inverse Fast Fourier Transform
 */
void
zyn_fft_freqs2smps(
  zyn_fft_handle handle,
  struct zyn_fft_freqs freqs,
  REALTYPE * smps)
{
  int i;
  fftw_real * tmp_data_ptr;

  fft_ptr->tmp_data2[fft_ptr->size / 2] = 0.0;

#ifdef FFTW_VERSION_2
  tmp_data_ptr = fft_ptr->tmp_data1;
#else
  tmp_data_ptr = fft_ptr->tmp_data2;
#endif

  for (i = 0 ; i < fft_ptr->size / 2 ; i++)
  {
    tmp_data_ptr[i] = freqs.c[i];
    if (i != 0)
    {
      tmp_data_ptr[fft_ptr->size - i] = freqs.s[i];
    }
  }

#ifdef FFTW_VERSION_2
  rfftw_one(fft_ptr->plan_inv, fft_ptr->tmp_data1, fft_ptr->tmp_data2);
#else
  fftw_execute(fft_ptr->plan_inv);
#endif

  for (i = 0 ; i < fft_ptr->size ; i++)
  {
    smps[i] = fft_ptr->tmp_data2[i];
  }
}

void
zyn_fft_freqs_init(
  struct zyn_fft_freqs * f,
  int size)
{
  f->c = malloc(size * sizeof(zyn_sample_type));
  f->s = malloc(size * sizeof(zyn_sample_type));

  silence_two_buffers(f->c, f->s, size);
}

void
zyn_fft_freqs_uninit(
  struct zyn_fft_freqs * f)
{
  free(f->c);
  free(f->s);

  f->c = NULL;
  f->s = NULL;
}
