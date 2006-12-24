/*
  ZynAddSubFX - a software synthesizer
 
  EnvelopeParams.C - Parameters for Envelope
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

#include <math.h>               // pow()

#include "globals.h"
#include "envelope_parameters.h"

EnvelopeParams::EnvelopeParams(
  unsigned char Penvstretch_,
  unsigned char Pforcedrelease_)
{
  int i;
    
  m_attack_duration = 10;
  m_decay_duration = 10;
  m_release_duration = 10;
  m_attack_value = 64;
  m_decay_value = 64;
  m_sustain_value = 64;
  m_release_value = 64;
    
  for (i = 0 ; i < MAX_ENVELOPE_POINTS ; i++)
  {
    Penvdt[i] = 32;
    Penvval[i] = 64;
  }

  Penvdt[0] = 0;                // no used
  Penvsustain = 1;
  Penvpoints = 1;
  m_mode = ZYN_ENVELOPE_MODE_ADSR;
  Penvstretch = Penvstretch_;
  Pforcedrelease = Pforcedrelease_;        
  m_free_mode = TRUE;
  Plinearenvelope = 0;
    
  store2defaults();
}

EnvelopeParams::~EnvelopeParams()
{
}

REALTYPE EnvelopeParams::getdt(char i)
{
  REALTYPE result=(pow(2.0,Penvdt[i]/127.0*12.0)-1.0)*10.0;//miliseconds
  return(result);
}

/*
 * ADSR/ASR... initialisations
 */
void EnvelopeParams::ADSRinit(char A_dt,char D_dt,char S_val,char R_dt)
{
  m_mode = ZYN_ENVELOPE_MODE_ADSR;

  m_attack_duration = A_dt;
  m_decay_duration = D_dt;
  m_sustain_value = S_val;
  m_release_duration = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ADSRinit_dB(char A_dt,char D_dt,char S_val,char R_dt)
{
  m_mode = ZYN_ENVELOPE_MODE_ADSR_DB;

  m_attack_duration = A_dt;
  m_decay_duration = D_dt;
  m_sustain_value = S_val;
  m_release_duration = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ASRinit(char A_val,char A_dt,char R_val,char R_dt)
{
  m_mode = ZYN_ENVELOPE_MODE_ASR;

  m_attack_value = A_val;
  m_attack_duration = A_dt;
  m_release_value = R_val;
  m_release_duration = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ADSRinit_filter(char A_val,char A_dt,char D_val,char D_dt,char R_dt,char R_val)
{
  m_mode = ZYN_ENVELOPE_MODE_ADSR_FILTER;

  m_attack_value = A_val;
  m_attack_duration = A_dt;
  m_decay_value = D_val;
  m_decay_duration = D_dt;
  m_release_duration = R_dt;
  m_release_value = R_val;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ASRinit_bw(char A_val,char A_dt,char R_val,char R_dt)
{
  m_mode = ZYN_ENVELOPE_MODE_ASR_BW;

  m_attack_value = A_val;
  m_attack_duration = A_dt;
  m_release_value = R_val;
  m_release_duration = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

/*
 * Convert the Envelope to freemode
 */
void
EnvelopeParams::converttofree()
{
  switch (m_mode)
  {
  case ZYN_ENVELOPE_MODE_ADSR:
    Penvpoints = 4;
    Penvsustain = 2;
    Penvval[0] = 0;
    Penvdt[1] = m_attack_duration;
    Penvval[1] = 127;
    Penvdt[2] = m_decay_duration;
    Penvval[2] = m_sustain_value;
    Penvdt[3] = m_release_duration;
    Penvval[3] = 0;    
    break;
  case ZYN_ENVELOPE_MODE_ADSR_DB:
    Penvpoints = 4;
    Penvsustain = 2;
    Penvval[0] = 0;
    Penvdt[1] = m_attack_duration;
    Penvval[1] = 127;
    Penvdt[2] = m_decay_duration;
    Penvval[2] = m_sustain_value;
    Penvdt[3] = m_release_duration;
    Penvval[3] = 0;    
    break;  
  case ZYN_ENVELOPE_MODE_ASR:
    Penvpoints = 3;
    Penvsustain = 1;
    Penvval[0] = m_attack_value;
    Penvdt[1] = m_attack_duration;
    Penvval[1] = 64;
    Penvdt[2] = m_release_duration;
    Penvval[2] = m_release_value;
    break;
  case ZYN_ENVELOPE_MODE_ADSR_FILTER:
    Penvpoints = 4;
    Penvsustain = 2;
    Penvval[0] = m_attack_value;
    Penvdt[1] = m_attack_duration;
    Penvval[1] = m_decay_value;
    Penvdt[2] = m_decay_duration;
    Penvval[2] = 64;
    Penvdt[3] = m_release_duration;
    Penvval[3] = m_release_value;
    break;
  case ZYN_ENVELOPE_MODE_ASR_BW:
    Penvpoints = 3;
    Penvsustain = 1;
    Penvval[0] = m_attack_value;
    Penvdt[1] = m_attack_duration;
    Penvval[1] = 64;
    Penvdt[2] = m_release_duration;
    Penvval[2] = m_release_value;
    break;
  }
}

void EnvelopeParams::defaults()
{
  Penvstretch = Denvstretch;
  Pforcedrelease = Dforcedrelease;
  Plinearenvelope = Dlinearenvelope;

  m_attack_duration = m_attack_duration_default;
  m_decay_duration = m_decay_duration_default;
  m_release_duration = m_release_duration_default;

  m_attack_value = m_attack_value_default;
  m_decay_value = m_decay_value_default;
  m_sustain_value = m_sustain_value_default;
  m_release_value = m_release_value_default;

  m_free_mode = FALSE;

  converttofree();
}

void EnvelopeParams::store2defaults()
{
  Denvstretch = Penvstretch;
  Dforcedrelease = Pforcedrelease;
  Dlinearenvelope = Plinearenvelope;

  m_attack_duration_default = m_attack_duration;
  m_decay_duration_default = m_decay_duration;
  m_release_duration_default = m_release_duration;

  m_attack_value_default = m_attack_value;
  m_decay_value_default = m_decay_value;
  m_sustain_value_default = m_sustain_value;
  m_release_value_default = m_release_value;
}
