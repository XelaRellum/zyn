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
#include "addsynth_component.h"

//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

void
zyn_portamento_init(
  struct zyn_portamento * portamento_ptr)
{
  portamento_ptr->enabled = false;
  portamento_ptr->used = false;
  portamento_ptr->time = 64;
  portamento_ptr->updowntimestretch = 64;
  portamento_ptr->pitchthresh = 3;
  portamento_ptr->pitch_threshold_type = ZYN_PORTAMENTO_PITCH_THRESHOLD_TYPE_MAX;

  zyn_portamento_start(portamento_ptr, 440.0, 440.0);
}

// returns true if the portamento's conditions are true, else returns false
bool
zyn_portamento_start(
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

  portamentotime = pow(100.0, portamento_ptr->time / 127.0) / 50.0; // portamento time in seconds

  if (portamento_ptr->updowntimestretch >= 64 &&
      newfreq < oldfreq)
  {
    if (portamento_ptr->updowntimestretch == 127)
    {
      LOG_DEBUG("Not using portamento 2");
      return false;
    }

    portamentotime *= pow(0.1, (portamento_ptr->updowntimestretch - 64) / 63.0);
  } 

  if (portamento_ptr->updowntimestretch < 64 &&
      newfreq > oldfreq)
  {
    if (portamento_ptr->updowntimestretch == 0)
    {
      LOG_DEBUG("Not using portamento 3");
      return false;
    }

    portamentotime *= pow(0.1, (64.0 - portamento_ptr->updowntimestretch) / 64.0);
  }
    
  portamento_ptr->dx = SOUND_BUFFER_SIZE / (portamentotime * SAMPLE_RATE);
  portamento_ptr->origfreqrap = oldfreq / newfreq;

  if (portamento_ptr->origfreqrap > 1.0)
  {
    tmprap = portamento_ptr->origfreqrap;
  }
  else
  {
    tmprap = 1.0 / portamento_ptr->origfreqrap;
  }

  thresholdrap = pow(2.0, portamento_ptr->pitchthresh / 12.0);

  if (portamento_ptr->pitch_threshold_type == ZYN_PORTAMENTO_PITCH_THRESHOLD_TYPE_MIN &&
      tmprap - 0.00001 > thresholdrap)
  {
    LOG_DEBUG("Not using portamento 4");
    return false;
  }

  if (portamento_ptr->pitch_threshold_type == ZYN_PORTAMENTO_PITCH_THRESHOLD_TYPE_MAX &&
      tmprap + 0.00001 < thresholdrap)
  {
    LOG_DEBUG("Not using portamento 5");
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
  assert(0);
  return 0;
}

void
zyn_component_portamento_set_int(
  void * context,
  unsigned int parameter,
  signed int value)
{
  assert(0);
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
  default:
    LOG_ERROR("Unknown bool portamento parameter %u", parameter);
    assert(0);
  }
}

unsigned int
zyn_component_portamento_get_shape(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_portamento_set_shape(
  void * context,
  unsigned int value)
{
  assert(0);
}

unsigned int
zyn_component_portamento_get_filter_type(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_portamento_set_filter_type(
  void * context,
  unsigned int value)
{
  assert(0);
}

unsigned int
zyn_component_portamento_get_analog_filter_type(
  void * context)
{
  assert(0);
  return 0;
}

void
zyn_component_portamento_set_analog_filter_type(
  void * context,
  unsigned int value)
{
  assert(0);
}

#undef portamento_ptr

void
zyn_addsynth_component_init_portamento(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_portamento * portamento_ptr)
{
  ZYN_INIT_COMPONENT(component_ptr, portamento_ptr, zyn_component_portamento_);
}
