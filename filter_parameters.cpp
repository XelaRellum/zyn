/*
  ZynAddSubFX - a software synthesizer
 
  FilterParams.C - Parameters for filter
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

#include "globals.h"
#include "filter_parameters.h"

void FilterParams::init(
  float sample_rate,
  unsigned char Ptype_,
  unsigned char Pfreq_,
  unsigned  char Pq_)
{
  m_sample_rate = sample_rate;

  Dtype=Ptype_;
  Dfreq=Pfreq_;
  Dq=Pq_;

  changed=false;
  defaults();    
}

void FilterParams::defaults(){
  Ptype=Dtype;
  Pfreq=Dfreq;
  Pq=Dq;

  m_additional_stages = 0;
  m_frequency_tracking = 0;
  m_gain = 0;
  Pcategory=0;
    
  Pnumformants=3;
  Pformantslowness=64;
  for (int j=0;j<FF_MAX_VOWELS;j++){
    defaults(j);
  };
    
  Psequencesize=3;
  for (int i=0;i<FF_MAX_SEQUENCE;i++) Psequence[i].nvowel=i%FF_MAX_VOWELS;

  Psequencestretch=40;
  Psequencereversed=0;
  Pcenterfreq=64;//1 kHz
  Poctavesfreq=64;
  Pvowelclearness=64;
};

void FilterParams::defaults(int n){
  int j=n;
  for (int i=0;i<FF_MAX_FORMANTS;i++){
    Pvowels[j].formants[i].freq=(int)(RND*127.0);//some random freqs
    Pvowels[j].formants[i].q=64;
    Pvowels[j].formants[i].amp=127;
  };
};

/*
 * Parameter control
 */
REALTYPE FilterParams::getfreq(){
  return((Pfreq/64.0-1.0)*5.0);
};

REALTYPE FilterParams::getq(){
  return(exp(pow((REALTYPE) Pq/127.0,2)*log(1000.0))-0.9);
};
REALTYPE FilterParams::getfreqtracking(REALTYPE notefreq)
{
  return log(notefreq / 440.0) * m_frequency_tracking / LOG_2;
};

/*
 * Get the center frequency of the formant's graph
 */
REALTYPE FilterParams::getcenterfreq(){
  return(10000.0*pow(10,-(1.0-Pcenterfreq/127.0)*2.0));
};

/*
 * Get the number of octave that the formant functions applies to
 */
REALTYPE FilterParams::getoctavesfreq(){
  return(0.25+10.0*Poctavesfreq/127.0);
};

/*
 * Get the frequency from x, where x is [0..1]
 */
REALTYPE FilterParams::getfreqx(REALTYPE x){
  if (x>1.0) x=1.0;
  REALTYPE octf=pow(2.0,getoctavesfreq());
  return(getcenterfreq()/sqrt(octf)*pow(octf,x));
};

/*
 * Get the x coordinate from frequency (used by the UI)
 */
REALTYPE FilterParams::getfreqpos(REALTYPE freq){
  return((log(freq)-log(getfreqx(0.0)))/log(2.0)/getoctavesfreq());
};


/*
 * Get the freq. response of the formant filter
 */
void FilterParams::formantfilterH(int nvowel,int nfreqs,REALTYPE *freqs)
{
  REALTYPE c[3],d[3];
  REALTYPE filter_freq,filter_q,filter_amp;
  REALTYPE omega,sn,cs,alpha;

  for (int i=0;i<nfreqs;i++) freqs[i]=0.0;

  //for each formant...
  for (int nformant=0;nformant<Pnumformants;nformant++)
  {
    //compute formant parameters(frequency,amplitude,etc.)
    filter_freq=getformantfreq(Pvowels[nvowel].formants[nformant].freq);
    filter_q=getformantq(Pvowels[nvowel].formants[nformant].q)*getq();

    if (m_additional_stages > 0)
    {
      filter_q = (filter_q > 1.0 ? pow(filter_q, 1.0 / (m_additional_stages + 1)) : filter_q);
    }

    filter_amp=getformantamp(Pvowels[nvowel].formants[nformant].amp);


    if (filter_freq <= (m_sample_rate / 2 - 100.0))
    {
      omega = 2 * PI * filter_freq / m_sample_rate;
      sn=sin(omega);
      cs=cos(omega);
      alpha=sn/(2*filter_q);
      REALTYPE tmp=1+alpha;
      c[0]=alpha/tmp*sqrt(filter_q+1);
      c[1]=0;
      c[2]=-alpha/tmp*sqrt(filter_q+1);
      d[1]=-2*cs/tmp*(-1);
      d[2]=(1-alpha)/tmp*(-1);
    }
    else
    {
      continue;
    }

    for (int i=0;i<nfreqs;i++) {
      REALTYPE freq=getfreqx(i/(REALTYPE) nfreqs);

      if (freq > m_sample_rate / 2)
      {
        for (int tmp = i ; tmp < nfreqs ; tmp++)
        {
          freqs[tmp] = 0.0;
        }

        break;
      }

      REALTYPE fr = freq / m_sample_rate * PI * 2.0;
      REALTYPE x=c[0],y=0.0;
      for (int n=1;n<3;n++){
        x+=cos(n*fr)*c[n];
        y-=sin(n*fr)*c[n];
      };
      REALTYPE h=x*x+y*y;
      x=1.0;y=0.0;
      for (int n=1;n<3;n++){
        x-=cos(n*fr)*d[n];
        y+=sin(n*fr)*d[n];
      };
      h=h/(x*x+y*y);
      
      freqs[i] += pow(h, (m_additional_stages + 1.0) / 2.0) * filter_amp;
    };
  };
  for (int i=0;i<nfreqs;i++) {
    if (freqs[i]>0.000000001) freqs[i]=rap2dB(freqs[i]) + m_gain;
    else freqs[i]=-90.0;    
  };

};

/*
 * Transforms a parameter to the real value
 */
REALTYPE FilterParams::getformantfreq(unsigned char freq){
  REALTYPE result=getfreqx(freq/127.0);
  return(result);
};

REALTYPE FilterParams::getformantamp(unsigned char amp){
  REALTYPE result=pow(0.1,(1.0-amp/127.0)*4.0);
  return(result);
};

REALTYPE FilterParams::getformantq(unsigned char q){
  //temp
  REALTYPE result=pow(25.0,(q-32.0)/64.0);
  return(result);
};
