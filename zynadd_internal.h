/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007,2008,2009 Nedko Arnaudov <nedko@arnaudov.name>
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

#ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED
#define ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED

/*
 * The number of voices of additive synth for a single note
 */
#define VOICES_COUNT 8

#define LV2DYNPARAM_PARAMETER_TYPE_BOOL               1
#define LV2DYNPARAM_PARAMETER_TYPE_FLOAT              2
#define LV2DYNPARAM_PARAMETER_TYPE_INT                3
#define LV2DYNPARAM_PARAMETER_TYPE_ENUM               4

#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS          0 /* always visible, not interacting with other parameters */
#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER      1 /* always visible bool, when true, other param appearing */
#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER      2 /* always visible bool, when true, other param disappearing */
#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SEMI            3 /* sometimes visible parameter */

#define LV2DYNPARAM_GROUP_INVALID                                   -2
#define LV2DYNPARAM_GROUP_ROOT                                      -1

#define HINT_HIDDEN           "http://home.gna.org/zynjacku/hints#hidden"
#define HINT_TOGGLE_FLOAT     "http://home.gna.org/zynjacku/hints#togglefloat"
#define HINT_ONE_SUBGROUP     "http://home.gna.org/zynjacku/hints#onesubgroup"

#include "zynadd_dynparam_forest_map.h"

struct zynadd_parameter
{
  struct list_head siblings;
  struct zynadd * synth_ptr;
  zyn_addsynth_component addsynth_component;
  unsigned int addsynth_parameter; /* one of ZYNADD_PARAMETER_XXX */
  unsigned int scope;           /* one of LV2DYNPARAM_PARAMETER_SCOPE_TYPE_XXX */
  struct zynadd_parameter * other_parameter; /* used for bools controling other parameters appear/disappear */

  struct zynadd_group * parent_ptr; /* NULL for parameters, children of root */
  const char * name_ptr;            /* parameter name, points to somewhere in forest map */
  unsigned int type;            /* one of LV2DYNPARAM_PARAMETER_TYPE_XXX */
  struct lv2dynparam_hints * hints_ptr; /* parameter hints, points to somewhere in forest map */

  struct parameter_descriptor * map_element_ptr;

  lv2dynparam_plugin_parameter lv2parameter;
};

struct zynadd_group
{
  struct list_head siblings;
  struct zynadd_group * parent_ptr; /* NULL for groups, children of root */
  const char * name_ptr;            /* group name, points to somewhere in forest map */
  struct lv2dynparam_hints * hints_ptr; /* group hints, points to somewhere in forest map */
  lv2dynparam_plugin_group lv2group;
};

struct zyn_forest_initializer
{
  struct zyn_forest_map * map_ptr;

  size_t groups_count;
  size_t parameters_count;

  struct zynadd_group ** groups;
  struct zynadd_parameter ** parameters;
};

struct zynadd
{
  double sample_rate;
  char * bundle_path;
  void ** ports;

  zyn_addsynth_handle synth;
  zyn_addsynth_component synth_global_components[ZYNADD_GLOBAL_COMPONENTS_COUNT];
  zyn_addsynth_component synth_voice_components[VOICES_COUNT * ZYNADD_VOICE_COMPONENTS_COUNT];

  zyn_sample_type synth_output_left[SOUND_BUFFER_SIZE];
  zyn_sample_type synth_output_right[SOUND_BUFFER_SIZE];

  uint32_t synth_output_offset; /* offset of unread data within synth_output_xxx audio buffers */

  lv2dynparam_plugin_instance dynparams;

  struct list_head groups;
  struct list_head parameters;

  const LV2_Feature * const * host_features;
};

bool zynadd_dynparam_init(struct zynadd * zynadd_ptr);

void zynadd_dynparam_uninit(struct zynadd * zynadd_ptr);

bool
zynadd_appear_parameter(
  struct zynadd * zynadd_ptr,
  struct zynadd_parameter * parameter_ptr);

#endif /* #ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED */
