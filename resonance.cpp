/*
  ZynAddSubFX - a software synthesizer
 
  Resonance.C - Resonance
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
#include <stdlib.h>

#include "globals.h"
#include "resonance.h"
#include "fft.h"

Resonance::Resonance()
{
  defaults();
}

Resonance::~Resonance()
{
}

void Resonance::defaults(){
    Penabled=0;
    PmaxdB=20;
    Pcenterfreq=64;//1 kHz
    Poctavesfreq=64;
    Pprotectthefundamental=0;
    ctlcenter=1.0;
    ctlbw=1.0;
    for (int i=0;i<N_RES_POINTS;i++) Prespoints[i]=64;
}

/*
 * Set a point of resonance function with a value
 */
void Resonance::setpoint(int n,unsigned char p){
    if ((n<0)||(n>=N_RES_POINTS)) return;
    Prespoints[n]=p;
};

/*
 * Apply the resonance to FFT data
 */
void
Resonance::applyres(
  int n,
  struct zyn_fft_freqs fftdata,
  REALTYPE freq)
{
    if (Penabled==0) return;//if the resonance is disabled
    REALTYPE sum=0.0,
	     l1=log(getfreqx(0.0)*ctlcenter),
	     l2=log(2.0)*getoctavesfreq()*ctlbw;

    for (int i=0;i<N_RES_POINTS;i++) if (sum<Prespoints[i]) sum=Prespoints[i];
    if (sum<1.0) sum=1.0;

    for (int i=1;i<n;i++){
        REALTYPE x=(log(freq*i)-l1)/l2;//compute where the n-th hamonics fits to the graph
	if (x<0.0) x=0.0;

	x*=N_RES_POINTS;
	REALTYPE dx=x-floor(x);x=floor(x);
	int kx1=(int)x; if (kx1>=N_RES_POINTS) kx1=N_RES_POINTS-1;
	int kx2=kx1+1;if (kx2>=N_RES_POINTS) kx2=N_RES_POINTS-1;
	REALTYPE y=(Prespoints[kx1]*(1.0-dx)+Prespoints[kx2]*dx)/127.0-sum/127.0;
	
	y=pow(10.0,y*PmaxdB/20.0);
	
	if ((Pprotectthefundamental!=0)&&(i==1)) y=1.0;
	
        fftdata.c[i]*=y;
        fftdata.s[i]*=y;
    };    
};

/*
 * Gets the response at the frequency "freq"
 */

REALTYPE Resonance::getfreqresponse(REALTYPE freq){
    REALTYPE l1=log(getfreqx(0.0)*ctlcenter),
	     l2=log(2.0)*getoctavesfreq()*ctlbw,sum=0.0;
	
    for (int i=0;i<N_RES_POINTS;i++) if (sum<Prespoints[i]) sum=Prespoints[i];
    if (sum<1.0) sum=1.0;

    REALTYPE x=(log(freq)-l1)/l2;//compute where the n-th hamonics fits to the graph
    if (x<0.0) x=0.0;
    x*=N_RES_POINTS;
    REALTYPE dx=x-floor(x);x=floor(x);
    int kx1=(int)x; if (kx1>=N_RES_POINTS) kx1=N_RES_POINTS-1;
    int kx2=kx1+1;if (kx2>=N_RES_POINTS) kx2=N_RES_POINTS-1;
    REALTYPE result=(Prespoints[kx1]*(1.0-dx)+Prespoints[kx2]*dx)/127.0-sum/127.0;
    result=pow(10.0,result*PmaxdB/20.0);
    return(result);
};


/*
 * Smooth the resonance function
 */
void Resonance::smooth(){
    REALTYPE old=Prespoints[0];
    for (int i=0;i<N_RES_POINTS;i++){
        old=old*0.4+Prespoints[i]*0.6;
	Prespoints[i]=(int) old;
    };
    old=Prespoints[N_RES_POINTS-1];
    for (int i=N_RES_POINTS-1;i>0;i--){
        old=old*0.4+Prespoints[i]*0.6;
	Prespoints[i]=(int) old+1;
	if (Prespoints[i]>127) Prespoints[i]=127;
    };
};

/*
 * Randomize the resonance function
 */
void Resonance::randomize(int type)
{
  int r;
  int i;

  r = (int)(zyn_random() * 127.0);

  for (i = 0 ; i < N_RES_POINTS ; i++)
  {
    Prespoints[i] = r;
    if ((zyn_random() < 0.1) && (type == 0))
    {
      r = (int)(zyn_random() * 127.0);
    }

    if ((zyn_random() < 0.3) && (type == 1))
    {
      r = (int)(zyn_random() * 127.0);
    }

    if (type == 2)
    {
      r = (int)(zyn_random() * 127.0);
    }
  };

  smooth();
};

/*
 * Interpolate the peaks
 */
void Resonance::interpolatepeaks(int type){
    int x1=0,y1=Prespoints[0];
    for (int i=1;i<N_RES_POINTS;i++){
	if ((Prespoints[i]!=64)||(i+1==N_RES_POINTS)){
	    int y2=Prespoints[i];
	    for (int k=0;k<i-x1;k++){
		float x=(float) k/(i-x1);
		if (type==0) x=(1-cos(x*PI))*0.5;
		Prespoints[x1+k]=(int)(y1*(1.0-x)+y2*x);
	    };
	    x1=i;
	    y1=y2;
	};
    };
};

/*
 * Get the frequency from x, where x is [0..1]; x is the x coordinate
 */
REALTYPE Resonance::getfreqx(REALTYPE x){
    if (x>1.0) x=1.0;
    REALTYPE octf=pow(2.0,getoctavesfreq());
    return(getcenterfreq()/sqrt(octf)*pow(octf,x));
};

/*
 * Get the x coordinate from frequency (used by the UI)
 */
REALTYPE Resonance::getfreqpos(REALTYPE freq){
    return((log(freq)-log(getfreqx(0.0)))/log(2.0)/getoctavesfreq());
};

/*
 * Get the center frequency of the resonance graph
 */
REALTYPE Resonance::getcenterfreq(){
    return(10000.0*pow(10,-(1.0-Pcenterfreq/127.0)*2.0));
};

/*
 * Get the number of octave that the resonance functions applies to
 */
REALTYPE Resonance::getoctavesfreq(){
    return(0.25+10.0*Poctavesfreq/127.0);
};

// -1 .. 1
void Resonance::set_center(float center)
{
  ctlcenter = pow(3.0, center);
}

// -1 .. 1
void Resonance::set_badnwidth(float bandwidth)
{
  ctlbw = pow(1.5, bandwidth * 0.5);
}
