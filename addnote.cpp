/*
  ZynAddSubFX - a software synthesizer
 
  ADnote.C - The "additive" synthesizer
  Copyright (C) 2006,2007,2008,2009 Nedko Arnaudov <nedko@arnaudov.name>
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
#include <stdio.h>
#include <assert.h>

#include "globals.h"
#include "resonance.h"
#include "fft.h"
#include "oscillator.h"
#include "resonance.h"
#include "envelope_parameters.h"
#include "lfo_parameters.h"
#include "filter_parameters.h"
#include "lfo.h"
#include "filter_base.h"
#include "analog_filter.h"
#include "sv_filter.h"
#include "formant_filter.h"
#include "filter.h"
#include "envelope.h"
#include "addsynth.h"
#include "portamento.h"
#include "addsynth_internal.h"
#include "addnote.h"

#define LOG_LEVEL LOG_LEVEL_ERROR
#include "log.h"

/***********************************************************/
/*                    VOICE PARAMETERS                     */
/***********************************************************/
struct addsynth_voice
{
  /* If the voice is enabled */
  bool enabled; 

  /* Voice Type (sound/noise)*/
  bool white_noise;

  /* Filter Bypass */
  int filterbypass;
          
  /* Delay (ticks) */
  int DelayTicks;
    
  /* Waveform of the Voice */ 
  zyn_sample_type * OscilSmp;    

  /************************************
   *     FREQUENCY PARAMETERS          *
   ************************************/

  struct zyn_fixed_detune fixed_detune;

  // cents = basefreq*VoiceDetune
  REALTYPE Detune,FineDetune;
    
  Envelope m_frequency_envelope;
  LFO m_frequency_lfo;

  /***************************
   *   AMPLITUDE PARAMETERS   *
   ***************************/

  /* Panning 0.0=left, 0.5 - center, 1.0 = right */
  REALTYPE Panning;
  REALTYPE Volume;// [-1.0 .. 1.0]

  Envelope m_amplitude_envelope;
  LFO m_amplitude_lfo;

  /*************************
   *   FILTER PARAMETERS    *
   *************************/
    
  Filter m_voice_filter;
    
  REALTYPE FilterCenterPitch;/* Filter center Pitch*/
  REALTYPE FilterFreqTracking;

  Envelope m_filter_envelope;
  LFO m_filter_lfo;

  /****************************
   *   MODULLATOR PARAMETERS   *
   ****************************/

  unsigned int fm_type;

  int FMVoice;

  // Voice Output used by other voices if use this as modullator
  zyn_sample_type * VoiceOut;

  /* Wave of the Voice */ 
  zyn_sample_type * FMSmp;

  REALTYPE FMVolume;
  REALTYPE FMDetune; //in cents
    
  Envelope m_fm_frequency_envelope;
  Envelope m_fm_amplitude_envelope;
};

//FM amplitude tune
#define FM_AMP_MULTIPLIER 14.71280603

#define OSCIL_SMP_EXTRA_SAMPLES 5

// ADDitive note
struct addnote
{
  // GLOBALS
  bool stereo;                // if the note is stereo (allows note Panning)
  int midinote;
  REALTYPE velocity;
  REALTYPE basefreq;

  bool note_enabled;

  /***********************************************************/
  /*                    VOICE PARAMETERS                     */
  /***********************************************************/
  struct addsynth_voice * voices_ptr; // array with one entry per voice

  /********************************************************/
  /*    INTERNAL VALUES OF THE NOTE AND OF THE VOICES     */
  /********************************************************/

  // time from the start of the note
  REALTYPE time;

  // fractional part (skip)
  float * osc_pos_lo_ptr;     // array with one entry per voice
  float * osc_freq_lo_ptr;    // array with one entry per voice

  // integer part (skip)
  int * osc_pos_hi_ptr;       // array with one entry per voice
  int * osc_freq_hi_ptr;      // array with one entry per voice

  // fractional part (skip) of the Modullator
  float * osc_pos_lo_FM_ptr;  // array with one entry per voice
  float * osc_freq_lo_FM_ptr; // array with one entry per voice

  // integer part (skip) of the Modullator
  unsigned short int * osc_pos_hi_FM_ptr; // array with one entry per voice
  unsigned short int * osc_freq_hi_FM_ptr; // array with one entry per voice

  // used to compute and interpolate the amplitudes of voices and modullators
  float * old_amplitude_ptr;  // array with one entry per voice
  float * new_amplitude_ptr;  // array with one entry per voice
  float * FM_old_amplitude_ptr; // array with one entry per voice
  float * FM_new_amplitude_ptr; // array with one entry per voice

  // used by Frequency Modulation (for integration)
  float * FM_old_smp_ptr;     // array with one entry per voice
    
  //temporary buffer
  zyn_sample_type * tmpwave;
    
  //Filter bypass samples
  zyn_sample_type * bypassl;
  zyn_sample_type * bypassr;

  //interpolate the amplitudes    
  REALTYPE globaloldamplitude;
  REALTYPE globalnewamplitude;
    
  // whether it is the first tick (used to fade in the sound)
  bool * first_tick_ptr;        // array with one entry per voice
    
  // whether note has portamento 
  bool portamento;
    
  //how the fine detunes are made bigger or smaller
  REALTYPE bandwidth_detune_multiplier;

  LFO amplitude_lfo;
  LFO filter_lfo;
  LFO frequency_lfo;

  int filter_category;     // One of ZYN_FILTER_TYPE_XXX
  Filter filter_left;
  Filter filter_right;
  zyn_filter_processor_handle filter_sv_processor_left;
  zyn_filter_processor_handle filter_sv_processor_right;

  float filter_center_pitch;  // octaves
  float filter_q_factor;
    
  Envelope amplitude_envelope;
  Envelope filter_envelope;
  Envelope frequency_envelope;

  float detune;               // cents

  struct zyn_addsynth * synth_ptr;

  float volume;               // [ 0 .. 1 ]

  float panning;              // [ 0 .. 1 ]

  bool punch_enabled;
  float punch_initial_value;
  float punch_duration;
  float punch_t;
};

bool
zyn_addnote_create(
  struct zyn_addsynth * synth_ptr,
  zyn_addnote_handle * handle_ptr)
{
  struct addnote * note_ptr;
  unsigned int voice_index;

  // we still need to use C++ allocation because some constructors need to be invoked
  // For example, AnalogFilter has virtual methods
  note_ptr = new addnote;
  if (note_ptr == NULL)
  {
    return false;
  }

  note_ptr->tmpwave = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * SOUND_BUFFER_SIZE);
  note_ptr->bypassl = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * SOUND_BUFFER_SIZE);
  note_ptr->bypassr = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * SOUND_BUFFER_SIZE);

  note_ptr->voices_ptr = (struct addsynth_voice *)malloc(sizeof(struct addsynth_voice) * synth_ptr->voices_count);
  for (voice_index = 0 ; voice_index < synth_ptr->voices_count ; voice_index++)
  {
    // the extra points contains the first point
    note_ptr->voices_ptr[voice_index].OscilSmp = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * (OSCIL_SIZE + OSCIL_SMP_EXTRA_SAMPLES));
    note_ptr->voices_ptr[voice_index].FMSmp = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * (OSCIL_SIZE + OSCIL_SMP_EXTRA_SAMPLES));
    note_ptr->voices_ptr[voice_index].VoiceOut = (zyn_sample_type *)malloc(sizeof(zyn_sample_type) * SOUND_BUFFER_SIZE);
  }

  note_ptr->osc_pos_hi_ptr = (int *)malloc(sizeof(int) * synth_ptr->voices_count);
  note_ptr->osc_pos_lo_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  note_ptr->osc_pos_hi_FM_ptr = (unsigned short int *)malloc(sizeof(unsigned short int) * synth_ptr->voices_count);
  note_ptr->osc_pos_lo_FM_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  note_ptr->osc_freq_hi_ptr = (int *)malloc(sizeof(int) * synth_ptr->voices_count);
  note_ptr->osc_freq_lo_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  note_ptr->osc_freq_hi_FM_ptr = (unsigned short int *)malloc(sizeof(unsigned short int) * synth_ptr->voices_count);
  note_ptr->osc_freq_lo_FM_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  note_ptr->FM_old_smp_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  note_ptr->first_tick_ptr = (bool *)malloc(sizeof(bool) * synth_ptr->voices_count);

  note_ptr->old_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  note_ptr->new_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  note_ptr->FM_old_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);
  note_ptr->FM_new_amplitude_ptr = (float *)malloc(sizeof(float) * synth_ptr->voices_count);

  note_ptr->stereo = synth_ptr->stereo;

  note_ptr->detune = zyn_get_detune(
    synth_ptr->detune.type,
    synth_ptr->detune.coarse,
    synth_ptr->detune.fine);

  /*
   * Get the Multiplier of the fine detunes of the voices
   */
  note_ptr->bandwidth_detune_multiplier = synth_ptr->detune_bandwidth;
  note_ptr->bandwidth_detune_multiplier =
    pow(
      2.0,
      note_ptr->bandwidth_detune_multiplier * pow(fabs(note_ptr->bandwidth_detune_multiplier), 0.2) * 5.0);

  note_ptr->note_enabled = false;

  note_ptr->synth_ptr = synth_ptr;

  if (!zyn_filter_sv_processor_create(synth_ptr->filter_sv, &note_ptr->filter_sv_processor_left))
  {
  }

  if (!zyn_filter_sv_processor_create(synth_ptr->filter_sv, &note_ptr->filter_sv_processor_right))
  {
  }

  *handle_ptr = (zyn_addnote_handle)note_ptr;
  return true;
}

/*
 * Get Voice base frequency
 */
static
inline
REALTYPE
getvoicebasefreq(
  struct addnote * note_ptr,
  int nvoice)
{
  REALTYPE detune;
  REALTYPE frequency;
  int equal_temperate;
  REALTYPE tmp;

  detune = note_ptr->voices_ptr[nvoice].Detune / 100.0;
  detune += note_ptr->voices_ptr[nvoice].FineDetune / 100.0 * note_ptr->synth_ptr->bandwidth_relbw * note_ptr->bandwidth_detune_multiplier;
  detune += note_ptr->detune / 100.0;

  switch (note_ptr->voices_ptr[nvoice].fixed_detune.mode)
  {
  case ZYN_DETUNE_NORMAL:
    frequency = note_ptr->basefreq;
    break;
  case ZYN_DETUNE_FIXED_440:
    frequency = 440.0;
    break;
  case ZYN_DETUNE_EQUAL_TEMPERATE:
    // the frequency varies according the keyboard note
    equal_temperate = note_ptr->voices_ptr[nvoice].fixed_detune.equal_temperate;
    tmp = (note_ptr->midinote - 69.0) / 12.0 * (pow(2.0, (equal_temperate - 1) / 63.0) - 1.0);
							 
    if (equal_temperate <= 64)
    {
      frequency = 440 * pow(2.0, tmp);
    }
    else
    {
      frequency = 440 * pow(3.0, tmp);
    }
    break;
  default:
    assert(0);
    return 440;
  }

  return frequency * pow(2.0, detune / 12.0);
}

/*
 * Get Voice's Modullator base frequency
 */
static
inline
REALTYPE
getFMvoicebasefreq(
  struct addnote * note_ptr,
  int nvoice)
{
  REALTYPE detune = note_ptr->voices_ptr[nvoice].FMDetune / 100.0;
  return getvoicebasefreq(note_ptr, nvoice) * pow(2, detune / 12.0);
}

/*
 * Kill a voice of ADnote
 */
static
inline
void
kill_voice(
  struct addnote * note_ptr,
  unsigned int voice_index)
{
  // silence the voice, perhaps is used by another voice
  silence_buffer(note_ptr->voices_ptr[voice_index].VoiceOut, SOUND_BUFFER_SIZE);

  note_ptr->voices_ptr[voice_index].enabled = false;
}

/*
 * Computes the frequency of an oscillator
 */
static
inline
void
setfreq(
  struct addnote * note_ptr,
  int nvoice,
  REALTYPE freq)
{
  REALTYPE speed;

  freq = fabs(freq);

  speed = freq * REALTYPE(OSCIL_SIZE) / note_ptr->synth_ptr->sample_rate;
  if (speed > OSCIL_SIZE)
  {
    speed = OSCIL_SIZE;
  }

  F2I(speed, note_ptr->osc_freq_hi_ptr[nvoice]);

  note_ptr->osc_freq_lo_ptr[nvoice] = speed - floor(speed);
}

/*
 * Computes the frequency of an modullator oscillator
 */
static
inline
void
setfreqFM(
  struct addnote * note_ptr,
  int nvoice,
  REALTYPE freq)
{
  REALTYPE speed;

  freq = fabs(freq);

  speed = freq * REALTYPE(OSCIL_SIZE) / note_ptr->synth_ptr->sample_rate;
  if (speed > OSCIL_SIZE)
  {
    speed = OSCIL_SIZE;
  }

  F2I(speed, note_ptr->osc_freq_hi_FM_ptr[nvoice]);
  note_ptr->osc_freq_lo_FM_ptr[nvoice] = speed - floor(speed);
}

/*
 * Fadein in a way that removes clicks but keep sound "punchy"
 */
static
inline
void
fadein(
  struct addnote * note_ptr,
  REALTYPE *smps)
{
  REALTYPE tmp;
  int zerocrossings;
  int i;
  int n;

  zerocrossings = 0;
  for (i = 1 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    if ((smps[i - 1] < 0.0) && (smps[i] > 0.0))
    {
      // this is only the possitive crossings
      zerocrossings++;
    }
  }

  tmp = (SOUND_BUFFER_SIZE - 1.0) / (zerocrossings + 1) / 3.0;
  if (tmp < 8.0)
  {
    tmp=8.0;
  }

  F2I(tmp, n); // how many samples is the fade-in

  if (n > SOUND_BUFFER_SIZE)
  {
    n = SOUND_BUFFER_SIZE;
  }

  for (i = 0 ; i < n ; i++)
  {
    // fade-in
    tmp = 0.5 - cos((REALTYPE)i / (REALTYPE)n * PI) * 0.5;
    smps[i] *= tmp;
  }
}

/*
 * Computes the Oscillator (Without Modulation) - LinearInterpolation
 */
static
inline
void
ComputeVoiceOscillator_LinearInterpolation(
  struct addnote * note_ptr,
  int voice_index)
{
  int i,poshi;
  REALTYPE poslo;

  poshi = note_ptr->osc_pos_hi_ptr[voice_index];
  poslo = note_ptr->osc_pos_lo_ptr[voice_index];
  REALTYPE * smps = note_ptr->voices_ptr[voice_index].OscilSmp;

  for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    note_ptr->tmpwave[i] = smps[poshi] * (1.0 - poslo) + smps[poshi + 1] * poslo;
    poslo += note_ptr->osc_freq_lo_ptr[voice_index];

    if (poslo >= 1.0)
    {
      poslo -= 1.0;
      poshi++;
    }

    poshi += note_ptr->osc_freq_hi_ptr[voice_index];
    poshi &= OSCIL_SIZE - 1;
  }

  note_ptr->osc_pos_hi_ptr[voice_index] = poshi;
  note_ptr->osc_pos_lo_ptr[voice_index] = poslo;
}



/*
 * Computes the Oscillator (Without Modulation) - CubicInterpolation
 *
 The differences from the Linear are to little to deserve to be used. This is because I am using a large OSCIL_SIZE (>512)
 inline void ADnote::ComputeVoiceOscillator_CubicInterpolation(int voice_index){
 int i,poshi;
 REALTYPE poslo;

 poshi=note_ptr->osc_pos_hi_ptr[voice_index];
 poslo=note_ptr->osc_pos_lo_ptr[voice_index];
 REALTYPE *smps=note_ptr->voices_ptr[voice_index].OscilSmp;
 REALTYPE xm1,x0,x1,x2,a,b,c;
 for (i=0;i<SOUND_BUFFER_SIZE;i++){
 xm1=smps[poshi];
 x0=smps[poshi+1];
 x1=smps[poshi+2];
 x2=smps[poshi+3];
 a=(3.0 * (x0-x1) - xm1 + x2) / 2.0;
 b = 2.0*x1 + xm1 - (5.0*x0 + x2) / 2.0;
 c = (x1 - xm1) / 2.0;
 note_ptr->tmpwave[i]=(((a * poslo) + b) * poslo + c) * poslo + x0;
 printf("a\n");
 //note_ptr->tmpwave[i]=smps[poshi]*(1.0-poslo)+smps[poshi+1]*poslo;
 poslo+=note_ptr->osc_freq_lo_ptr[voice_index];
 if (poslo>=1.0) {
 poslo-=1.0;
 poshi++;
 }
 poshi+=note_ptr->osc_freq_hi_ptr[voice_index];
 poshi&=OSCIL_SIZE-1;
 }
 note_ptr->osc_pos_hi_ptr[voice_index]=poshi;
 note_ptr->osc_pos_lo_ptr[voice_index]=poslo;
 }
*/

