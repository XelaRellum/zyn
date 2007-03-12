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

#include "common.h"
#include "lv2dynparam/lv2.h"
#include "addsynth.h"
#include "lv2dynparam/lv2dynparam.h"
#include "lv2dynparam/plugin.h"
#include "zynadd_internal.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

#define ZYN_MAX_HINTS 10

#define HINT_HIDDEN           "http://home.gna.org/zynjacku/hints#hidden"
#define HINT_TOGGLE_FLOAT     "http://home.gna.org/zynjacku/hints#togglefloat"
#define HINT_NOTEBOOK         "http://home.gna.org/zynjacku/hints#notebook"

struct group_descriptor
{
  int parent;                   /* index of parent, LV2DYNPARAM_GROUP_ROOT for root children */

  const char * name;            /* group name */

  struct lv2dynparam_hints hints;
  const char * hint_names[ZYN_MAX_HINTS];
  const char * hint_values[ZYN_MAX_HINTS];
};

struct parameter_descriptor
{
  int parent;                   /* index of parent, LV2DYNPARAM_GROUP_ROOT for root children */
  const char * name;            /* parameter name */

  struct lv2dynparam_hints hints;
  const char * hint_names[ZYN_MAX_HINTS];
  const char * hint_values[ZYN_MAX_HINTS];

  unsigned int type;            /* one of LV2DYNPARAM_PARAMETER_TYPE_XXX */

  unsigned int addsynth_component; /* one of ZYNADD_COMPONENT_XXX */
  unsigned int addsynth_parameter; /* one of ZYNADD_PARAMETER_XXX */

  unsigned int scope;                   /* one of LV2DYNPARAM_PARAMETER_SCOPE_TYPE_XXX */
  unsigned int scope_specific;

  union
  {
    float fpoint;
  } min;

  union
  {
    float fpoint;
  } max;
};

/* descriptors containing parent group index */
/* array elements through child index */
/* this defines the tree hierarchy */
struct group_descriptor g_map_groups[LV2DYNPARAM_GROUPS_COUNT];
struct parameter_descriptor g_map_parameters[LV2DYNPARAM_PARAMETERS_COUNT];
const char * g_shape_names[ZYN_LFO_SHAPES_COUNT];
const char * g_analog_filter_type_names[ZYN_FILTER_ANALOG_TYPES_COUNT];

BOOL
zynadd_bool_parameter_changed(
  void * context,
  BOOL value);

BOOL
zynadd_float_parameter_changed(
  void * context,
  float value);

BOOL
zynadd_shape_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

BOOL
zynadd_analog_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

