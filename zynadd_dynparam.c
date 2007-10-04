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
#include "zynadd_dynparam_value_changed_callbacks.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

bool
zynadd_appear_parameter(
  struct zynadd * zynadd_ptr,
  struct zynadd_parameter * parameter_ptr)
{
  lv2dynparam_plugin_group parent_group;

  LOG_DEBUG(
    "Appearing parameter \"%s\" -> %u",
    parameter_ptr->name_ptr,
    parameter_ptr->addsynth_parameter);

  if (parameter_ptr->parent_ptr == NULL)
  {
    parent_group = NULL;
  }
  else
  {
    parent_group = parameter_ptr->parent_ptr->lv2group;
  }

  switch (parameter_ptr->type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_BOOL:
    if (!lv2dynparam_plugin_param_boolean_add(
          zynadd_ptr->dynparams,
          parent_group,
          parameter_ptr->name_ptr,
          parameter_ptr->hints_ptr,
          zyn_addsynth_get_bool_parameter(
            zynadd_ptr->synth,
            parameter_ptr->addsynth_component,
            parameter_ptr->addsynth_parameter),
          zynadd_bool_parameter_changed,
          parameter_ptr,
          &parameter_ptr->lv2parameter))
    {
      return false;
    }

    return true;

  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    if (!lv2dynparam_plugin_param_float_add(
          zynadd_ptr->dynparams,
          parent_group,
          parameter_ptr->name_ptr,
          parameter_ptr->hints_ptr,
          zyn_addsynth_get_float_parameter(
            zynadd_ptr->synth,
            parameter_ptr->addsynth_component,
            parameter_ptr->addsynth_parameter),
          parameter_ptr->map_element_ptr->min.fpoint,
          parameter_ptr->map_element_ptr->max.fpoint,
          zynadd_float_parameter_changed,
          parameter_ptr,
          &parameter_ptr->lv2parameter))
    {
      return false;
    }

    return true;

  case LV2DYNPARAM_PARAMETER_TYPE_INT:
    if (!lv2dynparam_plugin_param_int_add(
          zynadd_ptr->dynparams,
          parent_group,
          parameter_ptr->name_ptr,
          parameter_ptr->hints_ptr,
          zyn_addsynth_get_int_parameter(
            zynadd_ptr->synth,
            parameter_ptr->addsynth_component,
            parameter_ptr->addsynth_parameter),
          parameter_ptr->map_element_ptr->min.integer,
          parameter_ptr->map_element_ptr->max.integer,
          zynadd_int_parameter_changed,
          parameter_ptr,
          &parameter_ptr->lv2parameter))
    {
      return false;
    }

    return true;

  case LV2DYNPARAM_PARAMETER_TYPE_SHAPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          parameter_ptr->name_ptr,
          parameter_ptr->hints_ptr,
          g_shape_names,
          ZYN_LFO_SHAPES_COUNT,
          zyn_addsynth_get_shape_parameter(zynadd_ptr->synth, parameter_ptr->addsynth_component),
          zynadd_shape_parameter_changed,
          parameter_ptr,
          &parameter_ptr->lv2parameter))
    {
      return false;
    }

    return true;
  case LV2DYNPARAM_PARAMETER_TYPE_FILTER_TYPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          parameter_ptr->name_ptr,
          parameter_ptr->hints_ptr,
          g_filter_type_names,
          ZYN_FILTER_TYPES_COUNT,
          zyn_addsynth_get_filter_type_parameter(zynadd_ptr->synth, parameter_ptr->addsynth_component),
          zynadd_filter_type_parameter_changed,
          parameter_ptr,
          &parameter_ptr->lv2parameter))
    {
      return false;
    }

    return true;
  case LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE:
    if (!lv2dynparam_plugin_param_enum_add(
          zynadd_ptr->dynparams,
          parent_group,
          parameter_ptr->name_ptr,
          parameter_ptr->hints_ptr,
          g_analog_filter_type_names,
          ZYN_FILTER_ANALOG_TYPES_COUNT,
          zyn_addsynth_get_analog_filter_type_parameter(zynadd_ptr->synth, parameter_ptr->addsynth_component),
          zynadd_analog_filter_type_parameter_changed,
          parameter_ptr,
          &parameter_ptr->lv2parameter))
    {
      return false;
    }

    return true;
  }

  assert(0);
  return false;
}

/* create forest groups and parameters without exposing them */
bool
zynadd_dynparam_forest_prepare(
  struct zyn_forest_initializer * forest_ptr,
  struct zynadd * zynadd_ptr,
  struct list_head * groups_list_ptr,
  struct list_head * parameters_list_ptr)
{
  int i;
  struct zynadd_group * group_ptr;
  struct zynadd_parameter * parameter_ptr;

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    LOG_DEBUG("Preparing group \"%s\"", forest_ptr->map_ptr->groups[i].name);

    group_ptr = malloc(sizeof(struct zynadd_group));
    if (group_ptr == NULL)
    {
      return false;
    }

    group_ptr->name_ptr = forest_ptr->map_ptr->groups[i].name;
    group_ptr->hints_ptr = &forest_ptr->map_ptr->groups[i].hints;
    group_ptr->lv2group = NULL;

    if (forest_ptr->map_ptr->groups[i].parent == LV2DYNPARAM_GROUP_ROOT)
    {
      group_ptr->parent_ptr = NULL;
    }
    else
    {
      group_ptr->parent_ptr = forest_ptr->groups[forest_ptr->map_ptr->groups[i].parent];
    }

    forest_ptr->groups[i] = group_ptr;