/*
 * Computes the Oscillator (Morphing)
 */
static
inline
void
ComputeVoiceOscillatorMorph(
  struct addnote * note_ptr,
  int voice_index)
{
  int i;
  REALTYPE amp;

  ComputeVoiceOscillator_LinearInterpolation(note_ptr, voice_index);

  if (note_ptr->FM_new_amplitude_ptr[voice_index] > 1.0)
  {
    note_ptr->FM_new_amplitude_ptr[voice_index] = 1.0;
  }

  if (note_ptr->FM_old_amplitude_ptr[voice_index] > 1.0)
  {
    note_ptr->FM_old_amplitude_ptr[voice_index] = 1.0;
  }

  if (note_ptr->voices_ptr[voice_index].FMVoice >= 0)
  {
    //if I use VoiceOut[] as modullator
    int FMVoice = note_ptr->voices_ptr[voice_index].FMVoice;
    for (i=0;i<SOUND_BUFFER_SIZE;i++)
    {
      amp = INTERPOLATE_AMPLITUDE(
        note_ptr->FM_old_amplitude_ptr[voice_index],
        note_ptr->FM_new_amplitude_ptr[voice_index],
        i,
        SOUND_BUFFER_SIZE);

      note_ptr->tmpwave[i] = note_ptr->tmpwave[i] * (1.0 - amp) + amp * note_ptr->voices_ptr[FMVoice].VoiceOut[i];
    }
  }
  else
  {
    int poshiFM = note_ptr->osc_pos_hi_FM_ptr[voice_index];
    REALTYPE posloFM = note_ptr->osc_pos_lo_FM_ptr[voice_index];

    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      amp = INTERPOLATE_AMPLITUDE(
        note_ptr->FM_old_amplitude_ptr[voice_index],
        note_ptr->FM_new_amplitude_ptr[voice_index],
        i,
        SOUND_BUFFER_SIZE);

      note_ptr->tmpwave[i] = note_ptr->tmpwave[i] * (1.0 - amp) + amp * (note_ptr->voices_ptr[voice_index].FMSmp[poshiFM] * (1 - posloFM) + note_ptr->voices_ptr[voice_index].FMSmp[poshiFM + 1] * posloFM);

      posloFM += note_ptr->osc_freq_lo_FM_ptr[voice_index];

      if (posloFM >= 1.0)
      {
        posloFM -= 1.0;
        poshiFM++;
      }

      poshiFM += note_ptr->osc_freq_hi_FM_ptr[voice_index];
      poshiFM &= OSCIL_SIZE - 1;
    }

    note_ptr->osc_pos_hi_FM_ptr[voice_index] = poshiFM;
    note_ptr->osc_pos_lo_FM_ptr[voice_index] = posloFM;
  }
}

/*
 * Computes the Oscillator (Ring Modulation)
 */
static
inline
void
ComputeVoiceOscillatorRingModulation(
  struct addnote * note_ptr,
  int voice_index)
{
  int i;
  REALTYPE amp;

  ComputeVoiceOscillator_LinearInterpolation(note_ptr, voice_index);

  if (note_ptr->FM_new_amplitude_ptr[voice_index] > 1.0)
  {
    note_ptr->FM_new_amplitude_ptr[voice_index] = 1.0;
  }

  if (note_ptr->FM_old_amplitude_ptr[voice_index] > 1.0)
  {
    note_ptr->FM_old_amplitude_ptr[voice_index] = 1.0;
  }

  if (note_ptr->voices_ptr[voice_index].FMVoice >= 0)
  {
    // if I use VoiceOut[] as modullator
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      amp = INTERPOLATE_AMPLITUDE(
        note_ptr->FM_old_amplitude_ptr[voice_index],
        note_ptr->FM_new_amplitude_ptr[voice_index],
        i,
        SOUND_BUFFER_SIZE);

      int FMVoice = note_ptr->voices_ptr[voice_index].FMVoice;

      for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
      {
        note_ptr->tmpwave[i] *= (1.0 - amp) + amp * note_ptr->voices_ptr[FMVoice].VoiceOut[i];
      }
    }
  }
  else
  {
    int poshiFM=note_ptr->osc_pos_hi_FM_ptr[voice_index];
    REALTYPE posloFM=note_ptr->osc_pos_lo_FM_ptr[voice_index];

    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      amp = INTERPOLATE_AMPLITUDE(note_ptr->FM_old_amplitude_ptr[voice_index], note_ptr->FM_new_amplitude_ptr[voice_index], i, SOUND_BUFFER_SIZE);
      note_ptr->tmpwave[i] *= (note_ptr->voices_ptr[voice_index].FMSmp[poshiFM] * (1.0 - posloFM) + note_ptr->voices_ptr[voice_index].FMSmp[poshiFM + 1] * posloFM) * amp + (1.0 - amp);

      posloFM += note_ptr->osc_freq_lo_FM_ptr[voice_index];

      if (posloFM >= 1.0)
      {
        posloFM -= 1.0;
        poshiFM++;
      }

      poshiFM += note_ptr->osc_freq_hi_FM_ptr[voice_index];
      poshiFM &= OSCIL_SIZE-1;
    }

    note_ptr->osc_pos_hi_FM_ptr[voice_index] = poshiFM;
    note_ptr->osc_pos_lo_FM_ptr[voice_index] = posloFM;
  }
}

/*
 * Computes the Oscillator (Phase Modulation or Frequency Modulation)
 */
static
inline
void
ComputeVoiceOscillatorFrequencyModulation(
  struct addnote * note_ptr,
  int voice_index,
  int FMmode)
{
  int carposhi;
  int i,FMmodfreqhi;
  REALTYPE FMmodfreqlo,carposlo;

  if (note_ptr->voices_ptr[voice_index].FMVoice>=0){
    //if I use VoiceOut[] as modulator
    for (i=0;i<SOUND_BUFFER_SIZE;i++) note_ptr->tmpwave[i]=note_ptr->voices_ptr[note_ptr->voices_ptr[voice_index].FMVoice].VoiceOut[i];
  } else {
    //Compute the modulator and store it in note_ptr->tmpwave[]
    int poshiFM=note_ptr->osc_pos_hi_FM_ptr[voice_index];
    REALTYPE posloFM=note_ptr->osc_pos_lo_FM_ptr[voice_index];

    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      note_ptr->tmpwave[i]=(note_ptr->voices_ptr[voice_index].FMSmp[poshiFM]*(1.0-posloFM)
                  +note_ptr->voices_ptr[voice_index].FMSmp[poshiFM+1]*posloFM);
      posloFM+=note_ptr->osc_freq_lo_FM_ptr[voice_index];
      if (posloFM>=1.0) {
        posloFM=fmod(posloFM,1.0);
        poshiFM++;
      }
      poshiFM+=note_ptr->osc_freq_hi_FM_ptr[voice_index];
      poshiFM&=OSCIL_SIZE-1;
    }
    note_ptr->osc_pos_hi_FM_ptr[voice_index]=poshiFM;
    note_ptr->osc_pos_lo_FM_ptr[voice_index]=posloFM;
  }
  // Amplitude interpolation
  if (ABOVE_AMPLITUDE_THRESHOLD(note_ptr->FM_old_amplitude_ptr[voice_index],note_ptr->FM_new_amplitude_ptr[voice_index])){
    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      note_ptr->tmpwave[i]*=INTERPOLATE_AMPLITUDE(note_ptr->FM_old_amplitude_ptr[voice_index]
                                        ,note_ptr->FM_new_amplitude_ptr[voice_index],i,SOUND_BUFFER_SIZE);
    }
  } else for (i=0;i<SOUND_BUFFER_SIZE;i++) note_ptr->tmpwave[i]*=note_ptr->FM_new_amplitude_ptr[voice_index];


  //normalize makes all sample-rates, oscil_sizes toproduce same sound
  if (FMmode!=0){//Frequency modulation
    REALTYPE normalize = OSCIL_SIZE / 262144.0 * 44100.0 / note_ptr->synth_ptr->sample_rate;
    for (i=0;i<SOUND_BUFFER_SIZE;i++){
      note_ptr->FM_old_smp_ptr[voice_index]=fmod(note_ptr->FM_old_smp_ptr[voice_index]+note_ptr->tmpwave[i]*normalize,OSCIL_SIZE);
      note_ptr->tmpwave[i]=note_ptr->FM_old_smp_ptr[voice_index];
    }
  } else {//Phase modulation
    REALTYPE normalize=OSCIL_SIZE/262144.0;
    for (i=0;i<SOUND_BUFFER_SIZE;i++) note_ptr->tmpwave[i]*=normalize;
  }

  for (i=0;i<SOUND_BUFFER_SIZE;i++){
    F2I(note_ptr->tmpwave[i],FMmodfreqhi);
    FMmodfreqlo=fmod(note_ptr->tmpwave[i]+0.0000000001,1.0);
    if (FMmodfreqhi<0) FMmodfreqlo++;

    //carrier
    carposhi=note_ptr->osc_pos_hi_ptr[voice_index]+FMmodfreqhi;
    carposlo=note_ptr->osc_pos_lo_ptr[voice_index]+FMmodfreqlo;

    if (carposlo>=1.0) {
      carposhi++;
      carposlo=fmod(carposlo,1.0);
    }
    carposhi&=(OSCIL_SIZE-1);

    note_ptr->tmpwave[i]=note_ptr->voices_ptr[voice_index].OscilSmp[carposhi]*(1.0-carposlo)
      +note_ptr->voices_ptr[voice_index].OscilSmp[carposhi+1]*carposlo;

    note_ptr->osc_pos_lo_ptr[voice_index]+=note_ptr->osc_freq_lo_ptr[voice_index];
    if (note_ptr->osc_pos_lo_ptr[voice_index]>=1.0) {
      note_ptr->osc_pos_lo_ptr[voice_index]=fmod(note_ptr->osc_pos_lo_ptr[voice_index],1.0);
      note_ptr->osc_pos_hi_ptr[voice_index]++;
    }

    note_ptr->osc_pos_hi_ptr[voice_index]+=note_ptr->osc_freq_hi_ptr[voice_index];
    note_ptr->osc_pos_hi_ptr[voice_index]&=OSCIL_SIZE-1;
  }
}

#if 0
/*Calculeaza Oscilatorul cu PITCH MODULATION*/
static
inline
void
ComputeVoiceOscillatorPitchModulation(
  struct addnote * note_ptr,
  int voice_index)
{
  // TODO
}
#endif

/*
 * Computes the Noise
 */
