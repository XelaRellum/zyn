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

#include "filter_sv.h"

//Globals

//FM amplitude tune
#define FM_AMP_MULTIPLIER 14.71280603

#define OSCIL_SMP_EXTRA_SAMPLES 5

// ADDitive note
class ADnote
{
public:
  ADnote(
    struct zyn_addsynth * synth_ptr);

  ~ADnote();

  void
  note_on(
    float panorama,
    bool random_grouping,
    REALTYPE freq,
    REALTYPE velocity,
    bool portamento,
    int midinote);

  void noteout(REALTYPE *outl,REALTYPE *outr); 
  void relasekey();
  bool finished();

private:
  void setfreq(int nvoice,REALTYPE freq);
  void setfreqFM(int nvoice,REALTYPE freq);
  void computecurrentparameters();

  void initparameters(bool random_grouping);

  void KillVoice(unsigned int voice_index);
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
  bool m_stereo;                // if the note is stereo (allows note Panning)
  int m_midinote;
  REALTYPE m_velocity;
  REALTYPE m_basefreq;

  bool m_note_enabled;

  /***********************************************************/
  /*                    VOICE PARAMETERS                     */
  /***********************************************************/
  struct addsynth_voice * m_voices_ptr; // array with one entry per voice

  /********************************************************/
  /*    INTERNAL VALUES OF THE NOTE AND OF THE VOICES     */
  /********************************************************/

  // time from the start of the note
  REALTYPE m_time;

  // fractional part (skip)
  float * m_osc_pos_lo_ptr;     // array with one entry per voice
  float * m_osc_freq_lo_ptr;    // array with one entry per voice

  // integer part (skip)
  int * m_osc_pos_hi_ptr;       // array with one entry per voice
  int * m_osc_freq_hi_ptr;      // array with one entry per voice

  // fractional part (skip) of the Modullator
  float * m_osc_pos_lo_FM_ptr;  // array with one entry per voice
  float * m_osc_freq_lo_FM_ptr; // array with one entry per voice

  // integer part (skip) of the Modullator
  unsigned short int * m_osc_pos_hi_FM_ptr; // array with one entry per voice
  unsigned short int * m_osc_freq_hi_FM_ptr; // array with one entry per voice

  // used to compute and interpolate the amplitudes of voices and modullators
  float * m_old_amplitude_ptr;  // array with one entry per voice
  float * m_new_amplitude_ptr;  // array with one entry per voice
  float * m_FM_old_amplitude_ptr; // array with one entry per voice
  float * m_FM_new_amplitude_ptr; // array with one entry per voice

  // used by Frequency Modulation (for integration)
  float * m_FM_old_smp_ptr;     // array with one entry per voice
    
  //temporary buffer
  zyn_sample_type * m_tmpwave;
    
  //Filter bypass samples
  zyn_sample_type * m_bypassl;
  zyn_sample_type * m_bypassr;

  //interpolate the amplitudes    
  REALTYPE globaloldamplitude,globalnewamplitude;
    
  // whether it is the first tick (used to fade in the sound)
  bool * m_first_tick_ptr;        // array with one entry per voice
    
  // whether note has portamento 
  bool m_portamento;
    
  //how the fine detunes are made bigger or smaller
  REALTYPE m_bandwidth_detune_multiplier;

  LFO m_amplitude_lfo;
  LFO m_filter_lfo;
  LFO m_frequency_lfo;

  Filter m_filter_left;
  Filter m_filter_right;
  zyn_filter_sv_processor_handle m_filter_sv_processor_left;
  zyn_filter_sv_processor_handle m_filter_sv_processor_right;

  float m_filter_center_pitch;  // octaves
  float m_filter_q_factor;
    
  Envelope m_amplitude_envelope;
  Envelope m_filter_envelope;
  Envelope m_frequency_envelope;

  float m_detune;               // cents

  struct zyn_addsynth * m_synth_ptr;

  float m_volume;               // [ 0 .. 1 ]

  float m_panning;              // [ 0 .. 1 ]

  bool m_punch_enabled;
  float m_punch_initial_value;
  float m_punch_duration;
  float m_punch_t;
};

#endif
