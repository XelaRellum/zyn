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

#define zyn_addsynth_ptr ((struct zyn_addsynth *)context)

float
zyn_component_filter_analog_get_float(
  void * context,
  unsigned int parameter)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_FREQUNECY:
      return percent_from_0_127(zyn_addsynth_ptr->m_filter_params.Pfreq) / 100;
    case ZYNADD_PARAMETER_FLOAT_Q_FACTOR:
      return percent_from_0_127(zyn_addsynth_ptr->m_filter_params.Pq) / 100;
    case ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING:
      return zyn_addsynth_ptr->m_filter_params.m_frequency_tracking;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      return zyn_addsynth_ptr->m_filter_params.m_gain;
    }

    LOG_ERROR("Unknown analog filter float parameter %u", parameter);
    assert(0);

    return 0.0;
}

void
zyn_component_filter_analog_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_FREQUNECY:
      zyn_addsynth_ptr->m_filter_params.Pfreq = percent_to_0_127(value * 100);
      return;
    case ZYNADD_PARAMETER_FLOAT_Q_FACTOR:
      zyn_addsynth_ptr->m_filter_params.Pq = percent_to_0_127(value * 100);
      return;
    case ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING:
      zyn_addsynth_ptr->m_filter_params.m_frequency_tracking = value;
      return;
    case ZYNADD_PARAMETER_FLOAT_VOLUME:
      zyn_addsynth_ptr->m_filter_params.m_gain = value;
      return;
    }

    LOG_ERROR("Unknown analog filter float parameter %u", parameter);
    assert(0);
}

signed int
zyn_component_filter_analog_get_int(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_INT_STAGES:
    return zyn_addsynth_ptr->m_filter_params.m_additional_stages + 1;
  case ZYNADD_PARAMETER_ENUM_FILTER_TYPE:
    return ZYN_FILTER_ANALOG_TYPE_LPF1;
  }

  LOG_ERROR("Unknown analog filter int/enum parameter %u", parameter);
  assert(0);

  return -1;
}

void
zyn_component_filter_analog_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_INT_STAGES:
    assert(value > 0);
    assert(value <= MAX_FILTER_STAGES);
    zyn_addsynth_ptr->m_filter_params.m_additional_stages = value - 1;
    return;
  case ZYNADD_PARAMETER_ENUM_FILTER_TYPE:
    return;
  }

  LOG_ERROR("Unknown analog filter int/enum parameter %u", parameter);
  assert(0);
}

bool
zyn_component_filter_analog_get_bool(
  void * context,
  unsigned int parameter)
{
  LOG_ERROR("Unknown analog filter bool parameter %u", parameter);
  assert(0);

  return false;
}

void
zyn_component_filter_analog_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  LOG_ERROR("Unknown analog filter bool parameter %u", parameter);
  assert(0);
}

#undef zyn_addsynth_ptr

void
zyn_addsynth_component_init_filter_analog(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_addsynth * zyn_addsynth_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, zyn_addsynth_ptr, zyn_component_filter_analog_);
}
