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

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "lv2.h"
#include "lv2dynparam.h"
#include "dynparam.h"
#include "list.h"
#include "dynparam_internal.h"

void
lv2dynparam_plugin_parameter_free(struct lv2dynparam_plugin_parameter * param_ptr)
{
  free(param_ptr->name);
  free(param_ptr);
}

#define parameter_ptr ((struct lv2dynparam_plugin_parameter *)parameter)

unsigned char
lv2dynparam_plugin_parameter_get_type_uri(
  lv2dynparam_parameter_handle parameter,
  char * buffer,
  size_t buffer_size)
{
  size_t s;
  const char * uri;

  switch (parameter_ptr->type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    uri = LV2DYNPARAM_PARAMETER_TYPE_FLOAT_URI;
    break;
  case LV2DYNPARAM_PARAMETER_TYPE_INT:
    uri = LV2DYNPARAM_PARAMETER_TYPE_INT_URI;
    break;
  case LV2DYNPARAM_PARAMETER_TYPE_NOTE:
    uri = LV2DYNPARAM_PARAMETER_TYPE_NOTE_URI;
    break;
  case LV2DYNPARAM_PARAMETER_TYPE_STRING:
    uri = LV2DYNPARAM_PARAMETER_TYPE_STRING_URI;
    break;
  case LV2DYNPARAM_PARAMETER_TYPE_FILENAME:
    uri = LV2DYNPARAM_PARAMETER_TYPE_FILENAME_URI;
    break;
  case LV2DYNPARAM_PARAMETER_TYPE_BOOLEAN:
    uri = LV2DYNPARAM_PARAMETER_TYPE_BOOLEAN_URI;
    break;
  default:
    return FALSE;
  }

  s = strlen(uri);

  s++;

  if (s > buffer_size)
  {
    return FALSE;
  }

  memcpy(buffer, uri, s);

  return TRUE;
}

unsigned char
lv2dynparam_plugin_parameter_get_name(
  lv2dynparam_parameter_handle parameter,
  char * buffer,
  size_t buffer_size)
{
  size_t s;

  s = strlen(parameter_ptr->name);

  s++;

  if (s > buffer_size)
  {
    return FALSE;
  }

  memcpy(buffer, parameter_ptr->name, s);

  return TRUE;
}

void
lv2dynparam_plugin_parameter_get_value(
  lv2dynparam_parameter_handle parameter,
  void ** value_buffer)
{
  switch (parameter_ptr->type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    *value_buffer = &parameter_ptr->data.fpoint.value;
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_INT:
    *value_buffer = &parameter_ptr->data.integer.value;
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_NOTE:
    *value_buffer = &parameter_ptr->data.note.value;
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_STRING:
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_FILENAME:
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_BOOLEAN:
    *value_buffer = &parameter_ptr->data.boolean;
    return;
  }
}

void
lv2dynparam_plugin_parameter_get_range(
  lv2dynparam_parameter_handle parameter,
  void ** value_min_buffer,
  void ** value_max_buffer)
{
  switch (parameter_ptr->type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    *value_min_buffer = &parameter_ptr->data.fpoint.min;
    *value_max_buffer = &parameter_ptr->data.fpoint.max;
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_INT:
    *value_min_buffer = &parameter_ptr->data.integer.min;
    *value_max_buffer = &parameter_ptr->data.integer.max;
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_NOTE:
    *value_min_buffer = &parameter_ptr->data.note.min;
    *value_max_buffer = &parameter_ptr->data.note.max;
    return;
  }
}

void
lv2dynparam_plugin_parameter_change(
  lv2dynparam_parameter_handle parameter)
{
  printf("lv2dynparam_plugin_parameter_change() called.\n");

  switch (parameter_ptr->type)
  {
  case LV2DYNPARAM_PARAMETER_TYPE_FLOAT:
    parameter_ptr->plugin_callback.fpoint(parameter_ptr->plugin_callback_context, parameter_ptr->data.fpoint.value);
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_INT:
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_NOTE:
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_STRING:
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_FILENAME:
    return;
  case LV2DYNPARAM_PARAMETER_TYPE_BOOLEAN:
    parameter_ptr->plugin_callback.boolean(parameter_ptr->plugin_callback_context, parameter_ptr->data.boolean);
    return;
  }
}

