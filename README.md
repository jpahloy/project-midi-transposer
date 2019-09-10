# project-midi-transposer
Takes a midi file and transposes the notes to the specified tone

## Inspiration
This project was inspired from me trying to learn the Venezuelan Cuatro. Coming from a background of classical guitar, I thought
I would play simple classical guitar ppieces. Unfortunately the cuatro has a different note range and so I had to transpose
some pieces. That's where I got the idea of transposing midi files.

## Implementation
The program is implemented with a simple deterministic finite automaton which reads midi events until it finds the events that
handle notes. Then it simple changes the notes and writes the midi file out

## Usage

```
$ transpose-midi [option] [input file] [output file]
```

The options are -[count][unit]

count : is the number of units to tranpose the piece

unit  : either 'o' for octave or 's' for semi-tone
