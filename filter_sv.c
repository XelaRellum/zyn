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
#include "filter_common.h"
#include "filter_sv.h"
#include "util.h"

//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

/* log2(1000) = 9.95748 */
#define ZYN_FILTER_GET_REAL_FREQUENCY(freq_pitch) (pow(2.0, freqpitch + 9.96578428))

struct zyn_filter_sv
{
  float sample_rate;
  int type;                     /* The type of the filter, one of ZYN_FILTER_SV_TYPE_XXX */
  float frequency;              /* -5..5  */
  float q_factor;               /* 0..1 */
  float frequency_tracking;     /* -1..1 */
  int additional_stages;        /* how many times the filter is applied (0->1, 1->2, etc.) */
  float gain;                   /* gain, in dB */
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
  struct zyn_filter_sv * filter;

  float sample_rate;

  struct zyn_filter_sv_stage stages[MAX_FILTER_STAGES + 1];

  struct zyn_filter_sv_parameters parameters;

  zyn_sample_type interpolation_buffer[SOUND_BUFFER_SIZE];

  int additional_stages;        /* how many times the filter is applied (0->1, 1->2, etc.) */
  float frequency;              /* Frequency, before conversion to Hz */
  float frequency_real;         /* Frequency, in Hz */
  float q_factor;
  float q_factor_computed;
  int type;
    
  bool above_nyquist;                 /* this is true if the frequency is above the nyquist */
  bool old_above_nyquist;
  bool first_time;

  float note_base_frequency;
  float velocity_adjust;
};

bool
zyn_filter_sv_create(
  float sample_rate,
  float frequency,
  float q_factor,
  zyn_filter_sv_handle * handle_ptr)
{
  struct zyn_filter_sv * filter_ptr;

  filter_ptr = malloc(sizeof(struct zyn_filter_sv));
  if (filter_ptr == NULL)
  {
    return false;
  }

  filter_ptr->type = ZYN_FILTER_SV_TYPE_LOWPASS;
  filter_ptr->sample_rate = sample_rate;
  filter_ptr->frequency = frequency;
  filter_ptr->q_factor = q_factor;
  filter_ptr->frequency_tracking = 0.0;
  filter_ptr->additional_stages = 0;
  filter_ptr->gain = 0;

  *handle_ptr = (zyn_filter_sv_handle)filter_ptr;
  return true;
}

#define filter_ptr ((struct zyn_filter_sv *)filter_handle)

int
zyn_filter_sv_get_type(
  zyn_filter_sv_handle filter_handle)
{
  LOG_DEBUG("SV filter type is %d", filter_ptr->type);
  return filter_ptr->type;
}

void
zyn_filter_sv_set_type(
  zyn_filter_sv_handle filter_handle,
  int type)
{
  assert(type >= 0 && type < ZYN_FILTER_SV_TYPES_COUNT);
  filter_ptr->type = type;
  LOG_DEBUG("SV filter type set to %d", filter_ptr->type);
}

float
zyn_filter_sv_get_frequency(
  zyn_filter_sv_handle filter_handle)
{
  return filter_ptr->frequency;
}

void
zyn_filter_sv_set_frequency(
  zyn_filter_sv_handle filter_handle,
  float frequency)
{
  filter_ptr->frequency = frequency;
}

float
zyn_filter_sv_get_q_factor(
  zyn_filter_sv_handle filter_handle)
{
  return filter_ptr->q_factor;
}

void
zyn_filter_sv_set_q_factor(
  zyn_filter_sv_handle filter_handle,
  float q_factor)
{
  filter_ptr->q_factor = q_factor;
}

float
zyn_filter_sv_get_frequency_tracking(
  zyn_filter_sv_handle filter_handle)
{
  return filter_ptr->frequency_tracking;
}

void
zyn_filter_sv_set_frequency_tracking(
  zyn_filter_sv_handle filter_handle,
  float frequency_tracking)
{
  filter_ptr->frequency_tracking = frequency_tracking;
}

float
zyn_filter_sv_get_gain(
  zyn_filter_sv_handle filter_handle)
{
  return filter_ptr->gain;
}

