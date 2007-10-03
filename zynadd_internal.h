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

#ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED
#define ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED

#define LV2DYNPARAM_PARAMETER_TYPE_BOOL               1
#define LV2DYNPARAM_PARAMETER_TYPE_FLOAT              2
#define LV2DYNPARAM_PARAMETER_TYPE_INT                3
#define LV2DYNPARAM_PARAMETER_TYPE_SHAPE              4
#define LV2DYNPARAM_PARAMETER_TYPE_FILTER_TYPE        5
#define LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE 6

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

#define LV2DYNPARAM_PARAMETER_AMP_LFO_SHAPE                         17
#define LV2DYNPARAM_PARAMETER_AMP_LFO_FREQUENCY                     18
#define LV2DYNPARAM_PARAMETER_AMP_LFO_DEPTH                         19
#define LV2DYNPARAM_PARAMETER_AMP_LFO_RANDOM_START_PHASE            20
#define LV2DYNPARAM_PARAMETER_AMP_LFO_START_PHASE                   21
#define LV2DYNPARAM_PARAMETER_AMP_LFO_DELAY                         22
#define LV2DYNPARAM_PARAMETER_AMP_LFO_STRETCH                       23
#define LV2DYNPARAM_PARAMETER_AMP_LFO_RANDOM_DEPTH                  24
#define LV2DYNPARAM_PARAMETER_AMP_LFO_DEPTH_RANDOMNESS              25
#define LV2DYNPARAM_PARAMETER_AMP_LFO_RANDOM_FREQUENCY              26
#define LV2DYNPARAM_PARAMETER_AMP_LFO_FREQUENCY_RANDOMNESS          27

#define LV2DYNPARAM_PARAMETER_FILTER_ENV_ATTACK_VALUE               28
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_ATTACK_DURATION            29
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_DECAY_VALUE                30
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_DECAY_DURATION             31
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_RELEASE_VALUE              32
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_RELEASE_DURATION           33
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_STRETCH                    34
#define LV2DYNPARAM_PARAMETER_FILTER_ENV_FORCED_RELEASE             35

#define LV2DYNPARAM_PARAMETER_FILTER_LFO_SHAPE                      36
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_FREQUENCY                  37
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_DEPTH                      38
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_RANDOM_START_PHASE         39
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_START_PHASE                40
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_DELAY                      41
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_STRETCH                    42
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_RANDOM_DEPTH               43
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_DEPTH_RANDOMNESS           44
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_RANDOM_FREQUENCY           45
#define LV2DYNPARAM_PARAMETER_FILTER_LFO_FREQUENCY_RANDOMNESS       46

#define LV2DYNPARAM_PARAMETER_FREQUENCY_ENV_ATTACK_VALUE            47
#define LV2DYNPARAM_PARAMETER_FREQUENCY_ENV_ATTACK_DURATION         48
#define LV2DYNPARAM_PARAMETER_FREQUENCY_ENV_RELEASE_DURATION        49
#define LV2DYNPARAM_PARAMETER_FREQUENCY_ENV_RELEASE_VALUE           50
#define LV2DYNPARAM_PARAMETER_FREQUENCY_ENV_STRETCH                 51
#define LV2DYNPARAM_PARAMETER_FREQUENCY_ENV_FORCED_RELEASE          52

#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_SHAPE                   53
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_FREQUENCY               54
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_DEPTH                   55
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_RANDOM_START_PHASE      56
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_START_PHASE             57
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_DELAY                   58
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_STRETCH                 59
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_RANDOM_DEPTH            60
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_DEPTH_RANDOMNESS        61
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_RANDOM_FREQUENCY        62
#define LV2DYNPARAM_PARAMETER_FREQUENCY_LFO_FREQUENCY_RANDOMNESS    63

#define LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE                    64

#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE             65
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_FREQUENCY        66
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_Q_FACTOR         67
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_AMOUNT    68
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_FUNCTION  69
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_FREQUENCY_TRACKING         70
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_GAIN             71
#define LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_STAGES           72

