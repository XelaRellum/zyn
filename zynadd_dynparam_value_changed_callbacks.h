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

#ifndef ZYNADD_DYNPARAM_VALUE_CHANGED_CALLBACKS_H__202DD0B9_EA0C_46A2_8735_481EFF9D4128__INCLUDED
#define ZYNADD_DYNPARAM_VALUE_CHANGED_CALLBACKS_H__202DD0B9_EA0C_46A2_8735_481EFF9D4128__INCLUDED

bool
zynadd_bool_parameter_changed(
  void * context,
  bool value);

bool
zynadd_float_parameter_changed(
  void * context,
  float value);

bool
zynadd_int_parameter_changed(
  void * context,
  signed int value);

bool
zynadd_shape_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

bool
zynadd_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

bool
zynadd_analog_filter_type_parameter_changed(
  void * context,
  const char * value,
  unsigned int value_index);

#endif /* #ifndef ZYNADD_DYNPARAM_VALUE_CHANGED_CALLBACKS_H__202DD0B9_EA0C_46A2_8735_481EFF9D4128__INCLUDED */
