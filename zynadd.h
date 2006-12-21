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

#ifndef ZYNADD_H__6EC1E456_7DD7_4536_B8D3_F23BE4583A23__INCLUDED
#define ZYNADD_H__6EC1E456_7DD7_4536_B8D3_F23BE4583A23__INCLUDED

LV2_Handle
zynadd_instantiate(
  const LV2_Descriptor * descriptor,
  uint32_t sample_rate,
  const char * bundle_path,
  const LV2_Host_Feature ** host_features);

void
zynadd_connect_port(
  LV2_Handle instance,
  uint32_t port,
  void * data_location);

void
zynadd_run(
  LV2_Handle instance,
  uint32_t samples_count);

void
zynadd_cleanup(
  LV2_Handle instance);

void *
zynadd_extension_data(
  const char * URI); 

#endif /* #ifndef ZYNADD_H__6EC1E456_7DD7_4536_B8D3_F23BE4583A23__INCLUDED */
