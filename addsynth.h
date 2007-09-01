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

#ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED
#define ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

typedef void * zyn_addsynth_handle;

bool
zyn_addsynth_create(
  unsigned int voices_count,
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

#define ZYNADD_COMPONENT_AMP_GLOBALS              0
#define ZYNADD_COMPONENT_AMP_ENV                  1
#define ZYNADD_COMPONENT_AMP_LFO                  2
#define ZYNADD_COMPONENT_FILTER_GLOBALS           3
#define ZYNADD_COMPONENT_FILTER_ENV               4
#define ZYNADD_COMPONENT_FILTER_LFO               5
#define ZYNADD_COMPONENT_FREQUENCY_GLOBALS        6
#define ZYNADD_COMPONENT_FREQUENCY_ENV            7
#define ZYNADD_COMPONENT_FREQUENCY_LFO            8

/* float - globals */
#define ZYNADD_PARAMETER_FLOAT_PANORAMA                    0 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VOLUME                      1 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING            2 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH              3 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_TIME                  4 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH               5 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING      6 /* 0 .. 100 */

/* float - envelope */
#define ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE            7 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION         8 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_DECAY_VALUE             9 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION         10 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_SUSTAIN_VALUE          11 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE          12 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION       13 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_STRETCH                14 /* 0 .. 200 */

/* float - lfo */
#define ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY              15 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_LFO_DEPTH                  16 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_LFO_START_PHASE            17 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_LFO_DELAY                  18 /* 0 .. 4?, seconds */
#define ZYNADD_PARAMETER_FLOAT_LFO_STRETCH                19 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_LFO_DEPTH_RANDOMNESS       20 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY_RANDOMNESS   21 /* 0 .. 100 */

#define ZYNADD_PARAMETER_FLOAT_FREQUNECY                  22 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_Q_FACTOR                   23 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_AMOUNT    24 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_FUNCTION  25 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING         26 /* -1 .. 1 */

/* bool - globals */
#define ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA                       0
#define ZYNADD_PARAMETER_BOOL_STEREO                                1
#define ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING                       2

/* bool - envelope */
#define ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE                3
#define ZYNADD_PARAMETER_BOOL_ENV_LINEAR                        4

/* bool - lfo */
#define ZYNADD_PARAMETER_BOOL_LFO_RANDOM_START_PHASE            5
#define ZYNADD_PARAMETER_BOOL_LFO_RANDOM_DEPTH                  6
#define ZYNADD_PARAMETER_BOOL_LFO_RANDOM_FREQUENCY              7

#define ZYNADD_PARAMETER_INT_STAGES                             0 /* 1 .. 5 */

float
zyn_addsynth_get_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter);

void
zyn_addsynth_set_float_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter,
  float value);

signed int
zyn_addsynth_get_int_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter);

void
zyn_addsynth_set_int_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter,
  signed int value);

bool
zyn_addsynth_get_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter);

void
zyn_addsynth_set_bool_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int parameter,
  bool value);

unsigned int
zyn_addsynth_get_shape_parameter(
  zyn_addsynth_handle handle,
  unsigned int component);

void
zyn_addsynth_set_shape_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int value);

unsigned int
zyn_addsynth_get_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component);

void
zyn_addsynth_set_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int value);

unsigned int
zyn_addsynth_get_analog_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component);

void
zyn_addsynth_set_analog_filter_type_parameter(
  zyn_addsynth_handle handle,
  unsigned int component,
  unsigned int value);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED */
