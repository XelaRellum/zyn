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

#include "common.h"
#include "list.h"
#include "addsynth.h"
#include "lv2dynparam/lv2.h"
#include "lv2dynparam/lv2dynparam.h"
#include "lv2dynparam/plugin.h"
#include "zynadd_internal.h"
#include "zynadd_dynparam_value_changed_callbacks.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

#define parameter_ptr ((struct zynadd_parameter *)context)

bool
zynadd_bool_parameter_changed(
  void * context,
  bool value)
{
  bool current_value;

  LOG_DEBUG("bool parameter \"%s\" changed to \"%s\"", parameter_ptr->name_ptr, value ? "true" : "false");

  if (parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER ||
      parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER)
  {
    current_value = zyn_addsynth_get_bool_parameter(
      parameter_ptr->synth_ptr->synth,
      parameter_ptr->addsynth_component,
      parameter_ptr->addsynth_parameter);

    if ((current_value && value) ||
        (!current_value && !value))
    {
      /* value not changed */
      return true;
    }

    if ((parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER && value) ||
        (parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER && !value))
    {
      /* enabling randomize -> remove panorama parameter */
      if (!lv2dynparam_plugin_param_remove(
            parameter_ptr->synth_ptr->dynparams,
            parameter_ptr->other_parameter->lv2parameter))
      {
        return false;
      }
    }
    else
    {
      if (!zynadd_appear_parameter(parameter_ptr->synth_ptr, parameter_ptr->other_parameter->lv2parameter))
      {
        return false;
      }
    }
  }

  zyn_addsynth_set_bool_parameter(
    parameter_ptr->synth_ptr->synth,
    parameter_ptr->addsynth_component,
    parameter_ptr->addsynth_parameter,
    value);

  return true;
}

bool
zynadd_float_parameter_changed(
  void * context,
  float value)
{
  zyn_addsynth_set_float_parameter(
    parameter_ptr->synth_ptr->synth,
    parameter_ptr->addsynth_component,
    parameter_ptr->addsynth_parameter,
    value);

  return true;
}

bool
zynadd_shape_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index)
{
  zyn_addsynth_set_shape_parameter(
    parameter_ptr->synth_ptr->synth,
    parameter_ptr->addsynth_component,
    value_index);

  return true;
}

bool
zynadd_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index)
{
  return true;
}

bool
zynadd_analog_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index)
{
  return true;
}

bool
zynadd_int_parameter_changed(
  void * context,
  signed int value)
{
//  LOG_ERROR("int parameter changed to value %d", value);
  return true;
}
