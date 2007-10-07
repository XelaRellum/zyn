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

#define LV2DYNPARAM_PARAMETER_PORTAMENTO_ENABLED                    73

#define LV2DYNPARAM_PARAMETERS_COUNT                                74

#define LV2DYNPARAM_GROUP_AMP                                        0
#define LV2DYNPARAM_GROUP_FILTER                                     1
#define LV2DYNPARAM_GROUP_FREQUENCY                                  2
#define LV2DYNPARAM_GROUP_PORTAMENTO                                 3
#define LV2DYNPARAM_GROUP_VOICES                                     4

#define LV2DYNPARAM_GROUP_AMP_PANORAMA                               5
#define LV2DYNPARAM_GROUP_AMP_PUNCH                                  6
#define LV2DYNPARAM_GROUP_AMP_ENV                                    7
#define LV2DYNPARAM_GROUP_AMP_LFO                                    8
#define LV2DYNPARAM_GROUP_AMP_LFO_START_PHASE                        9
#define LV2DYNPARAM_GROUP_AMP_LFO_DEPTH_RANDOMNESS                  10
#define LV2DYNPARAM_GROUP_AMP_LFO_FREQUENCY_RANDOMNESS              11

#define LV2DYNPARAM_GROUP_FILTER_FILTERS                            12
#define LV2DYNPARAM_GROUP_FILTER_ANALOG                             13
#define LV2DYNPARAM_GROUP_FILTER_FORMANT                            14
#define LV2DYNPARAM_GROUP_FILTER_SVF                                15
#define LV2DYNPARAM_GROUP_FILTER_ENV                                16
#define LV2DYNPARAM_GROUP_FILTER_LFO                                17
#define LV2DYNPARAM_GROUP_FILTER_LFO_START_PHASE                    18
#define LV2DYNPARAM_GROUP_FILTER_LFO_DEPTH_RANDOMNESS               19
#define LV2DYNPARAM_GROUP_FILTER_LFO_FREQUENCY_RANDOMNESS           20
 
#define LV2DYNPARAM_GROUP_FREQUENCY_ENV                             21
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO                             22
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO_START_PHASE                 23
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO_DEPTH_RANDOMNESS            24
#define LV2DYNPARAM_GROUP_FREQUENCY_LFO_FREQUENCY_RANDOMNESS        25

#define LV2DYNPARAM_GROUPS_COUNT                                    26

struct group_descriptor g_top_forest_map_groups[LV2DYNPARAM_GROUPS_COUNT];
struct parameter_descriptor g_top_forest_map_parameters[LV2DYNPARAM_PARAMETERS_COUNT];
struct zyn_forest_map g_top_forest_map;
#define map_ptr (&g_top_forest_map)

