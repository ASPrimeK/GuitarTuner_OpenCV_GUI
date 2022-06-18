
#include <Windows.h>
#include <mmsystem.h>
#include <cmath>
#include <iostream>

#include "record.hpp"

int idx_buf;
short buf[LEN_BUF * NUM_BUF];
short temp_buf[LEN_BUF];

WAVEFORMATEX waveformat;
HWAVEIN handle_wave_in;
WAVEHDR header[NUM_BUF];

HANDLE event_buffer_ready;
HANDLE handle_thread;
DWORD id_thread;

extern void detectPitch(short*);

void CALLBACK myWaveInProc(HWAVEIN hwi,
                           UINT uMsg, DWORD dwInstance,
                           DWORD dwParam1, DWORD dwParam2) {
    if (uMsg != WIM_DATA) return;
    SetEvent(event_buffer_ready);
}

DWORD WINAPI recordingWaitingThread(LPVOID ivalue) {
    while (1) {
        WaitForSingleObject(event_buffer_ready, INFINITE);
        waveInUnprepareHeader(handle_wave_in, &header[idx_buf], sizeof(WAVEHDR));
        memcpy(temp_buf, header[idx_buf].lpData, LEN_BUF * sizeof(short));

        detectPitch(temp_buf);

        memcpy(header[idx_buf].lpData, temp_buf, LEN_BUF * sizeof(short));
        ++idx_buf;

        if (idx_buf == NUM_BUF) idx_buf = 0;
        waveInPrepareHeader(handle_wave_in, &header[idx_buf], sizeof(WAVEHDR));
        waveInAddBuffer(handle_wave_in, &header[idx_buf], sizeof(WAVEHDR));
    }
    return 0;
}

void initWaveformat() {
    waveformat.wFormatTag = WAVE_FORMAT_PCM;
    waveformat.nChannels = 1;  // 1=mono, 2=stereo
    waveformat.nSamplesPerSec = SAMPLE_RATE;
    waveformat.wBitsPerSample = 16;  // 16 for high quality, 8 for telephone-grade
    waveformat.nBlockAlign = waveformat.nChannels * waveformat.wBitsPerSample / 8;
    waveformat.nAvgBytesPerSec = waveformat.nSamplesPerSec *
                                 waveformat.nChannels * waveformat.wBitsPerSample / 8;
    waveformat.cbSize = 0;
}

void initBuffer() {
    for (int i = 0; i < NUM_BUF; i++) {
        header[i].lpData = (LPSTR)&buf[i * LEN_BUF];
        header[i].dwBufferLength = LEN_BUF * sizeof(short);
        header[i].dwFlags = 0L;
        header[i].dwLoops = 0L;
    }
}

void initEventThread() {
    event_buffer_ready = CreateEvent(NULL, FALSE, FALSE, NULL);
    handle_thread = CreateThread(NULL, 0, recordingWaitingThread,
                                 NULL, 0, &id_thread);
}

void initWaveDevice() {
    waveInOpen(&handle_wave_in, WAVE_MAPPER,
               &waveformat, (DWORD_PTR)myWaveInProc, 0L,
               CALLBACK_FUNCTION);
    waveInPrepareHeader(handle_wave_in, &header[idx_buf], sizeof(WAVEHDR));
    waveInAddBuffer(handle_wave_in, &header[idx_buf], sizeof(WAVEHDR));
}

void initAudio() {
    printf("init audio...\n");
    initWaveformat();
    initBuffer();
    initEventThread();
    initWaveDevice();

    printf("start recording...\n");
    waveInStart(handle_wave_in);
}