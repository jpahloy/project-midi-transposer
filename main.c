#include <stdio.h>
#include <string.h>

#include "midi.h"
#include "transposer.h"

/*
main entry to the program, 
 */
int main(int argc, char * argv[]){

  FILE * o_in = fopen("test.mid", "rb");
  FILE * o_out = fopen("out.mid", "wb");

  midi_chunk_t o_chunk;

  while (!midi_get_chunk_eof_flag((o_chunk = midi_read_chunk(o_in)))) {
    if (strcmp(midi_get_chunk_type(o_chunk), "MTrk") == 0) {
      
      transpose(midi_get_chunk_bytes(o_chunk),
      		midi_get_chunk_length(o_chunk),
      		+7);
    }
    
    midi_write_chunk(o_out, o_chunk);
    midi_free_chunk(o_chunk);
  }
  
  fclose(o_in);
  fclose(o_out);
  return 0;
}
