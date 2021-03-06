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

#ifndef COMMON_H__B9D0FFDE_1F2E_4378_9040_C1B0802DDC9E__INCLUDED
#define COMMON_H__B9D0FFDE_1F2E_4378_9040_C1B0802DDC9E__INCLUDED

#include <stddef.h>

/* What float type I use for internal sampledata */
#define zyn_sample_type float
#define REALTYPE float          /* legacy */

/* 
 * The size of a sound buffer (or the granularity)
 * All internal transfer of sound data use buffer of this size
 * All parameters are constant during this period of time, exception
 * some parameters(like amplitudes) which are linear interpolated.
 * If you increase this you'll ecounter big latencies, but if you 
 * decrease this the CPU requirements gets high.
 */
#define SOUND_BUFFER_SIZE 128

/*
 * How is applied the velocity sensing
 */
#define VELOCITY_MAX_SCALE 8.0

#define ZYN_LFO_SHAPE_TYPE_SINE        0
#define ZYN_LFO_SHAPE_TYPE_TRIANGLE    1
#define ZYN_LFO_SHAPE_TYPE_SQUARE      2
#define ZYN_LFO_SHAPE_TYPE_RAMP_UP     3
#define ZYN_LFO_SHAPE_TYPE_RAMP_DOWN   4
#define ZYN_LFO_SHAPE_TYPE_EXP_DOWN_1  5
#define ZYN_LFO_SHAPE_TYPE_EXP_DOWN_2  6
#define ZYN_LFO_SHAPES_COUNT           7

#define ZYN_FILTER_TYPE_ANALOG         0
#define ZYN_FILTER_TYPE_FORMANT        1
#define ZYN_FILTER_TYPE_STATE_VARIABLE 2
#define ZYN_FILTER_TYPES_COUNT         3

#define ZYN_FILTER_ANALOG_TYPE_LPF1    0 /* LPF 1 pole */
#define ZYN_FILTER_ANALOG_TYPE_HPF1    1 /* HPF 1 pole */
#define ZYN_FILTER_ANALOG_TYPE_LPF2    2 /* LPF 2 poles */
#define ZYN_FILTER_ANALOG_TYPE_HPF2    3 /* HPF 2 poles */
#define ZYN_FILTER_ANALOG_TYPE_BPF2    4 /* BPF 2 poles */
#define ZYN_FILTER_ANALOG_TYPE_NF2     5 /* NOTCH 2 poles */
#define ZYN_FILTER_ANALOG_TYPE_PKF2    6 /* PEAK (2 poles) */
#define ZYN_FILTER_ANALOG_TYPE_LSH2    7 /* Low Shelf - 2 poles */
#define ZYN_FILTER_ANALOG_TYPE_HSH2    8 /* High Shelf - 2 poles */
#define ZYN_FILTER_ANALOG_TYPES_COUNT  9

#define ZYN_FILTER_SV_TYPE_LOWPASS     0
#define ZYN_FILTER_SV_TYPE_HIGHPASS    1
#define ZYN_FILTER_SV_TYPE_BANDPASS    2
#define ZYN_FILTER_SV_TYPE_NOTCH       3
#define ZYN_FILTER_SV_TYPES_COUNT      4

#define ZYN_OSCILLATOR_BASE_FUNCTION_SINE               0
#define ZYN_OSCILLATOR_BASE_FUNCTION_TRIANGLE           1
#define ZYN_OSCILLATOR_BASE_FUNCTION_PULSE              2
#define ZYN_OSCILLATOR_BASE_FUNCTION_SAW                3
#define ZYN_OSCILLATOR_BASE_FUNCTION_POWER              4
#define ZYN_OSCILLATOR_BASE_FUNCTION_GAUSS              5
#define ZYN_OSCILLATOR_BASE_FUNCTION_DIODE              6
#define ZYN_OSCILLATOR_BASE_FUNCTION_ABS_SINE           7
#define ZYN_OSCILLATOR_BASE_FUNCTION_PULSE_SINE         8
#define ZYN_OSCILLATOR_BASE_FUNCTION_STRETCH_SINE       9
#define ZYN_OSCILLATOR_BASE_FUNCTION_CHIRP             10
#define ZYN_OSCILLATOR_BASE_FUNCTION_ABS_STRETCH_SINE  11
#define ZYN_OSCILLATOR_BASE_FUNCTION_CHEBYSHEV         12
#define ZYN_OSCILLATOR_BASE_FUNCTION_SQRT              13
#define ZYN_OSCILLATOR_BASE_FUNCTIONS_COUNT            14

#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_NONE              0
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_ATAN              1
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_ASYM1             2
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_POW               3
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_SINE              4
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_QUANTISIZE        5
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_ZIGZAG            6
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_LIMITER           7
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_UPPER_LIMITER     8
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_LOWER_LIMITER     9
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_INVERSE_LIMITER  10
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_CLIP             11
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_ASYM2            12
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_POW2             13
#define ZYN_OSCILLATOR_WAVESHAPE_TYPE_SIGMOID          14
#define ZYN_OSCILLATOR_WAVESHAPE_TYPES_COUNT           15

#define ZYN_OSCILLATOR_SPECTRUM_ADJUST_TYPE_NONE            0
#define ZYN_OSCILLATOR_SPECTRUM_ADJUST_TYPE_POW             1
#define ZYN_OSCILLATOR_SPECTRUM_ADJUST_TYPE_THERSHOLD_DOWN  2
#define ZYN_OSCILLATOR_SPECTRUM_ADJUST_TYPE_THERSHOLD_UP    3
#define ZYN_OSCILLATOR_SPECTRUM_ADJUST_TYPES_COUNT          4

#define ZYN_DETUNE_TYPE_GLOBAL        0 /* use global detune type, valid only for voice parameters */
#define ZYN_DETUNE_TYPE_L35CENTS      1
#define ZYN_DETUNE_TYPE_L10CENTS      2
#define ZYN_DETUNE_TYPE_E100CENTS     3
#define ZYN_DETUNE_TYPE_E1200CENTS    4

#define ZYN_DETUNE_MODE_NORMAL           0 /* the base frequency is normal one */
#define ZYN_DETUNE_MODE_FIXED_440        1 /* the base frequency is fixed to 440 Hz */
#define ZYN_DETUNE_MODE_EQUAL_TEMPERATE  2 /* Equal temperate */

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef COMMON_H__B9D0FFDE_1F2E_4378_9040_C1B0802DDC9E__INCLUDED */
