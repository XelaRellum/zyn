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

//#include <stdio.h>

#include "common.h"
#include "lv2.h"
#include "lv2plugin.h"
#include "zynadd.h"
#include "lv2dynparam/plugin.h"

static LV2_Descriptor g_lv2_plugins[] =
{
  {
    .URI = "http://home.gna.org/zyn/zynadd/0",
    .instantiate = zynadd_instantiate,
    .connect_port = zynadd_connect_port,
    .run = zynadd_run,
    .cleanup = zynadd_cleanup,
    .extension_data = zynadd_extension_data
  },
  {
    .URI = NULL
  }
};

static int g_lv2_plugins_count;
static BOOL g_lv2dynparam_inited;

void lv2_initialise() __attribute__((constructor));
void lv2_initialise()
{
  const LV2_Descriptor * descr_ptr;

//  printf("lv2_initialise() called.\n");

  descr_ptr = g_lv2_plugins;

  while (descr_ptr->URI != NULL)
  {
    g_lv2_plugins_count++;
    descr_ptr++;
  }

  g_lv2dynparam_inited = lv2dynparam_plugin_init(100000, 10, 100);
}

const LV2_Descriptor* lv2_descriptor(uint32_t index)
{
  if (!g_lv2dynparam_inited)
  {
    return NULL;
  }

/*   printf("lv2_descriptor(%u) called.\n", (unsigned int)index); */

  if (index >= g_lv2_plugins_count)
  {
/*     printf("plugin at index %u not found.\n", (unsigned int)index); */
    return NULL;
  }

/*   printf("<%s> found.\n", g_lv2_plugins[index].URI); */
  return g_lv2_plugins + index;
}
