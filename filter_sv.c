/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2007 Nedko Arnaudov <nedko@arnaudov.name>
 *   Copyright (C) 2002-2005 Nasca Octavian Paul
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

#include <stddef.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>

#include "common.h"
#include "globals.h"
#include "filter_sv.h"
#include "util.h"

/* log2(1000) = 9.95748 */
#define ZYN_FILTER_GET_REAL_FREQUENCY(freq_pitch) (pow(2.0, freqpitch + 9.96578428))

struct zyn_filter_sv
{
  float sample_rate;
  int type;                     /* The type of the filter, one of ZYN_FILTER_SV_TYPE_XXX */
};

struct zyn_filter_sv_stage
{
  float low;
  float high;
  float band;
  float notch;
};

struct zyn_filter_sv_parameters
{
  float f;
  float q;
  float q_sqrt;
};

struct zyn_filter_sv_processor
{
  float sample_rate;

  struct zyn_filter_sv_stage stages[MAX_FILTER_STAGES + 1];

  struct zyn_filter_sv_parameters parameters;
  struct zyn_filter_sv_parameters interpolation_parameters;

  zyn_sample_type interpolation_buffer[SOUND_BUFFER_SIZE];

  int type;                     /* The type of the filter, one of ZYN_FILTER_SV_TYPE_XXX */
  int additional_stages;        /* how many times the filter is applied (0->1, 1->2, etc.) */
  float freq;                   /* Frequency given in Hz */
  float q;                      /* Q factor (resonance or Q factor) */
  float gain;
    
  bool above_nyquist;                 /* this is true if the frequency is above the nyquist */
  bool old_above_nyquist;
  bool needs_interpolation;
  bool firsttime;
};

bool
zyn_filter_sv_create(
  float sample_rate,
  zyn_filter_sv_handle * handle_ptr)
{
  struct zyn_filter_sv * filter_ptr;

  filter_ptr = malloc(sizeof(struct zyn_filter_sv));
  if (filter_ptr == NULL)
  {
    return false;
  }

  filter_ptr->sample_rate = sample_rate;

  *handle_ptr = (zyn_filter_sv_handle)filter_ptr;
  return true;
}

#define filter_ptr ((struct zyn_filter_sv *)filter_handle)

void
zyn_filter_sv_destroy(
  zyn_filter_sv_handle filter_handle)
{
  free(filter_ptr);
}

void
zyn_filter_sv_processor_cleanup(
  struct zyn_filter_sv_processor * processor_ptr)
{
  int i;

  for (i = 0 ; i < MAX_FILTER_STAGES + 1 ; i++)
  {
    processor_ptr->stages[i].low = 0.0;
    processor_ptr->stages[i].high = 0.0;
    processor_ptr->stages[i].band = 0.0;
    processor_ptr->stages[i].notch = 0.0;
  }

  processor_ptr->old_above_nyquist = false;
  processor_ptr->above_nyquist = false;
}

void
zyn_filter_sv_processor_compute_coefs(
  struct zyn_filter_sv_processor * processor_ptr)
{
  processor_ptr->parameters.f = processor_ptr->freq / processor_ptr->sample_rate * 4.0;

  if (processor_ptr->parameters.f > 0.99999)
  {
    processor_ptr->parameters.f = 0.99999;
  }

  processor_ptr->parameters.q = 1.0 - atan(sqrt(processor_ptr->q)) * 2.0 / PI;
  processor_ptr->parameters.q = pow(processor_ptr->parameters.q, 1.0 / (processor_ptr->additional_stages + 1));
  processor_ptr->parameters.q_sqrt = sqrt(processor_ptr->parameters.q);
}

bool
zyn_filter_sv_processor_create(
  zyn_filter_sv_handle filter_handle,
  zyn_filter_sv_processor_handle * processor_handle_ptr)
{
  struct zyn_filter_sv_processor * processor_ptr;

  processor_ptr = malloc(sizeof(struct zyn_filter_sv_processor));
  if (processor_ptr == NULL)
  {
    return false;
  }

  processor_ptr->sample_rate = filter_ptr->sample_rate;
  processor_ptr->type = filter_ptr->type;
  processor_ptr->needs_interpolation = false;

  *processor_handle_ptr = (zyn_filter_sv_processor_handle)processor_ptr;
  return true;
}

#define processor_ptr ((struct zyn_filter_sv_processor *)processor_handle)

void
zyn_filter_sv_processor_destroy(
  zyn_filter_sv_processor_handle processor_handle)
{
  free(processor_ptr);
}

void
zyn_filter_sv_process_single(
  zyn_filter_sv_processor_handle processor_handle,
  zyn_sample_type * samples,
  struct zyn_filter_sv_stage * stage_ptr,
  struct zyn_filter_sv_parameters * parameters_ptr)
{
  int i;
  float * out_ptr;

  switch(processor_ptr->type)
  {
  case ZYN_FILTER_SV_TYPE_LOWPASS:
    out_ptr = &stage_ptr->low;
    break;
  case ZYN_FILTER_SV_TYPE_HIGHPASS:
    out_ptr = &stage_ptr->high;
    break;
  case ZYN_FILTER_SV_TYPE_BANDPASS:
    out_ptr = &stage_ptr->band;
    break;
  case ZYN_FILTER_SV_TYPE_NOTCH:
    out_ptr = &stage_ptr->notch;
    break;
  default:
    assert(0);
    return;
  }

  for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    stage_ptr->low = stage_ptr->low + parameters_ptr->f * stage_ptr->band;
    stage_ptr->high = parameters_ptr->q_sqrt * samples[i] - stage_ptr->low - parameters_ptr->q * stage_ptr->band;
    stage_ptr->band = parameters_ptr->f * stage_ptr->high + stage_ptr->band;
    stage_ptr->notch = stage_ptr->high + stage_ptr->low;

    samples[i] = *out_ptr;
  }
}

void
zyn_filter_sv_process(
  zyn_filter_sv_processor_handle processor_handle,
  zyn_sample_type * samples)
{
  int i;
  float x;

  if (processor_ptr->needs_interpolation)
  {
    copy_buffer(processor_ptr->interpolation_buffer, samples, SOUND_BUFFER_SIZE);

    for (i = 0 ; i < processor_ptr->additional_stages + 1 ; i++)
    {
      zyn_filter_sv_process_single(
        processor_handle,
        processor_ptr->interpolation_buffer,
        processor_ptr->stages + i,
        &processor_ptr->interpolation_parameters);
    }
  }

  for (i = 0 ; i < processor_ptr->additional_stages + 1 ; i++)
  {
      zyn_filter_sv_process_single(
        processor_handle,
        samples,
        processor_ptr->stages + i,
        &processor_ptr->parameters);
  }

  if (processor_ptr->needs_interpolation)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      x = i / (float)SOUND_BUFFER_SIZE;
      samples[i] = processor_ptr->interpolation_buffer[i] * (1.0 - x) + samples[i] * x;
    }

    processor_ptr->needs_interpolation = false;
  }

  multiply_buffer(samples, processor_ptr->gain, SOUND_BUFFER_SIZE);
}
