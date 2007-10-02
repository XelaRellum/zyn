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

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdbool.h>

#include "common.h"
#include "lv2dynparam/lv2.h"
#include "addsynth.h"
#include "lv2dynparam/lv2dynparam.h"
#include "lv2dynparam/plugin.h"
#include "list.h"
#include "zynadd_internal.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

#define HINT_HIDDEN           "http://home.gna.org/zynjacku/hints#hidden"
#define HINT_TOGGLE_FLOAT     "http://home.gna.org/zynjacku/hints#togglefloat"
#define HINT_ONE_SUBGROUP     "http://home.gna.org/zynjacku/hints#onesubgroup"

/* descriptors containing parent group index */
/* array elements through child index */
/* this defines the tree hierarchy */

const char * g_shape_names[ZYN_LFO_SHAPES_COUNT];
const char * g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPES_COUNT];
const char * g_filter_type_names[ZYN_FILTER_TYPES_COUNT];

bool
zynadd_bool_parameter_changed(
  void * context,
  bool value);

bool
zynadd_float_parameter_changed(
  void * context,
  float value);

bool
zynadd_int_parameter_changed(
  void * context,
  signed int value);

bool
zynadd_shape_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

bool
zynadd_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

bool
zynadd_analog_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

bool
zynadd_appear_parameter(
  struct zynadd * zynadd_ptr,
  struct zyn_forest_initializer * forest_ptr,
  unsigned int parameter_index)
{
  lv2dynparam_plugin_group parent_group;

  LOG_DEBUG(
    "Appearing parameter %u (\"%s\") -> %u",
    parameter_index,
    forest_ptr->parameters_map[parameter_index].name,
    forest_ptr->parameters[parameter_index]->addsynth_parameter);

  if (forest_ptr->parameters_map[parameter_index].parent == LV2DYNPARAM_GROUP_ROOT)
  {
    parent_group = NULL;
  }
  else
  {
    parent_group = forest_ptr->groups[forest_ptr->parameters_map[parameter_index].parent]->lv2group;
  }

  switch (forest_ptr->parameters_map[parameter_index].type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_BOOL:
    if (!lv2dynparam_plugin_param_boolean_add(
          zynadd_ptr->dynparams,
          parent_group,
          forest_ptr->parameters_map[parameter_index].name,
          &forest_ptr->parameters_map[parameter_index].hints,
          zyn_addsynth_get_bool_parameter(
            zynadd_ptr->synth,
            forest_ptr->parameters[parameter_index]->addsynth_component,
            forest_ptr->parameters[parameter_index]->addsynth_parameter),
          zynadd_bool_parameter_changed,
          forest_ptr->parameters[parameter_index],
          &forest_ptr->parameters[parameter_index]->lv2parameter))
    {
      return false;
    }

    return true;

  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    if (!lv2dynparam_plugin_param_float_add(
          zynadd_ptr->dynparams,
          parent_group,
          forest_ptr->parameters_map[parameter_index].name,
          &forest_ptr->parameters_map[parameter_index].hints,
          zyn_addsynth_get_float_parameter(
            zynadd_ptr->synth,
            forest_ptr->parameters[parameter_index]->addsynth_component,
            forest_ptr->parameters[parameter_index]->addsynth_parameter),
          forest_ptr->parameters_map[parameter_index].min.fpoint,
          forest_ptr->parameters_map[parameter_index].max.fpoint,
          zynadd_float_parameter_changed,
          forest_ptr->parameters[parameter_index],
          &forest_ptr->parameters[parameter_index]->lv2parameter))
    {
      return false;
    }

    return true;

  case LV2DYNPARAM_PARAMETER_TYPE_INT:
    if (!lv2dynparam_plugin_param_int_add(
          zynadd_ptr->dynparams,
          parent_group,
          forest_ptr->parameters_map[parameter_index].name,
          &forest_ptr->parameters_map[parameter_index].hints,
          zyn_addsynth_get_int_parameter(
            zynadd_ptr->synth,
            forest_ptr->parameters[parameter_index]->addsynth_component,
            forest_ptr->parameters[parameter_index]->addsynth_parameter),
          forest_ptr->parameters_map[parameter_index].min.integer,
          forest_ptr->parameters_map[parameter_index].max.integer,
          zynadd_int_parameter_changed,
          forest_ptr->parameters[parameter_index],
          &forest_ptr->parameters[parameter_index]->lv2parameter))
    {
      return false;
    }

    return true;

  case LV2DYNPARAM_PARAMETER_TYPE_SHAPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          forest_ptr->parameters_map[parameter_index].name,
          &forest_ptr->parameters_map[parameter_index].hints,
          g_shape_names,
          ZYN_LFO_SHAPES_COUNT,
          zyn_addsynth_get_shape_parameter(zynadd_ptr->synth, forest_ptr->parameters[parameter_index]->addsynth_component),
          zynadd_shape_parameter_changed,
          forest_ptr->parameters[parameter_index],
          &forest_ptr->parameters[parameter_index]->lv2parameter))
    {
      return false;
    }

    return true;
  case LV2DYNPARAM_PARAMETER_TYPE_FILTER_TYPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          forest_ptr->parameters_map[parameter_index].name,
          &forest_ptr->parameters_map[parameter_index].hints,
          g_filter_type_names,
          ZYN_FILTER_TYPES_COUNT,
          zyn_addsynth_get_filter_type_parameter(zynadd_ptr->synth, forest_ptr->parameters[parameter_index]->addsynth_component),
          zynadd_filter_type_parameter_changed,
          forest_ptr->parameters[parameter_index],
          &forest_ptr->parameters[parameter_index]->lv2parameter))
    {
      return false;
    }

    return true;
  case LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          forest_ptr->parameters_map[parameter_index].name,
          &forest_ptr->parameters_map[parameter_index].hints,
          g_analog_filter_type_names,
          ZYN_FILTER_ANALOG_TYPES_COUNT,
          zyn_addsynth_get_analog_filter_type_parameter(zynadd_ptr->synth, forest_ptr->parameters[parameter_index]->addsynth_component),
          zynadd_analog_filter_type_parameter_changed,
          forest_ptr->parameters[parameter_index],
          &forest_ptr->parameters[parameter_index]->lv2parameter))
    {
      return false;
    }

    return true;
  }

  assert(0);
  return false;
}

