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

#define ZYNADD_PARAMETER_FLOAT_PANORAMA                             0 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VOLUME                               1 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING                     2 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH                       3 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_TIME                           4 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH                        5 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING               6 /* 0 .. 100 */

#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_ATTACK                       7 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_DECAY                        8 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_SUSTAIN                      9 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_RELEASE                     10 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_ENV_STRETCH                     11 /* 0 .. 200 */

#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_FREQUENCY                   12 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_DEPTH                       13 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_START_PHASE                 14 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_DELAY                       15 /* 0 .. 4?, seconds */
#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_STRETCH                     16 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_DEPTH_RANDOMNESS            17 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_AMP_LFO_FREQUENCY_RANDOMNESS        18 /* 0 .. 100 */

#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_ATTACK_VALUE             19 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_ATTACK_DURATION          20 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_DECAY_VALUE              21 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_DECAY_DURATION           22 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_RELEASE_VALUE            23 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_RELEASE_DURATION         24 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_ENV_STRETCH                  25 /* 0 .. 200 */

#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_FREQUENCY                26 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_DEPTH                    27 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_START_PHASE              28 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_DELAY                    29 /* 0 .. 4?, seconds */
#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_STRETCH                  30 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_DEPTH_RANDOMNESS         31 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FILTER_LFO_FREQUENCY_RANDOMNESS     32 /* 0 .. 100 */

#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_FREQUENCY             33 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_DEPTH                 34 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_START_PHASE           35 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_DELAY                 36 /* 0 .. 4?, seconds */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_STRETCH               37 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_DEPTH_RANDOMNESS      38 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_LFO_FREQUENCY_RANDOMNESS  39 /* 0 .. 100 */

#define ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA                       0
#define ZYNADD_PARAMETER_BOOL_STEREO                                1
#define ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING                       2
#define ZYNADD_PARAMETER_BOOL_AMP_ENV_FORCED_RELEASE                3
#define ZYNADD_PARAMETER_BOOL_AMP_ENV_LINEAR                        4
#define ZYNADD_PARAMETER_BOOL_AMP_LFO_RANDOM_START_PHASE            5
#define ZYNADD_PARAMETER_BOOL_AMP_LFO_RANDOM_DEPTH                  6
#define ZYNADD_PARAMETER_BOOL_AMP_LFO_RANDOM_FREQUENCY              7
#define ZYNADD_PARAMETER_BOOL_FILTER_ENV_FORCED_RELEASE             8
#define ZYNADD_PARAMETER_BOOL_FILTER_LFO_RANDOM_START_PHASE         9
#define ZYNADD_PARAMETER_BOOL_FILTER_LFO_RANDOM_DEPTH              10
#define ZYNADD_PARAMETER_BOOL_FILTER_LFO_RANDOM_FREQUENCY          11
#define ZYNADD_PARAMETER_BOOL_FREQUENCY_LFO_RANDOM_START_PHASE     12
#define ZYNADD_PARAMETER_BOOL_FREQUENCY_LFO_RANDOM_DEPTH           13
#define ZYNADD_PARAMETER_BOOL_FREQUENCY_LFO_RANDOM_FREQUENCY       14

#define ZYNADD_PARAMETER_SHAPE_AMP_LFO                              0
#define ZYNADD_PARAMETER_SHAPE_FILTER_LFO                           1
#define ZYNADD_PARAMETER_SHAPE_FREQUENCY_LFO                        2

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

unsigned int
zyn_addsynth_get_shape_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter);

void
zyn_addsynth_set_shape_parameter(
  zyn_addsynth_handle handle,
  unsigned int parameter,
  unsigned int value);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED */
