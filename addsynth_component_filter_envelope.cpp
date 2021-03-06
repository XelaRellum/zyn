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

#define envelope_params_ptr ((EnvelopeParams * )context)

float
zyn_component_filter_envelope_get_float(
  void * context,
  unsigned int parameter)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE:
      return percent_from_0_127(
        envelope_params_ptr->get_value(
          envelope_params_ptr->m_attack_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      return percent_from_0_127(
        envelope_params_ptr->get_duration(
          envelope_params_ptr->m_attack_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_VALUE:
      return percent_from_0_127(
        envelope_params_ptr->get_value(
          envelope_params_ptr->m_decay_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION:
      return percent_from_0_127(
        envelope_params_ptr->get_duration(
          envelope_params_ptr->m_decay_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE:
      return percent_from_0_127(
        envelope_params_ptr->get_value(
          envelope_params_ptr->m_release_value_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      return percent_from_0_127(
        envelope_params_ptr->get_duration(
          envelope_params_ptr->m_release_duration_index));
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      return percent_from_0_127(envelope_params_ptr->m_stretch) * 2;
    default:
      LOG_ERROR("Unknown filter envelope parameter %u", parameter);
      assert(0);
    }
}

void
zyn_component_filter_envelope_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE:
      envelope_params_ptr->set_value(
        envelope_params_ptr->m_attack_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION:
      envelope_params_ptr->set_duration(
        envelope_params_ptr->m_attack_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_VALUE:
      envelope_params_ptr->set_value(
        envelope_params_ptr->m_decay_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION:
      envelope_params_ptr->set_duration(
        envelope_params_ptr->m_decay_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE:
      envelope_params_ptr->set_value(
        envelope_params_ptr->m_release_value_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION:
      envelope_params_ptr->set_duration(
        envelope_params_ptr->m_release_duration_index,
        percent_to_0_127(value));
      return;
    case ZYNADD_PARAMETER_FLOAT_ENV_STRETCH:
      envelope_params_ptr->m_stretch = percent_to_0_127(value/2);
      return;
    default:
      LOG_ERROR("Unknown filter envelope parameter %u", parameter);
      assert(0);
    }
}

signed int
zyn_component_filter_envelope_get_int(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return 0;
}

void
zyn_component_filter_envelope_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  assert(0);
}

bool
zyn_component_filter_envelope_get_bool(
  void * context,
  unsigned int parameter)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      return envelope_params_ptr->m_forced_release;
    default:
      LOG_ERROR("Unknown bool filter envelope parameter %u", parameter);
      assert(0);
    }
}

void
zyn_component_filter_envelope_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
    switch (parameter)
    {
    case ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE:
      envelope_params_ptr->m_forced_release = value;
      return;
    default:
      LOG_ERROR("Unknown bool filter envelope parameter %u", parameter);
      assert(0);
    }
}

#undef envelope_params_ptr

void
zyn_addsynth_component_init_filter_envelope(
  struct zyn_component_descriptor * component_ptr,
  EnvelopeParams * envelope_params_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, envelope_params_ptr, zyn_component_filter_envelope_);
}
