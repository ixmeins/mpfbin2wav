/*
    mpfbin2wav [infile] generates output.wav

    Generates a Microprofessor 1 compatible tape file in WAV format from a binary input file.
    Input file size limited to 8k.

    Written by Ingmar Meins, May 28, 2022.
    All rights reserved blah blah.
    Free for non-commercial use.

    Feel free to submit comments on my rubbish programming style to:
    idontcare@localhost

    It's free, get over it.

*/
#include <stdio.h>
#include <ctype.h>
#include <stdint-gcc.h>


FILE *fpin, *fpout;
int audiobytes = 0;

// Standard audio WAV file format header for 8bit unsigned stereo at 8000Hz sample rate.
typedef struct {
    uint8_t RIFF[4];
    uint32_t length;
    uint8_t WAVE[4];
    uint8_t FMT[4];
    uint32_t FMTLEN;
    uint16_t typefmt; // type of format ie PCM = 1
    uint16_t numchan; // number of channels ie stereo = 2
    uint32_t samplerate; // sample rate ie 8000 for this purpose
    uint32_t bitrate; // (sr * bps * chan) /8
    uint16_t strmtype; // 1 - 8 bit mono, 2 - 8 bit stereo/16 bit mono, 4 - 16 bit stereo
    uint16_t bps; // bits per sample
    uint8_t DATA[4];
    uint32_t datasize; // size of the following data
} WaveHeaderFmt;

// Generate cycles of 1kHz
void Cyc1khz(int cycles) {
    audiobytes += cycles * 16; // this is used to determine the amount of data generated.
    
    while (cycles--) {
        fputc(64, fpout); // left
        fputc(64, fpout); // right
        fputc(64, fpout); // left
        fputc(64, fpout); // right
        fputc(64, fpout); // left
        fputc(64, fpout); // right
        fputc(64, fpout); // left
        fputc(64, fpout); // right

        fputc(192, fpout); // left
        fputc(192, fpout); // right
        fputc(192, fpout); // left
        fputc(192, fpout); // right
        fputc(192, fpout); // left
        fputc(192, fpout); // right
        fputc(192, fpout); // left
        fputc(192, fpout); // right
    }
}

// Generate cycles of 2kHz
void Cyc2khz(int cycles) {
    audiobytes += cycles * 8;
    
    while (cycles--) {
        fputc(64, fpout); // left
        fputc(64, fpout); // right
        fputc(64, fpout); // left
        fputc(64, fpout); // right
        fputc(192, fpout); // left
        fputc(192, fpout); // right
        fputc(192, fpout); // left
        fputc(192, fpout); // right
    }
}

// Generate a zero bit in MPF1 format
void zeroBit() {
    Cyc2khz(8);
    Cyc1khz(2);
}

// Generate a one bit in MPF1 format
void oneBit() {
    Cyc2khz(4);
    Cyc1khz(4);
}

// generate a full byte with start and stop bits LSB first
void tapeByte(uint8_t b) {
    uint8_t mask = 0x01;
    uint8_t bits = 8;

    zeroBit(); // start bit
    while (bits--) {
        //printf("bit:%i ",bits);
        if (b & mask) {
            //printf("1");
            oneBit();
        } else {
            //printf("0");
            zeroBit();
        }
        mask = mask << 1;
    }
    //printf(":");
    oneBit(); // stop bit
}

int main(int argc, char* argv[]) {
    uint8_t inbufr[8192]; // holds input file
    uint8_t checksum = 0; // checksum is bytes values added in 8 bits
    uint32_t temp = 0;

    WaveHeaderFmt wavehdr = {
        .RIFF = "RIFF",
        .length = 44, // header is 44 bytes plus length of data section below 
        .WAVE = "WAVE",
        .FMT = "fmt ", // note trailing space not null as per wrong wikipedia entry
        .FMTLEN = 16,
        .typefmt = 1, // PCM
        .numchan = 2, // stereo
        .samplerate = 8000, // 8kHz
        .strmtype = 2, // 8 bit stereo
        .bps = 8, // 8 bits per sample
        .bitrate = (8000 * 8 * 2) / 8,
        .DATA = "data",
        .datasize = 0 // size of the data section (fill in later)
    };

    if (argc < 2) {
        printf("Usage: mpfbin2wav [infile.bin] generates output.wav\n\r");
        return 1;
    }

    // argv[1] is the input filename for the binary source with default start of 1800h

    printf("Processing %s with default load address of 1800h\n\r",argv[1]);

    fpin = fopen(argv[1],"rb");

    if (! fpin) {
        printf("Could not open %s as input file\n\r",argv[1]);
        return 1;
    }

    // Check length of the input file, if it exceeds 8kb (crazy big) then abort.

    fseek(fpin, 0L, SEEK_END);
    int filesize = ftell(fpin); // get file size
    rewind(fpin); // back to the beginning

    if (filesize > 8192 || filesize == 0) {
        printf("Input file is too small or too large: %i bytes\n\r",filesize);
        return 1;
    } 

    printf("Input file size is %i bytes\n\r", filesize);

    // Read the input file and calculate the checksum.
    fread(inbufr, filesize, 1, fpin);

    for (int p=0; p<filesize; p++) {
        checksum += inbufr[p];
    }

    fpout = fopen("output.wav","wb");

    if (! fpout) {
        printf("Could not open %s as output file\n\r","output.wav");
        return 1;
    }

    // Write the WAV header
    wavehdr.datasize = 44; // this is updated later.
    wavehdr.length = wavehdr.datasize + 44; // updated later.

    fwrite(&wavehdr, 44, 1, fpout);

    // write audio data. 8kHz sample rate 8 bits per channel.
    audiobytes = 0; // this counts how many bytes are output in the data part of the wave.

    Cyc1khz(4000); // Lead sync

    // Filename 0x00, fnl, fnh
    tapeByte(0x23);
    tapeByte(0x12); // filename 1223h - this could be passed on the command line.

    // Start address
    tapeByte(0x00);
    tapeByte(0x18); // start address 1800h

    // End address is 0x1800 + filesize
    uint16_t endaddr = 0x1800 + filesize - 1;

    tapeByte(endaddr & 0xff); // low byte
    tapeByte(endaddr >> 8); // high byte

    // Checksum
    tapeByte(checksum);

    Cyc2khz(4000); // Mid Sync

    // Tape file data

    for (int p=0; p<filesize; p++) {
        tapeByte(inbufr[p]);
    }
    
    Cyc2khz(4000); // Tail sync

    printf("Wrote %u bytes of audio data\n\r", audiobytes);

    // Write out total file size.
    temp = 44+audiobytes;
    fseek(fpout, 4, SEEK_SET);
    fwrite(&temp, 4, 1, fpout);

    // Write out data block size.
    temp = audiobytes;
    fseek(fpout, 40, SEEK_SET);
    fwrite(&temp, 4, 1, fpout);


    fclose(fpout);
    fclose(fpin);

    return 0;
}