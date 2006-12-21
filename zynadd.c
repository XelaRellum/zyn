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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "common.h"
#include "lv2.h"
#include "lv2-miditype.h"
#include "lv2-midifunctions.h"
#include "zynadd.peg"
#include "zynadd.h"
#include "addsynth.h"
#include "dynparam.h"
#include "lv2dynparam.h"

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

struct group_descriptor
{
  int parent;                   /* index of parent, LV2DYNPARAM_GROUP_ROOT for root children */
  const char * name;            /* group name */
};

struct parameter_descriptor
{
  int parent;                   /* index of parent, LV2DYNPARAM_GROUP_ROOT for root children */
  const char * name;            /* parameter name */

  unsigned int type;            /* one of LV2DYNPARAM_PARAMETER_TYPE_XXX */

  union
  {
    lv2dynparam_plugin_param_boolean_changed boolean;
    lv2dynparam_plugin_param_float_changed fpoint;
  } value_changed_callback;

  union
  {
    BOOL (* boolean)(zyn_addsynth_handle handle);
    float (* fpoint)(zyn_addsynth_handle handle);
  } get_value;

  union
  {
    float fpoint;
  } min;

  union
  {
    float fpoint;
  } max;
};

/* descriptors containing parent group index */
/* array elements through child index */
/* this defines the tree hierarchy */
struct group_descriptor g_map_groups[LV2DYNPARAM_GROUPS_COUNT];
struct parameter_descriptor g_map_parameters[LV2DYNPARAM_PARAMETERS_COUNT];

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

#define zynadd_ptr ((struct zynadd *)context)

#define IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(ident)      \
  void                                                    \
  zynadd_ ## ident ## _changed(                           \
    void * context,                                       \
    BOOL value)                                           \
  {                                                       \
    printf("zynadd_" #ident "_changed() called.\n");      \
    zyn_addsynth_set_ ## ident(zynadd_ptr->synth, value); \
  }

#define IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(ident)     \
  void                                                    \
  zynadd_ ## ident ## _changed(                           \
    void * context,                                       \
    float value)                                          \
  {                                                       \
    printf("zynadd_" #ident "_changed() called.\n");      \
    zyn_addsynth_set_ ## ident(zynadd_ptr->synth, value); \
  }

IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(stereo)
IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(pan_random)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(panorama)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(volume)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(velocity_sensing)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_strength)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_time)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_stretch)
IMPLEMENT_FLOAT_PARAM_CHANGED_CALLBACK(punch_velocity_sensing)
IMPLEMENT_BOOL_PARAM_CHANGED_CALLBACK(random_grouping)

#undef zynadd_ptr

#define LV2DYNPARAM_GROUP(group) LV2DYNPARAM_GROUP_ ## group
#define LV2DYNPARAM_PARAMETER(parameter) LV2DYNPARAM_PARAMETER_ ## parameter

#define LV2DYNPARAM_GROUP_INIT(parent_group, group, name_value)         \
  g_map_groups[LV2DYNPARAM_GROUP(group)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_groups[LV2DYNPARAM_GROUP(group)].name = name_value;

#define LV2DYNPARAM_PARAMETER_INIT_BOOL(parent_group, parameter, name_value, ident) \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_BOOL; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].value_changed_callback.boolean = zynadd_ ## ident ## _changed; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].get_value.boolean = zyn_addsynth_is_ ## ident;

#define LV2DYNPARAM_PARAMETER_INIT_FLOAT(parent_group, parameter, name_value, ident, min_value, max_value) \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].parent = LV2DYNPARAM_GROUP(parent_group); \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].name = name_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].value_changed_callback.fpoint = zynadd_ ## ident ## _changed; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].get_value.fpoint = zyn_addsynth_get_ ## ident; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].min.fpoint = min_value; \
  g_map_parameters[LV2DYNPARAM_PARAMETER(parameter)].max.fpoint = max_value;

void zynadd_map_initialise() __attribute__((constructor));
void zynadd_map_initialise()
{
  int i;

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    g_map_groups[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    g_map_parameters[i].parent = LV2DYNPARAM_GROUP_INVALID;
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, AMPLITUDE, "Amplitude");
  {
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, STEREO, "Stereo", stereo);
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, RANDOM_GROUPING, "Random Grouping", random_grouping);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, MASTER_VOLUME, "Master Volume", volume, 0, 100);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, VELOCITY_SENSING, "Velocity sensing", velocity_sensing, 0, 100);
    LV2DYNPARAM_PARAMETER_INIT_BOOL(AMPLITUDE, PAN_RANDOMIZE, "Pan randomize", pan_random);
    LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE, PANORAMA, "Panorama", panorama, -1, 1);

    LV2DYNPARAM_GROUP_INIT(AMPLITUDE, AMPLITUDE_PUNCH, "Punch");
    {
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_STRENGTH, "Strength", punch_strength, 0, 100);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_TIME, "Time", punch_time, 0, 100);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_STRETCH, "Stretch", punch_stretch, 0, 100);
      LV2DYNPARAM_PARAMETER_INIT_FLOAT(AMPLITUDE_PUNCH, PUNCH_VELOCITY_SENSING, "Velocity sensing", punch_velocity_sensing, 0, 100);
    }

    LV2DYNPARAM_GROUP_INIT(AMPLITUDE, AMPLITUDE_ENVELOPE, "Envelope");
    {
    }

    LV2DYNPARAM_GROUP_INIT(AMPLITUDE, AMPLITUDE_LFO, "LFO");
    {
    }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, FILTER, "Filter");
  {
    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_ENVELOPE, "Envelope");
    {
    }

    LV2DYNPARAM_GROUP_INIT(FILTER, FILTER_LFO, "LFO");
    {
    }
  }

  LV2DYNPARAM_GROUP_INIT(ROOT, FREQUENCY, "Frequency");
  {
    LV2DYNPARAM_GROUP_INIT(FREQUENCY, FREQUENCY_ENVELOPE, "Envelope");
    {
    }

    LV2DYNPARAM_GROUP_INIT(FREQUENCY, FREQUENCY_LFO, "LFO");
    {
    }
  }

  /* santity check that we have filled all values */

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    assert(g_map_parameters[i].parent != LV2DYNPARAM_GROUP_INVALID);
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    assert(g_map_groups[i].parent != LV2DYNPARAM_GROUP_INVALID);

    assert(g_map_groups[i].name != NULL);

    /* check that parents are with smaller indexes than children */
    /* this checks for loops too */
    assert(g_map_groups[i].parent < i);
  }
}