BOOL
zynadd_appear_parameter(
  struct zynadd * zynadd_ptr,
  unsigned int parameter_index)
{
  void * parent_group;

  LOG_DEBUG(
    "Appearing parameter %u (\"%s\") -> %u",
    parameter_index,
    g_map_parameters[parameter_index].name,
    zynadd_ptr->parameters[parameter_index].addsynth_parameter);

  if (g_map_parameters[parameter_index].parent == LV2DYNPARAM_GROUP_ROOT)
  {
    parent_group = NULL;
  }
  else
  {
    parent_group = zynadd_ptr->groups[g_map_parameters[parameter_index].parent];
  }

  switch (g_map_parameters[parameter_index].type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_BOOL:
    if (!lv2dynparam_plugin_param_boolean_add(
          zynadd_ptr->dynparams,
          parent_group,
          g_map_parameters[parameter_index].name,
          &g_map_parameters[parameter_index].hints,
          zyn_addsynth_get_bool_parameter(
            zynadd_ptr->synth,
            zynadd_ptr->parameters[parameter_index].addsynth_component,
            zynadd_ptr->parameters[parameter_index].addsynth_parameter),
          zynadd_bool_parameter_changed,
          zynadd_ptr->parameters + parameter_index,
          &zynadd_ptr->parameters[parameter_index].lv2parameter))
    {
      return FALSE;
    }

    return TRUE;

  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    if (!lv2dynparam_plugin_param_float_add(
          zynadd_ptr->dynparams,
          parent_group,
          g_map_parameters[parameter_index].name,
          &g_map_parameters[parameter_index].hints,
          zyn_addsynth_get_float_parameter(
            zynadd_ptr->synth,
            zynadd_ptr->parameters[parameter_index].addsynth_component,
            zynadd_ptr->parameters[parameter_index].addsynth_parameter),
          g_map_parameters[parameter_index].min.fpoint,
          g_map_parameters[parameter_index].max.fpoint,
          zynadd_float_parameter_changed,
          zynadd_ptr->parameters + parameter_index,
          &zynadd_ptr->parameters[parameter_index].lv2parameter))
    {
      return FALSE;
    }

    return TRUE;

  case LV2DYNPARAM_PARAMETER_TYPE_SHAPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          g_map_parameters[parameter_index].name,
          &g_map_parameters[parameter_index].hints,
          g_shape_names,
          ZYN_LFO_SHAPES_COUNT,
          zyn_addsynth_get_shape_parameter(zynadd_ptr->synth, zynadd_ptr->parameters[parameter_index].addsynth_component),
          zynadd_shape_parameter_changed,
          zynadd_ptr->parameters + parameter_index,
          &zynadd_ptr->parameters[parameter_index].lv2parameter))
    {
      return FALSE;
    }

    return TRUE;
  case LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          g_map_parameters[parameter_index].name,
          &g_map_parameters[parameter_index].hints,
          g_analog_filter_type_names,
          ZYN_FILTER_ANALOG_TYPES_COUNT,
          zyn_addsynth_get_analog_filter_type_parameter(zynadd_ptr->synth, zynadd_ptr->parameters[parameter_index].addsynth_component),
          zynadd_analog_filter_type_parameter_changed,
          zynadd_ptr->parameters + parameter_index,
          &zynadd_ptr->parameters[parameter_index].lv2parameter))
    {
      return FALSE;
    }

    return TRUE;
  }

  assert(0);
  return FALSE;
}

#define parameter_ptr ((struct zynadd_parameter *)context)

BOOL
zynadd_bool_parameter_changed(
  void * context,
  BOOL value)
{
  BOOL current_value;

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
      return TRUE;
    }

    if ((parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER && value) ||
        (parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER && !value))
    {
      /* enabling randomize -> remove panorama parameter */
      if (!lv2dynparam_plugin_param_remove(
            parameter_ptr->synth_ptr->dynparams,
            parameter_ptr->synth_ptr->parameters[parameter_ptr->scope_specific].lv2parameter))
      {
        return FALSE;
      }
    }
    else
    {
      if (!zynadd_appear_parameter(parameter_ptr->synth_ptr, parameter_ptr->scope_specific))
      {
        return FALSE;
      }
    }
  }

  zyn_addsynth_set_bool_parameter(
    parameter_ptr->synth_ptr->synth,
    parameter_ptr->addsynth_component,
    parameter_ptr->addsynth_parameter,
    value);

  return TRUE;
}

BOOL
zynadd_float_parameter_changed(
  void * context,
  float value)
{
  zyn_addsynth_set_float_parameter(
    parameter_ptr->synth_ptr->synth,
    parameter_ptr->addsynth_component,
    parameter_ptr->addsynth_parameter,
    value);

  return TRUE;
}

BOOL
zynadd_shape_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index)
{
  zyn_addsynth_set_shape_parameter(
    parameter_ptr->synth_ptr->synth,
    parameter_ptr->addsynth_component,
    value_index);

  return TRUE;
}

BOOL
zynadd_analog_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index)
{
  return TRUE;
}

#undef parameter_ptr

#define LV2DYNPARAM_GROUP(group) LV2DYNPARAM_GROUP_ ## group
#define LV2DYNPARAM_PARAMETER(parameter) LV2DYNPARAM_PARAMETER_ ## parameter
#define LV2DYNPARAM_PARAMETER_SHAPE(parameter) LV2DYNPARAM_PARAMETER_ ## parameter ## _SHAPE

