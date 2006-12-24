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

  unsigned int addsynth_parameter; /* one of ZYNADD_PARAMETER_XXX */

  BOOL placeholder;

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

BOOL
zynadd_float_parameter_changed(
  void * context,
  float value);

#define parameter_ptr ((struct zynadd_parameter *)context)

BOOL
zynadd_bool_parameter_changed(
  void * context,
  BOOL value)
{
  BOOL current_value;

  switch (parameter_ptr->addsynth_parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
    current_value = zyn_addsynth_get_bool_parameter(
      parameter_ptr->synth_ptr->synth,
      ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA);

    if ((current_value && value) ||
        (!current_value && !value))
    {
      /* value not changed */
      return TRUE;
    }

    if (value)
    {
      /* enabling randomize -> remove panorama parameter */
      if (!lv2dynparam_plugin_param_remove(
            parameter_ptr->synth_ptr->dynparams,
            parameter_ptr->synth_ptr->parameters[LV2DYNPARAM_PARAMETER_PANORAMA].lv2parameter))
      {
        return FALSE;
      }
    }
    else
    {
      /* enabling randomize -> add panorama parameter */
      if (!lv2dynparam_plugin_param_float_add(
            parameter_ptr->synth_ptr->dynparams,
            g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].parent == LV2DYNPARAM_GROUP_ROOT ?
            NULL :
            parameter_ptr->synth_ptr->groups[g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].parent],
            g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].name,
            zyn_addsynth_get_float_parameter(parameter_ptr->synth_ptr->synth, ZYNADD_PARAMETER_FLOAT_PANORAMA),
            g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].min.fpoint,
            g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].max.fpoint,
            zynadd_float_parameter_changed,
            parameter_ptr->synth_ptr->parameters + LV2DYNPARAM_PARAMETER_PANORAMA,
            &parameter_ptr->synth_ptr->parameters[LV2DYNPARAM_PARAMETER_PANORAMA].lv2parameter))
      {
        return FALSE;
      }
    }
    break;
  }

  zyn_addsynth_set_bool_parameter(
    parameter_ptr->synth_ptr->synth,
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
    parameter_ptr->addsynth_parameter,
    value);

  return TRUE;
}

#undef parameter_ptr

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

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, parameter, name_value, placeholder_value) \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].placeholder = placeholder_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## parameter;

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, parameter, name_value, min_value, max_value, placeholder_value) \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].placeholder = placeholder_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].min.fpoint = min_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].max.fpoint = max_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].addsynth_parameter = ZYNADD_PARAMETER_FLOAT_ ## parameter;

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
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, STEREO, "Stereo", FALSE);
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, RANDOM_GROUPING, "Random Grouping", FALSE);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, VOLUME, "Master Volume", 0, 100, FALSE);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, VELOCITY_SENSING, "Velocity sensing", 0, 100, FALSE);

    LV2DYNPARAM_GROUP_INIT_CUSTOM(AMPLITUDE, AMPLITUDE_PANORAMA, "Panorama", LV2DYNPARAM_GROUP_TYPE_TOGGLE_FLOAT_URI);
    {
      LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE_PANORAMA, RANDOM_PANORAMA, "Randomize", TRUE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PANORAMA, PANORAMA, "Panorama", -1, 1, TRUE);
    }

    LV2DYNPARAM_GROUP_INIT_GENERIC(AMPLITUDE, AMPLITUDE_PUNCH, "Punch");
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_STRENGTH, "Strength", 0, 100, FALSE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_TIME, "Time", 0, 100, FALSE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_STRETCH, "Stretch", 0, 100, FALSE);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_VELOCITY_SENSING, "Velocity sensing", 0, 100, FALSE);
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

    zynadd_ptr->parameters[i].synth_ptr = zynadd_ptr;
    zynadd_ptr->parameters[i].addsynth_parameter = g_map_parameters[i].addsynth_parameter;

    if (g_map_parameters[i].placeholder)
    {
      /* we handle these manually */
      switch (i)
      {
      case LV2DYNPARAM_PARAMETER_RANDOM_PANORAMA:
        tmp_bool = zyn_addsynth_get_bool_parameter(
          zynadd_ptr->synth,
          ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA);

        if (!lv2dynparam_plugin_param_boolean_add(
              zynadd_ptr->dynparams,
              g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
              g_map_parameters[i].name,
              tmp_bool,
              zynadd_bool_parameter_changed,
              zynadd_ptr->parameters + i,
              &zynadd_ptr->parameters[i].lv2parameter))
        {
          goto fail_clean_dynparams;
        }

        if (!tmp_bool)
        {
          if (!lv2dynparam_plugin_param_float_add(
                zynadd_ptr->dynparams,
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].name,
                zyn_addsynth_get_float_parameter(zynadd_ptr->synth, ZYNADD_PARAMETER_FLOAT_PANORAMA),
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].min.fpoint,
                g_map_parameters[LV2DYNPARAM_PARAMETER_PANORAMA].max.fpoint,
                zynadd_float_parameter_changed,
                zynadd_ptr->parameters + LV2DYNPARAM_PARAMETER_PANORAMA,
                &zynadd_ptr->parameters[LV2DYNPARAM_PARAMETER_PANORAMA].lv2parameter))
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
            zyn_addsynth_get_bool_parameter(zynadd_ptr->synth, zynadd_ptr->parameters[i].addsynth_parameter),
            zynadd_bool_parameter_changed,
            zynadd_ptr->parameters + i,
            &zynadd_ptr->parameters[i].lv2parameter))
      {
        goto fail_clean_dynparams;
      }
      break;

    case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
      if (!lv2dynparam_plugin_param_float_add(
            zynadd_ptr->dynparams,
            g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
            g_map_parameters[i].name,
            zyn_addsynth_get_float_parameter(zynadd_ptr->synth, zynadd_ptr->parameters[i].addsynth_parameter),
            g_map_parameters[i].min.fpoint,
            g_map_parameters[i].max.fpoint,
            zynadd_float_parameter_changed,
            zynadd_ptr->parameters + i,
            &zynadd_ptr->parameters[i].lv2parameter))
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