static
inline
void
ComputeVoiceNoise(
  struct addnote * note_ptr,
  int voice_index)
{
  for (int i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
  {
    note_ptr->tmpwave[i] = zyn_random() * 2.0 - 1.0;
  }
}

#define note_ptr ((struct addnote *)handle)

void
zyn_addnote_note_on(
  zyn_addnote_handle handle,
  float panorama,
  bool random_grouping,
  REALTYPE freq,
  REALTYPE velocity,
  bool portamento,
  int midinote)
{
  unsigned int voice_index;
  unsigned int i;
  float filter_velocity_adjust;
  int voice_oscillator_index;
  REALTYPE tmp;
  unsigned char detune_type;

  note_ptr->portamento = portamento;
  note_ptr->midinote = midinote;
  note_ptr->note_enabled = true;
  note_ptr->basefreq = freq;

  if (velocity > 1.0)
  {
    note_ptr->velocity = 1.0;
  }
  else
  {
    note_ptr->velocity = velocity;
  }

  note_ptr->time = 0.0;

  note_ptr->panning = (panorama + 1.0) / 2; // -1..1 -> 0 - 1

  note_ptr->filter_category = note_ptr->synth_ptr->filter_type;

  if (note_ptr->filter_category == ZYN_FILTER_TYPE_STATE_VARIABLE)
  {
    filter_velocity_adjust = note_ptr->synth_ptr->m_filter_velocity_sensing_amount * 6.0 * // velocity sensing
      (zyn_velocity_scale(note_ptr->velocity, note_ptr->synth_ptr->m_filter_velocity_scale_function) - 1);

    zyn_filter_sv_processor_init(note_ptr->filter_sv_processor_left, freq, filter_velocity_adjust);
    if (note_ptr->stereo)
    {
      zyn_filter_sv_processor_init(note_ptr->filter_sv_processor_right, freq, filter_velocity_adjust);
    }
  }
  else
  {
    note_ptr->filter_center_pitch =
      note_ptr->synth_ptr->m_filter_params.getfreq() + // center freq
      note_ptr->synth_ptr->m_filter_velocity_sensing_amount * 6.0 * // velocity sensing
      (zyn_velocity_scale(note_ptr->velocity, note_ptr->synth_ptr->m_filter_velocity_scale_function) - 1);
    note_ptr->filter_center_pitch += note_ptr->synth_ptr->m_filter_params.getfreqtracking(note_ptr->basefreq);
  }

  if (note_ptr->synth_ptr->PPunchStrength != 0)
  {
    note_ptr->punch_enabled = true;
    note_ptr->punch_t = 1.0; // start from 1.0 and to 0.0
    note_ptr->punch_initial_value = pow(10, 1.5 * note_ptr->synth_ptr->PPunchStrength / 127.0) - 1.0;
    note_ptr->punch_initial_value *= VelF(note_ptr->velocity, note_ptr->synth_ptr->PPunchVelocitySensing);

    REALTYPE time = pow(10, 3.0 * note_ptr->synth_ptr->PPunchTime / 127.0) / 10000.0; // 0.1 .. 100 ms

    REALTYPE stretch = pow(440.0/freq, note_ptr->synth_ptr->PPunchStretch / 64.0);

    note_ptr->punch_duration = 1.0 / (time * note_ptr->synth_ptr->sample_rate * stretch);
  }
  else
  {
    note_ptr->punch_enabled = false;
  }

  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    zyn_oscillator_new_rand_seed(
      &note_ptr->synth_ptr->voices_params_ptr[voice_index].oscillator,
      rand());

    note_ptr->voices_ptr[voice_index].FMVoice = -1;

    if (!note_ptr->synth_ptr->voices_params_ptr[voice_index].enabled)
    {
      note_ptr->voices_ptr[voice_index].enabled = false;
      continue; // the voice is disabled
    }

    note_ptr->voices_ptr[voice_index].enabled = true;
    note_ptr->voices_ptr[voice_index].fixed_detune = note_ptr->synth_ptr->voices_params_ptr[voice_index].fixed_detune;

    // calculate voice detune
    {
      if (note_ptr->synth_ptr->voices_params_ptr[voice_index].detune.type == ZYN_DETUNE_TYPE_GLOBAL)
      {
        detune_type = note_ptr->synth_ptr->detune.type;
      }
      else
      {
        detune_type = note_ptr->synth_ptr->voices_params_ptr[voice_index].detune.type;
      }

      // coarse detune
      note_ptr->voices_ptr[voice_index].Detune = zyn_get_detune(detune_type, note_ptr->synth_ptr->voices_params_ptr[voice_index].detune.coarse, 8192);

      // fine detune
      note_ptr->voices_ptr[voice_index].FineDetune = zyn_get_detune(detune_type, 0, note_ptr->synth_ptr->voices_params_ptr[voice_index].detune.fine);
    }

    // calculate voice fm detune
    {
      if (note_ptr->synth_ptr->voices_params_ptr[voice_index].fm_detune.type == ZYN_DETUNE_TYPE_GLOBAL)
      {
        detune_type = note_ptr->synth_ptr->detune.type;
      }
      else
      {
        detune_type = note_ptr->synth_ptr->voices_params_ptr[voice_index].fm_detune.type;
      }

      note_ptr->voices_ptr[voice_index].FMDetune =
        zyn_get_detune(
          detune_type,
          note_ptr->synth_ptr->voices_params_ptr[voice_index].fm_detune.coarse,
          note_ptr->synth_ptr->voices_params_ptr[voice_index].fm_detune.fine);
    }

    note_ptr->osc_pos_hi_ptr[voice_index] = 0;
    note_ptr->osc_pos_lo_ptr[voice_index] = 0.0;
    note_ptr->osc_pos_hi_FM_ptr[voice_index] = 0;
    note_ptr->osc_pos_lo_FM_ptr[voice_index] = 0.0;

    // Get the voice's oscil or external's voice oscil
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].Pextoscil != -1)
    {
      voice_oscillator_index = note_ptr->synth_ptr->voices_params_ptr[voice_index].Pextoscil;
    }
    else
    {
      voice_oscillator_index = voice_index;
    }

    if (!random_grouping)
    {
      zyn_oscillator_new_rand_seed(
        &note_ptr->synth_ptr->voices_params_ptr[voice_oscillator_index].oscillator,
        rand());
    }

    note_ptr->osc_pos_hi_ptr[voice_index] =
      zyn_oscillator_get(
        &note_ptr->synth_ptr->voices_params_ptr[voice_oscillator_index].oscillator,
        note_ptr->voices_ptr[voice_index].OscilSmp,
        getvoicebasefreq(note_ptr, voice_index),
        note_ptr->synth_ptr->voices_params_ptr[voice_index].resonance);

    // I store the first elments to the last position for speedups
    for (i = 0 ; i < OSCIL_SMP_EXTRA_SAMPLES ; i++)
    {
      note_ptr->voices_ptr[voice_index].OscilSmp[OSCIL_SIZE + i] = note_ptr->voices_ptr[voice_index].OscilSmp[i];
    }

    note_ptr->osc_pos_hi_ptr[voice_index] += (int)((note_ptr->synth_ptr->voices_params_ptr[voice_index].Poscilphase - 64.0) / 128.0 * OSCIL_SIZE + OSCIL_SIZE * 4);
    note_ptr->osc_pos_hi_ptr[voice_index] %= OSCIL_SIZE;

    note_ptr->voices_ptr[voice_index].FilterCenterPitch = note_ptr->synth_ptr->voices_params_ptr[voice_index].m_filter_params.getfreq();
    note_ptr->voices_ptr[voice_index].filterbypass = note_ptr->synth_ptr->voices_params_ptr[voice_index].Pfilterbypass;

    note_ptr->voices_ptr[voice_index].fm_type = note_ptr->synth_ptr->voices_params_ptr[voice_index].fm_type;

    note_ptr->voices_ptr[voice_index].FMVoice = note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVoice;

    // Compute the Voice's modulator volume (incl. damping)
    REALTYPE fmvoldamp = pow(440.0 / getvoicebasefreq(note_ptr, voice_index), note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVolumeDamp / 64.0 - 1.0);
    switch (note_ptr->voices_ptr[voice_index].fm_type)
    {
    case ZYN_FM_TYPE_PHASE_MOD:
      fmvoldamp = pow(440.0 / getvoicebasefreq(note_ptr, voice_index), note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVolumeDamp / 64.0);
      note_ptr->voices_ptr[voice_index].FMVolume = (exp(note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVolume / 127.0 * FM_AMP_MULTIPLIER) - 1.0) * fmvoldamp * 4.0;
      break;
    case ZYN_FM_TYPE_FREQ_MOD:
      note_ptr->voices_ptr[voice_index].FMVolume = exp(note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVolume / 127.0 * FM_AMP_MULTIPLIER);
      note_ptr->voices_ptr[voice_index].FMVolume -= 1.0;
      note_ptr->voices_ptr[voice_index].FMVolume *= fmvoldamp * 4.0;
      break;
#if 0                           // ???????????
    case ZYN_FM_TYPE_PITCH_MOD:
      note_ptr->voices_ptr[voice_index].FMVolume = (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVolume / 127.0 * 8.0) * fmvoldamp;
      break;
#endif
    default:
      if (fmvoldamp > 1.0)
      {
        fmvoldamp = 1.0;
      }

      note_ptr->voices_ptr[voice_index].FMVolume = note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVolume / 127.0 * fmvoldamp;
    }

    // Voice's modulator velocity sensing
    note_ptr->voices_ptr[voice_index].FMVolume *= VelF(note_ptr->velocity, note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMVelocityScaleFunction);

    note_ptr->FM_old_smp_ptr[voice_index] = 0.0; // this is for FM (integration)

    note_ptr->first_tick_ptr[voice_index] = true;
    note_ptr->voices_ptr[voice_index].DelayTicks = (int)((exp(note_ptr->synth_ptr->voices_params_ptr[voice_index].PDelay / 127.0 * log(50.0)) - 1.0) / SOUND_BUFFER_SIZE / 10.0 * note_ptr->synth_ptr->sample_rate);
  } // voices loop

  // Global Parameters
  note_ptr->frequency_envelope.init(note_ptr->synth_ptr->sample_rate, &note_ptr->synth_ptr->m_frequency_envelope_params, note_ptr->basefreq);

  note_ptr->frequency_lfo.init(
    note_ptr->synth_ptr->sample_rate,
    note_ptr->basefreq,
    &note_ptr->synth_ptr->frequency_lfo_params,
    ZYN_LFO_TYPE_FREQUENCY);

  note_ptr->amplitude_envelope.init(note_ptr->synth_ptr->sample_rate, &note_ptr->synth_ptr->m_amplitude_envelope_params, note_ptr->basefreq);

  note_ptr->amplitude_lfo.init(
    note_ptr->synth_ptr->sample_rate,
    note_ptr->basefreq,
    &note_ptr->synth_ptr->amplitude_lfo_params,
    ZYN_LFO_TYPE_AMPLITUDE);

  note_ptr->volume = 4.0 * pow(0.1, 3.0 * (1.0 - note_ptr->synth_ptr->PVolume / 96.0)); // -60 dB .. 0 dB
  note_ptr->volume *= VelF(note_ptr->velocity, note_ptr->synth_ptr->PAmpVelocityScaleFunction); // velocity sensing

  note_ptr->amplitude_envelope.envout_dB(); // discard the first envelope output

  note_ptr->globalnewamplitude = note_ptr->volume * note_ptr->amplitude_envelope.envout_dB() * note_ptr->amplitude_lfo.amplfoout();

  note_ptr->filter_left.init(note_ptr->synth_ptr->sample_rate, &note_ptr->synth_ptr->m_filter_params);
  if (note_ptr->stereo)
  {
    note_ptr->filter_right.init(note_ptr->synth_ptr->sample_rate, &note_ptr->synth_ptr->m_filter_params);
  }

  note_ptr->filter_envelope.init(note_ptr->synth_ptr->sample_rate, &note_ptr->synth_ptr->m_filter_envelope_params, note_ptr->basefreq);

  note_ptr->filter_lfo.init(
    note_ptr->synth_ptr->sample_rate,
    note_ptr->basefreq,
    &note_ptr->synth_ptr->filter_lfo_params,
    ZYN_LFO_TYPE_FILTER);

  note_ptr->filter_q_factor = note_ptr->synth_ptr->m_filter_params.getq();

  // Forbids the Modulation Voice to be greater or equal than voice
  for (i = 0 ; i < note_ptr->synth_ptr->voices_count ; i++)
  {
    if (note_ptr->voices_ptr[i].FMVoice >= (int)i)
    {
      note_ptr->voices_ptr[i].FMVoice = -1;
    }
  }

  // Voice Parameter init
  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    if (!note_ptr->voices_ptr[voice_index].enabled)
    {
      continue;
    }

    LOG_DEBUG("Starting %s voice (%u, %p)", note_ptr->synth_ptr->voices_params_ptr[voice_index].white_noise ? "white noise" : "signal", voice_index, note_ptr->synth_ptr->voices_params_ptr + voice_index);

    note_ptr->voices_ptr[voice_index].white_noise = note_ptr->synth_ptr->voices_params_ptr[voice_index].white_noise;

    /* Voice Amplitude Parameters Init */

    note_ptr->voices_ptr[voice_index].Volume = pow(0.1, 3.0 * (1.0 - note_ptr->synth_ptr->voices_params_ptr[voice_index].PVolume / 127.0)); // -60 dB .. 0 dB
    note_ptr->voices_ptr[voice_index].Volume *= VelF(note_ptr->velocity, note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpVelocityScaleFunction); // velocity

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PVolumeminus != 0)
    {
      note_ptr->voices_ptr[voice_index].Volume = -note_ptr->voices_ptr[voice_index].Volume;
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PPanning == 0)
    {
      note_ptr->voices_ptr[voice_index].Panning = zyn_random(); // random panning
    }
    else
    {
      note_ptr->voices_ptr[voice_index].Panning = note_ptr->synth_ptr->voices_params_ptr[voice_index].PPanning / 128.0;
    }

    note_ptr->new_amplitude_ptr[voice_index] = 1.0;
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_amplitude_envelope.init(
        note_ptr->synth_ptr->sample_rate,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].m_amplitude_envelope_params,
        note_ptr->basefreq);

      note_ptr->voices_ptr[voice_index].m_amplitude_envelope.envout_dB(); // discard the first envelope sample
      note_ptr->new_amplitude_ptr[voice_index] *= note_ptr->voices_ptr[voice_index].m_amplitude_envelope.envout_dB();
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpLfoEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_amplitude_lfo.init(
        note_ptr->synth_ptr->sample_rate,
        note_ptr->basefreq,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].amplitude_lfo_params,
        ZYN_LFO_TYPE_AMPLITUDE);

      note_ptr->new_amplitude_ptr[voice_index] *= note_ptr->voices_ptr[voice_index].m_amplitude_lfo.amplfoout();
    }

    /* Voice Frequency Parameters Init */
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_frequency_envelope.init(
        note_ptr->synth_ptr->sample_rate,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].m_frequency_envelope_params,
        note_ptr->basefreq);
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFreqLfoEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_frequency_lfo.init(
        note_ptr->synth_ptr->sample_rate,
        note_ptr->basefreq,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].frequency_lfo_params,
        ZYN_LFO_TYPE_FREQUENCY);
    }

    /* Voice Filter Parameters Init */
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_voice_filter.init(
        note_ptr->synth_ptr->sample_rate,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].m_filter_params);
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_filter_envelope.init(
        note_ptr->synth_ptr->sample_rate,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].m_filter_envelope_params,
        note_ptr->basefreq);
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterLfoEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_filter_lfo.init(
        note_ptr->synth_ptr->sample_rate,
        note_ptr->basefreq,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].filter_lfo_params,
        ZYN_LFO_TYPE_FILTER);
    }

    note_ptr->voices_ptr[voice_index].FilterFreqTracking = note_ptr->synth_ptr->voices_params_ptr[voice_index].m_filter_params.getfreqtracking(note_ptr->basefreq);

    /* Voice Modulation Parameters Init */
    if (note_ptr->voices_ptr[voice_index].fm_type != ZYN_FM_TYPE_NONE &&
        note_ptr->voices_ptr[voice_index].FMVoice < 0)
    {
      zyn_oscillator_new_rand_seed(
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].modulator_oscillator,
        rand());

      // Perform Anti-aliasing only on MORPH or RING MODULATION

      if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PextFMoscil != -1)
      {
        voice_oscillator_index = note_ptr->synth_ptr->voices_params_ptr[voice_index].PextFMoscil;
      }
      else
      {
        voice_oscillator_index = voice_index;
      }

      if ((note_ptr->synth_ptr->voices_params_ptr[voice_oscillator_index].modulator_oscillator.Padaptiveharmonics != 0) ||
          (note_ptr->voices_ptr[voice_index].fm_type == ZYN_FM_TYPE_MORPH) ||
          (note_ptr->voices_ptr[voice_index].fm_type == ZYN_FM_TYPE_RING_MOD))
      {
        tmp = getFMvoicebasefreq(note_ptr, voice_index);
      }
      else
      {
        tmp = 1.0;
      }

      if (!random_grouping)
      {
        zyn_oscillator_new_rand_seed(
          &note_ptr->synth_ptr->voices_params_ptr[voice_oscillator_index].modulator_oscillator,
          rand());
      }

      note_ptr->osc_pos_hi_FM_ptr[voice_index] = note_ptr->osc_pos_hi_ptr[voice_index];
      note_ptr->osc_pos_hi_FM_ptr[voice_index] += zyn_oscillator_get(
        &note_ptr->synth_ptr->voices_params_ptr[voice_oscillator_index].modulator_oscillator,
        note_ptr->voices_ptr[voice_index].FMSmp,
        tmp,
        false);
      note_ptr->osc_pos_hi_FM_ptr[voice_index] %= OSCIL_SIZE;

      for (i = 0 ; i < OSCIL_SMP_EXTRA_SAMPLES ; i++)
      {
        note_ptr->voices_ptr[voice_index].FMSmp[OSCIL_SIZE + i] = note_ptr->voices_ptr[voice_index].FMSmp[i];
      }

      note_ptr->osc_pos_hi_FM_ptr[voice_index] += (int)((note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMoscilphase - 64.0) / 128.0 * OSCIL_SIZE + OSCIL_SIZE * 4);
      note_ptr->osc_pos_hi_FM_ptr[voice_index] %= OSCIL_SIZE;
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_fm_frequency_envelope.init(
        note_ptr->synth_ptr->sample_rate,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].m_fm_frequency_envelope_params,
        note_ptr->basefreq);
    }

    note_ptr->FM_new_amplitude_ptr[voice_index] = note_ptr->voices_ptr[voice_index].FMVolume;
    //m_FM_new_amplitude_ptr[voice_index] *= note_ptr->ctl->fmamp.relamp; // 0..1

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled != 0)
    {
      note_ptr->voices_ptr[voice_index].m_fm_amplitude_envelope.init(
        note_ptr->synth_ptr->sample_rate,
        &note_ptr->synth_ptr->voices_params_ptr[voice_index].m_fm_amplitude_envelope_params,
        note_ptr->basefreq);

      note_ptr->FM_new_amplitude_ptr[voice_index] *= note_ptr->voices_ptr[voice_index].m_fm_amplitude_envelope.envout_dB();
    }
  } // voice parameter init loop

  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    for (i = voice_index + 1 ; i < note_ptr->synth_ptr->voices_count ; i++)
    {
      if (note_ptr->voices_ptr[i].FMVoice == (int)voice_index)
      {
        silence_buffer(note_ptr->voices_ptr[voice_index].VoiceOut, SOUND_BUFFER_SIZE);
      }
    }
  }
}

