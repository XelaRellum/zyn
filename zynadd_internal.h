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

#ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED
#define ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED

#define LV2DYNPARAM_PARAMETER_TYPE_BOOL      1
#define LV2DYNPARAM_PARAMETER_TYPE_FLOAT     2

#define LV2DYNPARAM_PARAMETER_STEREO                        0
#define LV2DYNPARAM_PARAMETER_RANDOM_GROUPING               1
#define LV2DYNPARAM_PARAMETER_MASTER_VOLUME                 2
#define LV2DYNPARAM_PARAMETER_VELOCITY_SENSING              3
#define LV2DYNPARAM_PARAMETER_PAN_RANDOMIZE                 4
#define LV2DYNPARAM_PARAMETER_PANORAMA                      5
#define LV2DYNPARAM_PARAMETER_PUNCH_STRENGTH                6
#define LV2DYNPARAM_PARAMETER_PUNCH_TIME                    7
#define LV2DYNPARAM_PARAMETER_PUNCH_STRETCH                 8
#define LV2DYNPARAM_PARAMETER_PUNCH_VELOCITY_SENSING        9
#define LV2DYNPARAM_PARAMETERS_COUNT                       10

#define LV2DYNPARAM_GROUP_INVALID                          -2
#define LV2DYNPARAM_GROUP_ROOT                             -1

#define LV2DYNPARAM_GROUP_AMPLITUDE                         0
#define LV2DYNPARAM_GROUP_FILTER                            1
#define LV2DYNPARAM_GROUP_FREQUENCY                         2

#define LV2DYNPARAM_GROUP_AMPLITUDE_PUNCH                   3
#define LV2DYNPARAM_GROUP_AMPLITUDE_ENVELOPE                4
#define LV2DYNPARAM_GROUP_AMPLITUDE_LFO                     5

#define LV2DYNPARAM_GROUP_FILTER_ENVELOPE                   6
#define LV2DYNPARAM_GROUP_FILTER_LFO                        7

#define LV2DYNPARAM_GROUP_FREQUENCY_ENVELOPE                8
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO                     9

#define LV2DYNPARAM_GROUPS_COUNT                           10

struct zynadd
{
  uint32_t sample_rate;
  char * bundle_path;
  void ** ports;

  zyn_addsynth_handle synth;

  zyn_sample_type synth_output_left[SOUND_BUFFER_SIZE];
  zyn_sample_type synth_output_right[SOUND_BUFFER_SIZE];

  uint32_t synth_output_offset; /* offset of unread data within synth_output_xxx audio buffers */

  lv2dynparam_plugin_instance dynparams;

  lv2dynparam_plugin_group groups[LV2DYNPARAM_GROUPS_COUNT];
  lv2dynparam_plugin_parameter parameters[LV2DYNPARAM_PARAMETERS_COUNT];
};

BOOL zynadd_dynparam_init(struct zynadd * zynadd_ptr);

#endif /* #ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED */
