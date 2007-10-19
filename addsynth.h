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

typedef void * zyn_addsynth_component;

bool
zyn_addsynth_create(
  float sample_rate,
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
#define ZYNADD_COMPONENT_PORTAMENTO               9

#define ZYNADD_GLOBAL_COMPONENTS_COUNT           10

#define ZYNADD_COMPONENT_VOICE_GLOBALS            0
#define ZYNADD_COMPONENT_VOICE_OSCILLATOR         1

#define ZYNADD_VOICE_COMPONENTS_COUNT             2

/* float - reused */
#define ZYNADD_PARAMETER_REUSED_OFFSET                   100
#define ZYNADD_PARAMETER_FLOAT_VOLUME           (ZYNADD_PARAMETER_REUSED_OFFSET + 0) /* 0 .. 100 */

/* float - globals */
#define ZYNADD_PARAMETER_FLOAT_PANORAMA                    0 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING            1 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRENGTH              2 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_TIME                  3 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_STRETCH               4 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PUNCH_VELOCITY_SENSING      5 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_PITCH_BEND_RANGE            6 /* -6400 .. 6400, in cents */
#define ZYNADD_PARAMETER_FLOAT_PITCH_BEND                  7 /* -1 .. 1 */

/* float - envelope */
#define ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_VALUE            0 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_ATTACK_DURATION         1 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_DECAY_VALUE             2 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_DECAY_DURATION          3 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_SUSTAIN_VALUE           4 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_VALUE           5 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_RELEASE_DURATION        6 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_ENV_STRETCH                 7 /* 0 .. 200 */

/* float - lfo */
#define ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY               0 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_LFO_DEPTH                   1 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_LFO_START_PHASE             2 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_LFO_DELAY                   3 /* 0 .. 4?, seconds */
#define ZYNADD_PARAMETER_FLOAT_LFO_STRETCH                 4 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_LFO_DEPTH_RANDOMNESS        5 /* 0 .. 100 */
#define ZYNADD_PARAMETER_FLOAT_LFO_FREQUENCY_RANDOMNESS    6 /* 0 .. 100 */

/* float filter */
#define ZYNADD_PARAMETER_FLOAT_FREQUNECY                   0 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_Q_FACTOR                    1 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_AMOUNT     2 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_VELOCITY_SENSING_FUNCTION   3 /* -1 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_FREQUENCY_TRACKING          4 /* -1 .. 1 */

/* float - portamento */
#define ZYNADD_PARAMETER_FLOAT_PORTAMENTO_TIME             0 /* 0 .. 1 */
#define ZYNADD_PARAMETER_FLOAT_PORTAMENTO_TIME_STRETCH     1 /* -1 .. 1 */

/* bool - globals */
#define ZYNADD_PARAMETER_BOOL_RANDOM_PANORAMA                       0
#define ZYNADD_PARAMETER_BOOL_STEREO                                1
#define ZYNADD_PARAMETER_BOOL_RANDOM_GROUPING                       2

/* bool - envelope */
#define ZYNADD_PARAMETER_BOOL_ENV_FORCED_RELEASE                0
#define ZYNADD_PARAMETER_BOOL_ENV_LINEAR                        1

/* bool - lfo */
#define ZYNADD_PARAMETER_BOOL_LFO_RANDOM_START_PHASE            0
#define ZYNADD_PARAMETER_BOOL_LFO_RANDOM_DEPTH                  1
#define ZYNADD_PARAMETER_BOOL_LFO_RANDOM_FREQUENCY              2

/* bool - portamento */
#define ZYNADD_PARAMETER_BOOL_PORTAMENTO_ENABLED                0
#define ZYNADD_PARAMETER_BOOL_PORTAMENTO_PITCH_THRESHOLD_ABOVE  1

/* bool - voice */
#define ZYNADD_PARAMETER_BOOL_RESONANCE                         0
#define ZYNADD_PARAMETER_BOOL_WHITE_NOISE                       1

#define ZYNADD_PARAMETER_ENUM_LFO_SHAPE                      1000
#define ZYNADD_PARAMETER_ENUM_FILTER_CATEGORY                1001
#define ZYNADD_PARAMETER_ENUM_ANALOG_FILTER_TYPE             1002
#define ZYNADD_PARAMETER_ENUM_OSCILLATOR_BASE_FUNCTION       1003
#define ZYNADD_PARAMETER_ENUM_OSCILLATOR_WAVESHAPE_TYPE      1004

#define ZYNADD_PARAMETER_INT_STAGES                             0 /* 1 .. 5 */

#define ZYNADD_PARAMETER_INT_PORTAMENTO_PITCH_THRESHOLD  1 /* 0 .. 127 */

zyn_addsynth_component
zyn_addsynth_get_global_component(
  zyn_addsynth_handle handle,
  unsigned int component);

zyn_addsynth_component
zyn_addsynth_get_voice_component(
  zyn_addsynth_handle handle,
  unsigned int component);

float
zyn_addsynth_get_float_parameter(
  zyn_addsynth_component component,
  unsigned int parameter);

void
zyn_addsynth_set_float_parameter(
  zyn_addsynth_component component,
  unsigned int parameter,
  float value);

signed int
zyn_addsynth_get_int_parameter(
  zyn_addsynth_component component,
  unsigned int parameter);

void
zyn_addsynth_set_int_parameter(
  zyn_addsynth_component component,
  unsigned int parameter,
  signed int value);

bool
zyn_addsynth_get_bool_parameter(
  zyn_addsynth_component component,
  unsigned int parameter);

void
zyn_addsynth_set_bool_parameter(
  zyn_addsynth_component component,
  unsigned int parameter,
  bool value);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_H__D1C82A9B_D028_4BAE_9D98_BEC4DFCD0240__INCLUDED */
