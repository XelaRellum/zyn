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

#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS          0 /* always visible, not interacting with other parameters */
#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SHOW_OTHER      1 /* always visible bool, when true, other param appearing */
#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_HIDE_OTHER      2 /* always visible bool, when true, other param disappearing */
#define LV2DYNPARAM_PARAMETER_SCOPE_TYPE_SEMI            3 /* sometimes visible parameter */

#define LV2DYNPARAM_PARAMETER_STEREO                                 0
#define LV2DYNPARAM_PARAMETER_RANDOM_GROUPING                        1
#define LV2DYNPARAM_PARAMETER_VOLUME                                 2
#define LV2DYNPARAM_PARAMETER_VELOCITY_SENSING                       3
#define LV2DYNPARAM_PARAMETER_RANDOM_PANORAMA                        4
#define LV2DYNPARAM_PARAMETER_PANORAMA                               5
#define LV2DYNPARAM_PARAMETER_PUNCH_STRENGTH                         6
#define LV2DYNPARAM_PARAMETER_PUNCH_TIME                             7
#define LV2DYNPARAM_PARAMETER_PUNCH_STRETCH                          8
#define LV2DYNPARAM_PARAMETER_PUNCH_VELOCITY_SENSING                 9
#define LV2DYNPARAM_PARAMETER_AMP_ENV_ATTACK                        10
#define LV2DYNPARAM_PARAMETER_AMP_ENV_DECAY                         11
#define LV2DYNPARAM_PARAMETER_AMP_ENV_SUSTAIN                       12
#define LV2DYNPARAM_PARAMETER_AMP_ENV_RELEASE                       13
#define LV2DYNPARAM_PARAMETER_AMP_ENV_STRETCH                       14
#define LV2DYNPARAM_PARAMETER_AMP_ENV_FORCED_RELEASE                15
#define LV2DYNPARAM_PARAMETER_AMP_ENV_LINEAR                        16
#define LV2DYNPARAM_PARAMETER_AMP_LFO_FREQUENCY                     17
#define LV2DYNPARAM_PARAMETER_AMP_LFO_DEPTH                         18
#define LV2DYNPARAM_PARAMETER_AMP_LFO_RANDOM_START_PHASE            19
#define LV2DYNPARAM_PARAMETER_AMP_LFO_START_PHASE                   20
#define LV2DYNPARAM_PARAMETER_AMP_LFO_DELAY                         21
#define LV2DYNPARAM_PARAMETER_AMP_LFO_STRETCH                       22
#define LV2DYNPARAM_PARAMETER_AMP_LFO_RANDOM_DEPTH                  23
#define LV2DYNPARAM_PARAMETER_AMP_LFO_DEPTH_RANDOMNESS              24
#define LV2DYNPARAM_PARAMETER_AMP_LFO_RANDOM_FREQUENCY              25
#define LV2DYNPARAM_PARAMETER_AMP_LFO_FREQUENCY_RANDOMNESS          26
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_FREQUENCY                  27
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_DEPTH                      28
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_RANDOM_START_PHASE         29
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_START_PHASE                30
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_DELAY                      31
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_STRETCH                    32
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_RANDOM_DEPTH               33
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_DEPTH_RANDOMNESS           34
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_RANDOM_FREQUENCY           35
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_FREQUENCY_RANDOMNESS       36
#define LV2DYNPARAM_PARAMETERS_COUNT                                37

#define LV2DYNPARAM_GROUP_INVALID                                   -2
#define LV2DYNPARAM_GROUP_ROOT                                      -1

#define LV2DYNPARAM_GROUP_AMPLITUDE                                  0
#define LV2DYNPARAM_GROUP_FILTER                                     1
#define LV2DYNPARAM_GROUP_FREQUENCY                                  2

#define LV2DYNPARAM_GROUP_AMPLITUDE_PANORAMA                         3
#define LV2DYNPARAM_GROUP_AMPLITUDE_PUNCH                            4
#define LV2DYNPARAM_GROUP_AMPLITUDE_ENVELOPE                         5
#define LV2DYNPARAM_GROUP_AMPLITUDE_LFO                              6
#define LV2DYNPARAM_GROUP_AMPLITUDE_LFO_START_PHASE                  7
#define LV2DYNPARAM_GROUP_AMPLITUDE_LFO_DEPTH_RANDOMNESS             8
#define LV2DYNPARAM_GROUP_AMPLITUDE_LFO_FREQUENCY_RANDOMNESS         9

#define LV2DYNPARAM_GROUP_FILTER_ENVELOPE                           10
#define LV2DYNPARAM_GROUP_FILTER_LFO                                11
#define LV2DYNPARAM_GROUP_FILTER_LFO_START_PHASE                    12
#define LV2DYNPARAM_GROUP_FILTER_LFO_DEPTH_RANDOMNESS               13
#define LV2DYNPARAM_GROUP_FILTER_LFO_FREQUENCY_RANDOMNESS           14
 
#define LV2DYNPARAM_GROUP_FREQUENCY_ENVELOPE                        15
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO                             16

#define LV2DYNPARAM_GROUPS_COUNT                                    17

struct zynadd_parameter
{
  struct zynadd * synth_ptr;
  unsigned int addsynth_parameter; /* one of ZYNADD_PARAMETER_XXX */
  unsigned int scope;           /* one of LV2DYNPARAM_PARAMETER_SCOPE_TYPE_XXX */
  unsigned int scope_specific;

  lv2dynparam_plugin_parameter lv2parameter;
};

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
  struct zynadd_parameter parameters[LV2DYNPARAM_PARAMETERS_COUNT];
};

BOOL zynadd_dynparam_init(struct zynadd * zynadd_ptr);

#endif /* #ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED */
