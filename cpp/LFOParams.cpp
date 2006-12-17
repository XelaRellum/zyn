/*
  ZynAddSubFX - a software synthesizer
 
  LFOParams.C - Parameters for LFO
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

#include <math.h>
#include <stdio.h>
#include "globals.h"
#include "LFOParams.h"

int LFOParams::time;

LFOParams::LFOParams(char Pfreq_,char Pintensity_,char Pstartphase_, char PLFOtype_,char Prandomness_, char Pdelay_,char Pcontinous_,char fel_):Presets(){
    switch(fel_) {
	case 0:setpresettype("Plfofrequency");
	    break;
	case 1:setpresettype("Plfoamplitude");
	    break;
	case 2:setpresettype("Plfofilter");
	    break;
    };
    Dfreq=Pfreq_;
    Dintensity=Pintensity_;
    Dstartphase=Pstartphase_;
    DLFOtype=PLFOtype_;
    Drandomness=Prandomness_;
    Ddelay=Pdelay_;
    Dcontinous=Pcontinous_;
    fel=fel_;
    time=0;
    
    defaults();
};

LFOParams::~LFOParams(){
};

void LFOParams::defaults(){
    Pfreq=Dfreq/127.0;
    Pintensity=Dintensity;
    Pstartphase=Dstartphase;
    PLFOtype=DLFOtype;
    Prandomness=Drandomness;
    Pdelay=Ddelay;
    Pcontinous=Dcontinous;
    Pfreqrand=0;
    Pstretch=64;
};
