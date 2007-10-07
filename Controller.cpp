/*
  ZynAddSubFX - a software synthesizer
 
  Controller.C - (Midi) Controllers implementation
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

#include "Controller.h"
#include <math.h>
#include <stdio.h>

//#define LOG_LEVEL LOG_LEVEL_DEBUG
#include "log.h"

Controller::Controller(){
  defaults();
  resetall();
};

Controller::~Controller(){
};

void Controller::defaults(){
  setpitchwheelbendrange(200);//2 halftones
  expression.receive=1;
  panning.depth=64;
  filtercutoff.depth=64;
  filterq.depth=64;
  bandwidth.depth=64;
  bandwidth.exponential=0;
  modwheel.depth=80;
  modwheel.exponential=0;
  fmamp.receive=1;
  volume.receive=0;
  sustain.receive=1;
  NRPN.receive=1;

  resonancecenter.depth=64;
  resonancebandwidth.depth=64;
}

void Controller::resetall(){
  setpitchwheel(0);//center
  setexpression(127);
  setpanning(64);
  setfiltercutoff(64);
  setfilterq(64);
  setbandwidth(64);
  setmodwheel(64);
  setfmamp(127);
  setvolume(127);
  setsustain(0);
  setresonancecenter(64);
  setresonancebw(64);
    
  //reset the NRPN
  NRPN.parhi=-1;
  NRPN.parlo=-1;
  NRPN.valhi=-1;
  NRPN.vallo=-1;
};

void Controller::setpitchwheel(int value){
  pitchwheel.data=value;
  REALTYPE cents=value/8192.0;
  cents*=pitchwheel.bendrange;
  pitchwheel.relfreq=pow(2,cents/1200.0);
  //fprintf(stderr,"%ld %ld -> %.3f\n",pitchwheel.bendrange,pitchwheel.data,pitchwheel.relfreq);fflush(stderr);
};

void Controller::setpitchwheelbendrange(unsigned short int value){
  pitchwheel.bendrange=value;
};

void Controller::setexpression(int value){
  expression.data=value;
  if (expression.receive!=0) expression.relvolume=value/127.0;
  else expression.relvolume=1.0;
};

void Controller::setpanning(int value){
  panning.data=value;
  panning.pan=(value/128.0-0.5)*(panning.depth/64.0);
};

void Controller::setfiltercutoff(int value){
  filtercutoff.data=value;
  filtercutoff.relfreq=(value-64.0)*filtercutoff.depth/4096.0*3.321928;//3.3219..=ln2(10)
};

void Controller::setfilterq(int value){
  filterq.data=value;
  filterq.relq=pow(30.0,(value-64.0)/64.0*(filterq.depth/64.0));
};

void Controller::setbandwidth(int value){
  bandwidth.data=value;
  if (bandwidth.exponential==0) {
    REALTYPE tmp=pow(25.0,pow(bandwidth.depth/127.0,1.5))-1.0;
    if ((value<64)&&(bandwidth.depth>=64)) tmp=1.0;
    bandwidth.relbw=(value/64.0-1.0)*tmp+1.0;
    if (bandwidth.relbw<0.01) bandwidth.relbw=0.01;
  } else {
    bandwidth.relbw=pow(25.0,(value-64.0)/64.0*(bandwidth.depth/64.0));
  };
};

void Controller::setmodwheel(int value){
  modwheel.data=value;
  if (modwheel.exponential==0) {
    REALTYPE tmp=pow(25.0,pow(modwheel.depth/127.0,1.5)*2.0)/25.0;
    if ((value<64)&&(modwheel.depth>=64)) tmp=1.0;
    modwheel.relmod=(value/64.0-1.0)*tmp+1.0;
    if (modwheel.relmod<0.0) modwheel.relmod=0.0;
  } else modwheel.relmod=pow(25.0,(value-64.0)/64.0*(modwheel.depth/80.0));
};

void Controller::setfmamp(int value){
  fmamp.data=value;
  fmamp.relamp=value/127.0;
  if (fmamp.receive!=0) fmamp.relamp=value/127.0;
  else fmamp.relamp=1.0;
};

void Controller::setvolume(int value){
  volume.data=value;
  if (volume.receive!=0) volume.volume=pow(0.1,(127-value)/127.0*2.0);
  else volume.volume=1.0;
};

void Controller::setsustain(int value){
  sustain.data=value;
  if (sustain.receive!=0) sustain.sustain=((value<64) ? 0 : 1 );
  else sustain.sustain=0;
};

void Controller::setresonancecenter(int value){
  resonancecenter.data=value;
  resonancecenter.relcenter=pow(3.0,(value-64.0)/64.0*(resonancecenter.depth/64.0));
};
void Controller::setresonancebw(int value){
  resonancebandwidth.data=value;
  resonancebandwidth.relbw=pow(1.5,(value-64.0)/64.0*(resonancebandwidth.depth/127.0));
};


//Returns 0 if there is NRPN or 1 if there is not
int Controller::getnrpn(int *parhi, int *parlo, int *valhi, int *vallo){
  if (NRPN.receive==0) return(1);
  if ((NRPN.parhi<0)||(NRPN.parlo<0)||(NRPN.valhi<0)||(NRPN.vallo<0)) 
    return(1);
        
  *parhi=NRPN.parhi;
  *parlo=NRPN.parlo;
  *valhi=NRPN.valhi;
  *vallo=NRPN.vallo;
  return(0);
};


void Controller::setparameternumber(unsigned int type,int value){
  switch(type){
  case C_nrpnhi:NRPN.parhi=value;
    NRPN.valhi=-1;NRPN.vallo=-1;//clear the values
    break;
  case C_nrpnlo:NRPN.parlo=value;
    NRPN.valhi=-1;NRPN.vallo=-1;//clear the values
    break;
  case C_dataentryhi:if ((NRPN.parhi>=0)&&(NRPN.parlo>=0)) NRPN.valhi=value;
    break;
  case C_dataentrylo:if ((NRPN.parhi>=0)&&(NRPN.parlo>=0)) NRPN.vallo=value;
    break;
  };
};

