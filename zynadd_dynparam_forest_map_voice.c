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
#define LV2DYNPARAM_PARAMETER_WHITE_NOISE                           1
#define LV2DYNPARAM_PARAMETER_OSCILLATOR_BASE_FUNCTION              2
#define LV2DYNPARAM_PARAMETER_OSCILLATOR_WAVESHAPE_TYPE             3
#define LV2DYNPARAM_PARAMETER_OSCILLATOR_WAVESHAPE_DRIVE            4
#define LV2DYNPARAM_PARAMETER_OSCILLATOR_BASE_FUNCTION_ADJUST       5

#define LV2DYNPARAM_PARAMETERS_COUNT                                6

#define LV2DYNPARAM_GROUPS_COUNT                                    0

struct group_descriptor g_voice_forest_map_groups[LV2DYNPARAM_GROUPS_COUNT];
struct parameter_descriptor g_voice_forest_map_parameters[LV2DYNPARAM_PARAMETERS_COUNT];
struct zyn_forest_map g_voice_forest_map;
#define map_ptr (&g_voice_forest_map)

void zynadd_init_voice_forest_map() __attribute__((constructor));
void zynadd_init_voice_forest_map()
{
  LV2DYNPARAM_FOREST_MAP_BEGIN(LV2DYNPARAM_GROUPS_COUNT, LV2DYNPARAM_PARAMETERS_COUNT, g_voice_forest_map_groups, g_voice_forest_map_parameters);

  LOG_DEBUG("zynadd_init_voice_forest_map() called");

  LV2DYNPARAM_PARAMETER_INIT_BOOL(ROOT, RESONANCE, VOICE_GLOBALS, RESONANCE, "Resonance", ALWAYS, NULL);
  LV2DYNPARAM_PARAMETER_INIT_BOOL(ROOT, WHITE_NOISE, VOICE_GLOBALS, WHITE_NOISE, "White Noise", ALWAYS, NULL);

  LV2DYNPARAM_PARAMETER_INIT_ENUM(ROOT, OSCILLATOR_BASE_FUNCTION, VOICE_OSCILLATOR, OSCILLATOR_BASE_FUNCTION, "Base function", g_oscillator_base_function_names, ZYN_OSCILLATOR_BASE_FUNCTIONS_COUNT, ALWAYS, NULL);
  LV2DYNPARAM_PARAMETER_INIT_FLOAT(ROOT, OSCILLATOR_BASE_FUNCTION_ADJUST, VOICE_OSCILLATOR, OSCILLATOR_BASE_FUNCTION_ADJUST, "Base function adjust", 0, 1, ALWAYS, NULL);

  LV2DYNPARAM_PARAMETER_INIT_ENUM(ROOT, OSCILLATOR_WAVESHAPE_TYPE, VOICE_OSCILLATOR, OSCILLATOR_WAVESHAPE_TYPE, "Waveshape type", g_oscillator_waveshape_type_names, ZYN_OSCILLATOR_WAVESHAPE_TYPES_COUNT, ALWAYS, NULL);
  LV2DYNPARAM_PARAMETER_INIT_FLOAT(ROOT, OSCILLATOR_WAVESHAPE_DRIVE, VOICE_OSCILLATOR, OSCILLATOR_WAVESHAPE_DRIVE, "Waveshape drive", 0, 100, ALWAYS, NULL);

  LV2DYNPARAM_FOREST_MAP_END;
}
