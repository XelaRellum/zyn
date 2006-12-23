/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006 Nedko Arnaudov <nedko@arnaudov.name>
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

#include "common.h"
#include "lv2.h"
#include "addsynth.h"
#include "dynparam.h"
#include "zynadd_internal.h"
#include "lv2dynparam.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

struct group_descriptor
{
  int parent;                   /* index of parent, LV2DYNPARAM_GROUP_ROOT for root children */
  const char * name;            /* group name */
  const char * type_uri;        /* group type */
};

struct parameter_descriptor
{
  int parent;                   /* index of parent, LV2DYNPARAM_GROUP_ROOT for root children */
  const char * name;            /* parameter name */

  unsigned int type;            /* one of LV2DYNPARAM_PARAMETER_TYPE_XXX */

  BOOL placeholder;

  union
  {
    lv2dynparam_plugin_param_boolean_changed boolean;
    lv2dynparam_plugin_param_float_changed fpoint;
  } value_changed_callback;

  union
  {
    BOOL (* boolean)(zyn_addsynth_handle handle);
    float (* fpoint)(zyn_addsynth_handle handle);
  } get_value;

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

#define zynadd_ptr ((struct zynadd *)context)

#define IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(ident)      \
  BOOL                                                    \
  zynadd_ ## ident ## _changed(                           \
    void * context,                                       \
    BOOL value)                                           \
  {                                                       \
    LOG_DEBUG("zynadd_" #ident "_changed() called.");     \
    zyn_addsynth_set_ ## ident(zynadd_ptr->synth, value); \
    return TRUE;                                          \
  }

#define IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(ident)     \
  BOOL                                                    \
  zynadd_ ## ident ## _changed(                           \
    void * context,                                       \
    float value)                                          \
  {                                                       \
    LOG_DEBUG("zynadd_" #ident "_changed() called.");     \
    zyn_addsynth_set_ ## ident(zynadd_ptr->synth, value); \
    return TRUE;                                          \
  }

BOOL
zynadd_pan_random_changed(
  void * context,
  BOOL value)
{
  if ((zyn_addsynth_is_pan_random(zynadd_ptr->synth) && value) ||
      (!zyn_addsynth_is_pan_random(zynadd_ptr->synth) && !value))
  {
    /* value not changed */
    return TRUE;
  }

  if (value)
  {
    /* enabling randomize -> remove panorama parameter */
    if (!lv2dynparam_plugin_param_remove(
          zynadd_ptr->dynparams,
          zynadd_ptr->parameters[LV2DYNPARAM_PARAMETER_PANORAMA]))
    {
      return FALSE;
    }
  }
  else
  {
    /* enabling randomize -> add panorama parameter */
    if (!lv2dynparam_plugin_param_float_add(
          zynadd_ptr->dynparams,
          g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].parent == LV2DYNPARAM_GROUP_ROOT ?
          NULL :
          zynadd_ptr->groups[g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].parent],
          g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].name,
          g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].get_value.fpoint(zynadd_ptr->synth),
          g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].min.fpoint,
          g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].max.fpoint,
          g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].value_changed_callback.fpoint,
          zynadd_ptr,
          zynadd_ptr->parameters + LV2DYNPARAM_PARAMETER_PANORAMA))
    {
      return FALSE;
    }
  }

  zyn_addsynth_set_pan_random(zynadd_ptr->synth, value);

  return TRUE;
}

IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(stereo)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(panorama)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(volume)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(velocity_sensing)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_strength)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_time)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_stretch)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_velocity_sensing)
IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(random_grouping)

#undef zynadd_ptr

#define LV2DYNPARAM_GROUP(group) LV2DYNPARAM_GROUP_ ## group
#define LV2DYNPARAM_PARAMETER(parameter) LV2DYNPARAM_PARAMETER_ ## parameter

#define LV2DYNPARAM_GROUP_INIT_GENERIC(parent_group, group, name_value) \
  g_map_groups[LV2DYNPARAM_GROUP(group)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_groups[LV2DYNPARAM_GROUP(group)].name = name_value;             \
  g_map_groups[LV2DYNPARAM_GROUP(group)].type_uri = LV2DYNPARAM_GROUP_TYPE_GENERIC_URI

#define LV2DYNPARAM_GROUP_INIT_CUSTOM(parent_group, group, name_value, type) \
  g_map_groups[LV2DYNPARAM_GROUP(group)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_groups[LV2DYNPARAM_GROUP(group)].name = name_value;             \
  g_map_groups[LV2DYNPARAM_GROUP(group)].type_uri = type

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, parameter, name_value, ident, placeholder_value) \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].placeholder = placeholder_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].value_changed_callback.boolean = zynadd_ ## ident ## _changed; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].get_value.boolean = zyn_addsynth_is_ ## ident;

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, parameter, name_value, ident, min_value, max_value, placeholder_value) \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].placeholder = placeholder_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].value_changed_callback.fpoint = zynadd_ ## ident ## _changed; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].get_value.fpoint = zyn_addsynth_get_ ## ident; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].min.fpoint = min_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].max.fpoint = max_value;

void zynadd_map_initialise() __attribute__((constructor));
void zynadd_map_initialise()
{
  int i;

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    g_map_groups[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    g_map_parameters[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  LV2DYNPARAM_GROUP_INIT_GENERIC(ROOT, AMPLITUDE, "Amplitude");
  {
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, STEREO, "Stereo", stereo, FALSE);
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, RANDOM_GROUPING, "Random Grouping", random_grouping, FALSE);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, MASTER_VOLUME, "Master Volume", volume, 0, 100, FALSE);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, VELOCITY_SENSING, "Velocity sensing", velocity_sensing, 0, 100, FALSE);

    LV2DYNPARAM_GROUP_INIT_CUSTOM(AMPLITUDE, AMPLITUDE_PANORAMA, "Panorama", LV2DYNPARAM_GROUP_TYPE_TOGGLE_FLOAT_URI);
    {
      LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE_PANORAMA, PAN_RANDOMIZE, "Randomize", pan_random, TRUE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PANORAMA, PANORAMA, "Panorama", panorama, -1, 1, TRUE);
    }

    LV2DYNPARAM_GROUP_INIT_GENERIC(AMPLITUDE, AMPLITUDE_PUNCH, "Punch");
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_STRENGTH, "Strength", punch_strength, 0, 100, FALSE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_TIME, "Time", punch_time, 0, 100, FALSE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_STRETCH, "Stretch", punch_stretch, 0, 100, FALSE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_VELOCITY_SENSING, "Velocity sensing", punch_velocity_sensing, 0, 100, FALSE);
    }

    LV2DYNPARAM_GROUP_INIT_GENERIC(AMPLITUDE, AMPLITUDE_ENVELOPE, "Envelope");
    {
    }

    LV2DYNPARAM_GROUP_INIT_GENERIC(AMPLITUDE, AMPLITUDE_LFO, "LFO");
    {
    }
  }

  LV2DYNPARAM_GROUP_INIT_GENERIC(ROOT, FILTER, "Filter");
  {
    LV2DYNPARAM_GROUP_INIT_GENERIC(FILTER, FILTER_ENVELOPE, "Envelope");
    {
    }

    LV2DYNPARAM_GROUP_INIT_GENERIC(FILTER, FILTER_LFO, "LFO");
    {
    }
  }

  LV2DYNPARAM_GROUP_INIT_GENERIC(ROOT, FREQUENCY, "Frequency");
  {
    LV2DYNPARAM_GROUP_INIT_GENERIC(FREQUENCY, FREQUENCY_ENVELOPE, "Envelope");
    {
    }

    LV2DYNPARAM_GROUP_INIT_GENERIC(FREQUENCY, FREQUENCY_LFO, "LFO");
    {
    }
  }

  /* santity check that we have filled all values */

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("parameter %d with parent %d", i, g_map_groups[i].parent);
    assert(g_map_parameters[i].parent != LV2DYNPARAM_GROUP_INVALID);
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
          g_map_groups[i].type_uri,
          zynadd_ptr->groups + i))
    {
      goto fail_clean_dynparams;
    }
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("Adding parameter \"%s\"", g_map_parameters[i].name);

    if (g_map_parameters[i].placeholder)
    {
      /* we handle these manually */
      switch (i)
      {
      case LV2DYNPARAM_PARAMETER_PAN_RANDOMIZE:
        tmp_bool = g_map_parameters[i].get_value.boolean(zynadd_ptr->synth);
        if (!lv2dynparam_plugin_param_boolean_add(
              zynadd_ptr->dynparams,
              g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
              g_map_parameters[i].name,
              tmp_bool,
              g_map_parameters[i].value_changed_callback.boolean,
              zynadd_ptr,
              zynadd_ptr->parameters + i))
        {
          goto fail_clean_dynparams;
        }

        if (!tmp_bool)
        {
          if (!lv2dynparam_plugin_param_float_add(
                zynadd_ptr->dynparams,
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].name,
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].get_value.fpoint(zynadd_ptr->synth),
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].min.fpoint,
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].max.fpoint,
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].value_changed_callback.fpoint,
                zynadd_ptr,
                zynadd_ptr->parameters + LV2DYNPARAM_PARAMETER_PANORAMA))
          {
            goto fail_clean_dynparams;
          }
        }

        continue;
      case LV2DYNPARAM_PARAMETER_PANORAMA:
        continue;
      default:
        assert(0);
      }
    }

    switch (g_map_parameters[i].type)
    {
    case LV2DYNPARAM_PARAMETER_TYPE_BOOL:
      if (!lv2dynparam_plugin_param_boolean_add(
            zynadd_ptr->dynparams,
            g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
            g_map_parameters[i].name,
            g_map_parameters[i].get_value.boolean(zynadd_ptr->synth),
            g_map_parameters[i].value_changed_callback.boolean,
            zynadd_ptr,
            zynadd_ptr->parameters + i))
      {
        goto fail_clean_dynparams;
      }
      break;

    case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
      if (!lv2dynparam_plugin_param_float_add(
            zynadd_ptr->dynparams,
            g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
            g_map_parameters[i].name,
            g_map_parameters[i].get_value.fpoint(zynadd_ptr->synth),
            g_map_parameters[i].min.fpoint,
            g_map_parameters[i].max.fpoint,
            g_map_parameters[i].value_changed_callback.fpoint,
            zynadd_ptr,
            zynadd_ptr->parameters + i))
      {
        goto fail_clean_dynparams;
      }
      break;
    default:
      assert(0);
    }
  }

  return TRUE;

fail_clean_dynparams:
  lv2dynparam_plugin_cleanup(zynadd_ptr->dynparams);

fail:
  return FALSE;
}
