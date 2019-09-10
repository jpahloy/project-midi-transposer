#include <stdlib.h>
#include "midi.h"

struct midi_chunk {
  /* flag if chunk is the end of the file */
  int eof_flag;
  
  /* the type of chunck, usually Mthd or Mtrk */
  char s_type[5];
  /* the length of the chunk in bytes */
  uint32_t ui_length;
  /* data buffer that stores the chunk info */
  char * b_bytes;
};


/* reads in 4 byte 32 ints from o_stream */
static uint32_t midi_read_32int(FILE * o_stream) {
  const unsigned int ui_bitshift = 24;

  uint32_t ui_ret = 0;
  int c_byte = 0;
  
  for (int i = 0; i < 4; i++) {
    c_byte = fgetc(o_stream);

    if (c_byte == EOF) {
      return 0;
    }

    ui_ret += (uint32_t) (c_byte << (ui_bitshift - (8 * i)));
  }
  return ui_ret;
}

/* writes 4 bytes of a 32bit int for midi files */
static void midi_write_32int(FILE * o_stream, uint32_t ui_data) {
  fputc((ui_data & 0xFF000000) >> 24, o_stream);
  fputc((ui_data & 0xFF0000) >> 16, o_stream);
  fputc((ui_data & 0xFF00) >> 8, o_stream);
  fputc((ui_data & 0xFF), o_stream);  
}


/* reads in data a byte at a time */
static char * midi_read_chunk_data(FILE * o_stream, uint32_t ui_length) {
  char * b_buf = (char *) malloc(ui_length);

  for (int i = 0; i < ui_length; i++) {
    b_buf[i] = (char) fgetc(o_stream);
  }

  return b_buf;
}


midi_chunk_t midi_read_chunk(FILE * o_stream) {
  midi_chunk_t o_chunk = (midi_chunk_t) malloc(sizeof(struct midi_chunk));

  o_chunk->eof_flag = 0;

  /* read type*/
  for (int i = 0; i < 4; i++) {
    o_chunk->s_type[i] = fgetc(o_stream);
    if (feof(o_stream)) {
      o_chunk->eof_flag = 1;
      return o_chunk;
    }
  }
  o_chunk->s_type[4] = '\0';

  /* read length */
  o_chunk->ui_length = midi_read_32int(o_stream);
  if (o_chunk->ui_length == 0) {
    return NULL;
  }

  /* read body */
  o_chunk->b_bytes = midi_read_chunk_data(o_stream, o_chunk->ui_length);
    
  return o_chunk;
}


void midi_write_chunk(FILE * o_stream, midi_chunk_t o_chunk) {
  /* write head */
  for (int i = 0; i < 4; i++) {
    fputc(o_chunk->s_type[i], o_stream);
  }

  /* write length */
  midi_write_32int(o_stream, o_chunk->ui_length);

  /* write data */
  for (int i = 0; i < o_chunk->ui_length; i++) {
    fputc(o_chunk->b_bytes[i], o_stream);
  }
}

void midi_free_chunk(midi_chunk_t o_chunk) {
  free(o_chunk->b_bytes);
  free(o_chunk);
}

char * midi_get_chunk_type(midi_chunk_t o_chunk) {
  return o_chunk->s_type;
}


uint32_t midi_get_chunk_length(midi_chunk_t o_chunk) {
  return o_chunk->ui_length;
}

char * midi_get_chunk_bytes(midi_chunk_t o_chunk) {
  return o_chunk->b_bytes;
}

int midi_get_chunk_eof_flag(midi_chunk_t o_chunk) {
  return o_chunk->eof_flag;
}
