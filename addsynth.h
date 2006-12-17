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

#ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED
#define ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

typedef void * zyn_addsynth_handle;

BOOL
zyn_addsynth_create(
  zyn_addsynth_handle * handle_ptr);

void
zyn_addsynth_get_audio_output(
  zyn_addsynth_handle handle,
  zyn_sample_type * buffer_left,
  zyn_sample_type * buffer_right);

void
zyn_addsynth_note_on(
  zyn_addsynth_handle handle,
  unsigned int note);

void
zyn_addsynth_note_off(
  zyn_addsynth_handle handle,
  unsigned int note);

void
zyn_addsynth_destroy(
  zyn_addsynth_handle handle);

float
zyn_addsynth_get_panorama(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_panorama(
  zyn_addsynth_handle handle,
  float value);

BOOL
zyn_addsynth_panorama_is_random(
  zyn_addsynth_handle handle);

void
zyn_addsynth_panorama_set_random(
  zyn_addsynth_handle handle,
  BOOL random);

float
zyn_addsynth_get_volume(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_volume(
  zyn_addsynth_handle handle,
  float value);

float
zyn_addsynth_get_velocity_sensing(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_velocity_sensing(
  zyn_addsynth_handle handle,
  float value);

float
zyn_addsynth_get_punch_strength(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_punch_strength(
  zyn_addsynth_handle handle,
  float value);

float
zyn_addsynth_get_punch_time(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_punch_time(
  zyn_addsynth_handle handle,
  float value);

float
zyn_addsynth_get_punch_stretch(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_punch_stretch(
  zyn_addsynth_handle handle,
  float value);

float
zyn_addsynth_get_punch_velocity_sensing(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_punch_velocity_sensing(
  zyn_addsynth_handle handle,
  float value);

BOOL
zyn_addsynth_is_stereo(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_stereo(
  zyn_addsynth_handle handle,
  BOOL stereo);

BOOL
zyn_addsynth_is_random_grouping(
  zyn_addsynth_handle handle);

void
zyn_addsynth_set_random_grouping(
  zyn_addsynth_handle handle,
  BOOL random_grouping);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED */
