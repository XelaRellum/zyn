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
#include "globals.h"
#include "addsynth.h"
#include "lfo_parameters.h"
#include "lfo.h"
#include "filter_parameters.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "sv_filter.h"
#include "formant_filter.h"
#include "filter.h"
#include "envelope_parameters.h"
#include "envelope.h"
#include "addnote.h"
#include "resonance.h"
#include "fft.h"
#include "oscillator.h"
#include "portamento.h"
#include "addsynth_internal.h"
#include "log.h"

#define filter ((zyn_filter_sv_handle)context)

float
zyn_component_filter_sv_get_float(
  void * context,
  unsigned int parameter)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_FREQUNECY:
      return zyn_filter_sv_get_frequency(filter);
    case ZYNADD_PARAMETER_FLOAT_Q_FACTOR:
      return zyn_filter_sv_get_q_factor(filter);
    case ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING:
      return zyn_filter_sv_get_frequency_tracking(filter);
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      return zyn_filter_sv_get_gain(filter);
    }

    LOG_ERROR("Unknown sv filter float parameter %u", parameter);
    assert(0);

    return 0.0;
}

void
zyn_component_filter_sv_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_FREQUNECY:
      zyn_filter_sv_set_frequency(filter, value);
      return;
    case ZYNADD_PARAMETER_FLOAT_Q_FACTOR:
      zyn_filter_sv_set_q_factor(filter, value);
      return;
    case ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING:
      zyn_filter_sv_set_frequency_tracking(filter, value);
      return;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      return zyn_filter_sv_set_gain(filter, value);
      return;
    }

    LOG_ERROR("Unknown sv filter float parameter %u", parameter);
    assert(0);
}

signed int
zyn_component_filter_sv_get_int(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_INT_STAGES:
    return zyn_filter_sv_get_stages(filter);
  case ZYNADD_PARAMETER_ENUM_FILTER_TYPE:
    return zyn_filter_sv_get_type(filter);
  }

  LOG_ERROR("Unknown sv filter int/enum parameter %u", parameter);
  assert(0);

  return -1;
}

void
zyn_component_filter_sv_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_INT_STAGES:
    return zyn_filter_sv_set_stages(filter, value);
  case ZYNADD_PARAMETER_ENUM_FILTER_TYPE:
    zyn_filter_sv_set_type(filter, value);
    return;
  }

  LOG_ERROR("Unknown sv filter int/enum parameter %u", parameter);
  assert(0);
}

bool
zyn_component_filter_sv_get_bool(
  void * context,
  unsigned int parameter)
{
  LOG_ERROR("Unknown sv filter bool parameter %u", parameter);
  assert(0);

  return false;
}

void
zyn_component_filter_sv_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  LOG_ERROR("Unknown sv filter bool parameter %u", parameter);
  assert(0);
}

#undef filter

void
zyn_addsynth_component_init_filter_sv(
  struct zyn_component_descriptor * component_ptr,
  zyn_filter_sv_handle filter)
{
  ZYN_INIT_COMPONENT(component_ptr, filter, zyn_component_filter_sv_);
}