#define parameter_ptr ((struct zynadd_parameter *)context)

bool
zynadd_bool_parameter_changed(
  void * context,
  bool value)
{
  bool current_value;

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
            parameter_ptr->synth_ptr->top_forest_initializer.parameters[parameter_ptr->scope_specific]->lv2parameter))
      {
        return false;
      }
    }
    else
    {
      if (!zynadd_appear_parameter(parameter_ptr->synth_ptr, &parameter_ptr->synth_ptr->top_forest_initializer, parameter_ptr->scope_specific))
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

#undef parameter_ptr

#define LV2DYNPARAM_GROUP(group) LV2DYNPARAM_GROUP_ ## group
#define LV2DYNPARAM_PARAMETER(parameter) LV2DYNPARAM_PARAMETER_ ## parameter
#define LV2DYNPARAM_PARAMETER_SHAPE(parameter) LV2DYNPARAM_PARAMETER_ ## parameter ## _SHAPE

static
void
lv2dynparam_group_init(
  struct zyn_forest_initializer * forest_ptr,
  unsigned int parent,
  unsigned int group,
  const char * name,
  ...)
{
  va_list ap;
  const char * hint_name;
  const char * hint_value;

  LOG_DEBUG("group \"%s\"", name);

  forest_ptr->groups_map[group].parent = parent;
  forest_ptr->groups_map[group].name = name;

  forest_ptr->groups_map[group].hints.count = 0;
  forest_ptr->groups_map[group].hints.names = (char **)forest_ptr->groups_map[group].hint_names;
  forest_ptr->groups_map[group].hints.values = (char **)forest_ptr->groups_map[group].hint_values;

  va_start(ap, name);
  while ((hint_name = va_arg(ap, const char *)) != NULL)
  {
    assert(forest_ptr->groups_map[group].hints.count < ZYN_MAX_HINTS);
    forest_ptr->groups_map[group].hint_names[forest_ptr->groups_map[group].hints.count] = hint_name;

    hint_value = va_arg(ap, const char *);
    if (hint_value == NULL)
    {
      LOG_DEBUG("hint \"%s\"", hint_name);
    }
    else
    {
      LOG_DEBUG("hint \"%s\":\"%s\"", hint_name, hint_value);
      forest_ptr->groups_map[group].hint_values[forest_ptr->groups_map[group].hints.count] = hint_value;
    }

    forest_ptr->groups_map[group].hints.count++;
  }
  va_end(ap);
}

#define LV2DYNPARAM_GROUP_INIT(parent_group, group, name_value, hints...) \
  lv2dynparam_group_init(forest_ptr, LV2DYNPARAM_GROUP(parent_group), LV2DYNPARAM_GROUP(group), name_value, ## hints)

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u",                       \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter);                    \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, other_parameter, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u; with other %u",        \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter,                     \
            LV2DYNPARAM_PARAMETER(other_parameter));                    \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value ## _OTHER; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].scope_specific = LV2DYNPARAM_PARAMETER(other_parameter);

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") float -> %u:%u",                   \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_FLOAT_ ## zynparameter);                   \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].min.fpoint = min_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].max.fpoint = max_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_FLOAT_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_INT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") int -> %u:%u",                     \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_INT_ ## zynparameter);                     \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_INT; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].min.integer = min_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].max.integer = max_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_INT_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_SHAPE(parent_group, lv2parameter, component, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") shape -> %u",                      \
            LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter),                  \
            name_value);                                                \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].name = name_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_SHAPE; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component;