LV2_Handle
zynadd_instantiate(
  const LV2_Descriptor * descriptor,
  uint32_t sample_rate,
  const char * bundle_path,
  const LV2_Host_Feature ** host_features)
{
  struct zynadd * zynadd_ptr;
  const LV2_Host_Feature * feature_ptr;
  int i;

  printf("zynadd_create_plugin_instance() called.\n");
  printf("sample_rate = %u\n", (unsigned int)sample_rate);
  printf("bundle_path = \"%s\"\n", bundle_path);

//  if (host_features != NULL && *host_features != NULL)
  {
    feature_ptr = *host_features;
    while (feature_ptr)
    {
//      printf("Host feature <%s> detected\n", feature_ptr->URI);
      feature_ptr++;
    }

//    printf("end of host features.\n");
  }

  zynadd_ptr = malloc(sizeof(struct zynadd));
  if (zynadd_ptr == NULL)
  {
    goto fail;
  }
  
  zynadd_ptr->bundle_path = strdup(bundle_path);
  if (zynadd_ptr->bundle_path == NULL)
  {
    goto fail_free_instance;
  }

  zynadd_ptr->ports = malloc(peg_n_ports * sizeof(void *));
  if (zynadd_ptr->ports == NULL)
  {
    goto fail_free_bundle_path;
  }

  zynadd_ptr->sample_rate = sample_rate;

  if (!zyn_addsynth_create(&zynadd_ptr->synth))
  {
    goto fail_free_ports;
  }

  zynadd_ptr->synth_output_offset = SOUND_BUFFER_SIZE;

  if (!lv2dynparam_plugin_instantiate(
        (LV2_Handle)zynadd_ptr,
        "zynadd",
        &zynadd_ptr->dynparams))
  {
    goto fail_destroy_synth;
  }

  for (i = 0 ; i < LV2DYNPARAM_GROUPS_COUNT ; i++)
  {
    if (!lv2dynparam_plugin_group_add(
          zynadd_ptr->dynparams,
          g_map_groups[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_groups[i].parent],
          g_map_groups[i].name,
          zynadd_ptr->groups + i))
    {
      goto fail_clean_dynparams;
    }
  }

  for (i = 0 ; i < LV2DYNPARAM_PARAMETERS_COUNT ; i++)
  {
    switch (g_map_parameters[i].type)
    {
    case LV2DYNPARAM_PARAMETER_TYPE_BOOL:
      if (!lv2dynparam_plugin_param_boolean_add(
            zynadd_ptr->dynparams,
            g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
            g_map_parameters[i].name,
            g_map_parameters[i].get_value.boolean(zynadd_ptr->synth),
            g_map_parameters[i].value_changed_callback.boolean,
            zynadd_ptr,
            zynadd_ptr->parameters + i))
      {
        goto fail_clean_dynparams;
      }
      break;

    case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
      if (!lv2dynparam_plugin_param_float_add(
            zynadd_ptr->dynparams,
            g_map_parameters[i].parent == LV2DYNPARAM_GROUP_ROOT ? NULL : zynadd_ptr->groups[g_map_parameters[i].parent],
            g_map_parameters[i].name,
            g_map_parameters[i].get_value.fpoint(zynadd_ptr->synth),
            g_map_parameters[i].min.fpoint,
            g_map_parameters[i].max.fpoint,
            g_map_parameters[i].value_changed_callback.fpoint,
            zynadd_ptr,
            zynadd_ptr->parameters + i))
      {
        goto fail_clean_dynparams;
      }
      break;
    default:
      assert(0);
    }
  }

  return (LV2_Handle)zynadd_ptr;

fail_clean_dynparams:
  lv2dynparam_plugin_cleanup(zynadd_ptr->dynparams);

fail_destroy_synth:
  zyn_addsynth_destroy(zynadd_ptr->synth);

fail_free_ports:
  free(zynadd_ptr->ports);

fail_free_bundle_path:
  free(zynadd_ptr->bundle_path);

fail_free_instance:
  free(zynadd_ptr);

fail:
  printf("zynadd_instantiate() failed.\n");
  return NULL;
}

