/*
  ZynAddSubFX - a software synthesizer
 
  EnvelopeParams.h - Parameters for Envelope
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

#ifndef ENVELOPE_PARAMS_H
#define ENVELOPE_PARAMS_H

#include "Presets.h"

#define MAX_ENVELOPE_POINTS 40
#define MIN_ENVELOPE_DB -40

#define ZYN_ENVELOPE_MODE_ADSR         1 // ADSR parameters (linear amplitude)
#define ZYN_ENVELOPE_MODE_ADSR_DB      2 // ADSR_dB parameters (dB amplitude)
#define ZYN_ENVELOPE_MODE_ASR          3 // ASR parameters (frequency LFO)
#define ZYN_ENVELOPE_MODE_ADSR_FILTER  4 // ADSR_filter parameters (filter parameters)
#define ZYN_ENVELOPE_MODE_ASR_BW       5 // ASR_bw parameters (bandwidth parameters)

class EnvelopeParams: public Presets
{
public:
  EnvelopeParams(unsigned char Penvstretch_,unsigned char Pforcedrelease_);
  ~EnvelopeParams();

  void ADSRinit(char A_dt,char D_dt,char S_val,char R_dt);
  void ADSRinit_dB(char A_dt,char D_dt,char S_val,char R_dt);
  void ASRinit(char A_val,char A_dt,char R_val,char R_dt);
  void ADSRinit_filter(char A_val,char A_dt,char D_val,char D_dt,char R_dt,char R_val);
  void ASRinit_bw(char A_val,char A_dt,char R_val,char R_dt);
  void converttofree();

  void defaults();

  REALTYPE getdt(char i);

  /* MIDI parameters? */
  BOOL m_free_mode;             // free or ADSR/ASR mode
  unsigned char Penvpoints;
  unsigned char Penvsustain;    // 127 pentru dezactivat
  unsigned char Penvdt[MAX_ENVELOPE_POINTS];
  unsigned char Penvval[MAX_ENVELOPE_POINTS];
  unsigned char Penvstretch;    // 64 = normal stretch (piano-like), 0 = no stretch
  unsigned char Pforcedrelease; // 0 - OFF, 1 - ON
  unsigned char Plinearenvelope; //if the amplitude envelope is linear

  unsigned char PA_dt;
  unsigned char PD_dt;
  unsigned char PR_dt;
  unsigned char PA_val;
  unsigned char PD_val;
  unsigned char PS_val;
  unsigned char PR_val;

  unsigned int m_mode;          // one of ZYN_ENVELOPE_MODE_XXX

private:
  void store2defaults();

  /* Default parameters */
  unsigned char Denvstretch;
  unsigned char Dforcedrelease;
  unsigned char Dlinearenvelope;
  unsigned char DA_dt;
  unsigned char DD_dt;
  unsigned char DR_dt;
  unsigned char DA_val;
  unsigned char DD_val;
  unsigned char DS_val;
  unsigned char DR_val;
};

#endif

