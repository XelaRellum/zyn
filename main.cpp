// -*- Mode: C++ ; c-basic-offset: 2 -*-
/*****************************************************************************
 *
 *   Copyright (C) 2006 Nedko Arnaudov <nedko@arnaudov.name>
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

#include <unistd.h>
#include <jack/jack.h>
extern "C"
{
#include <jack/midiport.h>
}
#include <assert.h>
#include "cpp/ADnote.h"
#include "util.h"

ADnote * note_ptr;
ADnoteParameters * params_ptr;
FFTwrapper * fft;
Controller ctl;
REALTYPE notebasefreq;
unsigned char note;
unsigned char velocity;
int masterkeyshift;
REALTYPE vel;
unsigned char velsns; // velocity sensing (amplitude velocity scale)
REALTYPE oldfreq; //this is used for portamento
int i;
jack_client_t * g_jack_client;
jack_port_t * port_left;
jack_port_t * port_right;
jack_port_t * port_midi;

void midi(unsigned long frames)
{
  void* midi_buf = jack_port_get_buffer(port_midi, frames);
  jack_midi_event_t jack_midi_event;
  jack_nframes_t event_index = 0;
  jack_nframes_t event_count = 
    jack_midi_port_get_info(midi_buf, frames)->event_count;
  unsigned char* midi_data;
  unsigned char type, chan;
  
  while (event_index < event_count)
  {
    jack_midi_event_get(&jack_midi_event, midi_buf, event_index, frames);
    midi_data = jack_midi_event.buffer;
    type = midi_data[0] & 0xF0;
    chan = midi_data[0] & 0x0F;
    
    switch (type)
    {
    case 0x90: /* note-on */

      note = midi_data[1];
      velocity = midi_data[2];
      masterkeyshift = 0;

      if (velocity != 0)
      {
        if (note_ptr == NULL)
        {
          vel = VelF(velocity/127.0, velsns);
          notebasefreq = 440.0*pow(2.0,(note-69.0)/12.0);

          // Portamento
          if (oldfreq < 1.0) oldfreq=notebasefreq;//this is only the first note is played
      
          int portamento=ctl.initportamento(oldfreq,notebasefreq);
      
          oldfreq = notebasefreq;

          printf("new note %u, velocity %u\n", note, velocity);
          note_ptr = new ADnote(params_ptr, &ctl, notebasefreq, vel, portamento, note);
        }

        break;
      }
    
    case 0x80: /* note-off */
      if (note_ptr != NULL)
      {
        printf("release note\n");
        note_ptr->relasekey();
      }
      
    case 0xB0: /* controller */
      //jackmaster->SetController(chan, midi_data[1], midi_data[2]);
      break;
    
    case 0xE0: /* pitch bend */
      //jackmaster->SetController(chan, C_pitchwheel,
      //((midi_data[2] << 7) | midi_data[1]));
      break;

      /* XXX TODO: handle MSB/LSB controllers and RPNs and NRPNs */
    }    
    
    event_index++;
  }
}

int
process(jack_nframes_t nframes, void *arg)
{
  jack_default_audio_sample_t * out_left;
  jack_default_audio_sample_t * out_right;

  assert(nframes == SOUND_BUFFER_SIZE);

  midi(nframes);

  out_left = (jack_default_audio_sample_t *)jack_port_get_buffer(port_left, nframes);
  out_right = (jack_default_audio_sample_t *)jack_port_get_buffer(port_right, nframes);

  if (note_ptr != NULL && note_ptr->ready)
  {
    note_ptr->noteout(out_left, out_right);
  }
  else
  {
    for (i = 0 ; i < nframes ; i++)
    {
      out_left[i] = 0.0;
      out_right[i] = 0.0;
    }
  }

  if (note_ptr != NULL && note_ptr->finished())
  {
    printf("delete note\n");
    delete (note_ptr);
    note_ptr = NULL;
  }
}

int main()
{
  int ret;

  g_jack_client = jack_client_new("zyn-add");
  if (g_jack_client == NULL)
  {
    fprintf(stderr, "Cannot create JACK client.\n");
    fprintf(stderr, "Please make sure JACK daemon is running.\n");
    goto exit;
  }

  printf("JACK client created\n");

  SAMPLE_RATE = jack_get_sample_rate(g_jack_client);

  if (SOUND_BUFFER_SIZE != jack_get_buffer_size(g_jack_client))
  {
    fprintf(
      stderr,
      "Compiled with buffer size of %u, but JACK uses %u.\n",
      (unsigned int)SOUND_BUFFER_SIZE,
      (unsigned int)jack_get_buffer_size(g_jack_client));
    goto exit;
  }

  port_left = jack_port_register(g_jack_client, "out L", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  if (port_left == NULL)
  {
    fprintf(stderr, "Cannot create JACK port");
    goto close_jack;
  }

  port_right = jack_port_register(g_jack_client, "out R", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
  if (port_right == NULL)
  {
    fprintf(stderr, "Cannot create JACK port");
    goto close_jack;
  }

  port_midi = jack_port_register(g_jack_client, "midi_input",	JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
  if (port_right == NULL)
  {
    fprintf(stderr, "Cannot create JACK port");
    goto close_jack;
  }

  ret = jack_set_process_callback(g_jack_client, process, NULL);
  if (ret != 0)
  {
    fprintf(stderr, "Cannot set JACK process callback");
    goto close_jack;
  }

  velsns = 64;
  fft = new FFTwrapper(OSCIL_SIZE);
  params_ptr = new ADnoteParameters(fft);
  note_ptr = NULL;
  ctl.defaults();
  oldfreq=-1.0;

  OscilGen::tmpsmps=new REALTYPE[OSCIL_SIZE];
  newFFTFREQS(&OscilGen::outoscilFFTfreqs,OSCIL_SIZE/2);

  ret = jack_activate(g_jack_client);
  if (ret != 0)
  {
    fprintf(stderr, "Cannot activate JACK client");
    goto close_jack;
  }

  while (1)
  {
    sleep(1);
  }

close_jack:
  jack_client_close(g_jack_client); /* this should clear all other resources we obtained through the client handle */

exit:
  return 0;
}
