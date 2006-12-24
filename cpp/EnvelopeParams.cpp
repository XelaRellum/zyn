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
#include "EnvelopeParams.h"

EnvelopeParams::EnvelopeParams(unsigned char Penvstretch_,unsigned char Pforcedrelease_):Presets()
{
  int i;
    
  PA_dt = 10;
  PD_dt = 10;
  PR_dt = 10;
  PA_val = 64;
  PD_val = 64;
  PS_val = 64;
  PR_val = 64;
    
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
  setpresettype("Penvamplitude");

  m_mode = ZYN_ENVELOPE_MODE_ADSR;

  PA_dt = A_dt;
  PD_dt = D_dt;
  PS_val = S_val;
  PR_dt = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ADSRinit_dB(char A_dt,char D_dt,char S_val,char R_dt)
{
  setpresettype("Penvamplitude");

  m_mode = ZYN_ENVELOPE_MODE_ADSR_DB;

  PA_dt = A_dt;
  PD_dt = D_dt;
  PS_val = S_val;
  PR_dt = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ASRinit(char A_val,char A_dt,char R_val,char R_dt)
{
  setpresettype("Penvfrequency");

  m_mode = ZYN_ENVELOPE_MODE_ASR;

  PA_val = A_val;
  PA_dt = A_dt;
  PR_val = R_val;
  PR_dt = R_dt;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ADSRinit_filter(char A_val,char A_dt,char D_val,char D_dt,char R_dt,char R_val)
{
  setpresettype("Penvfilter");

  m_mode = ZYN_ENVELOPE_MODE_ADSR_FILTER;

  PA_val = A_val;
  PA_dt = A_dt;
  PD_val = D_val;
  PD_dt = D_dt;
  PR_dt = R_dt;
  PR_val = R_val;

  m_free_mode = FALSE;

  converttofree();

  store2defaults();
}

void EnvelopeParams::ASRinit_bw(char A_val,char A_dt,char R_val,char R_dt)
{
  setpresettype("Penvbandwidth");

  m_mode = ZYN_ENVELOPE_MODE_ASR_BW;

  PA_val = A_val;
  PA_dt = A_dt;
  PR_val = R_val;
  PR_dt = R_dt;

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
    Penvdt[1] = PA_dt;
    Penvval[1] = 127;
    Penvdt[2] = PD_dt;
    Penvval[2] = PS_val;
    Penvdt[3] = PR_dt;
    Penvval[3] = 0;    
    break;
  case ZYN_ENVELOPE_MODE_ADSR_DB:
    Penvpoints = 4;
    Penvsustain = 2;
    Penvval[0] = 0;
    Penvdt[1] = PA_dt;
    Penvval[1] = 127;
    Penvdt[2] = PD_dt;
    Penvval[2] = PS_val;
    Penvdt[3] = PR_dt;
    Penvval[3] = 0;    
    break;  
  case ZYN_ENVELOPE_MODE_ASR:
    Penvpoints = 3;
    Penvsustain = 1;
    Penvval[0] = PA_val;
    Penvdt[1] = PA_dt;
    Penvval[1] = 64;
    Penvdt[2] = PR_dt;
    Penvval[2] = PR_val;
    break;
  case ZYN_ENVELOPE_MODE_ADSR_FILTER:
    Penvpoints = 4;
    Penvsustain = 2;
    Penvval[0] = PA_val;
    Penvdt[1] = PA_dt;
    Penvval[1] = PD_val;
    Penvdt[2] = PD_dt;
    Penvval[2] = 64;
    Penvdt[3] = PR_dt;
    Penvval[3] = PR_val;
    break;
  case ZYN_ENVELOPE_MODE_ASR_BW:
    Penvpoints = 3;
    Penvsustain = 1;
    Penvval[0] = PA_val;
    Penvdt[1] = PA_dt;
    Penvval[1] = 64;
    Penvdt[2] = PR_dt;
    Penvval[2] = PR_val;
    break;
  };
};

void EnvelopeParams::defaults()
{
  Penvstretch = Denvstretch;
  Pforcedrelease = Dforcedrelease;
  Plinearenvelope = Dlinearenvelope;

  PA_dt = DA_dt;
  PD_dt = DD_dt;
  PR_dt = DR_dt;
  PA_val = DA_val;
  PD_val = DD_val;
  PS_val = DS_val;
  PR_val = DR_val;

  m_free_mode = FALSE;

  converttofree();
};

void EnvelopeParams::store2defaults()
{
  Denvstretch=Penvstretch;
  Dforcedrelease=Pforcedrelease;
  Dlinearenvelope=Plinearenvelope;
  DA_dt=PA_dt;
  DD_dt=PD_dt;
  DR_dt=PR_dt;
  DA_val=PA_val;
  DD_val=PD_val;
  DS_val=PS_val;
  DR_val=PR_val;
};
