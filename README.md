# mpfbin2wav
Generate MPF1 (z80 dev system) tape WAV files from a binary ready to "play" into the MPF1.

I recently aquired an original Multitech Micro-Professor 1B (MPF1B) development system. The MPF1 has traditional tape recorder audio in and out to save and load programs but no serial port. So today I whipped up this utility to generate 8 bit unsigned stereo WAV files at 8k sampling rate from an input Z80 binary on your host machine. The resulting output WAV can be "played" into the MPF1.

Full of lazy coding, and a total lack of optimisation. If you don't like it, don't use it lol. All whinging will be piped directly to /dev/null!

Cheers.
Ingmar
28 May 2022
