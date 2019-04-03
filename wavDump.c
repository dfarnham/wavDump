#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

/*
 * A simple WAVE data extractor to test my understanding of the format
 */


/*****************************************
 * panic:
 *   Write a message to STDERR and exit(1)
 *****************************************/
void
panic(const char *fmt, ...) {
    va_list vptr;
    va_start(vptr, fmt);
    vfprintf(stderr, fmt, vptr);
    va_end(vptr);
    exit(1);
}

int
main(int argc, char *argv[]) {
    FILE *fp;
    char riffId[4], waveId[4], fmtId[4], dataId[4];
    int fileSize, dataSize, formatLength, sampleRate, avgBytesPerSec, frame;
    short formatTag, channels, blockAlignment, bitsPerSample;
    int seconds = -1;

    // Argument validation
    if (argc < 2 || argc > 3)        panic("Usage: %s file.wav [seconds]  # 0 seconds outputs header info only\n", argv[0]);
    if (!(fp = fopen(argv[1], "r"))) panic("Error: can't read file \"%s\"\n", argv[1]);

    // Number of seconds of frame data to output [0 seconds will output basic header info and exit(0)]
    if (argc == 3) {
        seconds = (int)strtol(argv[2], (char **)NULL, 10); // seconds = atoi(argv[2]);
    }

    // Validate RIFF header
    if (fread((void *)riffId, sizeof(char), 4, fp) != 4) panic("Error: can't read RIFF header\n");
    if (strncmp(riffId, "RIFF", 4))                      panic("Error: invalid RIFF id\n");

    // Read file size
    if (fread((void *)&fileSize, sizeof(int), 1, fp) != 1) panic("Error: can't read file size\n");

    // Validate WAVE header
    if (fread((void *)waveId, sizeof(char), 4, fp) != 4) panic("Error: can't read WAVE header\n");
    if (strncmp(waveId, "WAVE", 4))                      panic("Error: invalid WAVE id\n");

    // Validate FMT header
    if (fread((void *)fmtId, sizeof(char), 4, fp) != 4)  panic("Error: can't read FMT header\n");
    if (strncmp(fmtId, "fmt ", 4))                       panic("Error: invalid FMT id\n");

    // Read format length
    if (fread((void *)&formatLength, sizeof(int), 1, fp) != 1) panic("Error: can't read format length\n");
    if (formatLength != 16) panic("Error: unable to process format length WAVE of %d\n", formatLength);

    // Read format tag
    if (fread((void *)&formatTag, sizeof(short), 1, fp) != 1) panic("Error: can't read format tag\n");

    // format tag of 1 == PCM
    if (formatTag != 1) panic("Error: unable to process format tag of %d\n", formatTag);

    // Read channel info
    if (fread((void *)&channels, sizeof(short), 1, fp) != 1) panic("Error: can't read channel info\n");

    // Read sample rate
    if (fread((void *)&sampleRate, sizeof(int), 1, fp) != 1) panic("Error: can't read sample rate\n");

    // Read avg bytes per sec
    if (fread((void *)&avgBytesPerSec, sizeof(int), 1, fp) != 1) panic("Error: can't read average bytes per second\n");

    // Read block alignment
    if (fread((void *)&blockAlignment, sizeof(short), 1, fp) != 1) panic("Error: can't read block alignment\n");

    // Read bits per sample
    if (fread((void *)&bitsPerSample, sizeof(short), 1, fp) != 1) panic("Error: can't read bits per sample\n");

    // Find DATA header
    while (1) {
        if (fread((void *)dataId, sizeof(char), 4, fp) != 4) panic("Error: can't read DATA header\n");

        // Read data size
        if (fread((void *)&dataSize, sizeof(int), 1, fp) != 1) panic("Error: can't read data size\n");

        // Found it?
        if (!strncmp(dataId, "data", 4)) break;

        fseek(fp, dataSize, SEEK_CUR);
    }

    if (seconds == 0) {
        fprintf(stderr, "format tag %d\n", formatTag);
        fprintf(stderr, "channels %d\n", channels);
        fprintf(stderr, "sample rate %d Hz\n", sampleRate);
        fprintf(stderr, "sample rate %d bits per second\n", avgBytesPerSec*8);
        fprintf(stderr, "block alignment %d\n", blockAlignment);
        fprintf(stderr, "bits per sample %d\n", bitsPerSample);
        fprintf(stderr, "audio data bytes %d\n", dataSize);
        exit(0);
    }

    // 2 bytes per channel (a short)
    // 2 channels (typically)
    frame = 0;
    printf("Sample\tL\tR\n");
    for (int i = 0; i < dataSize/channels/2; i++) {
        short leftChannel, rightChannel;

        // Read left channel
        if (fread((void *)&leftChannel, sizeof(short), 1, fp) != 1) panic("Error: can't read left channel\n");

        // Read right channel
        if (fread((void *)&rightChannel, sizeof(short), 1, fp) != 1) panic("Error: can't read right channel\n");

        frame++;

        // Output raw frame data
        printf("%d\t%d\t%d\n", frame, leftChannel, rightChannel);

        if (seconds > 0 && frame == seconds*sampleRate) break;
    }

    fclose(fp);
    exit(0);
}
