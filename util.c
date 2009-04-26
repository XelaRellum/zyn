/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007,2008,2009 Nedko Arnaudov <nedko@arnaudov.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *****************************************************************************/

#include <math.h>
#include <stdlib.h>
#include <assert.h>

#include "common.h"
#include "util.h"

/*
 * Transform the velocity according the scaling parameter (velocity sensing)
 */
float
VelF(REALTYPE velocity,unsigned char scaling)
{
  float x;

  x = (64.0 - scaling) / 64.0;  /* 0 .. 127 -> 1 .. -1 */

  x = pow(VELOCITY_MAX_SCALE, x);

  if (scaling == 127 || velocity > 0.99)
  {
    return 1.0;
  }
  else
  {
    return pow(velocity, x);
  }
}

float
zyn_velocity_scale(float velocity, float scaling)
{
  float x;

  x = pow(VELOCITY_MAX_SCALE, scaling);

  if (scaling < -0.99 || velocity > 0.99)
  {
    return 1.0;
  }
  else
  {
    return pow(velocity, x);
  }
}

/*
 * Get the detune in cents 
 */
REALTYPE
zyn_get_detune(
  signed int type,
  signed int octave,
  signed int coarse,
  float fine)
{
  REALTYPE cdet;
  REALTYPE findet;

  switch (type)
  {
  case ZYN_DETUNE_TYPE_L35CENTS:
    cdet = coarse * 50.0;
    findet = fabs(fine) * 35.0; // almost like "Paul's Sound Designer 2"
  case ZYN_DETUNE_TYPE_L10CENTS:
    cdet = fabs(coarse * 10.0);
    findet = fabs(fine) * 10.0;
    break;
  case ZYN_DETUNE_TYPE_E100CENTS:
    cdet = coarse * 100;
    findet = pow(10, fabs(fine) * 3.0) / 10.0 - 0.1;
    break;
  case ZYN_DETUNE_TYPE_E1200CENTS:
    cdet = coarse * 701.95500087; // perfect fifth
    findet = (pow(2, fabs(fine) * 12.0) - 1.0) / 4095 * 1200;
    break;
  default:
    assert(0);
    return 0;
  }

  /* we compute detune using fabs(fine) so we need to adjust detune sign */
  if (fine < 0.0)
  {
    findet = -findet;
  }

  return octave * 1200.0 + cdet + findet;
}

void
silence_buffer(
  zyn_sample_type * buffer,
  size_t size)
{
  while (size)
  {
    size--;
    buffer[size] = 0.0;
  }
}

void
silence_two_buffers(
  zyn_sample_type * buffer1,
  zyn_sample_type * buffer2,
  size_t size)
{
  while (size)
  {
    size--;
    buffer1[size] = 0.0;
    buffer2[size] = 0.0;
  }
}

void
mix_add_two_buffers(
  zyn_sample_type * buffer_mix_1,
  zyn_sample_type * buffer_mix_2,
  zyn_sample_type * buffer1,
  zyn_sample_type * buffer2,
  size_t size)
{
  while (size)
  {
    size--;
    buffer_mix_1[size] += buffer1[size];
    buffer_mix_2[size] += buffer2[size];
  }
}

void
fadeout_two_buffers(
  zyn_sample_type * buffer1,
  zyn_sample_type * buffer2,
  size_t size)
{
  zyn_sample_type fade;

  while (size)
  {
    fade = 1.0 - (zyn_sample_type)size / (zyn_sample_type)SOUND_BUFFER_SIZE;
    size--;
    buffer1[size] *= fade;
    buffer2[size] *= fade;
  }
}

void
copy_buffer(
  zyn_sample_type * buffer_dest,
  zyn_sample_type * buffer_src,
  size_t size)
{
  while (size)
  {
    size--;
    buffer_dest[size] = buffer_src[size];
  }
}

void
multiply_buffer(
  zyn_sample_type * buffer,
  float multiplyer,
  size_t size)
{
  while (size)
  {
    size--;
    buffer[size] *= multiplyer;
  }
}

float
zyn_random()
{
  return rand() / (RAND_MAX + 1.0);
}
