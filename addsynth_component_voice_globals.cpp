/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2007 Nedko Arnaudov <nedko@arnaudov.name>
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

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

#define voice_params_ptr ((struct ADnoteVoiceParam * )context)

float
zyn_component_voice_globals_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown frequency envelope parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_voice_globals_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  default:
    LOG_ERROR("Unknown frequency envelope parameter %u", parameter);
    assert(0);
  }
}

signed int
zyn_component_voice_globals_get_int(
  void * context,
  unsigned int parameter)
{
  assert(0);
  return 0;
}

void
zyn_component_voice_globals_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  assert(0);
}

bool
zyn_component_voice_globals_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RESONANCE:
    return voice_params_ptr->resonance;
  case ZYNADD_PARAMETER_BOOL_WHITE_NOISE:
    return voice_params_ptr->white_noise;
  default:
    LOG_ERROR("Unknown bool voice global parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_voice_globals_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RESONANCE:
    LOG_DEBUG("voice resonance -> %s (%p)", value ? "on" : "off", voice_params_ptr);
    voice_params_ptr->resonance = value;
    return;
  case ZYNADD_PARAMETER_BOOL_WHITE_NOISE:
    LOG_DEBUG("voice white noise -> %s (%p)", value ? "on" : "off", voice_params_ptr);
    voice_params_ptr->white_noise = value;
    return;
  default:
    LOG_ERROR("Unknown bool voice global parameter %u", parameter);
    assert(0);
  }
}

unsigned int
zyn_component_voice_globals_get_shape(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_voice_globals_set_shape(
  void * context,
  unsigned int value)
{
  assert(0);
}

unsigned int
zyn_component_voice_globals_get_filter_type(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_voice_globals_set_filter_type(
  void * context,
  unsigned int value)
{
  assert(0);
}

unsigned int
zyn_component_voice_globals_get_analog_filter_type(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_voice_globals_set_analog_filter_type(
  void * context,
  unsigned int value)
{
  assert(0);
}

#undef voice_params_ptr

void
zyn_addsynth_component_init_voice_globals(
  struct zyn_component_descriptor * component_ptr,
  struct ADnoteVoiceParam * voice_params_ptr)
{
  //LOG_DEBUG("voice globals init (%p, %p)", component_ptr, voice_params_ptr);
  ZYN_INIT_COMPONENT(component_ptr, voice_params_ptr, zyn_component_voice_globals_);
}
