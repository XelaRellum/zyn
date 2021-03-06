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

void
zyn_resonance_init(
  struct zyn_resonance * resonance_ptr)
{
  resonance_ptr->enabled = false;
  resonance_ptr->maxdB = 20;
  resonance_ptr->centerfreq = 64;           // 1 kHz
  resonance_ptr->octavesfreq = 64;
  resonance_ptr->protectthefundamental = 0;
  resonance_ptr->center = 1.0;
  resonance_ptr->bw = 1.0;
  for (int i = 0 ; i < N_RES_POINTS ; i++)
  {
    resonance_ptr->points[i] = 64;
  }
}

/*
 * Get the center frequency of the resonance graph
 */
float
zyn_resonance_get_center_freq(
  struct zyn_resonance * resonance_ptr)
{
  return 10000.0 * pow(10, -(1.0 - resonance_ptr->centerfreq / 127.0) * 2.0);
}

/*
 * Get the number of octave that the resonance functions applies to
 */
float
zyn_resonance_get_octaves_freq(
  struct zyn_resonance * resonance_ptr)
{
  return 0.25 + 10.0 * resonance_ptr->octavesfreq / 127.0;
}

/*
 * Get the frequency from x, where x is [0..1]; x is the x coordinate
 */
float
zyn_resonance_get_freq_x(
  struct zyn_resonance * resonance_ptr,
  float x)
{
  float octf;

  if (x > 1.0)
  {
    x = 1.0;
  }

  octf = pow(2.0, zyn_resonance_get_octaves_freq(resonance_ptr));

  return zyn_resonance_get_center_freq(resonance_ptr) / sqrt(octf) * pow(octf,x);
}

/*
 * Apply the resonance to FFT data
 */
void
zyn_resonance_apply(
  struct zyn_resonance * resonance_ptr,
  int n,
  struct zyn_fft_freqs * fftdata_ptr,
  float freq)
{
  float sum;
  float l1;
  float l2;
  int i;
  float x;
  float dx;
  int kx1;
  int kx2;
  float y;

  // if the resonance is disabled
  if (!resonance_ptr->enabled)
  {
    return;
  }

  sum = 0.0;
  l1 = log(zyn_resonance_get_freq_x(resonance_ptr, 0.0) * resonance_ptr->center);
  l2 = log(2.0) * zyn_resonance_get_octaves_freq(resonance_ptr) * resonance_ptr->bw;

  for (i = 0 ; i < N_RES_POINTS ; i++)
  {
    if (sum < resonance_ptr->points[i])
    {
      sum = resonance_ptr->points[i];
    }
  }

  if (sum < 1.0)
  {
    sum = 1.0;
  }

  for (i = 1 ; i < n ; i++)
  {
    x = (log(freq * i) - l1) / l2; // compute where the n-th hamonics fits to the graph
    if (x < 0.0)
    {
      x = 0.0;
    }

    x *= N_RES_POINTS;
    dx = x - floor(x);
    x = floor(x);

    kx1 = (int)x;
    if (kx1 >= N_RES_POINTS)
    {
      kx1 = N_RES_POINTS - 1;
    }

    kx2 = kx1 + 1;
    if (kx2 >= N_RES_POINTS)
    {
      kx2 = N_RES_POINTS - 1;
    }

    y = (resonance_ptr->points[kx1] * (1.0 - dx) + resonance_ptr->points[kx2] * dx) / 127.0 - sum / 127.0;
	
    y = pow(10.0, y * resonance_ptr->maxdB / 20.0);
	
    if (resonance_ptr->protectthefundamental != 0 && i == 1)
    {
      y = 1.0;
    }
	
    fftdata_ptr->c[i] *= y;
    fftdata_ptr->s[i] *= y;
  }
}

#if 0

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

/*
 * Set a point of resonance function with a value
 */
void Resonance::setpoint(int n,unsigned char p){
  if ((n<0)||(n>=N_RES_POINTS)) return;
  Prespoints[n]=p;
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
 * Get the x coordinate from frequency (used by the UI)
 */
REALTYPE Resonance::getfreqpos(REALTYPE freq){
  return((log(freq)-log(getfreqx(0.0)))/log(2.0)/getoctavesfreq());
};

#endif
