/*
defines the interface for reading and writing midi files
 */

#include <stdio.h>
#include <stdint.h>

/* data structure for storing the midi chunk */
typedef struct midi_chunk * midi_chunk_t;

/*
reads in a chunk from a file stream, assumes the file stream 
is already at the beginning of a chunk
*/
midi_chunk_t midi_read_chunk(FILE * o_stream);

/*
writes a chunk to a given file stream
*/
void midi_write_chunk(FILE * o_stream, midi_chunk_t o_chunk);

/*
frees the memory of a midi chunk
*/
void midi_free_chunk(midi_chunk_t o_chunk);

/*
returns the type string of chunk o_chunk
*/
char * midi_get_chunk_type(midi_chunk_t o_chunk);

/*
returns the size of the chunk
*/
uint32_t midi_get_chunk_length(midi_chunk_t o_chunk);

/*
returns the buffer with the chunk data
*/
char * midi_get_chunk_bytes(midi_chunk_t o_chunk);

/*
returns if the EOF flag on a chunk is set
*/
int midi_get_chunk_eof_flag(midi_chunk_t o_chunk);