void
lv2dynparam_plugin_param_notify(
  struct lv2dynparam_plugin_instance * instance_ptr,
  struct lv2dynparam_plugin_group * group_ptr,
  struct lv2dynparam_plugin_parameter * param_ptr)
{
  if (instance_ptr->host_callbacks == NULL)
  {
    /* Host not attached */
    return;
  }

  if (param_ptr->host_notified)
  {
    /* Already notified */
    return;
  }

  if (instance_ptr->host_callbacks->parameter_appear(
        instance_ptr->host_context,
        group_ptr->host_context,
        param_ptr,
        &param_ptr->host_context))
  {
    param_ptr->host_notified = TRUE;
  }
}

#define instance_ptr ((struct lv2dynparam_plugin_instance *)instance_handle)

BOOL
lv2dynparam_plugin_param_boolean_add(
  lv2dynparam_plugin_instance instance_handle,
  lv2dynparam_plugin_group group,
  const char * name,
  int value,
  lv2dynparam_plugin_param_boolean_changed callback,
  void * callback_context,
  lv2dynparam_plugin_parameter * param_handle_ptr)
{
  struct lv2dynparam_plugin_parameter * param_ptr;
  struct lv2dynparam_plugin_group * group_ptr;

  if (group == NULL)
  {
    group_ptr = &instance_ptr->root_group;
  }
  else
  {
    group_ptr = (struct lv2dynparam_plugin_group *)group;
  }

  param_ptr = malloc(sizeof(struct lv2dynparam_plugin_parameter));
  if (param_ptr == NULL)
  {
    goto fail;
  }

  param_ptr->type = LV2DYNPARAM_PARAMETER_TYPE_BOOLEAN;

  param_ptr->name = strdup(name);
  if (param_ptr->name == NULL)
  {
    goto fail_free_param;
  }

  param_ptr->data.boolean = value;
  param_ptr->plugin_callback.boolean = callback;
  param_ptr->plugin_callback_context = callback_context;

  param_ptr->host_notified = FALSE;

  lv2dynparam_plugin_param_notify(instance_ptr, group_ptr, param_ptr);

  list_add_tail(&param_ptr->siblings, &group_ptr->child_parameters);

  *param_handle_ptr = (lv2dynparam_parameter_handle)param_ptr;

  return TRUE;

fail_free_param:
  free(param_ptr);

fail:
  return FALSE;
}

BOOL
lv2dynparam_plugin_param_float_add(
  lv2dynparam_plugin_instance instance_handle,
  lv2dynparam_plugin_group group,
  const char * name,
  float value,
  float min,
  float max,
  lv2dynparam_plugin_param_float_changed callback,
  void * callback_context,
  lv2dynparam_plugin_parameter * param_handle_ptr)
{
  struct lv2dynparam_plugin_parameter * param_ptr;
  struct lv2dynparam_plugin_group * group_ptr;

  if (group == NULL)
  {
    group_ptr = &instance_ptr->root_group;
  }
  else
  {
    group_ptr = (struct lv2dynparam_plugin_group *)group;
  }

  param_ptr = malloc(sizeof(struct lv2dynparam_plugin_parameter));
  if (param_ptr == NULL)
  {
    goto fail;
  }

  param_ptr->type = LV2DYNPARAM_PARAMETER_TYPE_FLOAT;

  param_ptr->name = strdup(name);
  if (param_ptr->name == NULL)
  {
    goto fail_free_param;
  }

  param_ptr->data.fpoint.value = value;
  param_ptr->data.fpoint.min = min;
  param_ptr->data.fpoint.max = max;
  param_ptr->plugin_callback.fpoint = callback;
  param_ptr->plugin_callback_context = callback_context;

  param_ptr->host_notified = FALSE;

  lv2dynparam_plugin_param_notify(instance_ptr, group_ptr, param_ptr);

  list_add_tail(&param_ptr->siblings, &group_ptr->child_parameters);

  *param_handle_ptr = (lv2dynparam_parameter_handle)param_ptr;

  return TRUE;

fail_free_param:
  free(param_ptr);

fail:
  return FALSE;
}
