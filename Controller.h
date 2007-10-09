/*
  ZynAddSubFX - a software synthesizer
 
  Controller.h - (Midi) Controllers implementation
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


#ifndef CONTROLLER_H
#define CONTROLLER_H

#include "globals.h"

class Controller{
public:
  Controller();
  ~Controller();
  void resetall();

  void defaults();

  //Controllers functions
  void setfiltercutoff(int value);
  void setfilterq(int value);
  void setbandwidth(int value);
  void setmodwheel(int value);
  void setfmamp(int value);
  void setsustain(int value);
  void setresonancecenter(int value);
  void setresonancebw(int value);

  void setparameternumber(unsigned int type,int value);//used for RPN and NRPN's
  int getnrpn(int *parhi, int *parlo, int *valhi, int *vallo);

  struct{//Filter cutoff
    int data;
    REALTYPE relfreq;
    unsigned char depth;
  } filtercutoff;

  struct{//Filter Q
    int data;
    REALTYPE relq;
    unsigned char depth;
  } filterq;

  struct{//Bandwidth
    int data;
    REALTYPE relbw;
    unsigned char depth;
    unsigned char exponential;
  } bandwidth;

  struct {//Modulation Wheel
    int data;
    REALTYPE relmod;
    unsigned char depth;
    unsigned char exponential;
  } modwheel;

  struct{//FM amplitude
    int data;
    REALTYPE relamp;
    unsigned char receive;
  } fmamp;

  struct{//Sustain
    int data,sustain;
    unsigned char receive;
  } sustain;

  struct{//Resonance Center Frequency
    int data;
    REALTYPE relcenter;
    unsigned char depth;
  } resonancecenter;

  struct{//Resonance Bandwidth
    int data;
    REALTYPE relbw;
    unsigned char depth;
  } resonancebandwidth;
    

  /* RPN and NPRPN */
  struct{//nrpn
    int parhi,parlo;
    int valhi,vallo;
    unsigned char receive;//this is saved to disk by Master
  } NRPN;
    
private:
};





#endif