/*
 * Kill the note
 */
void
zyn_addnote_force_disable(
  zyn_addnote_handle handle)
{
  unsigned int voice_index;

  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    if (note_ptr->voices_ptr[voice_index].enabled)
    {
      kill_voice(note_ptr, voice_index);
    }
  }

  note_ptr->note_enabled = false;
}

void
zyn_addnote_destroy(
  zyn_addnote_handle handle)
{
  unsigned int voice_index;

  if (note_ptr->note_enabled)
  {
    zyn_addnote_force_disable(handle);
  }

  zyn_filter_sv_processor_destroy(note_ptr->filter_sv_processor_left);
  zyn_filter_sv_processor_destroy(note_ptr->filter_sv_processor_right);

  free(note_ptr->old_amplitude_ptr);
  free(note_ptr->new_amplitude_ptr);

  free(note_ptr->FM_old_amplitude_ptr);
  free(note_ptr->FM_new_amplitude_ptr);

  free(note_ptr->first_tick_ptr);

  free(note_ptr->FM_old_smp_ptr);

  free(note_ptr->osc_freq_hi_ptr);
  free(note_ptr->osc_freq_lo_ptr);
  free(note_ptr->osc_freq_hi_FM_ptr);
  free(note_ptr->osc_freq_lo_FM_ptr);

  free(note_ptr->osc_pos_hi_ptr);
  free(note_ptr->osc_pos_lo_ptr);
  free(note_ptr->osc_pos_hi_FM_ptr);
  free(note_ptr->osc_pos_lo_FM_ptr);

  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    // the extra points contains the first point
    free(note_ptr->voices_ptr[voice_index].OscilSmp);
    free(note_ptr->voices_ptr[voice_index].FMSmp);
    free(note_ptr->voices_ptr[voice_index].VoiceOut);
  }

  free(note_ptr->voices_ptr);

  free(note_ptr->tmpwave);
  free(note_ptr->bypassl);
  free(note_ptr->bypassr);

  delete note_ptr;
}

