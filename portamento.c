/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
 *   Copyright (C) 2002-2005 Nasca Octavian Paul
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
#include <math.h>
#include <assert.h>

#include "globals.h"
#include "portamento.h"
#include "addsynth.h"
#include "fft.h"
#include "oscillator.h"
#include "addsynth_component.h"

//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

void
zyn_portamento_init(
  struct zyn_portamento * portamento_ptr)
{
  portamento_ptr->enabled = false;
  portamento_ptr->used = false;
  portamento_ptr->time = 0.5;
  portamento_ptr->up_down_time_stretch = 0.0;
  portamento_ptr->pitch_threshold = 3; /* 3 equally tempered semitones */
  portamento_ptr->pitch_threshold_above = true;
}

// returns true if the portamento's conditions are true, else returns false
bool
zyn_portamento_start(
  float sample_rate,
  struct zyn_portamento * portamento_ptr,
  float oldfreq,
  float newfreq)
{
  float portamentotime;
  float tmprap;
  float thresholdrap;

  portamento_ptr->x = 0.0;

  if (portamento_ptr->used)
  {
    LOG_DEBUG("Not using portamento, already used");
    return false;
  }

  if (!portamento_ptr->enabled)
  {
    LOG_DEBUG("Not using portamento, disabled");
    return false;
  }

  portamentotime = powf(100.0, portamento_ptr->time) / 50.0; // portamento time in seconds

  if (portamento_ptr->up_down_time_stretch >= 0.0 &&
      newfreq < oldfreq)
  {
    if (portamento_ptr->up_down_time_stretch == 1.0)
    {
      LOG_DEBUG("Not using portamento down portamento because of time stretch value");
      return false;
    }

    portamentotime *= pow(0.1, portamento_ptr->up_down_time_stretch);
  } 

  if (portamento_ptr->up_down_time_stretch < 0.0 &&
      newfreq > oldfreq)
  {
    if (portamento_ptr->up_down_time_stretch == -1.0)
    {
      LOG_DEBUG("Not using portamento up portamento because of time stretch value");
      return false;
    }

    portamentotime *= pow(0.1, -portamento_ptr->up_down_time_stretch);
  }
    
  portamento_ptr->dx = SOUND_BUFFER_SIZE / (portamentotime * sample_rate);
  portamento_ptr->origfreqrap = oldfreq / newfreq;

  if (portamento_ptr->origfreqrap > 1.0)
  {
    tmprap = portamento_ptr->origfreqrap;
  }
  else
  {
    tmprap = 1.0 / portamento_ptr->origfreqrap;
  }

  thresholdrap = pow(2.0, portamento_ptr->pitch_threshold / 12.0);

  if (!portamento_ptr->pitch_threshold_above &&
      tmprap - 0.00001 > thresholdrap)
  {
    LOG_DEBUG("Not using portamento because it is not below threshold");
    return false;
  }

  if (portamento_ptr->pitch_threshold_above &&
      tmprap + 0.00001 < thresholdrap)
  {
    LOG_DEBUG("Not using portamento because it is not above threshold");
    return false;
  }

  LOG_DEBUG("Using portamento");

  portamento_ptr->used = true;
  portamento_ptr->freqrap=portamento_ptr->origfreqrap;

  return true;
}

// update portamento values
void
zyn_portamento_update(
  struct zyn_portamento * portamento_ptr)
{
  if (!portamento_ptr->used)
  {
    return;
  }
    
  portamento_ptr->x += portamento_ptr->dx;

  if (portamento_ptr->x > 1.0)
  {
    portamento_ptr->x = 1.0;
    portamento_ptr->used = false;
  }

  portamento_ptr->freqrap = (1.0 - portamento_ptr->x) * portamento_ptr->origfreqrap + portamento_ptr->x;
}

#define portamento_ptr ((struct zyn_portamento * )context)

float
zyn_component_portamento_get_float(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_PORTAMENTO_TIME:
    return portamento_ptr->time;
  case ZYNADD_PARAMETER_FLOAT_PORTAMENTO_TIME_STRETCH:
    return portamento_ptr->up_down_time_stretch;
  default:
    LOG_ERROR("Unknown portamento float parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_portamento_set_float(
  void * context,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_PORTAMENTO_TIME:
    portamento_ptr->time = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_PORTAMENTO_TIME_STRETCH:
    portamento_ptr->up_down_time_stretch = value;
    return;
  default:
    LOG_ERROR("Unknown portamento float parameter %u", parameter);
    assert(0);
  }
}

signed int
zyn_component_portamento_get_int(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_INT_PORTAMENTO_PITCH_THRESHOLD:
    return portamento_ptr->pitch_threshold;
  default:
    LOG_ERROR("Unknown portamento int parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_portamento_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_INT_PORTAMENTO_PITCH_THRESHOLD:
    portamento_ptr->pitch_threshold = value;
    return;
  default:
    LOG_ERROR("Unknown portamento int parameter %u", parameter);
    assert(0);
  }
}

bool
zyn_component_portamento_get_bool(
  void * context,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_PORTAMENTO_ENABLED:
    return portamento_ptr->enabled;
  case ZYNADD_PARAMETER_BOOL_PORTAMENTO_PITCH_THRESHOLD_ABOVE:
    return portamento_ptr->pitch_threshold_above;
  default:
    LOG_ERROR("Unknown bool portamento parameter %u", parameter);
    assert(0);
  }
}

void
zyn_component_portamento_set_bool(
  void * context,
  unsigned int parameter,
  bool value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_PORTAMENTO_ENABLED:
    portamento_ptr->enabled = value;
    return;
  case ZYNADD_PARAMETER_BOOL_PORTAMENTO_PITCH_THRESHOLD_ABOVE:
    portamento_ptr->pitch_threshold_above = value;
    return;
  default:
    LOG_ERROR("Unknown bool portamento parameter %u", parameter);
    assert(0);
  }
}

#undef portamento_ptr

void
zyn_addsynth_component_init_portamento(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_portamento * portamento_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, portamento_ptr, zyn_component_portamento_);
}