void
zyn_filter_sv_set_gain(
  zyn_filter_sv_handle filter_handle,
  float gain)
{
  filter_ptr->gain = gain;
}

int
zyn_filter_sv_get_stages(
  zyn_filter_sv_handle filter_handle)
{
  return filter_ptr->additional_stages + 1;
}

void
zyn_filter_sv_set_stages(
  zyn_filter_sv_handle filter_handle,
  int stages)
{
  assert(stages > 0);
  assert(stages <= MAX_FILTER_STAGES);
  filter_ptr->additional_stages  = stages - 1;
}

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
  float sample_rate,
  float frequency,
  float q_factor,
  int additional_stages,
  struct zyn_filter_sv_parameters * parameters_ptr)
{
  parameters_ptr->f = frequency / sample_rate * 4.0;

  if (parameters_ptr->f > 0.99999)
  {
    parameters_ptr->f = 0.99999;
  }

  parameters_ptr->q = 1.0 - atan(sqrt(q_factor)) * 2.0 / PI;
  parameters_ptr->q = pow(parameters_ptr->q, 1.0 / (additional_stages + 1));
  parameters_ptr->q_sqrt = sqrt(parameters_ptr->q);
}

bool
zyn_filter_sv_processor_create(
  zyn_filter_sv_handle filter_handle,
  zyn_filter_processor_handle * processor_handle_ptr)
{
  struct zyn_filter_sv_processor * processor_ptr;

  processor_ptr = malloc(sizeof(struct zyn_filter_sv_processor));
  if (processor_ptr == NULL)
  {
    return false;
  }

  processor_ptr->filter = filter_ptr;
  processor_ptr->sample_rate = filter_ptr->sample_rate;

  *processor_handle_ptr = (zyn_filter_processor_handle)processor_ptr;
  return true;
}

#define processor_ptr ((struct zyn_filter_sv_processor *)processor_handle)

void
zyn_filter_sv_processor_destroy(
  zyn_filter_processor_handle processor_handle)
{
  free(processor_ptr);
}

void
zyn_filter_sv_process_single(
  int filter_type,
  zyn_sample_type * samples,
  struct zyn_filter_sv_stage * stage_ptr,
  struct zyn_filter_sv_parameters * parameters_ptr)
{
  int i;
  float * out_ptr;

