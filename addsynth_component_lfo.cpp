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

#include <stdbool.h>
#include <assert.h>

#include "common.h"
#include "addsynth.h"
#include "Controller.h"
#include "lfo_parameters.h"
#include "lfo.h"
#include "filter_parameters.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "filter.h"
#include "envelope_parameters.h"
#include "envelope.h"
#include "addnote.h"
#include "resonance.h"
#include "fft.h"
#include "oscillator.h"
#include "addsynth_internal.h"
#include "log.h"

#define lfo_params_ptr ((struct zyn_lfo_parameters * )context)

float
zyn_component_lfo_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY:
    return lfo_params_ptr->frequency;
  case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH:
    return lfo_params_ptr->depth * 100;
  case ZYNADD_PARAMETER_FLOAT_LFO_START_PHASE:
    return lfo_params_ptr->start_phase;
  case ZYNADD_PARAMETER_FLOAT_LFO_DELAY:
    return lfo_params_ptr->delay;
  case ZYNADD_PARAMETER_FLOAT_LFO_STRETCH:
    return lfo_params_ptr->stretch;
  case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH_RANDOMNESS:
    return lfo_params_ptr->depth_randomness * 100;
  case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY_RANDOMNESS:
    return lfo_params_ptr->frequency_randomness * 100;
  default:
    LOG_ERROR("Unknown LFO parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_lfo_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY:
    lfo_params_ptr->frequency = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH:
    lfo_params_ptr->depth = value / 100;
    return;
  case ZYNADD_PARAMETER_FLOAT_LFO_START_PHASE:
    lfo_params_ptr->start_phase = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_LFO_DELAY:
    lfo_params_ptr->delay = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_LFO_STRETCH:
    lfo_params_ptr->stretch = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_LFO_DEPTH_RANDOMNESS:
    lfo_params_ptr->depth_randomness = value / 100;
    return;
  case ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY_RANDOMNESS:
    lfo_params_ptr->frequency_randomness = value / 100;
    return;
  default:
    LOG_ERROR("Unknown LFO parameter %u", parameter);
    assert(0);
  }
}

signed int
zyn_component_lfo_get_int(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return 0;
}

void
zyn_component_lfo_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  assert(0);
}

bool
zyn_component_lfo_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_START_PHASE:
    return lfo_params_ptr->random_start_phase;
  case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_DEPTH:
    return lfo_params_ptr->depth_randomness_enabled;
  case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_FREQUENCY:
    return lfo_params_ptr->frequency_randomness_enabled;
  default:
    LOG_ERROR("Unknown bool LFO parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_lfo_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_START_PHASE:
    lfo_params_ptr->random_start_phase = value;
    return;
  case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_DEPTH:
    lfo_params_ptr->depth_randomness_enabled = value;
    return;
  case ZYNADD_PARAMETER_BOOL_LFO_RANDOM_FREQUENCY:
    lfo_params_ptr->frequency_randomness_enabled = value;
    return;
  default:
    LOG_ERROR("Unknown bool LFO parameter %u", parameter);
    assert(0);
  }
}

unsigned int
zyn_component_lfo_get_shape(
  void * context)
{
  return lfo_params_ptr->shape;
}

void
zyn_component_lfo_set_shape(
  void * context,
  unsigned int value)
{
  lfo_params_ptr->shape = value;
}

unsigned int
zyn_component_lfo_get_filter_type(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_lfo_set_filter_type(
  void * context,
  unsigned int value)
{
  assert(0);
}

unsigned int
zyn_component_lfo_get_analog_filter_type(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_lfo_set_analog_filter_type(
  void * context,
  unsigned int value)
{
  assert(0);
}

#undef lfo_params_ptr

void
zyn_addsynth_component_init_lfo(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_lfo_parameters * lfo_params_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, lfo_params_ptr, zyn_component_lfo_);
}