static void
lv2dynparam_group_init(unsigned int parent, unsigned int group, const char * name, ...)
{
  va_list ap;
  const char * hint_name;
  const char * hint_value;

  LOG_DEBUG("group \"%s\"", name);

  g_map_groups[group].parent = parent;
  g_map_groups[group].name = name;

  g_map_groups[group].hints.count = 0;
  g_map_groups[group].hints.names = (char **)g_map_groups[group].hint_names;
  g_map_groups[group].hints.values = (char **)g_map_groups[group].hint_values;

  va_start(ap, name);
  while ((hint_name = va_arg(ap, const char *)) != NULL)
  {
    assert(g_map_groups[group].hints.count < ZYN_MAX_HINTS);
    g_map_groups[group].hint_names[g_map_groups[group].hints.count] = hint_name;

    hint_value = va_arg(ap, const char *);
    if (hint_value == NULL)
    {
      LOG_DEBUG("hint \"%s\"", hint_name);
    }
    else
    {
      LOG_DEBUG("hint \"%s\":\"%s\"", hint_name, hint_value);
      g_map_groups[group].hint_values[g_map_groups[group].hints.count] = hint_value;
    }

    g_map_groups[group].hints.count++;
  }
  va_end(ap);
}

#define LV2DYNPARAM_GROUP_INIT(parent_group, group, name_value, hints...) \
  lv2dynparam_group_init(LV2DYNPARAM_GROUP(parent_group), LV2DYNPARAM_GROUP(group), name_value, ## hints)

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u",                       \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter);                    \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, other_parameter, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u; with other %u",        \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter,                     \
            LV2DYNPARAM_PARAMETER(other_parameter));                    \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value ## _OTHER; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope_specific = LV2DYNPARAM_PARAMETER(other_parameter);

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") float -> %u:%u",                   \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_FLOAT_ ## zynparameter);                   \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].min.fpoint = min_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].max.fpoint = max_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_FLOAT_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_SHAPE(parent_group, lv2parameter, component, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") shape -> %u",                      \
            LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter),                  \
            name_value);                                                \
  g_map_parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_SHAPE; \
  g_map_parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component;

void zynadd_map_initialise() __attribute__((constructor));
void zynadd_map_initialise()
{
  int i;

  g_shape_names[ZYN_LFO_SHAPE_TYPE_SINE] = "Sine";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_TRIANGLE] = "Triangle";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_SQUARE] = "Square";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_RAMP_UP] = "Ramp Up";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_RAMP_DOWN] = "Ramp Down";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_EXP_DOWN_1] = "E1Down";
  g_shape_names[ZYN_LFO_SHAPE_TYPE_EXP_DOWN_2] = "E2Down";

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
    g_map_groups[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    g_map_parameters[i].parent = LV2DYNPARAM_GROUP_INVALID;
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
    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_FILTERS, "Filters", HINT_NOTEBOOK, NULL, NULL);
    {
      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_ANALOG, "Analog", HINT_HIDDEN, NULL, NULL);
      {
        g_map_parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].parent = LV2DYNPARAM_GROUP(FILTER_ANALOG);
        g_map_parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].name = "Filter type";
        g_map_parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].type = LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE;
        g_map_parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS;
        g_map_parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].addsynth_component = ZYNADD_COMPONENT_FILTER_GLOBALS;

        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_FREQUENCY, FILTER_GLOBALS, FREQUNECY, "Frequency", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_Q_FACTOR, FILTER_GLOBALS, Q_FACTOR, "Q (resonance)", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_AMOUNT, FILTER_GLOBALS, VELOCITY_SENSING_AMOUNT, "Velocity sensing amount", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_FUNCTION, FILTER_GLOBALS, VELOCITY_SENSING_FUNCTION, "Velocity sensing function", -1, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_FREQUENCY_TRACKING, FILTER_GLOBALS, FREQUENCY_TRACKING, "Frequency tracking", -1, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_GAIN, FILTER_GLOBALS, VOLUME, "Gain", -30, 30, ALWAYS, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_FORMANT, "Formant", HINT_HIDDEN, NULL, NULL);
      {
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_SVF, "SVF", HINT_HIDDEN, NULL, NULL);
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

  /* santity check that we have filled all values */

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("parameter %d with parent %d", i, g_map_parameters[i].parent);
    assert(g_map_parameters[i].parent != LV2DYNPARAM_GROUP_INVALID);
    assert(g_map_parameters[i].parent < LV2DYNPARAM_GROUPS_COUNT);
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    LOG_DEBUG("group %d with parent %d", i, g_map_groups[i].parent);
    assert(g_map_groups[i].parent != LV2DYNPARAM_GROUP_INVALID);

    assert(g_map_groups[i].name != NULL);

    /* check that parents are with smaller indexes than children */
    /* this checks for loops too */
    assert(g_map_groups[i].parent < i);
  }
}

