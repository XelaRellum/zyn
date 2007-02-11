/*
  ZynAddSubFX - a software synthesizer
 
  ADnote.h - The "additive" synthesizer
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

#ifndef AD_NOTE_H
#define AD_NOTE_H

//Globals

//FM amplitude tune
#define FM_AMP_MULTIPLIER 14.71280603

#define OSCIL_SMP_EXTRA_SAMPLES 5

// ADDitive note
class ADnote
{
public:
  ADnote(
    struct zyn_addsynth * synth_ptr,
    Controller * ctl);

  ~ADnote();

  void
  note_on(
    float panorama,
    BOOL random_grouping,
    REALTYPE freq,
    REALTYPE velocity,
    BOOL portamento,
    int midinote);

  void noteout(REALTYPE *outl,REALTYPE *outr); 
  void relasekey();
  BOOL finished();

private:
  void setfreq(int nvoice,REALTYPE freq);
  void setfreqFM(int nvoice,REALTYPE freq);
  void computecurrentparameters();

  void initparameters(BOOL random_grouping);

  void KillVoice(int nvoice);
  void KillNote();
  inline REALTYPE getvoicebasefreq(int nvoice);
  inline REALTYPE getFMvoicebasefreq(int nvoice);
  inline void ComputeVoiceOscillator_LinearInterpolation(int nvoice);
  inline void ComputeVoiceOscillator_CubicInterpolation(int nvoice);
  inline void ComputeVoiceOscillatorMorph(int nvoice);
  inline void ComputeVoiceOscillatorRingModulation(int nvoice);
  inline void ComputeVoiceOscillatorFrequencyModulation(int nvoice,int FMmode);//FMmode=0 for phase modulation, 1 for Frequency modulation
//  inline void ComputeVoiceOscillatorFrequencyModulation(int nvoice);
  inline void ComputeVoiceOscillatorPitchModulation(int nvoice);

  inline void ComputeVoiceNoise(int nvoice);

  inline void fadein(REALTYPE *smps);


  // GLOBALS
  BOOL m_stereo;                // if the note is stereo (allows note Panning)
  int m_midinote;
  REALTYPE m_velocity;
  REALTYPE m_basefreq;

  BOOL m_note_enabled;
  Controller * m_ctl;

  /***********************************************************/
  /*                    VOICE PARAMETERS                     */
  /***********************************************************/
  struct ADnoteVoice{
    /* If the voice is enabled */
    BOOL enabled; 

    /* Voice Type (sound/noise)*/
    int noisetype;

    /* Filter Bypass */
    int filterbypass;
          
    /* Delay (ticks) */
    int DelayTicks;
    
    /* Waveform of the Voice */ 
    REALTYPE *OscilSmp;    

    /************************************
     *     FREQUENCY PARAMETERS          *
     ************************************/
    int fixedfreq;//if the frequency is fixed to 440 Hz
    int fixedfreqET;//if the "fixed" frequency varies according to the note (ET)

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

    FMTYPE FMEnabled;

    int FMVoice;

    // Voice Output used by other voices if use this as modullator
    REALTYPE *VoiceOut;

    /* Wave of the Voice */ 
    REALTYPE *FMSmp;    

    REALTYPE FMVolume;
    REALTYPE FMDetune; //in cents
    
    Envelope m_fm_frequency_envelope;
    Envelope m_fm_amplitude_envelope;
  } m_voices[NUM_VOICES]; 

  /********************************************************/
  /*    INTERNAL VALUES OF THE NOTE AND OF THE VOICES     */
  /********************************************************/

  // time from the start of the note
  REALTYPE m_time;

  //fractional part (skip)
  REALTYPE oscposlo[NUM_VOICES],oscfreqlo[NUM_VOICES];    

  //integer part (skip)
  int oscposhi[NUM_VOICES],oscfreqhi[NUM_VOICES];

  //fractional part (skip) of the Modullator
  REALTYPE oscposloFM[NUM_VOICES],oscfreqloFM[NUM_VOICES]; 

  //integer part (skip) of the Modullator
  unsigned short int oscposhiFM[NUM_VOICES],oscfreqhiFM[NUM_VOICES];

  //used to compute and interpolate the amplitudes of voices and modullators
  REALTYPE oldamplitude[NUM_VOICES],
    newamplitude[NUM_VOICES],
    FMoldamplitude[NUM_VOICES],
    FMnewamplitude[NUM_VOICES];

  //used by Frequency Modulation (for integration)
  REALTYPE FMoldsmp[NUM_VOICES];
    
  //temporary buffer
  REALTYPE * m_tmpwave;
    
  //Filter bypass samples
  REALTYPE * m_bypassl;
  REALTYPE * m_bypassr;

  //interpolate the amplitudes    
  REALTYPE globaloldamplitude,globalnewamplitude;
    
  //1 - if it is the fitst tick (used to fade in the sound)
  char firsttick[NUM_VOICES];
    
  // whether note has portamento 
  BOOL m_portamento;
    
  //how the fine detunes are made bigger or smaller
  REALTYPE m_bandwidth_detune_multiplier;

  LFO m_amplitude_lfo;
  LFO m_filter_lfo;
  LFO m_frequency_lfo;

  Filter m_filter_left;
  Filter m_filter_right;

  float m_filter_center_pitch;  // octaves
  float m_filter_q_factor;
  float m_filter_frequency_tracking;
    
  Envelope m_amplitude_envelope;
  Envelope m_filter_envelope;
  Envelope m_frequency_envelope;

  float m_detune;               // cents

  struct zyn_addsynth * m_synth_ptr;

  float m_volume;               // [ 0 .. 1 ]

  float m_panning;              // [ 0 .. 1 ]

  BOOL m_punch_enabled;
  float m_punch_initial_value;
  float m_punch_duration;
  float m_punch_t;
};

#endif
