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
#include "filter_common.h"
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

#define zyn_addsynth_ptr ((struct zyn_addsynth *)context)

float
zyn_component_filter_formant_get_float(
  void * context,
  unsigned int parameter)
{
  LOG_ERROR("Unknown formant filter float parameter %u", parameter);
  assert(0);

  return 0.0;
}

void
zyn_component_filter_formant_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  LOG_ERROR("Unknown formant filter float parameter %u", parameter);
  assert(0);
}

signed int
zyn_component_filter_formant_get_int(
  void * context,
  unsigned int parameter)
{
  LOG_ERROR("Unknown formant filter int/enum parameter %u", parameter);
  assert(0);

  return -1;
}

void
zyn_component_filter_formant_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  LOG_ERROR("Unknown formant filter int/enum parameter %u", parameter);
  assert(0);
}

bool
zyn_component_filter_formant_get_bool(
  void * context,
  unsigned int parameter)
{
  LOG_ERROR("Unknown formant filter bool parameter %u", parameter);
  assert(0);

  return false;
}

void
zyn_component_filter_formant_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  LOG_ERROR("Unknown formant filter bool parameter %u", parameter);
  assert(0);
}

#undef zyn_addsynth_ptr

void
zyn_addsynth_component_init_filter_formant(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_addsynth * zyn_addsynth_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, zyn_addsynth_ptr, zyn_component_filter_formant_);
}