/*
 * Compute the ADnote samples
 */
bool
zyn_addnote_noteout(
  zyn_addnote_handle handle,
  REALTYPE *outl,
  REALTYPE *outr)
{
  int i;
  unsigned int voice_index;
  float filter_adjust;

  silence_two_buffers(outl, outr, SOUND_BUFFER_SIZE);

  if (!note_ptr->note_enabled)
  {
    return false;
  }

  silence_two_buffers(note_ptr->bypassl, note_ptr->bypassr, SOUND_BUFFER_SIZE);

  /*
   * Compute all the parameters for each tick
   */
  {
    unsigned int voice_index;
    float voicefreq;
    float voicepitch;
    float filterpitch;
    float filterfreq;
    float FMfreq;
    float FMrelativepitch;
    float globalpitch;
    float temp_filter_frequency;

    globalpitch =
      0.01 * (note_ptr->frequency_envelope.envout() +
              note_ptr->frequency_lfo.lfoout() * note_ptr->synth_ptr->modwheel_relmod);

    note_ptr->globaloldamplitude = note_ptr->globalnewamplitude;

    note_ptr->globalnewamplitude =
      note_ptr->volume *
      note_ptr->amplitude_envelope.envout_dB() *
      note_ptr->amplitude_lfo.amplfoout();

    if (note_ptr->filter_category != ZYN_FILTER_TYPE_STATE_VARIABLE)
    {
      temp_filter_frequency = note_ptr->filter_left.getrealfreq(note_ptr->filter_center_pitch + note_ptr->filter_envelope.envout() + note_ptr->filter_lfo.lfoout());

      note_ptr->filter_left.setfreq_and_q(temp_filter_frequency, note_ptr->filter_q_factor);
      if (note_ptr->stereo)
      {
        note_ptr->filter_right.setfreq_and_q(temp_filter_frequency, note_ptr->filter_q_factor);
      }
    }

    // compute the portamento, if it is used by this note
    REALTYPE portamentofreqrap=1.0;
    if (note_ptr->portamento)
    {
      // this voice use portamento
      portamentofreqrap = note_ptr->synth_ptr->portamento.freqrap;

      if (!note_ptr->synth_ptr->portamento.used)
      {
        // the portamento has finished
        note_ptr->portamento = false;     // this note is no longer "portamented"
      }
    }

    //compute parameters for all voices
    for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
    {
      if (!note_ptr->voices_ptr[voice_index].enabled)
      {
        continue;
      }

      note_ptr->voices_ptr[voice_index].DelayTicks -= 1;

      if (note_ptr->voices_ptr[voice_index].DelayTicks > 0)
      {
        continue;
      }

      /*******************/
      /* Voice Amplitude */
      /*******************/
      note_ptr->old_amplitude_ptr[voice_index] = note_ptr->new_amplitude_ptr[voice_index];
      note_ptr->new_amplitude_ptr[voice_index] = 1.0;

      if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
      {
        note_ptr->new_amplitude_ptr[voice_index] *= note_ptr->voices_ptr[voice_index].m_amplitude_envelope.envout_dB();
      }

      if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpLfoEnabled)
      {
        note_ptr->new_amplitude_ptr[voice_index] *= note_ptr->voices_ptr[voice_index].m_amplitude_lfo.amplfoout();
      }

      /****************/
      /* Voice Filter */
      /****************/
      if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterEnabled)
      {
        filterpitch = note_ptr->voices_ptr[voice_index].FilterCenterPitch;

        if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled)
        {
          filterpitch += note_ptr->voices_ptr[voice_index].m_filter_envelope.envout();
        }

        if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterLfoEnabled)
        {
          filterpitch += note_ptr->voices_ptr[voice_index].m_filter_lfo.lfoout();
        }

        filterfreq = filterpitch + note_ptr->voices_ptr[voice_index].FilterFreqTracking;
        filterfreq = note_ptr->voices_ptr[voice_index].m_voice_filter.getrealfreq(filterfreq);

        note_ptr->voices_ptr[voice_index].m_voice_filter.setfreq(filterfreq);
      }

      // compute only if the voice isn't noise
      if (!note_ptr->voices_ptr[voice_index].white_noise)
      {
        /*******************/
        /* Voice Frequency */
        /*******************/
        voicepitch=0.0;
        if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFreqLfoEnabled)
        {
          voicepitch += note_ptr->voices_ptr[voice_index].m_frequency_lfo.lfoout() / 100.0 * note_ptr->synth_ptr->bandwidth_relbw;
        }

        if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled)
        {
          voicepitch += note_ptr->voices_ptr[voice_index].m_frequency_envelope.envout() / 100.0;
        }

        voicefreq = getvoicebasefreq(note_ptr, voice_index) * pow(2, (voicepitch + globalpitch) / 12.0); // Hz frequency
        voicefreq *= note_ptr->synth_ptr->pitch_bend_relative_frequency; // change the frequency by the controller
        setfreq(note_ptr, voice_index, voicefreq * portamentofreqrap);

        /***************/
        /*  Modulator */
        /***************/
        if (note_ptr->voices_ptr[voice_index].fm_type != ZYN_FM_TYPE_NONE)
        {
          FMrelativepitch = note_ptr->voices_ptr[voice_index].FMDetune / 100.0;
          if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled)
          {
            FMrelativepitch += note_ptr->voices_ptr[voice_index].m_fm_frequency_envelope.envout() / 100;
          }

          FMfreq = pow(2.0, FMrelativepitch / 12.0) * voicefreq * portamentofreqrap;
          setfreqFM(note_ptr, voice_index, FMfreq);

          note_ptr->FM_old_amplitude_ptr[voice_index] = note_ptr->FM_new_amplitude_ptr[voice_index];
          note_ptr->FM_new_amplitude_ptr[voice_index] = note_ptr->voices_ptr[voice_index].FMVolume;
          //m_FM_new_amplitude_ptr[voice_index] *= note_ptr->ctl->fmamp.relamp; // 0..1

          if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled)
          {
            note_ptr->FM_new_amplitude_ptr[voice_index] *= note_ptr->voices_ptr[voice_index].m_fm_amplitude_envelope.envout_dB();
          }
        }
      }
    }

    note_ptr->time += (REALTYPE)SOUND_BUFFER_SIZE / note_ptr->synth_ptr->sample_rate;
  }

  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    if (!note_ptr->voices_ptr[voice_index].enabled || note_ptr->voices_ptr[voice_index].DelayTicks > 0)
    {
      continue;
    }

    if (!note_ptr->voices_ptr[voice_index].white_noise) //voice mode = sound
    {
      switch (note_ptr->voices_ptr[voice_index].fm_type)
      {
      case ZYN_FM_TYPE_MORPH:
        ComputeVoiceOscillatorMorph(note_ptr, voice_index);
        break;
      case ZYN_FM_TYPE_RING_MOD:
        ComputeVoiceOscillatorRingModulation(note_ptr, voice_index);
        break;
      case ZYN_FM_TYPE_PHASE_MOD:
        ComputeVoiceOscillatorFrequencyModulation(note_ptr, voice_index, 0);
        break;
      case ZYN_FM_TYPE_FREQ_MOD:
        ComputeVoiceOscillatorFrequencyModulation(note_ptr, voice_index, 1);
        break;
#if 0
      case ZYN_FM_TYPE_PITCH_MOD:
        ComputeVoiceOscillatorPitchModulation(note_ptr, voice_index);
        break;
#endif
      default:
        ComputeVoiceOscillator_LinearInterpolation(note_ptr, voice_index);
        //if (config.cfg.Interpolation) ComputeVoiceOscillator_CubicInterpolation(note_ptr, voice_index);
      }
    }
    else
    {
      ComputeVoiceNoise(note_ptr, voice_index);
    }

    // Voice Processing

    // Amplitude
    if (ABOVE_AMPLITUDE_THRESHOLD(note_ptr->old_amplitude_ptr[voice_index],note_ptr->new_amplitude_ptr[voice_index])){
      int rest=SOUND_BUFFER_SIZE;
      //test if the amplitude if raising and the difference is high
      if ((note_ptr->new_amplitude_ptr[voice_index]>note_ptr->old_amplitude_ptr[voice_index])&&((note_ptr->new_amplitude_ptr[voice_index]-note_ptr->old_amplitude_ptr[voice_index])>0.25)){
        rest=10;
        if (rest>SOUND_BUFFER_SIZE) rest=SOUND_BUFFER_SIZE;
        for (int i=0;i<SOUND_BUFFER_SIZE-rest;i++) note_ptr->tmpwave[i]*=note_ptr->old_amplitude_ptr[voice_index];
      }
      // Amplitude interpolation
      for (i=0;i<rest;i++){
        note_ptr->tmpwave[i+(SOUND_BUFFER_SIZE-rest)]*=INTERPOLATE_AMPLITUDE(note_ptr->old_amplitude_ptr[voice_index]
                                                                   ,note_ptr->new_amplitude_ptr[voice_index],i,rest);
      }
    } else for (i=0;i<SOUND_BUFFER_SIZE;i++) note_ptr->tmpwave[i]*=note_ptr->new_amplitude_ptr[voice_index];

    // Fade in
    if (note_ptr->first_tick_ptr[voice_index])
    {
      fadein(note_ptr, &note_ptr->tmpwave[0]);
      note_ptr->first_tick_ptr[voice_index] = false;
    }

    // Filter
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterEnabled)
    {
      note_ptr->voices_ptr[voice_index].m_voice_filter.filterout(&note_ptr->tmpwave[0]);
    }

    //check if the amplitude envelope is finished, if yes, the voice will be fadeout
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      if (note_ptr->voices_ptr[voice_index].m_amplitude_envelope.finished())
      {
        for (i=0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          note_ptr->tmpwave[i] *= 1.0 - (REALTYPE)i / (REALTYPE)SOUND_BUFFER_SIZE;
        }
      }

      // the voice is killed later
    }


    // Put the ADnote samples in VoiceOut (without appling Global volume, because I wish to use this voice as a modullator)
    if (note_ptr->voices_ptr[voice_index].VoiceOut!=NULL)
    {
      for (i=0;i<SOUND_BUFFER_SIZE;i++)
      {
        note_ptr->voices_ptr[voice_index].VoiceOut[i] = note_ptr->tmpwave[i];
      }
    }

    // Add the voice that do not bypass the filter to out
    if (note_ptr->voices_ptr[voice_index].filterbypass==0)
    {
      // no bypass

      if (note_ptr->stereo)
      {
        // stereo
        for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          outl[i] += note_ptr->tmpwave[i] * note_ptr->voices_ptr[voice_index].Volume * note_ptr->voices_ptr[voice_index].Panning * 2.0;
          outr[i] += note_ptr->tmpwave[i] * note_ptr->voices_ptr[voice_index].Volume * (1.0 - note_ptr->voices_ptr[voice_index].Panning) * 2.0;
        }
      }
      else
      {
        // mono
        for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          outl[i] += note_ptr->tmpwave[i] * note_ptr->voices_ptr[voice_index].Volume;}
      }
    }
    else
    {
      // bypass the filter

      if (note_ptr->stereo)
      {
        // stereo
        for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          note_ptr->bypassl[i] += note_ptr->tmpwave[i] * note_ptr->voices_ptr[voice_index].Volume * note_ptr->voices_ptr[voice_index].Panning * 2.0;
          note_ptr->bypassr[i] += note_ptr->tmpwave[i] * note_ptr->voices_ptr[voice_index].Volume * (1.0 - note_ptr->voices_ptr[voice_index].Panning) * 2.0;
        }
      }
      else
      {
        // mono
        for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
        {
          note_ptr->bypassl[i] += note_ptr->tmpwave[i] * note_ptr->voices_ptr[voice_index].Volume;
        }
      }
    }
    // check if there is necesary to proces the voice longer (if the Amplitude envelope isn't finished)
    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      if (note_ptr->voices_ptr[voice_index].m_amplitude_envelope.finished())
      {
        kill_voice(note_ptr, voice_index);
      }
    }
  }

  // Processing Global parameters

  if (note_ptr->filter_category == ZYN_FILTER_TYPE_STATE_VARIABLE)
  {
    filter_adjust = note_ptr->filter_envelope.envout() + note_ptr->filter_lfo.lfoout();

    zyn_filter_sv_process(note_ptr->filter_sv_processor_left, filter_adjust, outl);

    if (note_ptr->stereo)
    {
      zyn_filter_sv_process(note_ptr->filter_sv_processor_right, filter_adjust, outr);
    }
  }
  else
  {
    note_ptr->filter_left.filterout(&outl[0]);

    if (note_ptr->stereo)
    {
      note_ptr->filter_right.filterout(&outr[0]);
    }
  }

  if (!note_ptr->stereo)
  {
    // set the right channel=left channel
    for (i=0;i<SOUND_BUFFER_SIZE;i++)
    {
      outr[i]=outl[i];
      note_ptr->bypassr[i]=note_ptr->bypassl[i];
    }
  }

  for (i=0;i<SOUND_BUFFER_SIZE;i++)
  {
//    outl[i]+=note_ptr->bypassl[i];
//    outr[i]+=note_ptr->bypassr[i];
  }

  if (ABOVE_AMPLITUDE_THRESHOLD(note_ptr->globaloldamplitude, note_ptr->globalnewamplitude))
  {
    // Amplitude Interpolation
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      REALTYPE tmpvol = INTERPOLATE_AMPLITUDE(note_ptr->globaloldamplitude, note_ptr->globalnewamplitude, i, SOUND_BUFFER_SIZE);
      outl[i] *= tmpvol * (1.0 - note_ptr->panning);
      outr[i] *= tmpvol * note_ptr->panning;
    }
  }
  else
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      outl[i] *= note_ptr->globalnewamplitude * (1.0 - note_ptr->panning);
      outr[i] *= note_ptr->globalnewamplitude * note_ptr->panning;
    }
  }

  // Apply the punch
  if (note_ptr->punch_enabled)
  {
    for (i=0;i<SOUND_BUFFER_SIZE;i++)
    {
      REALTYPE punchamp = note_ptr->punch_initial_value * note_ptr->punch_t + 1.0;
      outl[i] *= punchamp;
      outr[i] *= punchamp;
      note_ptr->punch_t -= note_ptr->punch_duration;
      if (note_ptr->punch_t < 0.0)
      {
        note_ptr->punch_enabled = false;
        break;
      }
    }
  }

  // Check if the global amplitude is finished.
  // If it does, disable the note
  if (note_ptr->amplitude_envelope.finished())
  {
    for (i = 0 ; i < SOUND_BUFFER_SIZE ; i++)
    {
      // fade-out

      REALTYPE tmp = 1.0 - (REALTYPE)i / (REALTYPE)SOUND_BUFFER_SIZE;

      outl[i] *= tmp;
      outr[i] *= tmp;
    }

    zyn_addnote_force_disable(note_ptr);
    return false;
  }
  else
  {
    return true;
  }
}

/*
 * Relase the key (NoteOff)
 */
void
zyn_addnote_note_off(
  zyn_addnote_handle handle)
{
  unsigned int voice_index;

  for (voice_index = 0 ; voice_index < note_ptr->synth_ptr->voices_count ; voice_index++)
  {
    if (!note_ptr->voices_ptr[voice_index].enabled)
    {
      continue;
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PAmpEnvelopeEnabled)
    {
      note_ptr->voices_ptr[voice_index].m_amplitude_envelope.relasekey();
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFreqEnvelopeEnabled)
    {
      note_ptr->voices_ptr[voice_index].m_frequency_envelope.relasekey();
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFilterEnvelopeEnabled)
    {
      note_ptr->voices_ptr[voice_index].m_filter_envelope.relasekey();
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMFreqEnvelopeEnabled)
    {
      note_ptr->voices_ptr[voice_index].m_fm_frequency_envelope.relasekey();
    }

    if (note_ptr->synth_ptr->voices_params_ptr[voice_index].PFMAmpEnvelopeEnabled)
    {
      note_ptr->voices_ptr[voice_index].m_fm_amplitude_envelope.relasekey();
    }
  }

  note_ptr->frequency_envelope.relasekey();
  note_ptr->filter_envelope.relasekey();
  note_ptr->amplitude_envelope.relasekey();
}

#undef note_ptr
