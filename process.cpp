
#include <Windows.h>
#include <mmsystem.h>
#include <cmath>
#include "fftw3.h"

#include "process.hpp"
#define PI 3.141592653589793

#define CUT_OFF 2400
#define FIR_TAPS 25

fftw_complex *fft_input, *fft_result, *log_pow_fft_result, *cepstrum_result;
fftw_plan plan_fft, plan_cepstrum;
double *fft_magnitude;  // for drawing fft graph

int cepstrum_min_idx, cepstrum_max_idx;

double energy;

double hanning_coef[FIR_TAPS];
short LPF_output[LEN_BUF];

short windowed[LEN_BUF];

// Extern Variables
double current_pitch;

int toFrequency(int n) {
    return SAMPLE_RATE * n / FFT_SIZE;
}

int toTab(int freq) {
    return freq * FFT_SIZE / SAMPLE_RATE;
}

void initHanningWindow() {
    double window[FIR_TAPS] = {};

    double f_norm = (double)CUT_OFF / SAMPLE_RATE;
    for (int i = 0; i < FIR_TAPS; i++) {
        window[i] = (double)(0.5 * (1 - cos(2 * PI * i / (FIR_TAPS - 1.))));
    }

    for (int n = 0; n < FIR_TAPS; n++) {
        int t = n - FIR_TAPS / 2;
        if (t == 0)
            hanning_coef[n] = window[n] * 2 * f_norm;
        else {
            hanning_coef[n] = sin(2 * PI * f_norm * t) / (PI * t);
            hanning_coef[n] *= window[n];
        }
    }
}

void initProcess(int min_freq, int max_freq) {
    fft_input = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);
    fft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE);

    cepstrum_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE / 2);
    log_pow_fft_result = (fftw_complex *)fftw_malloc(sizeof(fftw_complex) * FFT_SIZE / 2);

    plan_fft = fftw_plan_dft_1d(FFT_SIZE, fft_input, fft_result,
                                FFTW_FORWARD, FFTW_ESTIMATE);
    plan_cepstrum = fftw_plan_dft_1d(FFT_SIZE / 2, log_pow_fft_result, cepstrum_result,
                                     FFTW_FORWARD, FFTW_ESTIMATE);

    cepstrum_min_idx = SAMPLE_RATE / (2 * (max_freq));
    cepstrum_max_idx = SAMPLE_RATE / (2 * (min_freq));

    initHanningWindow();
}

// void windowing(short *input, short *output) {
//     int size = LEN_BUF / 2;
//     for (int i = 0; i < size; i++) {
//         // Hanning Window
//         double window_value = (double)(0.5 * (1 - cos(2 * PI * i / (size - 1.))));
//         output[size + i] = input[size + i] * window_value;
//         output[size - i] = input[size - i] * window_value;
//     }
// }

void LPF(short *data) {
    for (int n = 0; n < LEN_BUF; n++) {
        double cur = 0;
        for (int i = 0; i < FIR_TAPS; i++) {
            int k = n - FIR_TAPS / 2 + i;
            if (0 <= k && k < LEN_BUF)
                cur += data[k] * hanning_coef[i];
        }
        LPF_output[n] = (short)cur;
    }
}

// Extern function
void detectPitch(short *data) {
    // Volume Check
    energy = 0;
    for (int i = 0; i < LEN_BUF; i++) {
        // energy += pow((double)data[i], 2);
        energy += abs((double)data[i]);
    }
    energy /= LEN_BUF;

    // printf("calculated energy = %lf\n", energy);
    if (energy < VOLUME_THRESHOLD) {
        return;
    }
    // LPF (Optional)
    LPF(data);

    // Windowing
    int size = LEN_BUF / 2;
    for (int i = 0; i <= size; i++) {
        double window_value = (double)(0.5 * (1 - cos(2 * PI * i / (size - 1.))));
        fft_input[size + i][0] = LPF_output[size + i] * window_value;
        fft_input[size - i][0] = LPF_output[size - i] * window_value;
        // fft_input[size + i][0] = data[size + i] * window_value;
        // fft_input[size - i][0] = data[size - i] * window_value;
    }

    // Zero Padding
    // >>  zero valeus were already allocated in fftw_malloc function
    // for (int i = 0; i < LEN_BUF; i++) {
    //     fft_input[i][0] = windowed[i];
    // }

    // FFT
    fftw_execute(plan_fft);

    // Power Cepstrum
    for (int i = 0; i < FFT_SIZE / 2; i++) {
        log_pow_fft_result[i][0] = log10(pow(fft_result[i][0] / FFT_SIZE, 2));
        log_pow_fft_result[i][1] = 0;
    }
    fftw_execute(plan_cepstrum);

    // detect pitch
    double max_mag = -1;
    int max_idx = 0;
    for (int i = cepstrum_min_idx; i < cepstrum_max_idx; i++) {
        // double cur_mag = pow(cepstrum_result[i][0], 2);
        double cur_mag = abs(cepstrum_result[i][0]);
        if (max_mag < cur_mag) {
            max_mag = cur_mag;
            max_idx = i;
        }
    }
    current_pitch = SAMPLE_RATE / double(2 * max_idx);
}
