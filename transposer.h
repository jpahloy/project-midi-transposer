/*
this module is for transposing the midi note events 
it acts as a DFA for reading the track chunk events
 */


#include <stdint.h>

/* performs the transposition on b_bytes with i_diff as the change in tone */
void transpose(char * b_bytes, uint32_t ui_length, int i_diff);
