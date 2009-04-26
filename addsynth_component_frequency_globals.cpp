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

float
zyn_component_frequency_globals_get_float(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return 0;
}

void
zyn_component_frequency_globals_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  assert(0);
}

signed int
zyn_component_frequency_globals_get_int(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return 0;
}

void
zyn_component_frequency_globals_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  assert(0);
}

bool
zyn_component_frequency_globals_get_bool(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return false;
}

void
zyn_component_frequency_globals_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  assert(0);
}

void
zyn_addsynth_component_init_frequency_globals(
  struct zyn_component_descriptor * component_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, NULL, zyn_component_frequency_globals_);
}

#define detune_ptr ((struct zyn_detune * )context)

float
zyn_component_detune_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_DETUNE_FINE:
    return detune_ptr->fine;
  default:
    LOG_ERROR("Unknown detune parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_detune_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_DETUNE_FINE:
    detune_ptr->fine = value;
    return;
  default:
    LOG_ERROR("Unknown detune parameter %u", parameter);
    assert(0);
  }
}

signed int
zyn_component_detune_get_int(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_ENUM_DETUNE_TYPE:
    return detune_ptr->type;
  case ZYNADD_PARAMETER_INT_DETUNE_OCTAVE:
    return detune_ptr->octave;
  case ZYNADD_PARAMETER_INT_DETUNE_COARSE:
    return detune_ptr->coarse;
  }

  LOG_ERROR("Unknown int detune parameter %u", parameter);
  assert(0);
  return -1;
}

void
zyn_component_detune_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_ENUM_DETUNE_TYPE:
    detune_ptr->type = value;
    return;
  case ZYNADD_PARAMETER_INT_DETUNE_OCTAVE:
    detune_ptr->octave = value;
    return;
  case ZYNADD_PARAMETER_INT_DETUNE_COARSE:
    detune_ptr->coarse = value;
    return;
  }

  LOG_ERROR("Unknown int detune parameter %u", parameter);
  assert(0);
}

bool
zyn_component_detune_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown bool detune parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_detune_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown bool detune parameter %u", parameter);
    assert(0);
  }
}

#undef detune_ptr

void
zyn_addsynth_component_init_detune(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_detune * detune_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, detune_ptr, zyn_component_detune_);
}

#define fixed_detune_ptr ((struct zyn_fixed_detune * )context)

float
zyn_component_fixed_detune_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown fixed detune parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_fixed_detune_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown fixed detune parameter %u", parameter);
    assert(0);
  }
}

signed int
zyn_component_fixed_detune_get_int(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_ENUM_FIXED_DETUNE_MODE:
    return fixed_detune_ptr->mode;
  case ZYNADD_PARAMETER_INT_FIXED_DETUNE_EQUAL_TEMPERATE:
    return fixed_detune_ptr->equal_temperate;
  }

  LOG_ERROR("Unknown int fixed detune parameter %u", parameter);
  assert(0);
  return -1;
}

void
zyn_component_fixed_detune_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_ENUM_FIXED_DETUNE_MODE:
    fixed_detune_ptr->mode = value;
    return;
  case ZYNADD_PARAMETER_INT_FIXED_DETUNE_EQUAL_TEMPERATE:
    fixed_detune_ptr->equal_temperate = value;
    return;
  }

  LOG_ERROR("Unknown int fixed detune parameter %u", parameter);
  assert(0);
}

bool
zyn_component_fixed_detune_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown bool fixed detune parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_fixed_detune_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown bool fixed detune parameter %u", parameter);
    assert(0);
  }
}

#undef fixed_detune_ptr

void
zyn_addsynth_component_init_fixed_detune(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_fixed_detune * fixed_detune_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, fixed_detune_ptr, zyn_component_fixed_detune_);
}
