/* -*- Mode: C++ ; c-basic-offset: 2 -*- */
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
#include <math.h>
#include <assert.h>

#include "common.h"
#include "addsynth.h"
#include "globals.h"
#include "resonance.h"
#include "fft_wrapper.h"
#include "oscillator.h"
#include "envelope_parameters.h"
#include "envelope.h"
#include "lfo_parameters.h"
#include "lfo.h"
#include "filter_parameters.h"
#include "filter_base.h"
#include "filter.h"
#include "addnote_parameters.h"
#include "Controller.h"
#include "addnote.h"
#include "util.h"

#define ZYN_DEFAULT_POLYPHONY 60

struct note_channel
{
  int midinote;               // MIDI note, -1 when note "channel" is not playing
  ADnote * note_ptr;
};

struct zyn_addsynth
{
  unsigned int polyphony;
  struct note_channel * notes_array;
  ADnoteParameters * params_ptr;
  FFTwrapper * fft_ptr;
  Controller * ctl_ptr;
  unsigned char velsns;         // velocity sensing (amplitude velocity scale)
  zyn_sample_type oldfreq;      // this is used for portamento

  BOOL random_panorama;         // whether panorama is random for each note
  float panorama;               // -1.0 for left, 0.0 for center, 1.0 for right

  BOOL stereo;                  // stereo or mono

  // How the Harmonic Amplitude is applied to voices that use the same oscillator
  BOOL random_grouping;
};

BOOL
zyn_addsynth_create(
  zyn_addsynth_handle * handle_ptr)
{
  struct zyn_addsynth * zyn_addsynth_ptr;
  unsigned int note_index;

//  printf("zyn_addsynth_create\n");
  zyn_addsynth_ptr = (struct zyn_addsynth *)malloc(sizeof(struct zyn_addsynth));

  zyn_addsynth_ptr->polyphony = ZYN_DEFAULT_POLYPHONY;
  zyn_addsynth_ptr->notes_array = (struct note_channel *)malloc(ZYN_DEFAULT_POLYPHONY * sizeof(struct note_channel));

  zyn_addsynth_ptr->velsns = 64;
  zyn_addsynth_ptr->fft_ptr = new FFTwrapper(OSCIL_SIZE);
  zyn_addsynth_ptr->params_ptr = new ADnoteParameters(zyn_addsynth_ptr->fft_ptr);
  zyn_addsynth_ptr->ctl_ptr = new Controller;
  zyn_addsynth_ptr->ctl_ptr->defaults();
  zyn_addsynth_ptr->oldfreq = -1.0;

  zyn_addsynth_ptr->random_panorama = FALSE;
  zyn_addsynth_ptr->panorama = 0.0;

  zyn_addsynth_ptr->stereo = TRUE;

  zyn_addsynth_ptr->random_grouping = FALSE;

  for (note_index = 0 ; note_index < ZYN_DEFAULT_POLYPHONY ; note_index++)
  {
    zyn_addsynth_ptr->notes_array[note_index].note_ptr = new ADnote(zyn_addsynth_ptr->params_ptr, zyn_addsynth_ptr->ctl_ptr);
    zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
  }

  *handle_ptr = (zyn_addsynth_handle)zyn_addsynth_ptr;

  OscilGen::tmpsmps=new REALTYPE[OSCIL_SIZE];
  newFFTFREQS(&OscilGen::outoscilFFTfreqs,OSCIL_SIZE/2);

//  printf("zyn_addsynth_create(%08X)\n", (unsigned int)*handle_ptr);

  return TRUE;
}

#define zyn_addsynth_ptr ((struct zyn_addsynth *)handle)

void
zyn_addsynth_get_audio_output(
  zyn_addsynth_handle handle,
  zyn_sample_type * buffer_left,
  zyn_sample_type * buffer_right)
{
  unsigned int note_index;
  zyn_sample_type note_buffer_left[SOUND_BUFFER_SIZE];
  zyn_sample_type note_buffer_right[SOUND_BUFFER_SIZE];

  silence_two_buffers(buffer_left, buffer_right, SOUND_BUFFER_SIZE);

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote != -1)
    {
      //printf("mixing note channel %u\n", note_index);

      zyn_addsynth_ptr->notes_array[note_index].note_ptr->noteout(note_buffer_left, note_buffer_right);

      mix_add_two_buffers(
        buffer_left,
        buffer_right,
        note_buffer_left,
        note_buffer_right,
        SOUND_BUFFER_SIZE);

      if (zyn_addsynth_ptr->notes_array[note_index].note_ptr->finished())
      {
        zyn_addsynth_ptr->notes_array[note_index].midinote = -1;
      }
    }
  }
}

