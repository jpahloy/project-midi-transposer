#include "transposer.h"

#define META_EVENT_STATUS_CODE 0xFF
#define SYSEX_STATUS_CODE_1 0xF7
#define SYSEX_END_OF_EXCLUSIVE 0xF7
#define SYSEX_STATUS_CODE_2 0xF0
#define MIDI_VOICE_PITCH_BEND 0xE0
#define MIDI_VOICE_CHANNEL_KEY_PRESSURE 0xD0
#define MIDI_VOICE_PROGRAM_CHANGE 0xC0
#define MIDI_CHANNEL_MODE 0xB0
#define MIDI_VOICE_POLY 0xA0
#define MIDI_VOICE_NOTE_ON 0x90
#define MIDI_VOICE_NOTE_OFF 0x80

enum transposer_state_t {DELTA, EVENT_STATUS,
				MIDI_END, MIDI_NOTE, MIDI_OTHER,
				SYSEX, META_TYPE, META_LENGTH, META_SKIP};

enum last_event_status_t {PITCH_BEND_STATUS, CHANNEL_PROGRAM_STATUS,
				 MODE_STATUS, NOTE_STATUS};				 


/*ignores meta type bytes*/
static enum transposer_state_t dfa_meta_type() {
  return META_LENGTH;
}

static enum transposer_state_t dfa_meta_length(char * b_byte, uint32_t * ui_skip_length) {
  uint32_t ui_var_len_byte = *b_byte & 0xFF;
  
  if (*b_byte & 0x80 == 0x80) {
    ui_var_len_byte &= 0x7F; /* gets all but the msb */

    /* make room by shifting and add the data */
    *ui_skip_length <<= 8;
    *ui_skip_length += ui_var_len_byte;
    
    return META_LENGTH;
  }
  else {
    *ui_skip_length <<= 8;
    *ui_skip_length += ui_var_len_byte;
    
    return META_SKIP;
  }
}


/* performs the skip */
static enum transposer_state_t dfa_meta_skip(uint32_t * ui_buffer_pointer, uint32_t ui_skip_length) {
  *ui_buffer_pointer += ui_skip_length - 1;
  return DELTA;
}

/* changes the value of a note by the specified differnce */
static void transpose_note(char * b_note, int i_diff) {
  *b_note += i_diff;
}

/* reads in the variable length integer that 
represents the delta time
*/
static enum transposer_state_t dfa_delta(char * b_byte) {
  /* 0x80 gets the last bit for variable length integers */
  if (*b_byte & 0x80 == 0x80) {
    return DELTA;
  }
  else {
    return EVENT_STATUS;
  }
}


/* handles system exclusive events by checking for the end of exclusive byte */
static enum transposer_state_t dfa_sysex_event(char * b_byte) {
  if (*b_byte & SYSEX_END_OF_EXCLUSIVE == SYSEX_END_OF_EXCLUSIVE) {
    return DELTA;
  }
  else {
    return SYSEX;
  }
}


/* handles the end of midi events */
static enum transposer_state_t dfa_midi_end() {
  return DELTA;
}

/* handles misc midi events where the inner data isn't important */
static enum transposer_state_t dfa_midi_other() {
  return MIDI_END;
}

/* handles the midi note events */
static enum transposer_state_t dfa_midi_note(char * b_byte, int i_diff) {
  transpose_note(b_byte, i_diff);
  return MIDI_END;
}

/* determines the status of the event and handles running status */
static enum transposer_state_t dfa_event_status(char * b_byte, enum last_event_status_t * i_last_status, int i_diff) {
  char c = *b_byte;

  /* goes through all the status codes in decending order*/
  if (c & META_EVENT_STATUS_CODE == META_EVENT_STATUS_CODE) {
    return META_TYPE;
  }
  else if ((c & SYSEX_STATUS_CODE_1 == SYSEX_STATUS_CODE_1) ||
	   (c & SYSEX_STATUS_CODE_2 == SYSEX_STATUS_CODE_2)) {
    return SYSEX;
  }
  else if (c & MIDI_VOICE_PITCH_BEND == MIDI_VOICE_PITCH_BEND) {
    *i_last_status = PITCH_BEND_STATUS;
    return MIDI_OTHER;
  }
  else if ((c & MIDI_VOICE_CHANNEL_KEY_PRESSURE == MIDI_VOICE_CHANNEL_KEY_PRESSURE) ||
	   (c & MIDI_VOICE_PROGRAM_CHANGE == MIDI_VOICE_PROGRAM_CHANGE)) {
    *i_last_status = CHANNEL_PROGRAM_STATUS;
    return MIDI_END;
  }
  else if (c & MIDI_CHANNEL_MODE == MIDI_CHANNEL_MODE) {
    *i_last_status = MODE_STATUS;
    return MIDI_OTHER;
  }
  else if ((c & MIDI_VOICE_POLY == MIDI_VOICE_POLY) ||
	   (c & MIDI_VOICE_NOTE_ON == MIDI_VOICE_NOTE_ON) ||
	   (c & MIDI_VOICE_NOTE_OFF == MIDI_VOICE_NOTE_OFF)) {
    *i_last_status = NOTE_STATUS;
    return MIDI_NOTE;
  }
  else {
    /* goes through each status and handles the state accordingily */
    if ((*i_last_status == PITCH_BEND_STATUS) ||
	(*i_last_status == MODE_STATUS)) {
      return MIDI_END;
    }
    else if (*i_last_status == CHANNEL_PROGRAM_STATUS) {
      return DELTA;
    }
    else if (*i_last_status == NOTE_STATUS) {
      transpose_note(b_byte, i_diff);
      return MIDI_END;
    }
  }
  return DELTA;
}


void transpose(char * b_bytes, uint32_t ui_length, int i_diff) {
  enum transposer_state_t i_state = DELTA;
  enum last_event_status_t i_last_status;

  uint32_t ui_skip_length;
  uint32_t ui_buffer_pointer = 0;
  char * b_current_byte;

  while (ui_buffer_pointer < ui_length) {
    b_current_byte = b_bytes + ui_buffer_pointer;
    
    switch(i_state) {
    case DELTA:
      i_state = dfa_delta(b_current_byte);
      break;
    case EVENT_STATUS:
      i_state = dfa_event_status(b_current_byte, &i_last_status, i_diff);
      break;
    case MIDI_END:
      i_state = dfa_midi_end();
      break;
    case MIDI_OTHER:
      i_state = dfa_midi_other();
      break;
    case SYSEX:
      i_state = dfa_sysex_event(b_current_byte);
      break;
    case MIDI_NOTE:
      i_state = dfa_midi_note(b_current_byte, i_diff);
      break;
    case META_TYPE:
      i_state = dfa_meta_type();
      break;
    case META_LENGTH:
      i_state = dfa_meta_length(b_current_byte, &ui_skip_length);
      break;
    case META_SKIP:
      i_state = dfa_meta_skip(&ui_buffer_pointer, ui_skip_length);
      break;
    }

    ++ui_buffer_pointer;
  }
}

