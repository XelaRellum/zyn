/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
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
getdetune(
  unsigned char type,
  unsigned short int coarsedetune,
  unsigned short int finedetune)
{
  REALTYPE det=0.0,octdet=0.0,cdet=0.0,findet=0.0;

  //Get Octave
  int octave=coarsedetune/1024;
  if (octave>=8) octave-=16;
  octdet=octave*1200.0;

  //Coarse and fine detune
  int cdetune=coarsedetune%1024;
  if (cdetune>512) cdetune-=1024;
    
  int fdetune=finedetune-8192;

  switch (type){
//  case 1: is used for the default (see below)
  case 2: cdet=fabs(cdetune*10.0);
    findet=fabs(fdetune/8192.0)*10.0;
    break;
  case 3: cdet=fabs(cdetune*100);
    findet=pow(10,fabs(fdetune/8192.0)*3.0)/10.0-0.1;
    break;
  case 4: cdet=fabs(cdetune*701.95500087); //perfect fifth
    findet=(pow(2,fabs(fdetune/8192.0)*12.0)-1.0)/4095*1200;
    break;
    //case ...: need to update N_DETUNE_TYPES, if you'll add more
  default:cdet=fabs(cdetune*50.0);
    findet=fabs(fdetune/8192.0)*35.0;//almost like "Paul's Sound Designer 2"
    break;
  };
  if (finedetune<8192) findet=-findet;
  if (cdetune<0) cdet=-cdet;
    
  det=octdet+cdet+findet;
  return(det);
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

float
zyn_random()
{
  return rand() / (RAND_MAX + 1.0);
}