void zynadd_init_top_forest_initializator(struct zyn_forest_initializer * forest_ptr)
{
  int i;

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

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    forest_ptr->groups_map[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    forest_ptr->parameters_map[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, AMP, "Amplitude", NULL);
  {
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP, STEREO, AMP_GLOBALS, STEREO, "Stereo", ALWAYS, NULL);
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP, RANDOM_GROUPING, AMP_GLOBALS, RANDOM_GROUPING, "Random Grouping", ALWAYS, NULL);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP, VOLUME, AMP_GLOBALS, VOLUME, "Master Volume", 0, 100, ALWAYS, NULL);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP, VELOCITY_SENSING, AMP_GLOBALS, VELOCITY_SENSING, "Velocity sensing", 0, 100, ALWAYS, NULL);

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_PANORAMA, "Random:Panorama", HINT_TOGGLE_FLOAT, NULL, NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_PANORAMA, RANDOM_PANORAMA, AMP_GLOBALS, RANDOM_PANORAMA, "Random", HIDE, PANORAMA, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PANORAMA, PANORAMA, AMP_GLOBALS, PANORAMA, "Panorama", -1, 1, SEMI, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_PUNCH, "Punch", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_STRENGTH, AMP_GLOBALS, PUNCH_STRENGTH, "Strength", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_TIME, AMP_GLOBALS, PUNCH_TIME, "Time", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_STRETCH, AMP_GLOBALS, PUNCH_STRETCH, "Stretch", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_VELOCITY_SENSING, AMP_GLOBALS, PUNCH_VELOCITY_SENSING, "Velocity sensing", 0, 100, ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_ENV, "Envelope", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_ATTACK, AMP_ENV, ENV_ATTACK_DURATION, "Attack", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_DECAY, AMP_ENV, ENV_DECAY_DURATION, "Decay", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_SUSTAIN, AMP_ENV, ENV_SUSTAIN_VALUE, "Sustain", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_RELEASE, AMP_ENV, ENV_RELEASE_DURATION, "Release", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_STRETCH, AMP_ENV, ENV_STRETCH, "Stretch", 0, 200, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP_ENV, AMP_ENV_FORCED_RELEASE, AMP_ENV, ENV_FORCED_RELEASE, "Forced release", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP_ENV, AMP_ENV_LINEAR, AMP_ENV, ENV_LINEAR, "Linear", ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_LFO, "LFO", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_SHAPE(AMP_LFO, AMP_LFO, AMP_LFO, "Shape", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_FREQUENCY, AMP_LFO, LFO_FREQUENCY, "Frequency", 0, 1, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_DEPTH, AMP_LFO, LFO_DEPTH, "Depth", 0, 100, ALWAYS, NULL);

      LV2DYNPARAM_GROUP_INIT(AMP_LFO, AMP_LFO_START_PHASE, "Random:Start phase", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_LFO_START_PHASE, AMP_LFO_RANDOM_START_PHASE, AMP_LFO, LFO_RANDOM_START_PHASE, "Random", HIDE, AMP_LFO_START_PHASE, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO_START_PHASE, AMP_LFO_START_PHASE, AMP_LFO, LFO_START_PHASE, "Start phase", 0, 1, SEMI, NULL);
      }

      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_DELAY, AMP_LFO, LFO_DELAY, "Delay", 0, 4, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_STRETCH, AMP_LFO, LFO_STRETCH, "Stretch", -1, 1, ALWAYS, NULL); 

      LV2DYNPARAM_GROUP_INIT(AMP_LFO, AMP_LFO_DEPTH_RANDOMNESS, "Random depth:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_LFO_DEPTH_RANDOMNESS, AMP_LFO_RANDOM_DEPTH, AMP_LFO, LFO_RANDOM_DEPTH, "Random depth", SHOW, AMP_LFO_DEPTH_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO_DEPTH_RANDOMNESS, AMP_LFO_DEPTH_RANDOMNESS, AMP_LFO, LFO_DEPTH_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(AMP_LFO, AMP_LFO_FREQUENCY_RANDOMNESS, "Random frequency:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_LFO_FREQUENCY_RANDOMNESS, AMP_LFO_RANDOM_FREQUENCY, AMP_LFO, LFO_RANDOM_FREQUENCY, "Random frequency", SHOW, AMP_LFO_FREQUENCY_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO_FREQUENCY_RANDOMNESS, AMP_LFO_FREQUENCY_RANDOMNESS, AMP_LFO, LFO_FREQUENCY_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }
   }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, FILTER, "Filter", NULL);
  {
    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_FILTERS, "Filter parameters", HINT_ONE_SUBGROUP, NULL, NULL);
    {
      forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].parent = LV2DYNPARAM_GROUP(FILTER_FILTERS);
      forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].name = "Filter category";
      forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].type = LV2DYNPARAM_PARAMETER_TYPE_FILTER_TYPE;
      forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS;
      forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].addsynth_component = ZYNADD_COMPONENT_FILTER_GLOBALS;

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_ANALOG, "Analog", NULL);
      {
        forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].parent = LV2DYNPARAM_GROUP(FILTER_ANALOG);
        forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].name = "Filter type";
        forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].type = LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE;
        forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS;
        forest_ptr->parameters_map[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].addsynth_component = ZYNADD_COMPONENT_FILTER_GLOBALS;

        LV2DYNPARAM_PARAMETER_INIT_INT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_STAGES, FILTER_GLOBALS, STAGES, "Stages", 1, 5, ALWAYS, NULL);

        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_FREQUENCY, FILTER_GLOBALS, FREQUNECY, "Frequency", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_Q_FACTOR, FILTER_GLOBALS, Q_FACTOR, "Q (resonance)", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_AMOUNT, FILTER_GLOBALS, VELOCITY_SENSING_AMOUNT, "Velocity sensing amount", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_FUNCTION, FILTER_GLOBALS, VELOCITY_SENSING_FUNCTION, "Velocity sensing function", -1, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_FREQUENCY_TRACKING, FILTER_GLOBALS, FREQUENCY_TRACKING, "Frequency tracking", -1, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_GAIN, FILTER_GLOBALS, VOLUME, "Gain", -30, 30, ALWAYS, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_FORMANT, "Formant", NULL);
      {
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_SVF, "State variable", NULL);
      {
      }
    }

    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_ENV, "Envelope", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_ATTACK_VALUE, FILTER_ENV, ENV_ATTACK_VALUE, "Attack value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_ATTACK_DURATION, FILTER_ENV, ENV_ATTACK_DURATION, "Attack duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_DECAY_VALUE, FILTER_ENV, ENV_DECAY_VALUE, "Decay value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_DECAY_DURATION, FILTER_ENV, ENV_DECAY_DURATION, "Decay duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_RELEASE_VALUE, FILTER_ENV, ENV_RELEASE_VALUE, "Release value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_RELEASE_DURATION, FILTER_ENV, ENV_RELEASE_DURATION, "Release duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_STRETCH, FILTER_ENV, ENV_STRETCH, "Stretch", 0, 200, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(FILTER_ENV, FILTER_ENV_FORCED_RELEASE, FILTER_ENV, ENV_FORCED_RELEASE, "Forced release", ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_LFO, "LFO", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_SHAPE(FILTER_LFO, FILTER_LFO, FILTER_LFO, "Shape", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_FREQUENCY, FILTER_LFO, LFO_FREQUENCY, "Frequency", 0, 1, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_DEPTH, FILTER_LFO, LFO_DEPTH, "Depth", 0, 100, ALWAYS, NULL);

      LV2DYNPARAM_GROUP_INIT(FILTER_LFO, FILTER_LFO_START_PHASE, "Random:Start phase", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FILTER_LFO_START_PHASE, FILTER_LFO_RANDOM_START_PHASE, FILTER_LFO, LFO_RANDOM_START_PHASE, "Random", HIDE, FILTER_LFO_START_PHASE, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO_START_PHASE, FILTER_LFO_START_PHASE, FILTER_LFO, LFO_START_PHASE, "Start phase", 0, 1, SEMI, NULL);
      }

      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_DELAY, FILTER_LFO, LFO_DELAY, "Delay", 0, 4, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_STRETCH, FILTER_LFO, LFO_STRETCH, "Stretch", -1, 1, ALWAYS, NULL); 

      LV2DYNPARAM_GROUP_INIT(FILTER_LFO, FILTER_LFO_DEPTH_RANDOMNESS, "Random depth:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FILTER_LFO_DEPTH_RANDOMNESS, FILTER_LFO_RANDOM_DEPTH, FILTER_LFO, LFO_RANDOM_DEPTH, "Random depth", SHOW, FILTER_LFO_DEPTH_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO_DEPTH_RANDOMNESS, FILTER_LFO_DEPTH_RANDOMNESS, FILTER_LFO, LFO_DEPTH_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_LFO, FILTER_LFO_FREQUENCY_RANDOMNESS, "Random frequency:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FILTER_LFO_FREQUENCY_RANDOMNESS, FILTER_LFO_RANDOM_FREQUENCY, FILTER_LFO, LFO_RANDOM_FREQUENCY, "Random frequency", SHOW, FILTER_LFO_FREQUENCY_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO_FREQUENCY_RANDOMNESS, FILTER_LFO_FREQUENCY_RANDOMNESS, FILTER_LFO, LFO_FREQUENCY_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }
    }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, FREQUENCY, "Frequency", NULL);
  {
    LV2DYNPARAM_GROUP_INIT(FREQUENCY, FREQUENCY_ENV, "Envelope", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_ATTACK_VALUE, FREQUENCY_ENV, ENV_ATTACK_VALUE, "Attack value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_ATTACK_DURATION, FREQUENCY_ENV, ENV_ATTACK_DURATION, "Attack duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_RELEASE_VALUE, FREQUENCY_ENV, ENV_RELEASE_VALUE, "Release value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_RELEASE_DURATION, FREQUENCY_ENV, ENV_RELEASE_DURATION, "Release duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_STRETCH, FREQUENCY_ENV, ENV_STRETCH, "Stretch", 0, 200, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(FREQUENCY_ENV, FREQUENCY_ENV_FORCED_RELEASE, FREQUENCY_ENV, ENV_FORCED_RELEASE, "Forced release", ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(FREQUENCY, FREQUENCY_LFO, "LFO", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_SHAPE(FREQUENCY_LFO, FREQUENCY_LFO, FREQUENCY_LFO, "Shape", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_FREQUENCY, FREQUENCY_LFO, LFO_FREQUENCY, "Frequency", 0, 1, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_DEPTH, FREQUENCY_LFO, LFO_DEPTH, "Depth", 0, 100, ALWAYS, NULL);

      LV2DYNPARAM_GROUP_INIT(FREQUENCY_LFO, FREQUENCY_LFO_START_PHASE, "Random:Start phase", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FREQUENCY_LFO_START_PHASE, FREQUENCY_LFO_RANDOM_START_PHASE, FREQUENCY_LFO, LFO_RANDOM_START_PHASE, "Random", HIDE, FREQUENCY_LFO_START_PHASE, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO_START_PHASE, FREQUENCY_LFO_START_PHASE, FREQUENCY_LFO, LFO_START_PHASE, "Start phase", 0, 1, SEMI, NULL);
      }

      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_DELAY, FREQUENCY_LFO, LFO_DELAY, "Delay", 0, 4, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_STRETCH, FREQUENCY_LFO, LFO_STRETCH, "Stretch", -1, 1, ALWAYS, NULL); 

      LV2DYNPARAM_GROUP_INIT(FREQUENCY_LFO, FREQUENCY_LFO_DEPTH_RANDOMNESS, "Random depth:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FREQUENCY_LFO_DEPTH_RANDOMNESS, FREQUENCY_LFO_RANDOM_DEPTH, FREQUENCY_LFO, LFO_RANDOM_DEPTH, "Random depth", SHOW, FREQUENCY_LFO_DEPTH_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO_DEPTH_RANDOMNESS, FREQUENCY_LFO_DEPTH_RANDOMNESS, FREQUENCY_LFO, LFO_DEPTH_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FREQUENCY_LFO, FREQUENCY_LFO_FREQUENCY_RANDOMNESS, "Random frequency:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FREQUENCY_LFO_FREQUENCY_RANDOMNESS, FREQUENCY_LFO_RANDOM_FREQUENCY, FREQUENCY_LFO, LFO_RANDOM_FREQUENCY, "Random frequency", SHOW, FREQUENCY_LFO_FREQUENCY_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO_FREQUENCY_RANDOMNESS, FREQUENCY_LFO_FREQUENCY_RANDOMNESS, FREQUENCY_LFO, LFO_FREQUENCY_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }
    }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, VOICES, "Voices", NULL);
  {
  }

  /* santity check that we have filled all values */

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("parameter %d with parent %d", i, forest_ptr->parameters_map[i].parent);
    assert(forest_ptr->parameters_map[i].parent != LV2DYNPARAM_GROUP_INVALID);
    assert(forest_ptr->parameters_map[i].parent < LV2DYNPARAM_GROUPS_COUNT);
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    LOG_DEBUG("group %d with parent %d", i, forest_ptr->groups_map[i].parent);
    assert(forest_ptr->groups_map[i].parent != LV2DYNPARAM_GROUP_INVALID);

    assert(forest_ptr->groups_map[i].name != NULL);

    /* check that parents are with smaller indexes than children */
    /* this checks for loops too */
    assert(forest_ptr->groups_map[i].parent < i);
  }
}

