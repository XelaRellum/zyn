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

#ifndef ZYNADD_DYNPARAM_FOREST_MAP_H__3ED30D71_513B_4B84_9A9B_1C9C62EE0A87__INCLUDED
#define ZYNADD_DYNPARAM_FOREST_MAP_H__3ED30D71_513B_4B84_9A9B_1C9C62EE0A87__INCLUDED

extern const char * g_shape_names[];
extern const char * g_analog_filter_type_names[];
extern const char * g_filter_type_names[];
extern const char * g_oscillator_base_function_names[];
extern const char * g_oscillator_waveshape_type_names[];
extern const char * g_oscillator_spectrum_adjust_type_names[];

#define ZYN_MAX_HINTS 10

#define LV2DYNPARAM_GROUP(group) LV2DYNPARAM_GROUP_ ## group
#define LV2DYNPARAM_PARAMETER(parameter) LV2DYNPARAM_PARAMETER_ ## parameter

#define LV2DYNPARAM_GROUP_INIT(parent_group, group, name_value, hints...) \
  groups_map[LV2DYNPARAM_GROUP(group) + 2] = group_index;               \
  lv2dynparam_group_init(map_ptr, groups_map[LV2DYNPARAM_GROUP(parent_group) + 2], group_index, name_value, ## hints); \
  group_index++;

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u",                       \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter);                    \
  params_map[LV2DYNPARAM_PARAMETER(lv2parameter)] = param_index;        \
  map_ptr->parameters[param_index].parent = groups_map[LV2DYNPARAM_GROUP(parent_group) + 2]; \
  map_ptr->parameters[param_index].name = name_value;                   \
  map_ptr->parameters[param_index].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  map_ptr->parameters[param_index].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[param_index].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[param_index].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter; \
  param_index++;

#define LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, other_parameter, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u; with other %u",        \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter,                     \
            LV2DYNPARAM_PARAMETER(other_parameter));                    \
  params_map[LV2DYNPARAM_PARAMETER(lv2parameter)] = param_index;        \
  map_ptr->parameters[param_index].parent = groups_map[LV2DYNPARAM_GROUP(parent_group) + 2]; \
  map_ptr->parameters[param_index].name = name_value;                   \
  map_ptr->parameters[param_index].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  map_ptr->parameters[param_index].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value ## _OTHER; \
  map_ptr->parameters[param_index].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[param_index].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter; \
  map_ptr->parameters[param_index].scope_specific = LV2DYNPARAM_PARAMETER(other_parameter); \
  param_index++;

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") float -> %u:%u",                   \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_FLOAT_ ## zynparameter);                   \
  params_map[LV2DYNPARAM_PARAMETER(lv2parameter)] = param_index;        \
  map_ptr->parameters[param_index].parent = groups_map[LV2DYNPARAM_GROUP(parent_group) + 2]; \
  map_ptr->parameters[param_index].name = name_value;                   \
  map_ptr->parameters[param_index].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  map_ptr->parameters[param_index].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[param_index].min.fpoint = min_value;              \
  map_ptr->parameters[param_index].max.fpoint = max_value;              \
  map_ptr->parameters[param_index].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[param_index].addsynth_parameter = ZYNADD_PARAMETER_FLOAT_ ## zynparameter; \
  param_index++;

#define LV2DYNPARAM_PARAMETER_INIT_INT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") int -> %u:%u",                     \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_INT_ ## zynparameter);                     \
  params_map[LV2DYNPARAM_PARAMETER(lv2parameter)] = param_index;        \
  map_ptr->parameters[param_index].parent = groups_map[LV2DYNPARAM_GROUP(parent_group) + 2]; \
  map_ptr->parameters[param_index].name = name_value;                   \
  map_ptr->parameters[param_index].type = LV2DYNPARAM_PARAMETER_TYPE_INT; \
  map_ptr->parameters[param_index].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[param_index].min.integer = min_value;             \
  map_ptr->parameters[param_index].max.integer = max_value;             \
  map_ptr->parameters[param_index].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[param_index].addsynth_parameter = ZYNADD_PARAMETER_INT_ ## zynparameter; \
  param_index++;

#define LV2DYNPARAM_PARAMETER_INIT_ENUM(parent_group, lv2parameter, component, zynparameter, name_value, values, count, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") enum -> %u",                       \
            LV2DYNPARAM_PARAMETER(lv2parameter),                  \
            name_value);                                                \
  params_map[LV2DYNPARAM_PARAMETER(lv2parameter)] = param_index;  \
  map_ptr->parameters[param_index].parent = groups_map[LV2DYNPARAM_GROUP(parent_group) + 2]; \
  map_ptr->parameters[param_index].name = name_value;                   \
  map_ptr->parameters[param_index].type = LV2DYNPARAM_PARAMETER_TYPE_ENUM; \
  map_ptr->parameters[param_index].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[param_index].min.enum_values = values;            \
  map_ptr->parameters[param_index].max.enum_values_count = count;       \
  map_ptr->parameters[param_index].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[param_index].addsynth_parameter = ZYNADD_PARAMETER_ENUM_ ## zynparameter; \
  param_index++;

#define LV2DYNPARAM_FOREST_MAP_BEGIN(_groups_count, _params_count, _groups, _params) \
  int i;                                                                \
  int group_index;                                                      \
  int param_index;                                                      \
  int groups_map[_groups_count + 2];                                    \
  int params_map[_params_count];                                        \
                                                                        \
  map_ptr->groups_count = _groups_count;                                \
  map_ptr->parameters_count = _params_count;                            \
                                                                        \
  map_ptr->groups = _groups;                                            \
  map_ptr->parameters = _params;                                        \
                                                                        \
  groups_map[0] = LV2DYNPARAM_GROUP_INVALID;                            \
  groups_map[1] = LV2DYNPARAM_GROUP_ROOT;                               \
  group_index = 0;                                                      \
  param_index = 0;                                                      \
                                                                        \
  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)                      \
  {                                                                     \
    map_ptr->groups[i].parent = LV2DYNPARAM_GROUP_INVALID;              \
    groups_map[i + 2] = LV2DYNPARAM_GROUP_INVALID;                      \
  }                                                                     \
                                                                        \
  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)                  \
  {                                                                     \
    map_ptr->parameters[i].parent = LV2DYNPARAM_GROUP_INVALID;          \
    params_map[i] = -1;                                                 \
  }

#define LV2DYNPARAM_FOREST_MAP_END                                      \
  /* updated scope_specific when needed */                              \
  for (i = 0 ; i < map_ptr->parameters_count ; i++)                     \
  {                                                                     \
    if (map_ptr->parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER || \
        map_ptr->parameters[i].scope == LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER) \
    {                                                                   \
      map_ptr->parameters[i].scope_specific = params_map[map_ptr->parameters[i].scope_specific]; \
    }                                                                   \
  }                                                                     \
                                                                        \
  LV2DYNPARAM_FOREST_MAP_ASSERT_VALID;

#define LV2DYNPARAM_FOREST_MAP_ASSERT_VALID                             \
  /* santity check that we have filled all values */                    \
                                                                        \
  assert(group_index == LV2DYNPARAM_GROUPS_COUNT);                      \
                                                                        \
  assert(param_index == LV2DYNPARAM_PARAMETERS_COUNT);                  \
                                                                        \
  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)                  \
  {                                                                     \
    LOG_DEBUG("parameter %d with parent %d", i, map_ptr->parameters[i].parent); \
    assert(map_ptr->parameters[i].parent != LV2DYNPARAM_GROUP_INVALID); \
    assert(map_ptr->parameters[i].parent < LV2DYNPARAM_GROUPS_COUNT);   \
  }                                                                     \
                                                                        \
  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)                      \
  {                                                                     \
    LOG_DEBUG("group %d with parent %d", i, map_ptr->groups[i].parent); \
    assert(map_ptr->groups[i].parent != LV2DYNPARAM_GROUP_INVALID);     \
                                                                        \
    assert(map_ptr->groups[i].name != NULL);                            \
                                                                        \
    /* check that parents are with smaller indexes than children */     \
    /* this checks for loops too */                                     \
    assert(map_ptr->groups[i].parent < i);                              \
  }

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
    signed int integer;
    const char ** enum_values;
  } min;

  union
  {
    float fpoint;
    signed int integer;
    unsigned int enum_values_count;
  } max;
};

struct zyn_forest_map
{
  size_t groups_count;
  size_t parameters_count;

  struct group_descriptor * groups;
  struct parameter_descriptor * parameters;
};

void
lv2dynparam_group_init(
  struct zyn_forest_map * map_ptr,
  unsigned int parent,
  unsigned int group,
  const char * name,
  ...);

#endif /* #ifndef ZYNADD_DYNPARAM_FOREST_MAP_H__3ED30D71_513B_4B84_9A9B_1C9C62EE0A87__INCLUDED */