BOOL zynadd_dynparam_init(struct zynadd * zynadd_ptr)
{
  int i;
  BOOL tmp_bool;

  if (!lv2dynparam_plugin_instantiate(
        (LV2_Handle)zynadd_ptr,
        "zynadd",
        &zynadd_ptr->dynparams))
  {
    goto fail;
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    LOG_DEBUG("Adding group \"%s\"", g_map_groups[i].name);

    if (!lv2dynparam_plugin_group_add(
          zynadd_ptr->dynparams,
          g_map_groups[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_groups[i].parent],
          g_map_groups[i].name,
          &g_map_groups[i].hints,
          zynadd_ptr->groups + i))
    {
      goto fail_clean_dynparams;
    }
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    zynadd_ptr->parameters[i].synth_ptr = zynadd_ptr;
    zynadd_ptr->parameters[i].addsynth_parameter = g_map_parameters[i].addsynth_parameter;
    zynadd_ptr->parameters[i].addsynth_component = g_map_parameters[i].addsynth_component;
    zynadd_ptr->parameters[i].scope = g_map_parameters[i].scope;
    zynadd_ptr->parameters[i].scope_specific = g_map_parameters[i].scope_specific;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("Adding parameter \"%s\"", g_map_parameters[i].name);

    if (g_map_parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SEMI)
    {
      continue;
    }

    if (g_map_parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER ||
        g_map_parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER)
    {
      LOG_DEBUG("Apearing show/hide parameter \"%s\"", g_map_parameters[i].name);
      assert(g_map_parameters[i].type == LV2DYNPARAM_PARAMETER_TYPE_BOOL);

      tmp_bool = zyn_addsynth_get_bool_parameter(
        zynadd_ptr->synth,
        g_map_parameters[i].addsynth_component,
        g_map_parameters[i].addsynth_parameter);

      if (!zynadd_appear_parameter(zynadd_ptr, i))
      {
        goto fail_clean_dynparams;
      }

      if ((g_map_parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER && !tmp_bool) ||
          (g_map_parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER && tmp_bool))
      {
        LOG_DEBUG("Apearing semi parameter %u", g_map_parameters[i].scope_specific);
        LOG_DEBUG("Apearing semi parameter \"%s\"", g_map_parameters[g_map_parameters[i].scope_specific].name);
        if (!zynadd_appear_parameter(zynadd_ptr, g_map_parameters[i].scope_specific))
        {
          goto fail_clean_dynparams;
        }
      }

      continue;
    }

    assert(g_map_parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS);

    if (!zynadd_appear_parameter(zynadd_ptr, i))
    {
      LOG_ERROR("zynadd_appear_parameter() failed.");
      goto fail_clean_dynparams;
    }
  }

  return TRUE;

fail_clean_dynparams:
  lv2dynparam_plugin_cleanup(zynadd_ptr->dynparams);

fail:
  return FALSE;
}