    list_add_tail(&group_ptr->siblings, groups_list_ptr);
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("Preparing group \"%s\"", forest_ptr->map_ptr->parameters[i].name);

    parameter_ptr = malloc(sizeof(struct zynadd_parameter));
    if (parameter_ptr == NULL)
    {
      return false;
    }

    if (forest_ptr->map_ptr->parameters[i].parent == LV2DYNPARAM_GROUP_ROOT)
    {
      parameter_ptr->parent_ptr = NULL;
    }
    else
    {
      parameter_ptr->parent_ptr = forest_ptr->groups[forest_ptr->map_ptr->parameters[i].parent];
    }

    forest_ptr->parameters[i] = parameter_ptr;

    parameter_ptr->synth_ptr = zynadd_ptr;
    parameter_ptr->addsynth_parameter = forest_ptr->map_ptr->parameters[i].addsynth_parameter;
    parameter_ptr->addsynth_component = forest_ptr->map_ptr->parameters[i].addsynth_component;
    parameter_ptr->scope = forest_ptr->map_ptr->parameters[i].scope;
    parameter_ptr->other_parameter = NULL;
    parameter_ptr->lv2parameter = NULL;
    parameter_ptr->name_ptr = forest_ptr->map_ptr->parameters[i].name;
    parameter_ptr->type = forest_ptr->map_ptr->parameters[i].type;
    parameter_ptr->hints_ptr = &forest_ptr->map_ptr->parameters[i].hints;
    parameter_ptr->map_element_ptr = forest_ptr->map_ptr->parameters + i;

    list_add_tail(&parameter_ptr->siblings, parameters_list_ptr);
  }

  /* set other_parameter when needed */
  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    if (forest_ptr->map_ptr->parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER ||
        forest_ptr->map_ptr->parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER)
    {
      forest_ptr->parameters[i]->other_parameter = forest_ptr->parameters[forest_ptr->map_ptr->parameters[i].scope_specific];
    }
  }

  return true;
}

void
zynadd_dynparam_destroy_forests(
  struct zynadd * zynadd_ptr)
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
}

bool
zynadd_dynparam_init(
  struct zynadd * zynadd_ptr)
{
  bool tmp_bool;
  struct zynadd_group * group_ptr;
  struct zynadd_parameter * parameter_ptr;
  struct zyn_forest_initializer top_forest_initializer;
  struct list_head * node_ptr;

  top_forest_initializer.map_ptr = &g_top_forest_map;

  INIT_LIST_HEAD(&zynadd_ptr->groups);
  INIT_LIST_HEAD(&zynadd_ptr->parameters);

  if (!zynadd_dynparam_forest_prepare(
        &top_forest_initializer,
        zynadd_ptr,
        &zynadd_ptr->groups,
        &zynadd_ptr->parameters))
  {
    goto fail_destroy_forests;
  }

  if (!lv2dynparam_plugin_instantiate(
        (LV2_Handle)zynadd_ptr,
        "zynadd",
        &zynadd_ptr->dynparams))
  {
    goto fail_destroy_forests;
  }

  list_for_each(node_ptr, &zynadd_ptr->groups)
  {
    group_ptr = list_entry(node_ptr, struct zynadd_group, siblings);

    LOG_DEBUG("Adding group \"%s\"", group_ptr->name_ptr);

    if (!lv2dynparam_plugin_group_add(
          zynadd_ptr->dynparams,
          group_ptr->parent_ptr == NULL ? NULL : group_ptr->parent_ptr->lv2group,
          group_ptr->name_ptr,
          group_ptr->hints_ptr,
          &group_ptr->lv2group))
    {
      goto fail_clean_dynparams;
    }
  }

  list_for_each(node_ptr, &zynadd_ptr->parameters)
  {
    parameter_ptr = list_entry(node_ptr, struct zynadd_parameter, siblings);

    LOG_DEBUG("Adding parameter \"%s\"", parameter_ptr->name_ptr);

    if (parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SEMI)
    {
      continue;
    }

    if (parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER ||
        parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER)
    {
      LOG_DEBUG("Apearing show/hide parameter \"%s\"", parameter_ptr->name_ptr);
      assert(parameter_ptr->type == LV2DYNPARAM_PARAMETER_TYPE_BOOL);

      tmp_bool = zyn_addsynth_get_bool_parameter(
        zynadd_ptr->synth,
        parameter_ptr->addsynth_component,
        parameter_ptr->addsynth_parameter);

      if (!zynadd_appear_parameter(zynadd_ptr, parameter_ptr))
      {
        goto fail_clean_dynparams;
      }

      if ((parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER && !tmp_bool) ||
          (parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER && tmp_bool))
      {
        LOG_DEBUG("Apearing semi parameter \"%s\"", parameter_ptr->other_parameter->name_ptr);
        if (!zynadd_appear_parameter(zynadd_ptr, parameter_ptr->other_parameter))
        {
          goto fail_clean_dynparams;
        }
      }

      continue;
    }

    assert(parameter_ptr->scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS);

    if (!zynadd_appear_parameter(zynadd_ptr, parameter_ptr))
    {
      LOG_ERROR("zynadd_appear_parameter() failed.");
      goto fail_clean_dynparams;
    }
  }

  return true;

fail_clean_dynparams:
  zynadd_dynparam_uninit(zynadd_ptr);

fail_destroy_forests:
  zynadd_dynparam_destroy_forests(zynadd_ptr);

  return false;
}

void
zynadd_dynparam_uninit(struct zynadd * zynadd_ptr)
{
  zynadd_dynparam_destroy_forests(zynadd_ptr);

  lv2dynparam_plugin_cleanup(zynadd_ptr->dynparams);
}
