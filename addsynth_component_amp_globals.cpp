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

#include <stdbool.h>
#include <assert.h>
#include <math.h>

#include "common.h"
#include "globals.h"
#include "addsynth.h"
#include "lfo_parameters.h"
#include "filter_parameters.h"
#include "envelope_parameters.h"
#include "resonance.h"
#include "fft.h"
#include "oscillator.h"
#include "portamento.h"
#include "addsynth_internal.h"
#include "log.h"

#define zyn_addsynth_ptr ((struct zyn_addsynth *)context)

float
zyn_component_amp_globals_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_PANORAMA:
    return zyn_addsynth_ptr->panorama;
  case ZYNADD_PARAMETER_FLOAT_VOLUME:
    return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PVolume);
  case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING:
    return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PAmpVelocityScaleFunction);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH:
    return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchStrength);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_TIME:
    return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchTime);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH:
    return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchStretch);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING:
    return percent_from_0_127(zyn_addsynth_ptr->GlobalPar.PPunchVelocitySensing);
  case ZYNADD_PARAMETER_FLOAT_PITCH_BEND_RANGE:
    return zyn_addsynth_ptr->pitch_bend_range;
  case ZYNADD_PARAMETER_FLOAT_PITCH_BEND:
    return zyn_addsynth_ptr->pitch_bend;
  default:
    LOG_ERROR("Unknown float amplitude global parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_amp_globals_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_PANORAMA:
    zyn_addsynth_ptr->panorama = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_VOLUME:
    zyn_addsynth_ptr->GlobalPar.PVolume = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING:
    zyn_addsynth_ptr->GlobalPar.PAmpVelocityScaleFunction = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH:
    zyn_addsynth_ptr->GlobalPar.PPunchStrength = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_TIME:
    zyn_addsynth_ptr->GlobalPar.PPunchTime = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH:
    zyn_addsynth_ptr->GlobalPar.PPunchStretch = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING:
    zyn_addsynth_ptr->GlobalPar.PPunchVelocitySensing = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PITCH_BEND_RANGE:
    zyn_addsynth_ptr->pitch_bend_range = value;
    ZYN_UPDATE_PITCH_BEND(zyn_addsynth_ptr);
    return;
  case ZYNADD_PARAMETER_FLOAT_PITCH_BEND:
    zyn_addsynth_ptr->pitch_bend = value;
    ZYN_UPDATE_PITCH_BEND(zyn_addsynth_ptr);
    return;
  default:
    LOG_ERROR("Unknown float amplitude global parameter %u", parameter);
    assert(0);
  }
}

signed int
zyn_component_amp_globals_get_int(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return 0;
}

void
zyn_component_amp_globals_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  assert(0);
}

bool
zyn_component_amp_globals_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
    return zyn_addsynth_ptr->random_panorama;
  case ZYNADD_PARAMETER_BOOL_STEREO:
    return zyn_addsynth_ptr->stereo;
  case ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING:
    return zyn_addsynth_ptr->random_grouping;
  default:
    LOG_ERROR("Unknown bool amplitude global parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_amp_globals_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
    zyn_addsynth_ptr->random_panorama = value;
    return;
  case ZYNADD_PARAMETER_BOOL_STEREO:
    zyn_addsynth_ptr->stereo = value;
    return;
  case ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING:
    zyn_addsynth_ptr->random_grouping = value;
    return;
  default:
    LOG_ERROR("Unknown bool amplitude global parameter %u", parameter);
    assert(0);
  }
}

#undef zyn_addsynth_ptr

void
zyn_addsynth_component_init_amp_globals(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_addsynth * zyn_addsynth_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, zyn_addsynth_ptr, zyn_component_amp_globals_);
}
