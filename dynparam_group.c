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

#include "lv2.h"
#include "lv2dynparam.h"
#include "list.h"
#include "dynparam.h"
#include "dynparam_internal.h"

BOOL
lv2dynparam_plugin_group_init(
  struct lv2dynparam_plugin_group * group_ptr,
  unsigned int type,
  const char * name)
{
  group_ptr->name = strdup(name);
  if (group_ptr->name == NULL)
  {
    return FALSE;
  }

  group_ptr->type = type;
  INIT_LIST_HEAD(&group_ptr->child_groups);
  INIT_LIST_HEAD(&group_ptr->child_parameters);

  group_ptr->host_notified = FALSE;

  return TRUE;
}

BOOL
lv2dynparam_plugin_group_new(
  struct lv2dynparam_plugin_group * parent_group_ptr,
  unsigned int type,
  const char * name,
  struct lv2dynparam_plugin_group ** group_ptr_ptr)
{
  BOOL ret;
  struct lv2dynparam_plugin_group * group_ptr;

  group_ptr = malloc(sizeof(struct lv2dynparam_plugin_group));
  if (group_ptr == NULL)
  {
    ret = FALSE;
    goto exit;
  }

  if (!lv2dynparam_plugin_group_init(group_ptr, type, name))
  {
    ret = FALSE;
    goto free;
  }

  list_add_tail(&group_ptr->siblings, &parent_group_ptr->child_groups);

  *group_ptr_ptr = group_ptr;

  return TRUE;

free:
  free(group_ptr);

exit:
  return ret;
}

void
lv2dynparam_plugin_group_clean(
  struct lv2dynparam_plugin_group * group_ptr)
{
  struct list_head * node_ptr;
  struct lv2dynparam_plugin_group * child_group_ptr;
  struct lv2dynparam_plugin_parameter * child_param_ptr;

  free(group_ptr->name);

  while (!list_empty(&group_ptr->child_groups))
  {
    node_ptr = group_ptr->child_groups.next;
    child_group_ptr = list_entry(node_ptr, struct lv2dynparam_plugin_group, siblings);
    list_del(node_ptr);
    lv2dynparam_plugin_group_free(child_group_ptr);
  }

  while (!list_empty(&group_ptr->child_parameters))
  {
    node_ptr = group_ptr->child_groups.next;
    child_param_ptr = list_entry(node_ptr, struct lv2dynparam_plugin_parameter, siblings);
    list_del(node_ptr);
    lv2dynparam_plugin_parameter_free(child_param_ptr);
  }
}

void
lv2dynparam_plugin_group_free(
  struct lv2dynparam_plugin_group * group_ptr)
{
  lv2dynparam_plugin_group_clean(group_ptr);
  free(group_ptr);
}

void
lv2dynparam_plugin_group_notify(
  struct lv2dynparam_plugin_instance * instance_ptr,
  struct lv2dynparam_plugin_group * parent_group_ptr,
  struct lv2dynparam_plugin_group * group_ptr)
{
  struct list_head * node_ptr;
  struct lv2dynparam_plugin_group * child_group_ptr;
  struct lv2dynparam_plugin_parameter * child_param_ptr;

  if (!group_ptr->host_notified)
  {
    if (!instance_ptr->host_callbacks->group_appear(
          instance_ptr->host_context,
          (parent_group_ptr == NULL)?NULL:parent_group_ptr->host_context,
          (lv2dynparam_group_handle)group_ptr,
          &group_ptr->host_context))
    {
      return;
    }

    group_ptr->host_notified = TRUE;
  }

  list_for_each(node_ptr, &group_ptr->child_groups)
  {
    child_group_ptr = list_entry(node_ptr, struct lv2dynparam_plugin_group, siblings);
    lv2dynparam_plugin_group_notify(instance_ptr, group_ptr, child_group_ptr);
  }

  list_for_each(node_ptr, &group_ptr->child_parameters)
  {
    child_param_ptr = list_entry(node_ptr, struct lv2dynparam_plugin_parameter, siblings);
    lv2dynparam_plugin_param_notify(instance_ptr, group_ptr, child_param_ptr);
  }
}

#define group_ptr ((struct lv2dynparam_plugin_group *)group)

unsigned char
lv2dynparam_plugin_group_get_type_uri(
  lv2dynparam_group_handle group,
  char * buffer,
  size_t buffer_size)
{
  size_t s;
  const char * uri;

  switch (group_ptr->type)
  {
  case LV2DYNPARAM_GROUP_TYPE_GENERIC:
    uri = LV2DYNPARAM_GROUP_TYPE_GENERIC_URI;
    break;
  case LV2DYNPARAM_GROUP_TYPE_ADSR:
    uri = LV2DYNPARAM_GROUP_TYPE_ADSR_URI;
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
lv2dynparam_plugin_group_get_name(
  lv2dynparam_group_handle group,
  char * buffer,
  size_t buffer_size)
{
  size_t s;

  s = strlen(group_ptr->name);

  s++;

  if (s > buffer_size)
  {
    return FALSE;
  }

  memcpy(buffer, group_ptr->name, s);

  return TRUE;
}

#undef group_ptr
#define parent_group_ptr ((struct lv2dynparam_plugin_group *)parent_group)
#define instance_ptr ((struct lv2dynparam_plugin_instance *)instance_handle)

BOOL
lv2dynparam_plugin_group_add(
  lv2dynparam_plugin_instance instance_handle,
  lv2dynparam_plugin_group parent_group,
  const char * name,
  lv2dynparam_plugin_group * group_handle_ptr)
{
  struct lv2dynparam_plugin_group * group_ptr;

  if (!lv2dynparam_plugin_group_new(
        parent_group_ptr == NULL ? &instance_ptr->root_group: parent_group_ptr,
        LV2DYNPARAM_GROUP_TYPE_GENERIC,
        name,
        &group_ptr))
  {
    return FALSE;
  }

  *group_handle_ptr = (lv2dynparam_plugin_group)group_ptr;

  return TRUE;
}