#define zynadd_ptr ((struct zynadd *)instance)

/* The run() callback. This is the function that gets called by the host
   when it wants to run the plugin. The parameter is the number of sample
   frames to process. */
void
zynadd_run(
  LV2_Handle instance,
  uint32_t samples_count)
{
  LV2_MIDIState midi;
  double event_time;
  uint32_t event_size;
  unsigned char* event;
  uint32_t now;
  uint32_t fill;
  uint32_t synth_output_offset_future;
/*   char fake_event[2]; */

/*   printf("%u\n", sample_count); fflush(stdout); */

  midi.midi = (LV2_MIDI *)zynadd_ptr->ports[peg_midi_in];
  midi.frame_count = samples_count;
  midi.position = 0;

  now = 0;
  event_time = -1.0;
    
  while (now < samples_count)
  {
    fill = samples_count - now;
    synth_output_offset_future = zynadd_ptr->synth_output_offset;

    if (synth_output_offset_future == SOUND_BUFFER_SIZE)
    {
      synth_output_offset_future = 0;
    }

    if (fill > SOUND_BUFFER_SIZE - synth_output_offset_future)
    {
      fill = SOUND_BUFFER_SIZE - synth_output_offset_future;
    }

    while (event_time < now + fill)
    {
      if (event_time < 0)         /* we need to extract next event */
      {
        lv2midi_get_event(&midi, &event_time, &event_size, &event);
/*         if (event[0] == 0x90) */
/*         { */
/*           event_time = 0; */
/*           fake_event[0] = 0x90; */
/*           fake_event[1] = 69; */
/*         } */
        lv2midi_step(&midi);
      }

      if (event_time >= 0 && event_time < now + fill)
      {
/*         printf("%02X\n", (unsigned int)event[0]); */
        switch (event[0])
        {
        case 0x90:                /* note on */
          zyn_addsynth_note_on(zynadd_ptr->synth, event[1]);
          break;
        case 0x80:                /* note off */
          zyn_addsynth_note_off(zynadd_ptr->synth, event[1]);
          break;
        }

        event_time = -1.0;
      }
    }

    if (zynadd_ptr->synth_output_offset == SOUND_BUFFER_SIZE)
    {
      zyn_addsynth_get_audio_output(zynadd_ptr->synth, zynadd_ptr->synth_output_left, zynadd_ptr->synth_output_right);
      zynadd_ptr->synth_output_offset = 0;
    }

    assert(zynadd_ptr->synth_output_offset == synth_output_offset_future);

    memcpy((float *)(zynadd_ptr->ports[peg_output_left]) + now, zynadd_ptr->synth_output_left, fill * sizeof(float));
    memcpy((float *)(zynadd_ptr->ports[peg_output_right]) + now, zynadd_ptr->synth_output_right, fill * sizeof(float));

    zynadd_ptr->synth_output_offset += fill;
    assert(zynadd_ptr->synth_output_offset <= SOUND_BUFFER_SIZE);
    now += fill;
    assert(now <= samples_count);
  }
}

void
zynadd_cleanup(
  LV2_Handle instance)
{
/*   printf("zynadd_cleanup\n"); */
  zyn_addsynth_destroy(zynadd_ptr->synth);
  free(zynadd_ptr->ports);
  free(zynadd_ptr->bundle_path);
  free(zynadd_ptr);
}

void
zynadd_connect_port(
  LV2_Handle instance,
  uint32_t port,
  void * data_location)
{
  if (port >= peg_n_ports)
  {
    assert(0);
    return;
  }

  zynadd_ptr->ports[port] = data_location;
}

void *
zynadd_extension_data(
  const char * URI)
{
  if (strcmp(URI, LV2DYNPARAM_URI) == 0)
  {
    return g_lv2dynparam_plugin_extension_data;
  }

  return NULL;
}
