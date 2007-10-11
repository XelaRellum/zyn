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
  bandwidth.depth=64;
  bandwidth.exponential=0;
  modwheel.depth=80;
  modwheel.exponential=0;
}

void Controller::resetall(){
  setbandwidth(64);
  setmodwheel(64);
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
