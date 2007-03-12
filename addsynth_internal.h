/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 *   Copyright (C) 2006,2007 Nedko Arnaudov <nedko@arnaudov.name>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; version 2 of the License
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 *****************************************************************************/

#ifndef ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED
#define ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED

class ADnote;

struct note_channel
{
  int midinote;               // MIDI note, -1 when note "channel" is not playing
  ADnote * note_ptr;
};

#define ZYN_FM_TYPE_NONE          0
#define ZYN_FM_TYPE_MORPH         1
#define ZYN_FM_TYPE_RING_MOD      2
#define ZYN_FM_TYPE_PHASE_MOD     3
#define ZYN_FM_TYPE_FREQ_MOD      4
#define ZYN_FM_TYPE_PITCH_MOD     5 /* code for this is disabled for some reason */

struct ADnoteGlobalParam
{
  /******************************************
   *     FREQUENCY GLOBAL PARAMETERS        *
   ******************************************/
  unsigned short int PDetune;//fine detune
  unsigned short int PCoarseDetune;//coarse detune+octave
  unsigned char PDetuneType;//detune type

  unsigned char PBandwidth;//how much the relative fine detunes of the voices are changed

  /********************************************
   *     AMPLITUDE GLOBAL PARAMETERS          *
   ********************************************/

  // 0-127, Master volume
  unsigned char PVolume;

  // 0-127, velocity sensing
  unsigned char PAmpVelocityScaleFunction;

  // 0-127
  unsigned char PPunchStrength;

  // 0-127
  unsigned char PPunchTime;

  // 0-127
  unsigned char PPunchStretch;

  // 0-127
  unsigned char PPunchVelocitySensing;

  /******************************************
   *        FILTER GLOBAL PARAMETERS        *
   ******************************************/
  FilterParams *GlobalFilter;

  // Velocity sensing amount of the Filter
  // 0 .. 127
  unsigned char PFilterVelocityScale;

  // 0 .. 127
  // Velocity Sensing Function of the Filter
  unsigned char PFilterVelocityScaleFunction;

  // RESONANCE
  Resonance *Reson;
};

/***********************************************************/
/*                    VOICE PARAMETERS                     */
/***********************************************************/
struct ADnoteVoiceParam
{
  /* If the voice is enabled */
  unsigned char Enabled;

  /* Type of the voice (0=Sound,1=Noise)*/
  unsigned char Type;

  /* Voice Delay */
  unsigned char PDelay;

  /* If the resonance is enabled for this voice */
  unsigned char Presonance;

  // What external oscil should I use, -1 for internal OscilSmp&FMSmp
  short int Pextoscil,PextFMoscil;
  // it is not allowed that the externoscil,externFMoscil => current voice

  // oscillator phases
  unsigned char Poscilphase,PFMoscilphase;

  // filter bypass
  unsigned char Pfilterbypass;

  /* Voice oscillator */
  OscilGen *OscilSmp;

  /**********************************
   *     FREQUENCY PARAMETERS        *
   **********************************/

  /* If the base frequency is fixed to 440 Hz*/
  unsigned char Pfixedfreq;

  /* Equal temperate (this is used only if the Pfixedfreq is enabled)
     If this parameter is 0, the frequency is fixed (to 440 Hz);
     if this parameter is 64, 1 MIDI halftone -> 1 frequency halftone */
  unsigned char PfixedfreqET;

  /* Fine detune */
  unsigned short int PDetune;

  /* Coarse detune + octave */
  unsigned short int PCoarseDetune;

  /* Detune type */
  unsigned char PDetuneType;

  /* Frequency Envelope */
  unsigned char PFreqEnvelopeEnabled;
  EnvelopeParams m_frequency_envelope_params;

  /* Frequency LFO */
  unsigned char PFreqLfoEnabled;
  struct zyn_lfo_parameters frequency_lfo_params;

  /***************************
   *   AMPLITUDE PARAMETERS   *
   ***************************/

