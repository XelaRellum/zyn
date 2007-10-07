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

#ifndef ADDSYNTH_COMPONENT_H__19086C9D_82A0_42E0_87E4_8CA309CB382C__INCLUDED
#define ADDSYNTH_COMPONENT_H__19086C9D_82A0_42E0_87E4_8CA309CB382C__INCLUDED

typedef float
(* zyn_component_get_float_parameter)(
  zyn_addsynth_component component,
  unsigned int parameter);

typedef void
(* zyn_component_set_float_parameter)(
  void * context,
  unsigned int parameter,
  float value);

typedef signed int
(* zyn_component_get_int_parameter)(
  void * context,
  unsigned int parameter);

typedef void
(* zyn_component_set_int_parameter)(
  void * context,
  unsigned int parameter,
  signed int value);

typedef bool
(* zyn_component_get_bool_parameter)(
  void * context,
  unsigned int parameter);

typedef void
(* zyn_component_set_bool_parameter)(
  void * context,
  unsigned int parameter,
  bool value);

typedef unsigned int
(* zyn_component_get_shape_parameter)(
  void * context);

typedef void
(* zyn_component_set_shape_parameter)(
  void * context,
  unsigned int value);

typedef unsigned int
(* zyn_component_get_filter_type_parameter)(
  void * context);

typedef void
(* zyn_component_set_filter_type_parameter)(
  void * context,
  unsigned int value);

typedef unsigned int
(* zyn_component_get_analog_filter_type_parameter)(
  void * context);

typedef void
(* zyn_component_set_analog_filter_type_parameter)(
  void * context,
  unsigned int value);

struct zyn_component_descriptor
{
  void * context;

  zyn_component_get_float_parameter get_float;
  zyn_component_set_float_parameter set_float;
  zyn_component_get_int_parameter get_int;
  zyn_component_set_int_parameter set_int;
  zyn_component_get_bool_parameter get_bool;
  zyn_component_set_bool_parameter set_bool;
  zyn_component_get_shape_parameter get_shape;
  zyn_component_set_shape_parameter set_shape;
  zyn_component_get_filter_type_parameter get_filter_type;
  zyn_component_set_filter_type_parameter set_filter_type;
  zyn_component_get_analog_filter_type_parameter get_analog_filter_type;
  zyn_component_set_analog_filter_type_parameter set_analog_filter_type;
};

#define ZYN_INIT_COMPONENT(component_ptr, context_param, prefix)        \
  (component_ptr)->context = context_param;                             \
  (component_ptr)->get_float = prefix ## get_float;                     \
  (component_ptr)->set_float = prefix ## set_float;                     \
  (component_ptr)->get_int = prefix ## get_int;                         \
  (component_ptr)->set_int = prefix ## set_int;                         \
  (component_ptr)->get_bool = prefix ## get_bool;                       \
  (component_ptr)->set_bool = prefix ## set_bool;                       \
  (component_ptr)->get_shape = prefix ## get_shape;                     \
  (component_ptr)->set_shape = prefix ## set_shape;                     \
  (component_ptr)->get_filter_type = prefix ## get_filter_type;         \
  (component_ptr)->set_filter_type = prefix ## set_filter_type;         \
  (component_ptr)->get_analog_filter_type = prefix ## get_analog_filter_type; \
  (component_ptr)->set_analog_filter_type = prefix ## set_analog_filter_type; \

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

#ifdef __cplusplus
void
zyn_addsynth_component_init_amp_globals(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_addsynth * zyn_addsynth_ptr);

void
zyn_addsynth_component_init_amp_envelope(
  struct zyn_component_descriptor * component_ptr,
  EnvelopeParams * envelope_params_ptr);

void
zyn_addsynth_component_init_lfo(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_lfo_parameters * lfo_params_ptr);

void
zyn_addsynth_component_init_filter_globals(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_addsynth * zyn_addsynth_ptr);

void
zyn_addsynth_component_init_filter_envelope(
  struct zyn_component_descriptor * component_ptr,
  EnvelopeParams * envelope_params_ptr);

void
zyn_addsynth_component_init_frequency_globals(
  struct zyn_component_descriptor * component_ptr);

void
zyn_addsynth_component_init_frequency_envelope(
  struct zyn_component_descriptor * component_ptr,
  EnvelopeParams * envelope_params_ptr);

void
zyn_addsynth_component_init_voice_globals(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_addnote_voice_parameters * voice_params_ptr);
#endif

void
zyn_addsynth_component_init_portamento(
  struct zyn_component_descriptor * component_ptr,
  struct zyn_portamento * portamento_ptr);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_COMPONENT_H__19086C9D_82A0_42E0_87E4_8CA309CB382C__INCLUDED */
