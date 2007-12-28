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

#ifndef FILTER_SV_H__9B6BCECA_4958_4B6E_A4F6_D73B442FA1B6__INCLUDED
#define FILTER_SV_H__9B6BCECA_4958_4B6E_A4F6_D73B442FA1B6__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

typedef struct _zyn_filter_sv_handle { int unused; } * zyn_filter_sv_handle;
typedef struct { int unused; } * zyn_filter_sv_processor_handle;

bool
zyn_filter_sv_create(
  float sample_rate,
  float frequency,
  float q_factor,
  zyn_filter_sv_handle * handle_ptr);

int
zyn_filter_sv_get_type(
  zyn_filter_sv_handle filter_handle);

void
zyn_filter_sv_set_type(
  zyn_filter_sv_handle filter_handle,
  int type);

float
zyn_filter_sv_get_frequency(
  zyn_filter_sv_handle filter_handle);

void
zyn_filter_sv_set_frequency(
  zyn_filter_sv_handle filter_handle,
  float frequency);

float
zyn_filter_sv_get_q_factor(
  zyn_filter_sv_handle filter_handle);

void
zyn_filter_sv_set_q_factor(
  zyn_filter_sv_handle filter_handle,
  float q_factor);

float
zyn_filter_sv_get_frequency_tracking(
  zyn_filter_sv_handle filter_handle);

void
zyn_filter_sv_set_frequency_tracking(
  zyn_filter_sv_handle filter_handle,
  float frequency_tracking);

float
zyn_filter_sv_get_gain(
  zyn_filter_sv_handle filter_handle);

void
zyn_filter_sv_set_gain(
  zyn_filter_sv_handle filter_handle,
  float gain);

int
zyn_filter_sv_get_stages(
  zyn_filter_sv_handle filter_handle);

void
zyn_filter_sv_set_stages(
  zyn_filter_sv_handle filter_handle,
  int stages);

void
zyn_filter_sv_destroy(
  zyn_filter_sv_handle handle);

bool
zyn_filter_sv_processor_create(
  zyn_filter_sv_handle filter_handle,
  zyn_filter_sv_processor_handle * processor_handle_ptr);

void
zyn_filter_sv_processor_destroy(
  zyn_filter_sv_processor_handle processor_handle);

void
zyn_filter_sv_processor_init(
  zyn_filter_sv_processor_handle processor_handle,
  float note_base_frequency,
  float velocity_adjust);

void
zyn_filter_sv_process(
  zyn_filter_sv_processor_handle processor_handle,
  float frequency_adjust,       /* lfo/envelope adjust */
  zyn_sample_type *samples);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef FILTER_SV_H__9B6BCECA_4958_4B6E_A4F6_D73B442FA1B6__INCLUDED */