void
zyn_addsynth_note_on(
  zyn_addsynth_handle handle,
  unsigned int note)
{
  unsigned int note_index;
  unsigned char velocity;
  int masterkeyshift;
  float vel;
  zyn_sample_type notebasefreq;

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote == -1)
    {
      goto unused_note_channel_found;
    }
  }

  //printf("note on %u - ignored\n", note);

  return;

unused_note_channel_found:
  //printf("note on %u - channel %u\n", note, note_index);
  velocity = 100;
  masterkeyshift = 0;

  vel = VelF(velocity/127.0, zyn_addsynth_ptr->velsns);
  notebasefreq = 440.0*pow(2.0,(note-69.0)/12.0);

  // Portamento

  if (zyn_addsynth_ptr->oldfreq < 1.0) /* only when the first note is played */
  {
    zyn_addsynth_ptr->oldfreq = notebasefreq;
  }
      
  BOOL portamento = zyn_addsynth_ptr->ctl_ptr->initportamento(zyn_addsynth_ptr->oldfreq, notebasefreq);
      
  zyn_addsynth_ptr->oldfreq = notebasefreq;

  zyn_addsynth_ptr->notes_array[note_index].midinote = note;
  zyn_addsynth_ptr->notes_array[note_index].note_ptr->note_on(
    zyn_addsynth_ptr->random_panorama ? RND : zyn_addsynth_ptr->panorama,
    zyn_addsynth_ptr->random_grouping,
    notebasefreq,
    vel,
    portamento,
    note);
}

void
zyn_addsynth_note_off(
  zyn_addsynth_handle handle,
  unsigned int note)
{
  unsigned int note_index;

  //printf("note off %u\n", note);

  for (note_index = 0 ; note_index < zyn_addsynth_ptr->polyphony ; note_index++)
  {
    if (zyn_addsynth_ptr->notes_array[note_index].midinote == note)
    {
      zyn_addsynth_ptr->notes_array[note_index].note_ptr->relasekey();
    }
  }
}

void
zyn_addsynth_destroy(
  zyn_addsynth_handle handle)
{
//  printf("zyn_addsynth_destroy(%08X)\n", (unsigned int)handle);
  delete zyn_addsynth_ptr->fft_ptr;
  delete zyn_addsynth_ptr->params_ptr;
  delete zyn_addsynth_ptr->ctl_ptr;
  free(zyn_addsynth_ptr->notes_array);
  delete zyn_addsynth_ptr;
}

float percent_from_0_127(unsigned char value)
{
  return ((float)(value)/127.0)*100.0; // 0-127 -> percent
}

unsigned char percent_to_0_127(float value)
{
  return (unsigned char)roundf(value / 100.0 * 127.0);
}

float
zyn_addsynth_get_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_PANORAMA:
    return zyn_addsynth_ptr->panorama;
  case ZYNADD_PARAMETER_FLOAT_VOLUME:
    return percent_from_0_127(zyn_addsynth_ptr->params_ptr->GlobalPar.PVolume);
  case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING:
    return percent_from_0_127(zyn_addsynth_ptr->params_ptr->GlobalPar.PAmpVelocityScaleFunction);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH:
    return percent_from_0_127(zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchStrength);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_TIME:
    return percent_from_0_127(zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchTime);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH:
    return percent_from_0_127(zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchStretch);
  case ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING:
    return percent_from_0_127(zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchVelocitySensing);
  default:
    assert(0);
  }
}

void
zyn_addsynth_set_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter,
  float value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_FLOAT_PANORAMA:
    zyn_addsynth_ptr->panorama = value;
    return;
  case ZYNADD_PARAMETER_FLOAT_VOLUME:
    zyn_addsynth_ptr->params_ptr->GlobalPar.PVolume = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING:
    zyn_addsynth_ptr->params_ptr->GlobalPar.PAmpVelocityScaleFunction = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH:
    zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchStrength = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_TIME:
    zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchTime = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH:
    zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchStretch = percent_to_0_127(value);
    return;
  case ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING:
    zyn_addsynth_ptr->params_ptr->GlobalPar.PPunchVelocitySensing = percent_to_0_127(value);
    return;
  default:
    assert(0);
  }
}

BOOL
zyn_addsynth_get_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
    return zyn_addsynth_ptr->random_panorama;
  case ZYNADD_PARAMETER_BOOL_STEREO:
    return zyn_addsynth_ptr->stereo;
  case ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING:
    return zyn_addsynth_ptr->random_grouping;
  default:
    assert(0);
  }
}

void
zyn_addsynth_set_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter,
  BOOL value)
{
  switch (parameter)
  {
  case ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA:
    zyn_addsynth_ptr->random_panorama = value;
    return;
  case ZYNADD_PARAMETER_BOOL_STEREO:
    zyn_addsynth_ptr->stereo = value;
    return;
  case ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING:
    zyn_addsynth_ptr->random_grouping = value;
    return;
  default:
    assert(0);
  }
}
