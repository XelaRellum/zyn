/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2007,2007 Nedko Arnaudov <nedko@arnaudov.name>
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

#ifndef LFO_PARAMETERS_H__79449D30_40FF_47EB_959A_F83B68A925FC__INCLUDED
#define LFO_PARAMETERS_H__79449D30_40FF_47EB_959A_F83B68A925FC__INCLUDED

struct zyn_lfo_parameters
{
  float frequency;              /* 0.0 .. 1.0 */
  float depth;                  /* 0.0 .. 1.0 */
  bool random_start_phase;
  float start_phase;            /* 0.0 .. 1.0 */
  bool depth_randomness_enabled;
  float depth_randomness;       /* 0.0 .. 1.0 */
  bool frequency_randomness_enabled;
  float frequency_randomness;   /* 0.0 .. 1.0 */
  float delay;                  /* 0.0 .. 4.0, seconds */
  float stretch;                /* -1 .. 1, how the LFO is "stretched" according the note frequency (0=no stretch) */
  unsigned int shape;           /* one of ZYN_LFO_SHAPE_TYPE_XXX */
};

#endif /* #ifndef LFO_PARAMETERS_H__79449D30_40FF_47EB_959A_F83B68A925FC__INCLUDED */
