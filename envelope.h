/*
  ZynAddSubFX - a software synthesizer
 
  Envelope.h - Envelope implementation
  Copyright (C) 2002-2005 Nasca Octavian Paul
  Author: Nasca Octavian Paul

  This program is free software; you can redistribute it and/or modify
  it under the terms of version 2 of the GNU General Public License 
  as published by the Free Software Foundation.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License (version 2) for more details.

  You should have received a copy of the GNU General Public License (version 2)
  along with this program; if not, write to the Free Software Foundation,
  Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA

*/

#ifndef ENVELOPE_H
#define ENVELOPE_H

class Envelope
{
public:
  Envelope();
  ~Envelope();

  void
  init(
    float sample_rate,
    EnvelopeParams * envpars,
    float basefreq);

  void relasekey();

  float envout();
  float envout_dB();

  bool finished();               // returns whether envelope has finished or not

private:
  int envpoints;
  int envsustain;               // "-1" means disabled
  float envdt[MAX_ENVELOPE_POINTS]; // millisecons
  float envval[MAX_ENVELOPE_POINTS]; // [0.0 .. 1.0]
  float m_stretch;
  bool m_linear;

  int currentpoint;             // current envelope point (starts from 1)
  bool m_forced_release;
  bool m_key_released;          // whether the key was released or not
  bool m_finished;              // whether envelope has finished or not
  float t;                      // the time from the last point
  float inct;                   // the time increment
  float envoutval;              // used to do the forced release
};

#endif


