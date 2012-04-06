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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <lv2.h>

#include "common.h"
#include "lv2-midiport.h"
#include "lv2-midifunctions.h"
#include "zynadd.h"
#include "addsynth.h"
#include "lv2dynparam/lv2dynparam.h"
#include "lv2dynparam/lv2_rtmempool.h"
#include "lv2dynparam/plugin.h"
#include "list.h"
#include "zynadd_internal.h"
//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

#define LV2_PORT_MIDI_IN 0
#define LV2_PORT_OUTPUT_LEFT 1
#define LV2_PORT_OUTPUT_RIGHT 2
#define LV2_PORTS_COUNT 3

LV2_Handle
zynadd_instantiate(
  const LV2_Descriptor * descriptor,
  double sample_rate,
  const char * bundle_path,
  const LV2_Feature * const * host_features)
{
  struct zynadd * zynadd_ptr;
  struct lv2_rtsafe_memory_pool_provider * rtmempool_ptr;
  const LV2_Feature * const * feature_ptr_ptr;

  LOG_DEBUG("zynadd_create_plugin_instance() called.");
  LOG_DEBUG("sample_rate = %f", sample_rate);
  LOG_DEBUG("bundle_path = \"%s\"", bundle_path);

  rtmempool_ptr = NULL;

  feature_ptr_ptr = host_features;
  while (*feature_ptr_ptr)
  {
    LOG_DEBUG("Host feature <%s> detected", (*feature_ptr_ptr)->URI);

    if (strcmp((*feature_ptr_ptr)->URI, LV2_RTSAFE_MEMORY_POOL_URI) == 0)
    {
      rtmempool_ptr = (*feature_ptr_ptr)->data;
    }

    feature_ptr_ptr++;
  }

  if (rtmempool_ptr == NULL)
  {
    LOG_ERROR(LV2_RTSAFE_MEMORY_POOL_URI " extension is required");
    goto fail;
  }

  zynadd_ptr = malloc(sizeof(struct zynadd));
  if (zynadd_ptr == NULL)
  {
    goto fail;
  }
  
  zynadd_ptr->host_features = host_features;

  zynadd_ptr->bundle_path = strdup(bundle_path);
  if (zynadd_ptr->bundle_path == NULL)
  {
    goto fail_free_instance;
  }

  zynadd_ptr->ports = malloc(LV2_PORTS_COUNT * sizeof(void *));
  if (zynadd_ptr->ports == NULL)
  {
    goto fail_free_bundle_path;
  }

  zynadd_ptr->sample_rate = sample_rate;

  if (!zyn_addsynth_create(sample_rate, VOICES_COUNT, &zynadd_ptr->synth))
  {
    goto fail_free_ports;
  }

  zynadd_ptr->synth_output_offset = SOUND_BUFFER_SIZE;

  if (!zynadd_dynparam_init(zynadd_ptr))
  {
    LOG_ERROR("zynadd_dynparam_init() failed.");
    goto fail_destroy_synth;
  }

  return (LV2_Handle)zynadd_ptr;

fail_destroy_synth:
  zyn_addsynth_destroy(zynadd_ptr->synth);

fail_free_ports:
  free(zynadd_ptr->ports);

fail_free_bundle_path:
  free(zynadd_ptr->bundle_path);

fail_free_instance:
  free(zynadd_ptr);

fail:
/*   printf("zynadd_instantiate() failed.\n"); */
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

  midi.midi = (LV2_MIDI *)zynadd_ptr->ports[LV2_PORT_MIDI_IN];
  midi.frame_count = samples_count;
  midi.position = 0;

  now = 0;
  event_time = -1.0;
  event = NULL;
  event_size = 0;
    
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
        if (event_size == 3)
        {
          switch (event[0] & 0xF0)
          {
          case 0x90:                /* note on */
            zyn_addsynth_note_on(zynadd_ptr->synth, event[1], event[2]);
            break;
          case 0x80:                /* note off */
            zyn_addsynth_note_off(zynadd_ptr->synth, event[1]);
            break;
          case 0xB0:
            switch (event[1])
            {
            case 0x78:          /* all sound off */
              zyn_addsynth_all_sound_off(zynadd_ptr->synth);
              break;
            case 0x7B:          /* all notes off */
              zyn_addsynth_all_notes_off(zynadd_ptr->synth);
              break;
            }
            break;
          }
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

    memcpy((float *)(zynadd_ptr->ports[LV2_PORT_OUTPUT_LEFT]) + now, zynadd_ptr->synth_output_left, fill * sizeof(float));
    memcpy((float *)(zynadd_ptr->ports[LV2_PORT_OUTPUT_RIGHT]) + now, zynadd_ptr->synth_output_right, fill * sizeof(float));

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
  zynadd_dynparam_uninit(zynadd_ptr);
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
  if (port >= LV2_PORTS_COUNT)
  {
    assert(0);
    return;
  }

  zynadd_ptr->ports[port] = data_location;
}

const void *
zynadd_extension_data(
  const char * URI)
{
  if (strcmp(URI, LV2DYNPARAM_URI) == 0)
  {
    return get_lv2dynparam_plugin_extension_data();
  }

  return NULL;
}
