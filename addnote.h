/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2009 Nedko Arnaudov <nedko@arnaudov.name>
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

#ifndef AD_NOTE_H__INCLUDED
#define AD_NOTE_H__INCLUDED

typedef void * zyn_addnote_handle;

bool
zyn_addnote_create(
  struct zyn_addsynth * synth_ptr,
  zyn_addnote_handle * handle_ptr);

void
zyn_addnote_destroy(
  zyn_addnote_handle handle);

void
zyn_addnote_note_on(
  zyn_addnote_handle handle,
  float panorama,
  bool random_grouping,
  REALTYPE freq,
  REALTYPE velocity,
  bool portamento,
  int midinote);

/* returns true if note is still active and false otherwise */
bool
zyn_addnote_noteout(
  zyn_addnote_handle handle,
  REALTYPE *outl,
  REALTYPE *outr);

void
zyn_addnote_note_off(
  zyn_addnote_handle handle);

void
zyn_addnote_force_disable(
  zyn_addnote_handle handle);

#endif  /* #ifndef AD_NOTE_H__INCLUDED */
