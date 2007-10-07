/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
 *   Copyright (C) 2002-2005 Nasca Octavian Paul
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

#ifndef PORTAMENTO_H__83CC72B6_3F89_4791_ACEF_098A31CB368A__INCLUDED
#define PORTAMENTO_H__83CC72B6_3F89_4791_ACEF_098A31CB368A__INCLUDED

struct zyn_portamento
{
  bool enabled;                 // whether portamento is enabled
  
  float time;

  // the threshold of enabling protamento
  // Minimum or max. difference of the notes in order to do the portamento (x 100 cents)
  float pitch_threshold;

  // whether to enable the portamento only below (false) or above (true) the threshold
  bool pitch_threshold_above;

  // 'up portanemto' means when the frequency is rising (eg: the portamento is from 200Hz to 300 Hz)
  // 'down portanemto' means when the frequency is lowering (eg: the portamento is from 300Hz to 200 Hz)

  // Portamento time stretch (up/down)
  // this value represent how the portamento time is reduced
  // -1.0 - for down portamento
  // (-1.0,0) - the up portamento's time is smaller than the down portamento
  // 0 - the portamento time is always the same
  // (0,1.0) - the down portamento's time is smaller than the up portamento
  // 1.0 - for upper portamento
  float up_down_time_stretch;

  // this value is used to compute the actual portamento
  float freqrap;

  // whether portamento is used by a note
  bool used;

  // x is from 0.0 (start portamento) to 1.0 (finished portamento)
  float x;

  // dx is x increment
  float dx;

  // this is used for computing oldfreq value from x
  float origfreqrap;
};

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

void
zyn_portamento_init(
  struct zyn_portamento * portamento_ptr);

// returns true if the portamento's conditions are true, else returns false
bool
zyn_portamento_start(
  struct zyn_portamento * portamento_ptr,
  float oldfreq,
  float newfreq);

// update portamento values
void
zyn_portamento_update(
  struct zyn_portamento * portamento_ptr);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef PORTAMENTO_H__83CC72B6_3F89_4791_ACEF_098A31CB368A__INCLUDED */