  switch (filter_type)
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
zyn_filter_sv_processor_init(
  zyn_filter_processor_handle processor_handle,
  float note_base_frequency,
  float velocity_adjust)
{
  processor_ptr->note_base_frequency = note_base_frequency;
  processor_ptr->velocity_adjust = velocity_adjust;

  processor_ptr->first_time = true;
}

void
zyn_filter_sv_process(
  zyn_filter_processor_handle processor_handle,
  float frequency_adjust,
  zyn_sample_type * samples)
{
  int i;
  float x;
  float rap;
  bool nyquist_threshold;
  bool needs_interpolation;
  struct zyn_filter_sv_parameters interpolation_parameters;
  float frequency;
  float frequency_real;
  bool needs_coefs_recalculation;
  float gain;
  bool frequency_changed;

  LOG_DEBUG("---- SV process on %p, frequency_adjust is %f", processor_handle, frequency_adjust);

  needs_interpolation = false;
  needs_coefs_recalculation = false;

  /* center frequency */
  frequency = processor_ptr->filter->frequency;

  /* frequency tracking */
  frequency += log(processor_ptr->note_base_frequency / 440.0) * processor_ptr->filter->frequency_tracking / LOG_2;

  /* velocity adjust */
  frequency += processor_ptr->velocity_adjust;

  /* lfo/envelope adjust */
  frequency += frequency_adjust;

  frequency_changed = frequency != processor_ptr->frequency;

  if (frequency_changed)
  {
    LOG_DEBUG("Frequency really changed (%f != %f)", frequency, processor_ptr->frequency);
  }

  if (processor_ptr->first_time || frequency_changed)
  {
    /* convert to real frequency (Hz) */
    frequency_real = pow(2.0, frequency + 9.96578428); // log2(1000) = 9.95748

    if (frequency_real < 0.1)
    {
      frequency_real = 0.1;
    }
  }

  /* check if we need interpolation */
  if (!processor_ptr->first_time && frequency_changed)
  {
    if (frequency_real > processor_ptr->frequency_real)
    {
      rap = frequency_real / processor_ptr->frequency_real;
    }
    else
    {
      rap = processor_ptr->frequency_real / frequency_real;
    }

    processor_ptr->old_above_nyquist = processor_ptr->above_nyquist;
    processor_ptr->above_nyquist = frequency_real > (processor_ptr->sample_rate / 2 - 500.0);

    nyquist_threshold = ZYN_BOOL_XOR(processor_ptr->above_nyquist, processor_ptr->old_above_nyquist);

    // if the frequency is changed fast, it needs interpolation (now, filter and coeficients backup)
    if (rap > 3.0 || nyquist_threshold)
    {
      LOG_DEBUG("needs interpolation");
      needs_interpolation = true;
      interpolation_parameters = processor_ptr->parameters;
    }
  }

  /* now that we've checked for interpolation, update with current frequency values */

  if (processor_ptr->first_time || frequency_changed)
  {
    LOG_DEBUG("Updating changed frequency");
    LOG_DEBUG("Frequency is %f", frequency);
    LOG_DEBUG("Frequency real is %f", frequency_real);
    processor_ptr->frequency = frequency;
    processor_ptr->frequency_real = frequency_real;
    LOG_DEBUG("Frequency is %f", processor_ptr->frequency);
    LOG_DEBUG("Frequency real is %f", processor_ptr->frequency_real);

    needs_coefs_recalculation = true;
  }

  if (processor_ptr->first_time || processor_ptr->q_factor != processor_ptr->filter->q_factor)
  {
    LOG_DEBUG("Q factor changed");
    processor_ptr->q_factor_computed = exp(pow(processor_ptr->filter->q_factor, 2) * log(1000.0)) - 0.9;
    processor_ptr->q_factor = processor_ptr->filter->q_factor;
    needs_coefs_recalculation = true;
  }

  if (processor_ptr->first_time || processor_ptr->additional_stages != processor_ptr->filter->additional_stages)
  {
    LOG_DEBUG("Additional stages count changed");
    zyn_filter_sv_processor_cleanup(processor_ptr);
    processor_ptr->additional_stages = processor_ptr->filter->additional_stages;
    needs_coefs_recalculation = true;
  }

  if (processor_ptr->first_time || processor_ptr->type != processor_ptr->filter->type)
  {
    LOG_DEBUG("Type changed");
    processor_ptr->type = processor_ptr->filter->type;
    needs_coefs_recalculation = true;
  }

  if (needs_coefs_recalculation)
  {
    LOG_DEBUG("recalculating coefficients");
    zyn_filter_sv_processor_compute_coefs(
      processor_ptr->sample_rate,
      frequency_real,
      processor_ptr->q_factor_computed,
      processor_ptr->additional_stages,
      &processor_ptr->parameters);
  }

  if (needs_interpolation)
  {
    copy_buffer(processor_ptr->interpolation_buffer, samples, SOUND_BUFFER_SIZE);

    for (i = 0 ; i < processor_ptr->additional_stages + 1 ; i++)
    {
      zyn_filter_sv_process_single(
        processor_ptr->filter->type,
        processor_ptr->interpolation_buffer,
        processor_ptr->stages + i,
        &interpolation_parameters);
    }
  }

  for (i = 0 ; i < processor_ptr->additional_stages + 1 ; i++)
  {
      zyn_filter_sv_process_single(
        processor_ptr->filter->type,
        samples,
        processor_ptr->stages + i,
        &processor_ptr->parameters);
  }

  if (needs_interpolation)
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      x = i / (float)SOUND_BUFFER_SIZE;
      samples[i] = processor_ptr->interpolation_buffer[i] * (1.0 - x) + samples[i] * x;
    }
  }

  gain = dB2rap(processor_ptr->filter->gain);
  if (gain > 1.0)
  {
    gain = sqrt(gain);
  }

  LOG_DEBUG("Applying gain %f (%f dB)", gain, processor_ptr->filter->gain);

  multiply_buffer(samples, gain, SOUND_BUFFER_SIZE);

  if (processor_ptr->first_time)
  {
    processor_ptr->first_time = false;
  }
}