void zynadd_init_top_forest_map() __attribute__((constructor));
void zynadd_init_top_forest_map()
{
  int i;

  LOG_DEBUG("zynadd_init_top_forest_map() called");

  map_ptr->groups_count = LV2DYNPARAM_GROUPS_COUNT;
  map_ptr->parameters_count = LV2DYNPARAM_PARAMETERS_COUNT;

  map_ptr->groups = g_top_forest_map_groups;
  map_ptr->parameters = g_top_forest_map_parameters;

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    map_ptr->groups[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    map_ptr->parameters[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, PORTAMENTO, "Portamento", NULL);
  {
    LV2DYNPARAM_PARAMETER_INIT_BOOL(PORTAMENTO, PORTAMENTO_ENABLED, PORTAMENTO, PORTAMENTO_ENABLED, "Enabled", ALWAYS, NULL);
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, AMP, "Amplitude", NULL);
  {
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP, STEREO, AMP_GLOBALS, STEREO, "Stereo", ALWAYS, NULL);
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP, RANDOM_GROUPING, AMP_GLOBALS, RANDOM_GROUPING, "Random Grouping", ALWAYS, NULL);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP, VOLUME, AMP_GLOBALS, VOLUME, "Master Volume", 0, 100, ALWAYS, NULL);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP, VELOCITY_SENSING, AMP_GLOBALS, VELOCITY_SENSING, "Velocity sensing", 0, 100, ALWAYS, NULL);

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_PANORAMA, "Random:Panorama", HINT_TOGGLE_FLOAT, NULL, NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_PANORAMA, RANDOM_PANORAMA, AMP_GLOBALS, RANDOM_PANORAMA, "Random", HIDE, PANORAMA, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PANORAMA, PANORAMA, AMP_GLOBALS, PANORAMA, "Panorama", -1, 1, SEMI, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_PUNCH, "Punch", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_STRENGTH, AMP_GLOBALS, PUNCH_STRENGTH, "Strength", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_TIME, AMP_GLOBALS, PUNCH_TIME, "Time", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_STRETCH, AMP_GLOBALS, PUNCH_STRETCH, "Stretch", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_PUNCH, PUNCH_VELOCITY_SENSING, AMP_GLOBALS, PUNCH_VELOCITY_SENSING, "Velocity sensing", 0, 100, ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_ENV, "Envelope", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_ATTACK, AMP_ENV, ENV_ATTACK_DURATION, "Attack", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_DECAY, AMP_ENV, ENV_DECAY_DURATION, "Decay", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_SUSTAIN, AMP_ENV, ENV_SUSTAIN_VALUE, "Sustain", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_RELEASE, AMP_ENV, ENV_RELEASE_DURATION, "Release", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_ENV, AMP_ENV_STRETCH, AMP_ENV, ENV_STRETCH, "Stretch", 0, 200, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP_ENV, AMP_ENV_FORCED_RELEASE, AMP_ENV, ENV_FORCED_RELEASE, "Forced release", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(AMP_ENV, AMP_ENV_LINEAR, AMP_ENV, ENV_LINEAR, "Linear", ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(AMP, AMP_LFO, "LFO", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_SHAPE(AMP_LFO, AMP_LFO, AMP_LFO, "Shape", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_FREQUENCY, AMP_LFO, LFO_FREQUENCY, "Frequency", 0, 1, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_DEPTH, AMP_LFO, LFO_DEPTH, "Depth", 0, 100, ALWAYS, NULL);

      LV2DYNPARAM_GROUP_INIT(AMP_LFO, AMP_LFO_START_PHASE, "Random:Start phase", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_LFO_START_PHASE, AMP_LFO_RANDOM_START_PHASE, AMP_LFO, LFO_RANDOM_START_PHASE, "Random", HIDE, AMP_LFO_START_PHASE, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO_START_PHASE, AMP_LFO_START_PHASE, AMP_LFO, LFO_START_PHASE, "Start phase", 0, 1, SEMI, NULL);
      }

      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_DELAY, AMP_LFO, LFO_DELAY, "Delay", 0, 4, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO, AMP_LFO_STRETCH, AMP_LFO, LFO_STRETCH, "Stretch", -1, 1, ALWAYS, NULL); 

      LV2DYNPARAM_GROUP_INIT(AMP_LFO, AMP_LFO_DEPTH_RANDOMNESS, "Random depth:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_LFO_DEPTH_RANDOMNESS, AMP_LFO_RANDOM_DEPTH, AMP_LFO, LFO_RANDOM_DEPTH, "Random depth", SHOW, AMP_LFO_DEPTH_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO_DEPTH_RANDOMNESS, AMP_LFO_DEPTH_RANDOMNESS, AMP_LFO, LFO_DEPTH_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(AMP_LFO, AMP_LFO_FREQUENCY_RANDOMNESS, "Random frequency:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(AMP_LFO_FREQUENCY_RANDOMNESS, AMP_LFO_RANDOM_FREQUENCY, AMP_LFO, LFO_RANDOM_FREQUENCY, "Random frequency", SHOW, AMP_LFO_FREQUENCY_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMP_LFO_FREQUENCY_RANDOMNESS, AMP_LFO_FREQUENCY_RANDOMNESS, AMP_LFO, LFO_FREQUENCY_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }
   }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, FILTER, "Filter", NULL);
  {
    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_FILTERS, "Filter parameters", HINT_ONE_SUBGROUP, NULL, NULL);
    {
      map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].parent = LV2DYNPARAM_GROUP(FILTER_FILTERS);
      map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].name = "Filter category";
      map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].type = LV2DYNPARAM_PARAMETER_TYPE_FILTER_TYPE;
      map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS;
      map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_FILTER_TYPE].addsynth_component = ZYNADD_COMPONENT_FILTER_GLOBALS;

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_ANALOG, "Analog", NULL);
      {
        map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].parent = LV2DYNPARAM_GROUP(FILTER_ANALOG);
        map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].name = "Filter type";
        map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].type = LV2DYNPARAM_PARAMETER_TYPE_ANALOG_FILTER_TYPE;
        map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].scope = LV2DYNPARAM_PARAMETER_SCOPE_TYPE_ALWAYS;
        map_ptr->parameters[LV2DYNPARAM_PARAMETER_GLOBAL_ANALOG_FILTER_TYPE].addsynth_component = ZYNADD_COMPONENT_FILTER_GLOBALS;

        LV2DYNPARAM_PARAMETER_INIT_INT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_STAGES, FILTER_GLOBALS, STAGES, "Stages", 1, 5, ALWAYS, NULL);

        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_FREQUENCY, FILTER_GLOBALS, FREQUNECY, "Frequency", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_Q_FACTOR, FILTER_GLOBALS, Q_FACTOR, "Q (resonance)", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_AMOUNT, FILTER_GLOBALS, VELOCITY_SENSING_AMOUNT, "Velocity sensing amount", 0, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_VELOCITY_SENSING_FUNCTION, FILTER_GLOBALS, VELOCITY_SENSING_FUNCTION, "Velocity sensing function", -1, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_FREQUENCY_TRACKING, FILTER_GLOBALS, FREQUENCY_TRACKING, "Frequency tracking", -1, 1, ALWAYS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ANALOG, GLOBAL_ANALOG_FILTER_GAIN, FILTER_GLOBALS, VOLUME, "Gain", -30, 30, ALWAYS, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_FORMANT, "Formant", NULL);
      {
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_FILTERS, FILTER_SVF, "State variable", NULL);
      {
      }
    }

    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_ENV, "Envelope", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_ATTACK_VALUE, FILTER_ENV, ENV_ATTACK_VALUE, "Attack value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_ATTACK_DURATION, FILTER_ENV, ENV_ATTACK_DURATION, "Attack duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_DECAY_VALUE, FILTER_ENV, ENV_DECAY_VALUE, "Decay value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_DECAY_DURATION, FILTER_ENV, ENV_DECAY_DURATION, "Decay duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_RELEASE_VALUE, FILTER_ENV, ENV_RELEASE_VALUE, "Release value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_RELEASE_DURATION, FILTER_ENV, ENV_RELEASE_DURATION, "Release duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_ENV, FILTER_ENV_STRETCH, FILTER_ENV, ENV_STRETCH, "Stretch", 0, 200, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(FILTER_ENV, FILTER_ENV_FORCED_RELEASE, FILTER_ENV, ENV_FORCED_RELEASE, "Forced release", ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_LFO, "LFO", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_SHAPE(FILTER_LFO, FILTER_LFO, FILTER_LFO, "Shape", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_FREQUENCY, FILTER_LFO, LFO_FREQUENCY, "Frequency", 0, 1, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_DEPTH, FILTER_LFO, LFO_DEPTH, "Depth", 0, 100, ALWAYS, NULL);

      LV2DYNPARAM_GROUP_INIT(FILTER_LFO, FILTER_LFO_START_PHASE, "Random:Start phase", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FILTER_LFO_START_PHASE, FILTER_LFO_RANDOM_START_PHASE, FILTER_LFO, LFO_RANDOM_START_PHASE, "Random", HIDE, FILTER_LFO_START_PHASE, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO_START_PHASE, FILTER_LFO_START_PHASE, FILTER_LFO, LFO_START_PHASE, "Start phase", 0, 1, SEMI, NULL);
      }

      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_DELAY, FILTER_LFO, LFO_DELAY, "Delay", 0, 4, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO, FILTER_LFO_STRETCH, FILTER_LFO, LFO_STRETCH, "Stretch", -1, 1, ALWAYS, NULL); 

      LV2DYNPARAM_GROUP_INIT(FILTER_LFO, FILTER_LFO_DEPTH_RANDOMNESS, "Random depth:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FILTER_LFO_DEPTH_RANDOMNESS, FILTER_LFO_RANDOM_DEPTH, FILTER_LFO, LFO_RANDOM_DEPTH, "Random depth", SHOW, FILTER_LFO_DEPTH_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO_DEPTH_RANDOMNESS, FILTER_LFO_DEPTH_RANDOMNESS, FILTER_LFO, LFO_DEPTH_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FILTER_LFO, FILTER_LFO_FREQUENCY_RANDOMNESS, "Random frequency:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FILTER_LFO_FREQUENCY_RANDOMNESS, FILTER_LFO_RANDOM_FREQUENCY, FILTER_LFO, LFO_RANDOM_FREQUENCY, "Random frequency", SHOW, FILTER_LFO_FREQUENCY_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FILTER_LFO_FREQUENCY_RANDOMNESS, FILTER_LFO_FREQUENCY_RANDOMNESS, FILTER_LFO, LFO_FREQUENCY_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }
    }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, FREQUENCY, "Frequency", NULL);
  {
    LV2DYNPARAM_GROUP_INIT(FREQUENCY, FREQUENCY_ENV, "Envelope", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_ATTACK_VALUE, FREQUENCY_ENV, ENV_ATTACK_VALUE, "Attack value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_ATTACK_DURATION, FREQUENCY_ENV, ENV_ATTACK_DURATION, "Attack duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_RELEASE_VALUE, FREQUENCY_ENV, ENV_RELEASE_VALUE, "Release value", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_RELEASE_DURATION, FREQUENCY_ENV, ENV_RELEASE_DURATION, "Release duration", 0, 100, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_ENV, FREQUENCY_ENV_STRETCH, FREQUENCY_ENV, ENV_STRETCH, "Stretch", 0, 200, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_BOOL(FREQUENCY_ENV, FREQUENCY_ENV_FORCED_RELEASE, FREQUENCY_ENV, ENV_FORCED_RELEASE, "Forced release", ALWAYS, NULL);
    }

    LV2DYNPARAM_GROUP_INIT(FREQUENCY, FREQUENCY_LFO, "LFO", NULL);
    {
      LV2DYNPARAM_PARAMETER_INIT_SHAPE(FREQUENCY_LFO, FREQUENCY_LFO, FREQUENCY_LFO, "Shape", ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_FREQUENCY, FREQUENCY_LFO, LFO_FREQUENCY, "Frequency", 0, 1, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_DEPTH, FREQUENCY_LFO, LFO_DEPTH, "Depth", 0, 100, ALWAYS, NULL);

      LV2DYNPARAM_GROUP_INIT(FREQUENCY_LFO, FREQUENCY_LFO_START_PHASE, "Random:Start phase", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FREQUENCY_LFO_START_PHASE, FREQUENCY_LFO_RANDOM_START_PHASE, FREQUENCY_LFO, LFO_RANDOM_START_PHASE, "Random", HIDE, FREQUENCY_LFO_START_PHASE, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO_START_PHASE, FREQUENCY_LFO_START_PHASE, FREQUENCY_LFO, LFO_START_PHASE, "Start phase", 0, 1, SEMI, NULL);
      }

      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_DELAY, FREQUENCY_LFO, LFO_DELAY, "Delay", 0, 4, ALWAYS, NULL);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO, FREQUENCY_LFO_STRETCH, FREQUENCY_LFO, LFO_STRETCH, "Stretch", -1, 1, ALWAYS, NULL); 

      LV2DYNPARAM_GROUP_INIT(FREQUENCY_LFO, FREQUENCY_LFO_DEPTH_RANDOMNESS, "Random depth:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FREQUENCY_LFO_DEPTH_RANDOMNESS, FREQUENCY_LFO_RANDOM_DEPTH, FREQUENCY_LFO, LFO_RANDOM_DEPTH, "Random depth", SHOW, FREQUENCY_LFO_DEPTH_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO_DEPTH_RANDOMNESS, FREQUENCY_LFO_DEPTH_RANDOMNESS, FREQUENCY_LFO, LFO_DEPTH_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }

      LV2DYNPARAM_GROUP_INIT(FREQUENCY_LFO, FREQUENCY_LFO_FREQUENCY_RANDOMNESS, "Random frequency:Randomness", HINT_TOGGLE_FLOAT, NULL, NULL);
      {
        LV2DYNPARAM_PARAMETER_INIT_BOOL_SEMI(FREQUENCY_LFO_FREQUENCY_RANDOMNESS, FREQUENCY_LFO_RANDOM_FREQUENCY, FREQUENCY_LFO, LFO_RANDOM_FREQUENCY, "Random frequency", SHOW, FREQUENCY_LFO_FREQUENCY_RANDOMNESS, NULL);
        LV2DYNPARAM_PARAMETER_INIT_FLOAT(FREQUENCY_LFO_FREQUENCY_RANDOMNESS, FREQUENCY_LFO_FREQUENCY_RANDOMNESS, FREQUENCY_LFO, LFO_FREQUENCY_RANDOMNESS, "Randomness", 0, 100, SEMI, NULL);
      }
    }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, VOICES, "Voices", NULL);
  {
  }

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

unsigned int
zynadd_top_forest_map_get_voices_group()
{
  return LV2DYNPARAM_GROUP_VOICES;
}