bool zynadd_dynparam_init(struct zynadd * zynadd_ptr)
{
  int i;
  bool tmp_bool;
  struct zynadd_group * group_ptr;
  struct zynadd_parameter * parameter_ptr;
  struct zyn_forest_initializer * forest_ptr;

  forest_ptr = &zynadd_ptr->top_forest_initializer;

  zynadd_init_top_forest_initializator(forest_ptr);

  INIT_LIST_HEAD(&zynadd_ptr->groups);
  INIT_LIST_HEAD(&zynadd_ptr->parameters);

  if (!lv2dynparam_plugin_instantiate(
        (LV2_Handle)zynadd_ptr,
        "zynadd",
        &zynadd_ptr->dynparams))
  {
    goto fail;
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    LOG_DEBUG("Adding group \"%s\"", forest_ptr->groups_map[i].name);

    group_ptr = malloc(sizeof(struct zynadd_group));
    if (group_ptr == NULL)
    {
      goto fail_clean_dynparams;
    }

    list_add_tail(&group_ptr->siblings, &zynadd_ptr->groups);

    forest_ptr->groups[i] = group_ptr;

    if (!lv2dynparam_plugin_group_add(
          zynadd_ptr->dynparams,
          forest_ptr->groups_map[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : forest_ptr->groups[forest_ptr->groups_map[i].parent]->lv2group,
          forest_ptr->groups_map[i].name,
          &forest_ptr->groups_map[i].hints,
          &group_ptr->lv2group))
    {
      goto fail_clean_dynparams;
    }
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    parameter_ptr = malloc(sizeof(struct zynadd_parameter));
    if (parameter_ptr == NULL)
    {
      goto fail_clean_dynparams;
    }

    list_add_tail(&parameter_ptr->siblings, &zynadd_ptr->parameters);

    forest_ptr->parameters[i] = parameter_ptr;

    parameter_ptr->synth_ptr = zynadd_ptr;
    parameter_ptr->addsynth_parameter = forest_ptr->parameters_map[i].addsynth_parameter;
    parameter_ptr->addsynth_component = forest_ptr->parameters_map[i].addsynth_component;
    parameter_ptr->scope = forest_ptr->parameters_map[i].scope;
    parameter_ptr->scope_specific = forest_ptr->parameters_map[i].scope_specific;
    parameter_ptr->lv2parameter = NULL;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("Adding parameter \"%s\"", forest_ptr->parameters_map[i].name);

    if (forest_ptr->parameters_map[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SEMI)
    {
      continue;
    }

    if (forest_ptr->parameters_map[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER ||
        forest_ptr->parameters_map[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER)
    {
      LOG_DEBUG("Apearing show/hide parameter \"%s\"", forest_ptr->parameters_map[i].name);
      assert(forest_ptr->parameters_map[i].type == LV2DYNPARAM_PARAMETER_TYPE_BOOL);

      tmp_bool = zyn_addsynth_get_bool_parameter(
        zynadd_ptr->synth,
        forest_ptr->parameters_map[i].addsynth_component,
        forest_ptr->parameters_map[i].addsynth_parameter);

      if (!zynadd_appear_parameter(zynadd_ptr, forest_ptr, i))
      {
        goto fail_clean_dynparams;
      }

      if ((forest_ptr->parameters_map[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER && !tmp_bool) ||
          (forest_ptr->parameters_map[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER && tmp_bool))
      {
        LOG_DEBUG("Apearing semi parameter %u", forest_ptr->parameters_map[i].scope_specific);
        LOG_DEBUG("Apearing semi parameter \"%s\"", forest_ptr->parameters_map[forest_ptr->parameters_map[i].scope_specific].name);
        if (!zynadd_appear_parameter(zynadd_ptr, forest_ptr, forest_ptr->parameters_map[i].scope_specific))
        {
          goto fail_clean_dynparams;
        }
      }

      continue;
    }

    assert(forest_ptr->parameters_map[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS);

    if (!zynadd_appear_parameter(zynadd_ptr, forest_ptr, i))
    {
      LOG_ERROR("zynadd_appear_parameter() failed.");
      goto fail_clean_dynparams;
    }
  }

  return true;

fail_clean_dynparams:
  zynadd_dynparam_uninit(zynadd_ptr);

fail:
  return false;
}

void
zynadd_dynparam_uninit(struct zynadd * zynadd_ptr)
{
  struct list_head * node_ptr;
  struct zynadd_group * group_ptr;
  struct zynadd_parameter * parameter_ptr;

  while (!list_empty(&zynadd_ptr->parameters))
  {
    node_ptr = zynadd_ptr->parameters.next;
    parameter_ptr = list_entry(node_ptr, struct zynadd_parameter, siblings);
    list_del(node_ptr);
    free(parameter_ptr);
  }

  while (!list_empty(&zynadd_ptr->groups))
  {
    node_ptr = zynadd_ptr->groups.next;
    group_ptr = list_entry(node_ptr, struct zynadd_group, siblings);
    list_del(node_ptr);
    free(group_ptr);
  }

  lv2dynparam_plugin_cleanup(zynadd_ptr->dynparams);
}
