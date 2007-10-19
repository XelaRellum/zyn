/*
  ZynAddSubFX - a software synthesizer

  OscilGen.h - Waveform generator for ADnote
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

#ifndef OSCIL_GEN_H
#define OSCIL_GEN_H

#define ZYN_OSCILLATOR_EXTRA_POINTS 2

struct zyn_oscillator
{
  //Parameters
      
  /* 
     The hmag and hphase starts counting from 0, so the first harmonic(1) has the index 0,
     2-nd harmonic has index 1, ..the 128 harminic has index 127
  */
  unsigned char Phmag[MAX_AD_HARMONICS],Phphase[MAX_AD_HARMONICS];//the MIDI parameters for mag. and phases
  
  
  /*The Type of magnitude:
    0 - Linear
    1 - dB scale (-40)
    2 - dB scale (-60)
    3 - dB scale (-80)
    4 - dB scale (-100)*/
  unsigned char Phmagtype;

  unsigned int base_function; /* The base function used, one of ZYN_OSCILLATOR_BASE_FUNCTION_XXX */
  float base_function_adjust; /* the parameter of the base function, 0..1 */
  
  unsigned char Pbasefuncmodulation;//what modulation is applied to the basefunc
  unsigned char Pbasefuncmodulationpar1,Pbasefuncmodulationpar2,Pbasefuncmodulationpar3;//the parameter of the base function modulation

  /*the Randomness:
    64=no randomness
    63..0 - block type randomness - 0 is maximum
    65..127 - each harmonic randomness - 127 is maximum*/
  unsigned char Prand;

  float waveshaping_drive;      /* 0..100 */
  unsigned int waveshaping_function; /* waveshape type, one of ZYN_OSCILLATOR_WAVESHAPE_TYPE_XXX */

  unsigned char Pfiltertype,Pfilterpar1,Pfilterpar2;
  unsigned char Pfilterbeforews;
  unsigned char Psatype,Psapar;//spectrum adjust

  unsigned char Pamprandpower, Pamprandtype;//amplitude randomness
  int Pharmonicshift;//how the harmonics are shifted
  int Pharmonicshiftfirst;//if the harmonic shift is done before waveshaping and filter

  unsigned char Padaptiveharmonics;//the adaptive harmonics status (off=0,on=1,etc..)
  unsigned char Padaptiveharmonicsbasefreq;//the base frequency of the adaptive harmonic (30..3000Hz)
  unsigned char Padaptiveharmonicspower;//the strength of the effect (0=off,100=full)
  unsigned char Padaptiveharmonicspar;//the parameters in 2,3,4.. modes of adaptive harmonics

  unsigned char Pmodulation;//what modulation is applied to the oscil
  unsigned char Pmodulationpar1,Pmodulationpar2,Pmodulationpar3;//the parameter of the parameters

  bool ADvsPAD;//if it is used by ADsynth or by PADsynth

  // pointer to array of OSCIL_SIZE samples that stores some termporary data
  zyn_sample_type * temporary_samples_ptr;
  struct zyn_fft_freqs * oscillator_fft_frequencies_ptr;

  float sample_rate;
  
  REALTYPE hmag[MAX_AD_HARMONICS],hphase[MAX_AD_HARMONICS];//the magnituides and the phases of the sine/nonsine harmonics

  zyn_fft_handle fft;

  //Internal Data
  unsigned int old_base_function;
  float old_base_function_adjust;

  unsigned char oldhmagtype;

  unsigned int old_waveshaping_function;
  float old_waveshaping_drive;

  int oldfilterpars,oldsapars,oldbasefuncmodulation,oldbasefuncmodulationpar1,oldbasefuncmodulationpar2,oldbasefuncmodulationpar3,oldharmonicshift;
  int oldmodulation,oldmodulationpar1,oldmodulationpar2,oldmodulationpar3;

  // Base Function Frequencies
  struct zyn_fft_freqs basefuncFFTfreqs;

  // Oscillator Frequencies - this is different than the hamonics set-up by the user, it may contains time-domain data if the antialiasing is turned off
  struct zyn_fft_freqs oscilFFTfreqs;

  // true if the oscillator is prepared, false if it is not prepared and it is needed to call prepare() before get()
  bool prepared;
  
  struct zyn_resonance * resonance_ptr; 
  
  unsigned int randseed;

  float modulation_temp[OSCIL_SIZE + ZYN_OSCILLATOR_EXTRA_POINTS];
};

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif

void
zyn_oscillator_init(
  struct zyn_oscillator * oscillator_ptr,
  float sample_rate,
  zyn_fft_handle fft,
  struct zyn_resonance * resonance_ptr,
  zyn_sample_type * temporary_samples_ptr,
  struct zyn_fft_freqs * oscillator_fft_frequencies_ptr);

void
zyn_oscillator_uninit(
  struct zyn_oscillator * oscillator_ptr);

// makes a new random seed for Amplitude Randomness
// this should be called every note on event
void
zyn_oscillator_new_rand_seed(
  struct zyn_oscillator * oscillator_ptr,
  unsigned int randseed);

// do the antialiasing(cut off higher freqs.),apply randomness and do a IFFT
// returns where should I start getting samples, used in block type randomness
//if freqHz is smaller than 0, return the "un-randomized" sample for UI

short
zyn_oscillator_get(
  struct zyn_oscillator * oscillator_ptr,
  zyn_sample_type *smps,
  float freqHz,
  bool resonance);

#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
