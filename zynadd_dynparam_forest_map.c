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
#include <stdarg.h>
#include <assert.h>

#include "common.h"
#include "list.h"
#include "addsynth.h"
#include "lv2dynparam/lv2.h"
#include "lv2dynparam/lv2dynparam.h"
#include "lv2dynparam/plugin.h"
#include "zynadd_internal.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

const char * g_shape_names[ZYN_LFO_SHAPES_COUNT];
const char * g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPES_COUNT];
const char * g_filter_type_names[ZYN_FILTER_TYPES_COUNT];
const char * g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTIONS_COUNT];

void
lv2dynparam_group_init(
  struct zyn_forest_map * map_ptr,
  unsigned int parent,
  unsigned int group,
  const char * name,
  ...)
{
  va_list ap;
  const char * hint_name;
  const char * hint_value;

  LOG_DEBUG("group \"%s\"", name);

  map_ptr->groups[group].parent = parent;
  map_ptr->groups[group].name = name;

  map_ptr->groups[group].hints.count = 0;
  map_ptr->groups[group].hints.names = (char **)map_ptr->groups[group].hint_names;
  map_ptr->groups[group].hints.values = (char **)map_ptr->groups[group].hint_values;

  va_start(ap, name);
  while ((hint_name = va_arg(ap, const char *)) != NULL)
  {
    assert(map_ptr->groups[group].hints.count < ZYN_MAX_HINTS);
    map_ptr->groups[group].hint_names[map_ptr->groups[group].hints.count] = hint_name;

    hint_value = va_arg(ap, const char *);
    if (hint_value == NULL)
    {
      LOG_DEBUG("hint \"%s\"", hint_name);
    }
    else
    {
      LOG_DEBUG("hint \"%s\":\"%s\"", hint_name, hint_value);
      map_ptr->groups[group].hint_values[map_ptr->groups[group].hints.count] = hint_value;
    }

    map_ptr->groups[group].hints.count++;
  }
  va_end(ap);
}

void zynadd_init_forest_map_globals() __attribute__((constructor));
void zynadd_init_forest_map_globals()
{
  g_shape_names[ZYN_LFO_SHAPE_TYPE_SINE] = "Sine";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_TRIANGLE] = "Triangle";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_SQUARE] = "Square";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_RAMP_UP] = "Ramp Up";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_RAMP_DOWN] = "Ramp Down";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_EXP_DOWN_1] = "E1Down";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_EXP_DOWN_2] = "E2Down";

  g_filter_type_names[ZYN_FILTER_TYPE_ANALOG] = "Analog";
  g_filter_type_names[ZYN_FILTER_TYPE_FORMANT] = "Formant";
  g_filter_type_names[ZYN_FILTER_TYPE_STATE_VARIABLE] = "State variable";

  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_LPF1] = "LPF 1 pole";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_HPF1] = "HPF 1 pole";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_LPF2] = "LPF 2 poles";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_HPF2] = "HPF 2 poles";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_BPF2] = "BPF 2 poles";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_NF2] = "NOTCH 2 poles";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_PKF2] = "PEAK (2 poles)";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_LSH2] = "Low Shelf - 2 poles";
  g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPE_HSH2] = "High Shelf - 2 poles";

  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_SINE] = "Sine";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_TRIANGLE] = "Triangle";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_PULSE] = "Pulse";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_SAW] = "Saw";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_POWER] = "Power";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_GAUSS] = "Gauss";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_DIODE] = "Diode";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_ABS_SINE] = "Abs sine";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_PULSE_SINE] = "Pulse sine";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_STRETCH_SINE] = "Stretch sine";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_CHIRP] = "Chirp";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_ABS_STRETCH_SINE] = "Abs stretch sine";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_CHEBYSHEV] = "Chebyshev";
  g_oscillator_base_function_names[ZYN_OSCILLATOR_BASE_FUNCTION_SQRT] = "Sqr";
}
