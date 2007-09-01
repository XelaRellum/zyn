/* -*- Mode: C ; c-basic-offset: 2 -*- */
/*****************************************************************************
 *
 * DESCRIPTION:
 *  
 *
 * NOTES:
 *  
 *
 *****************************************************************************/

#ifndef ADDSYNTH_VOICE_H__9CE7A6FD_7DC3_4A78_8F9F_6065E6563DB9__INCLUDED
#define ADDSYNTH_VOICE_H__9CE7A6FD_7DC3_4A78_8F9F_6065E6563DB9__INCLUDED

/***********************************************************/
/*                    VOICE PARAMETERS                     */
/***********************************************************/
struct addsynth_voice
{
  /* If the voice is enabled */
  bool enabled; 

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

  unsigned int fm_type;

  int FMVoice;

  // Voice Output used by other voices if use this as modullator
  REALTYPE *VoiceOut;

  /* Wave of the Voice */ 
  REALTYPE *FMSmp;    

  REALTYPE FMVolume;
  REALTYPE FMDetune; //in cents
    
  Envelope m_fm_frequency_envelope;
  Envelope m_fm_amplitude_envelope;
};

#ifdef __cplusplus
extern "C" {
#endif
#if 0
} /* Adjust editor indent */
#endif


#if 0
{ /* Adjust editor indent */
#endif
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* #ifndef ADDSYNTH_VOICE_H__9CE7A6FD_7DC3_4A78_8F9F_6065E6563DB9__INCLUDED */
