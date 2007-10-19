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

#include "globals.h"
#include "fft.h"
#include "resonance.h"
#include "oscillator.h"
#include "addsynth.h"
#include "portamento.h"
#include "addsynth_component.h"
#include "log.h"

#define oscillator_ptr ((struct zyn_oscillator * )context)

float
zyn_oscillator_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_OSCILLATOR_WAVESHAPE_DRIVE:
    return oscillator_ptr->waveshaping_drive;
  case ZYNADD_PARAMETER_FLOAT_OSCILLATOR_BASE_FUNCTION_ADJUST:
    return oscillator_ptr->base_function_adjust;
  }

  LOG_ERROR("Unknown oscillator float parameter %u", parameter);
  assert(0);
  return 0.0;
}

void
zyn_oscillator_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_OSCILLATOR_WAVESHAPE_DRIVE:
    assert(value >= 0.0 && value <= 100.0);
    oscillator_ptr->waveshaping_drive = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_OSCILLATOR_BASE_FUNCTION_ADJUST:
    assert(value >= 0.0 && value <= 1.0);
    oscillator_ptr->base_function_adjust = value;
    return;
  }

  LOG_ERROR("Unknown oscillator float parameter %u", parameter);
  assert(0);
}

signed int
zyn_oscillator_get_int(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_ENUM_OSCILLATOR_BASE_FUNCTION:
    return oscillator_ptr->base_function;
  case ZYNADD_PARAMETER_ENUM_OSCILLATOR_WAVESHAPE_TYPE:
    return oscillator_ptr->waveshaping_function;
  }

  LOG_ERROR("Unknown oscillator int/enum parameter %u", parameter);
  assert(0);
  return 0;
}

void
zyn_oscillator_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_ENUM_OSCILLATOR_BASE_FUNCTION:
    assert(value >= 0 && value < ZYN_OSCILLATOR_BASE_FUNCTIONS_COUNT);
    oscillator_ptr->base_function = value;
    return;
  case ZYNADD_PARAMETER_ENUM_OSCILLATOR_WAVESHAPE_TYPE:
    assert(value >= 0 && value < ZYN_OSCILLATOR_WAVESHAPE_TYPES_COUNT);
    oscillator_ptr->waveshaping_function = value;
    return;
  }

  LOG_ERROR("Unknown oscillator int/enum parameter %u", parameter);
  assert(0);
}

bool
zyn_oscillator_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  }

  LOG_ERROR("Unknown oscillator bool parameter %u", parameter);
  assert(0);
}

void
zyn_oscillator_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  }

  LOG_ERROR("Unknown oscillator bool parameter %u", parameter);
  assert(0);
}

#undef oscillator_ptr

void
zyn_addsynth_component_init_oscillator(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_oscillator * oscillator_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, oscillator_ptr, zyn_oscillator_);
}
