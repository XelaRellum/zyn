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

#define ZYNADD_PARAMETER_FLOAT_PANORAMA                   0 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VOLUME                     1 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING           2 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH             3 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_TIME                 4 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH              5 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING     6 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_ATTACK             7 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_DECAY              8 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_SUSTAIN            9 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_RELEASE           10 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_STRETCH           11 /* 0 .. 200 */

#define ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA             0
#define ZYNADD_PARAMETER_BOOL_STEREO                      1
#define ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING             2
#define ZYNADD_PARAMETER_BOOL_AMP_ENV_FORCED_RELEASE      3
#define ZYNADD_PARAMETER_BOOL_AMP_ENV_LINEAR              4

float
zyn_addsynth_get_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter);

void
zyn_addsynth_set_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter,
  float value);

BOOL
zyn_addsynth_get_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter);

void
zyn_addsynth_set_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter,
  BOOL value);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED */
