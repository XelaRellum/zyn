/*
  ZynAddSubFX - a software synthesizer
 
  Util.h - Miscellaneous functions
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

#ifndef UTIL_H
#define UTIL_H

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

// Velocity Sensing function
extern REALTYPE VelF(REALTYPE velocity,unsigned char scaling);

float
zyn_velocity_scale(float velocity, float scaling);

#define N_DETUNE_TYPES 4 //the number of detune types
extern REALTYPE getdetune(unsigned char type,unsigned short int coarsedetune,unsigned short int finedetune);

void
silence_buffer(
  zyn_sample_type * buffer,
  size_t size);

void
silence_two_buffers(
  zyn_sample_type * buffer1,
  zyn_sample_type * buffer2,
  size_t size);

void
mix_add_two_buffers(
  zyn_sample_type * buffer_mix_1,
  zyn_sample_type * buffer_mix_2,
  zyn_sample_type * buffer1,
  zyn_sample_type * buffer2,
  size_t size);

void
fadeout_two_buffers(
  zyn_sample_type * buffer1,
  zyn_sample_type * buffer2,
  size_t size);

void
copy_buffer(
  zyn_sample_type * buffer_dest,
  zyn_sample_type * buffer_src,
  size_t size);

void
multiply_buffer(
  zyn_sample_type * buffer,
  float multiplyer,
  size_t size);

/*
 * Random generator (0.0..1.0)
 */
float
zyn_random();

#define ZYN_BOOL_XOR(p, q) (((p) && !(q)) || (!(p) && (q)))

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif

