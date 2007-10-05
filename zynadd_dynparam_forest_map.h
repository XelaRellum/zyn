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

#define ZYN_MAX_HINTS 10

#define LV2DYNPARAM_GROUP(group) LV2DYNPARAM_GROUP_ ## group
#define LV2DYNPARAM_PARAMETER(parameter) LV2DYNPARAM_PARAMETER_ ## parameter
#define LV2DYNPARAM_PARAMETER_SHAPE(parameter) LV2DYNPARAM_PARAMETER_ ## parameter ## _SHAPE

#define LV2DYNPARAM_GROUP_INIT(parent_group, group, name_value, hints...) \
  lv2dynparam_group_init(map_ptr, LV2DYNPARAM_GROUP(parent_group), LV2DYNPARAM_GROUP(group), name_value, ## hints)

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u",                       \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter);                    \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(parent_group, lv2parameter, component, zynparameter, name_value, scope_value, other_parameter, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") bool -> %u; with other %u",        \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_PARAMETER_BOOL_ ## zynparameter,                     \
            LV2DYNPARAM_PARAMETER(other_parameter));                    \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value ## _OTHER; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_BOOL_ ## zynparameter; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope_specific = LV2DYNPARAM_PARAMETER(other_parameter);

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") float -> %u:%u",                   \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_FLOAT_ ## zynparameter);                   \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].min.fpoint = min_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].max.fpoint = max_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_FLOAT_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_INT(parent_group, lv2parameter, component, zynparameter, name_value, min_value, max_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") int -> %u:%u",                     \
            LV2DYNPARAM_PARAMETER(lv2parameter),                        \
            name_value,                                                 \
            ZYNADD_COMPONENT_ ## component,                             \
            ZYNADD_PARAMETER_INT_ ## zynparameter);                     \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].name = name_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_INT; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].min.integer = min_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].max.integer = max_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER(lv2parameter)].addsynth_parameter = ZYNADD_PARAMETER_INT_ ## zynparameter;

#define LV2DYNPARAM_PARAMETER_INIT_SHAPE(parent_group, lv2parameter, component, name_value, scope_value, hints...) \
  LOG_DEBUG("Registering %u (\"%s\") shape -> %u",                      \
            LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter),                  \
            name_value);                                                \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].name = name_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_SHAPE; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ ## scope_value; \
  map_ptr->parameters[LV2DYNPARAM_PARAMETER_SHAPE(lv2parameter)].addsynth_component = ZYNADD_COMPONENT_ ## component;

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
  } min;

  union
  {
    float fpoint;
    signed int integer;
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