#define LV2DYNPARAM_PARAMETERS_COUNT                                73

#define LV2DYNPARAM_GROUP_INVALID                                   -2
#define LV2DYNPARAM_GROUP_ROOT                                      -1

#define LV2DYNPARAM_GROUP_AMP                                        0
#define LV2DYNPARAM_GROUP_FILTER                                     1
#define LV2DYNPARAM_GROUP_FREQUENCY                                  2
#define LV2DYNPARAM_GROUP_VOICES                                     3

#define LV2DYNPARAM_GROUP_AMP_PANORAMA                               4
#define LV2DYNPARAM_GROUP_AMP_PUNCH                                  5
#define LV2DYNPARAM_GROUP_AMP_ENV                                    6
#define LV2DYNPARAM_GROUP_AMP_LFO                                    7
#define LV2DYNPARAM_GROUP_AMP_LFO_START_PHASE                        8
#define LV2DYNPARAM_GROUP_AMP_LFO_DEPTH_RANDOMNESS                   9
#define LV2DYNPARAM_GROUP_AMP_LFO_FREQUENCY_RANDOMNESS              10

#define LV2DYNPARAM_GROUP_FILTER_FILTERS                            11
#define LV2DYNPARAM_GROUP_FILTER_ANALOG                             12
#define LV2DYNPARAM_GROUP_FILTER_FORMANT                            13
#define LV2DYNPARAM_GROUP_FILTER_SVF                                14
#define LV2DYNPARAM_GROUP_FILTER_ENV                                15
#define LV2DYNPARAM_GROUP_FILTER_LFO                                16
#define LV2DYNPARAM_GROUP_FILTER_LFO_START_PHASE                    17
#define LV2DYNPARAM_GROUP_FILTER_LFO_DEPTH_RANDOMNESS               18
#define LV2DYNPARAM_GROUP_FILTER_LFO_FREQUENCY_RANDOMNESS           19
 
#define LV2DYNPARAM_GROUP_FREQUENCY_ENV                             20
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO                             21
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO_START_PHASE                 22
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO_DEPTH_RANDOMNESS            23
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO_FREQUENCY_RANDOMNESS        24

#define LV2DYNPARAM_GROUPS_COUNT                                    25

#define ZYN_MAX_HINTS 10

struct zynadd_parameter
{
  struct list_head siblings;
  struct zynadd * synth_ptr;
  unsigned int addsynth_component; /* one of ZYNADD_COMPONENT_XXX */
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
  struct group_descriptor groups[LV2DYNPARAM_GROUPS_COUNT];
  struct parameter_descriptor parameters[LV2DYNPARAM_PARAMETERS_COUNT];
};

struct zyn_forest_initializer
{
  struct zyn_forest_map * map_ptr;

  struct zynadd_group * groups[LV2DYNPARAM_GROUPS_COUNT];
  struct zynadd_parameter * parameters[LV2DYNPARAM_PARAMETERS_COUNT];
};

struct zynadd
{
  double sample_rate;
  char * bundle_path;
  void ** ports;

  zyn_addsynth_handle synth;

  zyn_sample_type synth_output_left[SOUND_BUFFER_SIZE];
  zyn_sample_type synth_output_right[SOUND_BUFFER_SIZE];

  uint32_t synth_output_offset; /* offset of unread data within synth_output_xxx audio buffers */

  lv2dynparam_plugin_instance dynparams;

  struct list_head groups;
  struct list_head parameters;

  struct zyn_forest_initializer top_forest_initializer;
};

bool zynadd_dynparam_init(struct zynadd * zynadd_ptr);

void zynadd_dynparam_uninit(struct zynadd * zynadd_ptr);

bool
zynadd_appear_parameter(
  struct zynadd * zynadd_ptr,
  struct zynadd_parameter * parameter_ptr);

#endif /* #ifndef ZYNADD_INTERNAL_H__A38C6254_E7AD_443E_AA5F_A5AB3FCB8B06__INCLUDED */