  /* Panning       0 - random
     1 - left
     64 - center
     127 - right
     The Panning is ignored if the instrument is mono */
  unsigned char PPanning;

  /* Voice Volume */
  unsigned char PVolume;

  /* If the Volume negative */
  unsigned char PVolumeminus;

  /* Velocity sensing */
  unsigned char PAmpVelocityScaleFunction;

  /* Amplitude Envelope */
  unsigned char PAmpEnvelopeEnabled;
  EnvelopeParams m_amplitude_envelope_params;

  /* Amplitude LFO */
  unsigned char PAmpLfoEnabled;
  struct zyn_lfo_parameters amplitude_lfo_params;

  /*************************
   *   FILTER PARAMETERS    *
   *************************/

  /* Voice Filter */
  unsigned char PFilterEnabled;
  FilterParams *VoiceFilter;

  /* Filter Envelope */
  unsigned char PFilterEnvelopeEnabled;
  EnvelopeParams m_filter_envelope_params;

  /* LFO Envelope */
  unsigned char PFilterLfoEnabled;
  struct zyn_lfo_parameters filter_lfo_params;

  /****************************
   *   MODULLATOR PARAMETERS   *
   ****************************/

  /* Modullator Parameters, one of ZYN_FM_TYPE_XXX */
  unsigned int fm_type;

  /* Voice that I use as modullator instead of FMSmp.
     It is -1 if I use FMSmp(default).
     It maynot be equal or bigger than current voice */
  short int PFMVoice;

  /* Modullator oscillator */
  OscilGen *FMSmp;

  /* Modullator Volume */
  unsigned char PFMVolume;

  /* Modullator damping at higher frequencies */
  unsigned char PFMVolumeDamp;

  /* Modullator Velocity Sensing */
  unsigned char PFMVelocityScaleFunction;

  /* Fine Detune of the Modullator*/
  unsigned short int PFMDetune;

  /* Coarse Detune of the Modullator */
  unsigned short int PFMCoarseDetune;

  /* The detune type */
  unsigned char PFMDetuneType;

  /* Frequency Envelope of the Modullator */
  unsigned char PFMFreqEnvelopeEnabled;
  EnvelopeParams m_fm_frequency_envelope_params;

  /* Frequency Envelope of the Modullator */
  unsigned char PFMAmpEnvelopeEnabled;
  EnvelopeParams m_fm_amplitude_envelope_params;
};

struct zyn_addsynth
{
  unsigned int polyphony;
  struct note_channel * notes_array;
  FFTwrapper * fft_ptr;
  Controller * ctl_ptr;
  unsigned char velsns;         // velocity sensing (amplitude velocity scale)
  zyn_sample_type oldfreq;      // this is used for portamento

  BOOL random_panorama;         // whether panorama is random for each note
  float panorama;               // -1.0 for left, 0.0 for center, 1.0 for right

  /* The instrument type - MONO/STEREO
     If the mode is MONO, the panning of voices are not used
     Stereo=TRUE, Mono=FALSE. */
  BOOL stereo;                  // stereo or mono

  // How the Harmonic Amplitude is applied to voices that use the same oscillator
  BOOL random_grouping;

  // Amplitude LFO parameters
  struct zyn_lfo_parameters amplitude_lfo_params;

  EnvelopeParams m_amplitude_envelope_params;

  // Filter LFO parameters
  struct zyn_lfo_parameters filter_lfo_params;

  EnvelopeParams m_filter_envelope_params;

  // Frequency LFO parameters
  struct zyn_lfo_parameters frequency_lfo_params;

  EnvelopeParams m_frequency_envelope_params;

  ADnoteGlobalParam GlobalPar;

  ADnoteVoiceParam voices_params[NUM_VOICES];
};

#endif /* #ifndef ADDSYNTH_INTERNAL_H__9870368A_F1C9_4F0D_ADC1_B07ECFF2F9C7__INCLUDED */
