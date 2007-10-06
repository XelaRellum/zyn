/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2007 Nedko Arnaudov <nedko@arnaudov.name>
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

#define LV2DYNPARAM_PARAMETER_RESONANCE                             0

#define LV2DYNPARAM_PARAMETERS_COUNT                                1

#define LV2DYNPARAM_GROUPS_COUNT                                    0

struct group_descriptor g_voice_forest_map_groups[LV2DYNPARAM_GROUPS_COUNT];
struct parameter_descriptor g_voice_forest_map_parameters[LV2DYNPARAM_PARAMETERS_COUNT];
struct zyn_forest_map g_voice_forest_map;
#define map_ptr (&g_voice_forest_map)

void zynadd_init_voice_forest_map() __attribute__((constructor));
void zynadd_init_voice_forest_map()
{
  int i;

  LOG_DEBUG("zynadd_init_voice_forest_map() called");

  map_ptr->groups_count = LV2DYNPARAM_GROUPS_COUNT;
  map_ptr->parameters_count = LV2DYNPARAM_PARAMETERS_COUNT;

  map_ptr->groups = g_voice_forest_map_groups;
  map_ptr->parameters = g_voice_forest_map_parameters;

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    map_ptr->groups[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    map_ptr->parameters[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  LV2DYNPARAM_PARAMETER_INIT_BOOL(ROOT, RESONANCE, VOICE_GLOBALS, VOICE_RESONANCE, "Resonance", ALWAYS, NULL);

  /* santity check that we have filled all values */

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    LOG_DEBUG("parameter %d with parent %d", i, map_ptr->parameters[i].parent);
    assert(map_ptr->parameters[i].parent != LV2DYNPARAM_GROUP_INVALID);
    assert(map_ptr->parameters[i].parent < LV2DYNPARAM_GROUPS_COUNT);
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    LOG_DEBUG("group %d with parent %d", i, map_ptr->groups[i].parent);
    assert(map_ptr->groups[i].parent != LV2DYNPARAM_GROUP_INVALID);

    assert(map_ptr->groups[i].name != NULL);

    /* check that parents are with smaller indexes than children */
    /* this checks for loops too */
    assert(map_ptr->groups[i].parent < i);
  }
}
